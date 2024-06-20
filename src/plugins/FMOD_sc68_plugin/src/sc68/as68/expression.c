/**
 * @ingroup   as68_devel
 * @file      expression.c
 * @author    Penne Vincent
 * @date      1993
 * @brief     68000 macro assembler - expression evaluation
 */

/*
 *                      as68 - 68000 macro assembler
 *                    Copyright (C) 1993 Vincent Penne
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "as68.h"
#include "error.h"
char notdef, last_pass;

/* Evaluate primary integer expression
 */
int primaire()
{
  char *s;
  word *w;
  int m;
  char ond;

  switch (symbol_type)
    {
    case STRING_TYPE:
      m = 0;
      s = cur_string;
      while(*s)
	m = (m * 256) + *s++;
      get_symbol();
      return m;

    case NUM_TYPE:
      m = cur_num;
      get_symbol();
      return m;

    case '.':
      if(get_symbol() != WORD_TYPE)
	{
	  syntax_error();
	  return 1;
	}
      else
	{
	  char a[128];
	  strcpy(a, cur_string);
	  sprintf(cur_string, "_%xL%s", n_local, a);
	}

    case WORD_TYPE:
      w = search(cur_string);
      if(!w)
	{
	  if(cur_pass == 1)
	    {
	      get_symbol();
	      notdef = TRUE;
	      last_pass = FALSE;
	      return 0;
	    }
	  else
	    {
	      error_undefined(cur_string);
	      get_symbol();
	      return 1;
	    }
	}
      get_symbol();
      if(w->type != LABEL && w->type != VARIABLE)
	{
	  error(error_list[33]);
	  return(1);
	}
      if(opt_relocatable && cur_pass == 3)
	return w->dd.value2;
      else
	return w->pd.value;

    case '-':
      get_symbol();
      return -primaire();

    case '+':
      get_symbol();
      return primaire();

    case '(':
      get_symbol();
      ond = notdef;
      m = expression();
      notdef = (notdef || ond);
      if(symbol_type != ')')
	error_expected(")");
      else
	get_symbol();
      return m;

    case '~':
      get_symbol();
      return ~primaire();

    case '!':
      get_symbol();
      return !primaire();

    case '*':
      get_symbol();
      return CurPC+org;

    default:
      syntax_error();
      return 1;
    }
  return 1;
}

/* Evaluate integer termes, which priority is lower than primaries ('*' and '/')
 */
int terme()
{
  int m = primaire(), n;

  for(;;)
    switch (symbol_type)
      {
      case '*':
	get_symbol();
	m *= primaire();
	break;

      case '/':
	get_symbol();
	n = primaire();
	if(!notdef)
	  {
	    if (n == 0)
	      {
		error(error_list[4]);
		m = 1;
	      }
	    else
	      m /= n;
	  }
	break;

      default:

	return m;
      }
}

/* Evaluate integer expression which priority is lower than termes ( '+' '-' ... )
 */
int sous_expression()
{
  int m = terme();

  for(;;)
    switch (symbol_type)
      {
      case '+':
	get_symbol();
	m += terme();
	break;

      case '-':
	get_symbol();
	m -= terme();
	break;

      case '=':
	get_symbol();
	m = (m == sous_expression());
	break;

      case '<':
	if(get_symbol() == '<')
	  {
	    get_symbol();
	    m = m << sous_expression();
	  }
	else
	  if(symbol_type == '=')
	    {
	      get_symbol();
	      m = (m <= sous_expression());
	    }
	  else
	    m = (m < sous_expression());
	break;

      case '>':
	if(get_symbol() == '>')
	  {
	    get_symbol();
	    m = m >> sous_expression();
	  }
	else
	  if(symbol_type == '=')
	    {
	      get_symbol();
	      m = m >= sous_expression();
	    }
	  else
	    m = m > sous_expression();
	break;

      case '^':
	get_symbol();
	m = m ^ sous_expression();
	break;

      case '&':
	get_symbol();
	m = m & sous_expression();
	break;

      case '|':
	get_symbol();
	m = m | sous_expression();
	break;

      case '!':
	if(get_symbol() != '=')
	  error_expected("=");
	else
	  get_symbol();
	m = m != sous_expression();
	break;

      default:
	return m;
      }
}

/* Evaluate an integer expression
 */
int expression()
{
  int m;
  notdef = FALSE;

  m = sous_expression();
  while(symbol_type == WORD_TYPE)
    {
      if(!strcmp(cur_string, "and"))
	{
	  get_symbol();
	  m = m && sous_expression();
	}
      else if(!strcmp(cur_string, "or"))
	{
	  get_symbol();
	  m = m || sous_expression();
	}
      else break;
    }
  return m;
}
