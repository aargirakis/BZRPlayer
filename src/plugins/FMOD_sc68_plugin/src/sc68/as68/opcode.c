/**
 * @ingroup   as68_devel
 * @file      opcode.c
 * @author    Penne Vincent
 * @date      1993
 * @brief     68000 macro assembler - opcode generator
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

#include <config68.h>
//#define DEBUG _DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "as68.h"
#include "error.h"

#define opt(a) warning(a)

int	default_size;
int	cur_reg;
int	posopt, opt, optt;

extern int bss;

static int convchar(char a)
{
  a = toupper(a);
  if(!isgraph(a)) a=0;
  return a & 127;
}

static int ischareql(char a, char b)
{
  return convchar(a)==convchar(b);
}

int streql(char *a, char *b)
{
  char ac,bc;
  do
    {
      ac = convchar(*a++);
      bc = convchar(*b++);
    } while(ac==bc && ac);
  return ac==bc;

  /*	This is Ziggy original version ...
  //	while((c=*a++)!=0 || *b!=0)
  //	{
  //		if(c < 'a')
  //			c = c + 'a' - 'A';
  //		if(c != *b++)
  //			return FALSE;
  //	}
  //	return TRUE;
  */
}

void tstb(int a)
{
  if(opt_relocatable && cur_pass == 3)
    return;
  if(a < -128 || a > 255)
    {
      /*		fprintf(ferr,"\ntstb-->%d\n", a);*/
      bit_lost_error();
    }
}

void tstw(int a)
{
  if(opt_relocatable && cur_pass == 3)
    return;
  if(a < -32768 || a > 65535)
    {
      /*		fprintf(ferr,"\ntstw-->%d\n", a);*/
      bit_lost_error();
    }
}

void genB(int a)
{
  if(!opt_relocatable || cur_pass == 3)
    {
      tstb(a);
      code[pc] = a;
    }
  pc++;
  if(pc > max_pc)
    max_pc = pc;
}

void genW(int a)
{
  if(!opt_relocatable || cur_pass == 3)
    {
      tstw(a);
      code[pc+0] = a>>8;
      code[pc+1] = a;
    }
  pc += 2;
  if(pc > max_pc)
    max_pc = pc;
}

void putW(int a, int pc)
{
  if(!opt_relocatable || cur_pass == 3)
    {
      tstw(a);
      code[pc+0] = a>>8;
      code[pc+1] = a;
    }
  if(pc+2 > max_pc)
    max_pc = pc+2;
}

void gen24(int a)
{
#if DEBUG
  printf("gen24: pass=%d reloc=%d a=%x pc=%08x *pc=%08x a-*pc=%08x\n",
	 cur_pass, opt_relocatable, a, pc , getL(pc), a-getL(pc));
#endif


  if(!opt_relocatable || cur_pass == 3)
    {
      if(a > 0xFFFFFF || a < -0x800000)
	{
	  /*			fprintf(ferr,"\ngen24-->%d\n", a);*/
	  bit_lost_error();
	}
      code[pc+0] = a>>16;
      code[pc+1] = a>>8;
      code[pc+2] = a;
    }
  pc += 3;
  if(pc > max_pc)
    max_pc = pc;
}

extern int opt_relwarn;
void genL(int a)
{
#if DEBUG
  printf("genL: pass=%d reloc=%d a=%08x pc=%08x *pc=%08x a-*pc=%08x\n",
	 cur_pass, opt_relocatable, a, pc , getL(pc), a-getL(pc));
#endif

  if(opt_relocatable && cur_pass == 3 && (a-getL(pc)) == 0x12345678)
    {
      if( opt_relwarn ) relatif_PC_warning();
      if(nb_rel >= MAX_REL)
	fatal_error("relocation table overflow");
      relocation_table[nb_rel++] = pc;
    }
  else
    {
      code[pc+0] = a>>24;
      code[pc+1] = a>>16;
      code[pc+2] = a>>8;
      code[pc+3] = a;
    }

#if DEBUG
  printf("    : pass=%d reloc=%d a=%08x pc=%08x *pc=%08x a-*pc=%08x\n",
	 cur_pass, opt_relocatable, a, pc , getL(pc), a-getL(pc));
#endif


  pc += 4;
  if(pc > max_pc)
    max_pc = pc;
}

int getC(int a)
{
  return code[a];
}

int getN(int a)
{
  return	(code[a  ]<<8) +
    code[a+1];
}

int getL(int a)
{
  return
    (code[a]   << 24) +
    (code[a+1] << 16) +
    (code[a+2] <<  8) +
     code[a+3];
}

void adressing_mode_error()
{
  error(error_list[27]);
}

void size_error()
{
  error(error_list[26]);
}

int tt(int t, int b, int w, int l)
{
  switch(t)
    {
    case	Byte:
      return b;
    case	Word:
      return w;
    case	Long:
      return l;
    }
  return 0;
}

void erreur(int n)
{
  error(error_list[n]);
}

void tstvirg()
{
  if(symbol_type != ',')
    erreur(6);
  else
    get_symbol();
}

void tst(char v)
{
  if(symbol_type != v)
    erreur(1);
  get_symbol();
}

int get_size(int zp)
{
  char *s = plb;

  if(symbol_type == '.')
    {
      get_symbol();
      if(symbol_type == WORD_TYPE)
	{
	  if(streql(cur_string, "b") || streql(cur_string, "s"))
	    zp = BS;
	  else if(streql(cur_string, "l"))
	    zp = LS;
	  else if(streql(cur_string, "v"))
	    zp = VS;
	  else if(streql(cur_string, "w"))
	    zp = WS;
	  else
	    {
	      plb = s;
	      symbol_type = '.';
	      return zp;
	    }
	  get_symbol();
	}
    }
  return zp;
}

/* Return FALSE if string is not a register;
 * else cur_reg get register id :
 * d0-d7 : 0-7
 * a0-a7 : 8-15
 * sp    : 15 (defined in register_table)
 */
int isreg(char *c)
{
  struct fast_table *r;

  if(!c[2])
    {
      if((*c == 'd' || *c == 'D') && c[1] >= '0' && c[1] <= '7')
	{
	  cur_reg = c[1] - '0';
	  return TRUE;
	}
      if((*c == 'a' || *c == 'A') && c[1] >= '0' && c[1] <= '7')
	{
	  cur_reg = c[1] - '0' + 8;
	  return TRUE;
	}
    }
  r = look_in_fast_table(c, register_table);
  if(r)
    {
      cur_reg = r->value;
      return TRUE;
    }
  return FALSE;
}

int tstopt()
{
  return FALSE;
}

void addopt(char *c)
{
  opt(c);
}

int reglist()
{
  int a, b, c, swap;

  c = 0;
  for(;;)
    {
      if(symbol_type != WORD_TYPE || !isreg(cur_string))
	erreur(17);
      a = cur_reg;
      if(get_symbol() == '-')
	{
	  if(get_symbol() != WORD_TYPE || !isreg(cur_string))
	    erreur(17);
	  b = cur_reg;
	  if(b > a)
	    {
	      swap = b;
	      b = a;
	      a = swap;
	    }
	  get_symbol();
	}
      else
	b = a;
      c |= ( (1<<(a+1))-1 ) & ~( (1<<b)-1 );
      if(symbol_type != '/')
	break;
      get_symbol();
    }
  return c;
}

/* Generate opcode
 * t is instruction size : Byte, Word, Long
 * o is boolean (optimize)
 */
int op(int t, int o)
{
  int a;
  int b;
  int	neg = FALSE, rel = FALSE;
  char	*oplb=NULL;

  posopt++;
  o = o && opt;

  /* Immediat value */
  if(symbol_type == '#')
    {
      get_symbol();
      if(t == Long)
	genL(expression());
      else
	{
	  int a=expression();
	  genW(a);
	  if(t == Byte)
	    tstb(a);
	}
      return 074 + Data + Memory;
    }

  /* Source register */
  if(symbol_type == WORD_TYPE && isreg(cur_string))
    {
      get_symbol();
      a = cur_reg + Alterable;
      if(!(a & 8))
	a += Data;
      return a;
    }

  /* Predec */
  if(symbol_type == '-')
    {
      oplb = plb;
      get_symbol();
      neg = TRUE;
    }


  if(symbol_type == '(')
    {
      char *s = plb;

      get_symbol();
      if(!isreg(cur_string))
	{
	  plb = s;
	  symbol_type = '(';
	}
      else if(get_symbol() != ')')
	{
	  plb = s;
	  symbol_type = '(';
	  rel = TRUE;
	}
      else
	{
	  a = cur_reg;
	  if (!(a & 8))
	    erreur(10);
	  a += 020 - 010;
	  if(get_symbol() == '+')
	    {
	      get_symbol();
	      a += 010;
	      if(neg)
		erreur(12);
	    }
	  else if(neg)
	    a += 040 - 020;
	  else
	    a += Control;
	  return a + Data + Alterable + Memory;
	}
    }

  if(rel)
    b = 0;
  else
    {
      if(neg)
	{
	  plb = oplb;
	  symbol_type = '-';
	}
      b = expression();
    }
  if(symbol_type == '(')
    {
      if(get_symbol() != WORD_TYPE)
	erreur(10);
      if(!isreg(cur_string))
	{
	  if(!streql(cur_string, "pc"))
	    erreur(10);
	  a = 072;
	  b -= pc + org;
	}
      else
	a = cur_reg;
      if(!(a & 8))
	erreur(10);
      if(get_symbol() == ',')
	{
	  int c;

	  if(get_symbol() != WORD_TYPE || !isreg(cur_string))
	    erreur(17);
	  c = cur_reg;
	  if(!notdef)
	    tstb(b);
	  if(get_symbol() != '.')
	    genW((b & 255) + ((c & 15) << 12));
	  else
	    {
	      if(get_symbol() != WORD_TYPE)
		erreur(8);
	      if(streql(cur_string, "w"))
		genW((b & 255) + ((c & 15) << 12));
	      else if(streql(cur_string, "l"))
		genW((b & 255) + ((c & 15) << 12) + 0x0800);
	      else
		erreur(8);
	      get_symbol();
	    }
	  tst(')');
	  if(a == 072)
	    return 073 + Control + Data + Memory;
	  else
	    return 060 + (a & 7) + Control + Data + Memory + Alterable;
	}
      else
	{
	  if(opt && a != 072 && (cur_pass >= 2 || !notdef) && b == 0)
	    {
	      if(!notdef && (cur_pass == 1 || tstopt()))
		{
		  addopt("indexation removed");
		  tst(')');
		  return a - 010 + 020 + Data + Alterable + Memory + Control;
		}
	      opt("zero equal indexation");
	    }
	  if(cur_pass > 1 && !notdef && b >= 32768)
	    erreur(9);
	  if(notdef)
	    pc += 2;
	  else
	    genW(b);
	  tst(')');
	  if(a == 072)
	    return 072 + Control + Data + Memory;
	  else
	    return 040 + a + Control + Data + Memory + Alterable;
	}
    }

  if(symbol_type != '.' && o && (cur_pass >= 2 || !notdef) && b - pc - org >= -32768 && b - pc - org < 32768)
    {
      if(cur_pass == 1 || tstopt())
	{
	  addopt("assuming PC relatif");
	  genW(b - pc - org);
	  return 072 + Control + Data + Memory;
	}
      opt("Possible relatif PC");
    }
  if(symbol_type != '.')
    {
      if(opt && (cur_pass >= 2 || !notdef) && b >= -32768 && b < 32768)
	{
	  if(cur_pass == 1 || tstopt())
	    {
	      addopt("using word addressing mode");
	      genW(b);
	      return 070 + Control + Data + Memory + Alterable;
	    }
	  opt("word addressing mode possible");
	}
      genL(b);
      return 071 + Control + Data + Memory + Alterable;;
    }
  if(get_symbol() != WORD_TYPE)
    erreur(8);
  if(streql(cur_string, "w"))
    {
      get_symbol();
      if(b >= 32768)
	erreur(9);
      tstw(b);
      genW(b);
      return 070 + Control + Data + Memory + Alterable;
    }
  else if(streql(cur_string, "b") || streql(cur_string, "s"))
    erreur(11);
  else if(streql(cur_string, "l"))
    {
      get_symbol();
      genL(b);
      return 071 + Control + Data + Memory + Alterable;;
    }
  return 0;
}


void Cabcd_sbcd(int value)
{
  int a, b;
  if(get_size(Byte) != Byte)
    erreur(13);
  a = op(Byte, TRUE);
  if(!(a & 070))
    {
      tstvirg();
      b = op(Byte, FALSE);
      if((b & 070))
	erreur(12);
      if(value == 89)
	genW(0100400 + ((b & 7) << 9) + (a & 7));
      else
	genW(0140400 + ((b & 7) << 9) + (a & 7));
    }
  else if((a & 040) && !(a & 030))
    {
      tstvirg();
      b = op(Byte, FALSE);
      if(!(b & 040) || (b & 030))
	erreur(12);
      if(value == 89)
	genW(0100410 + ((b & 7) << 9) + (a & 7));
      else
	genW(0140410 + ((b & 7) << 9) + (a & 7));
    }
  else
    erreur(12);
}

void Cadd_cmp_sub(int value)
{
  int t, a, b;
  int which;
  int n;

  pc += 2;
  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  b = op(t, FALSE);
  if(value >= 106)
    which = 0;
  else if(value >= 32)
    which = 1;
  else
    which = 2;
  if(opt && which != 1 && cur_pass >= 2 && ((t == Long && (a & 077) == 074 && (n = getL(CurPC + 2)) <= 8 && n >= 0) ||
					    (t == Word && (a & 077) == 074 && (n = getN(CurPC + 2)) <= 8 && n >= 0)))
    opt("ADDQ ou SUBQ possible");
  if((b & 070) == 010)
    {
      if(t == Byte)
	erreur(11);
      if(t == Word)
	putW(0110300 + ((b & 7) << 9) + (a & 077) + (which << 13), CurPC);
      else
	putW(0110700 + ((b & 7) << 9) + (a & 077) + (which << 13), CurPC);
    }
  else if((a & 077) == 074 && ((b & 070) || value != 32))
    {
      if(!(b & Alterable))
	erreur(12);
      if(value < 32)
	putW(0003000 + (t << 6) + (b & 077), CurPC);
      else if(value < 106)
	putW(0006000 + (t << 6) + (b & 077), CurPC);
      else
	putW(0002000 + (t << 6) + (b & 077), CurPC);
    }
  else if(!(b & 070))
    putW(0110000 + (t << 6) + ((b & 7) << 9) + (a & 077) + (which << 13), CurPC);
  else if(!(a & 070))
    {
      if(!(b & Alterable) || which == 1)
	erreur(12);
      putW(0110400 + (t << 6) + ((a & 7) << 9) + (b & 077) + (which << 13), CurPC);
    }
  else
    erreur(12);
}
void Cadda_cmpa_suba(int value)
{
  int t, a, b, which;

  pc += 2;
  t = get_size(Word);
  if(t == Byte)
    erreur(11);
  a = op(t, TRUE);
  tstvirg();
  b = op(t, FALSE);
  if(value >= 106)
    which = 0;
  else if(value >= 32)
    which = 1;
  else
    which = 2;
  if((b & 070) == 010)
    if(t == Word)
      putW(0110300 + ((b & 7) << 9) + (a & 077) + (which << 13), CurPC);
    else
      putW(0110700 + ((b & 7) << 9) + (a & 077) + (which << 13), CurPC);
  else
    erreur(12);
}

void Caddi_cmpi_subi(int value)
{
  int t, a, b, n;

  pc += 2;
  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  b = op(t, FALSE);
  if(opt && (value < 32 || value > 100) && cur_pass >= 2 && ((t == Long && (a & 077) == 074 && (n = getL(CurPC + 2)) <= 8 && n >= 0) ||
							     (t == Word && (a & 077) == 074 && (n = getN(CurPC + 2)) <= 8 && n >= 0)))
    opt("ADDQ ou SUBQ possible");
  if((a & 077) == 074)
    {
      if((b & (Alterable + Data)) != Alterable + Data)
	erreur(12);
      if(value < 32)
	putW(0003000 + (t << 6) + (b & 077), CurPC);
      else if(value < 106)
	putW(0006000 + (t << 6) + (b & 077), CurPC);
      else
	putW(0002000 + (t << 6) + (b & 077), CurPC);
    }
  else
    erreur(12);
}

void Caddq_subq(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  if(symbol_type != '#')
    erreur(12);
  get_symbol();
  a = expression();
  tstvirg();
  b = op(t, FALSE);
  if(!(b & Alterable))
    erreur(12);
  if(a < 1 || a > 8)
    erreur(9);
  if(value == 110)
    putW(0050400 + (t << 6) + (b & 077) + ((a & 7) << 9), CurPC);
  else
    putW(0050000 + (t << 6) + (b & 077) + ((a & 7) << 9), CurPC);
}

void Caddx_subx(int value)
{
  int t, a, b;

  t = get_size(Word);
  a = op(t, TRUE);
  if(!(a & 070))
    {
      tstvirg();
      b = op(t, FALSE);
      if((b & 070))
	erreur(12);
      if(value == 111)
	genW(0110400 + ((b & 7) << 9) + (a & 7) + (t << 6));
      else
	genW(0150400 + ((b & 7) << 9) + (a & 7) + (t << 6));
    }
  else if((a & 040) && !(a & 030))
    {
      tstvirg();
      b = op(t, FALSE);
      if(!(b & 040) || (b & 030))
	erreur(12);
      if(value == 111)
	genW(0110410 + ((b & 7) << 9) + (a & 7) + (t << 6));
      else
	genW(0150410 + ((b & 7) << 9) + (a & 7) + (t << 6));
    }
  else
    erreur(12);
}

void Cand_or(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  if(symbol_type == WORD_TYPE && streql(cur_string, "ccr"))
    {
      get_symbol();
      if(t == Long)
	erreur(13);

      if(value == 78)
	putW(0x003c, CurPC);
      else if(value == 55)
	putW(0x0a3c, CurPC);
      else
	putW(0x023c, CurPC);
    }
  else if(symbol_type == WORD_TYPE && streql(cur_string, "sr"))
    {
      get_symbol();
      if(t == Long || t == Byte)
	erreur(13);
      if(value == 78)
	putW(0x007c, CurPC);
      else if(value == 55)
	putW(0x0a7c, CurPC);
      else
	putW(0x027c, CurPC);
    }
  else
    {
      b = op(t, FALSE);
      if((a & 077) == 074)
	{
	  if((b & (Alterable + Data)) != Alterable + Data)
	    erreur(12);
	  if(value == 78)
	    putW(0000000 + (t << 6) + (b & 077), CurPC);
	  else
	    putW(0001000 + (t << 6) + (b & 077), CurPC);
	}
      else if(!(b & 070))
	if(value == 78)
	  putW(0100000 + (t << 6) + ((b & 7) << 9) + (a & 077), CurPC);
	else
	  putW(0140000 + (t << 6) + ((b & 7) << 9) + (a & 077), CurPC);
      else if(!(a & 070))
	{
	  if((b & (Alterable + Data)) != Alterable + Data)
	    erreur(12);
	  if(value == 78)
	    putW(0100400 + (t << 6) + ((a & 7) << 9) + (b & 077), CurPC);
	  else
	    putW(0140400 + (t << 6) + ((a & 7) << 9) + (b & 077), CurPC);
	}
      else
	erreur(12);
    }
}

void Ceor(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  if(symbol_type == WORD_TYPE && streql(cur_string, "ccr"))
    {
      get_symbol();
      if(t == Long)
	erreur(13);

      putW(0x0a3c, CurPC);
    }
  else if(symbol_type == WORD_TYPE && streql(cur_string, "sr"))
    {
      get_symbol();
      if(t == Long || t == Byte)
	erreur(13);
      putW(0x0a7c, CurPC);
    }
  else
    {
      b = op(t, FALSE);
      if((a & 077) == 074)
	{
	  if((b & (Alterable + Data)) != Alterable + Data)
	    erreur(12);
	  putW(0005000 + (t << 6) + (b & 077), CurPC);
	}
      else if(!(a & 070))
	{
	  if((b & (Alterable + Data)) != Alterable + Data)
	    erreur(12);
	  putW(0130400 + (t << 6) + ((a & 7) << 9) + (b & 077), CurPC);
	}
      else
	erreur(12);
    }
}

void Candi_eori_ori(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  if(symbol_type == WORD_TYPE && streql(cur_string, "ccr"))
    {
      get_symbol();
      if(t == Long)
	erreur(13);

      if(value == 79)
	putW(0x003c, CurPC);
      else if(value == 56)
	putW(0x0a3c, CurPC);
      else
	putW(0x023c, CurPC);
    }
  else if(symbol_type == WORD_TYPE && streql(cur_string, "sr"))
    {
      get_symbol();
      if(t == Long || t == Byte)
	erreur(13);
      if(value == 79)
	putW(0x007c, CurPC);
      else if(value == 56)
	putW(0x0a7c, CurPC);
      else
	putW(0x027c, CurPC);
    }
  else
    {
      b = op(t, FALSE);
      if((a & 077) == 074)
	{
	  if((b & (Alterable + Data)) != Alterable + Data)
	    erreur(12);
	  if(value == 79)
	    putW(0000000 + (t << 6) + (b & 077), CurPC);
	  else if(value == 56)
	    putW(0005000 + (t << 6) + (b & 077), CurPC);
	  else
	    putW(0001000 + (t << 6) + (b & 077), CurPC);
	}
      else
	erreur(12);
    }
}

void Cdecale(int value)
{
  int t, a, b;
  int left = FALSE, reg;

  pc += 2;
  if(!(value & 1))
    left = TRUE;
  t = get_size(Word);
  if(symbol_type == '#')
    {
      get_symbol();
      a = expression();
      reg = FALSE;
    }
  else
    {
      a = op(t, TRUE);
      reg = TRUE;
    }
  if(symbol_type != ',' && (a & 070))
    {
      if((a & (Alterable + Memory)) != Alterable + Memory)
	erreur(12);
      if(t == Long)
	erreur(13);

      if(value >= 84)
	putW(0162300 + (left << 8) + (a & 077), CurPC);
      else if(value >= 82)
	putW(0163300 + (left << 8) + (a & 077), CurPC);
      else if(value > 60)
	putW(0161300 + (left << 8) + (a & 077), CurPC);
      else
	putW(0160300 + (left << 8) + (a & 077), CurPC);
    }
  else
    {
      if(symbol_type == ',')
	{
	  get_symbol();
	  b = op(t, FALSE);
	}
      else
	{
	  b = a;
	  a = 1;
	  reg = FALSE;
	}
      if((!(a & 070) || !reg) && !(b & 070))
	if(value >= 84)
	  putW(0160020 + (reg << 5) + (left << 8) + ((a & 7) << 9) + (b & 7) +
	       (t << 6), CurPC);
	else if(value >= 82)
	  putW(0160030 + (reg << 5) + (left << 8) + ((a & 7) << 9) + (b & 7) +
	       (t << 6), CurPC);
	else if(value > 60)
	  putW(0160010 + (reg << 5) + (left << 8) + ((a & 7) << 9) + (b & 7) +
	       (t << 6), CurPC);
	else
	  putW(0160000 + (reg << 5) + (left << 8) + ((a & 7) << 9) + (b & 7) +
	       (t << 6), CurPC);
      else
	erreur(12);
    }
}

void Cb(int value)
{
  int t, a;

  if(value >= 125)
    value += 14 - 125;
  t = get_size(Word);
  if(t == Long)
    erreur(13);
  a = expression() - (pc + org) - 2;
  if(notdef)
    a = 2;
  if(!a && cur_pass >= 2)
    erreur(9);
  if(opt && optt && (cur_pass >= 2 || !notdef) &&  a >= -128 && a < 128 && t == Word)
    {
      if(cur_pass == 1 || tstopt())
	{
	  addopt("use short branch");
	  t = Byte;
	}
      else
	opt("short branch possible");
    }
  if(a & 1)
    erreur(23);
  if(t == Word)
    {
      tstw(a);
      if(a >= 32768)
	erreur(9);
      genW(0060000 + ((value - 10) << 8));
      genW(a);
    }
  else
    {
      tstb(a);
      if(a >= 128)
	{
	  printf("\n-->%d\n", a);
	  erreur(9);
	}
      genW(0060000 + ((value - 10) << 8) + (a & 0xff));
    }
}

void Cbchg_bclr_bset_btst(int value)
{
  int t, a, b, which, c = (value == 29)? (Data): (Alterable + Data);

  pc += 2;
  t = get_size(Byte);
  if(t != Byte)
    erreur(13);
  a = op(Byte, TRUE);
  tstvirg();
  b = op(Byte, FALSE);
  if((b & c) != c)
    erreur(12);
  which = (value < 29) ? value - 26 + 1 : 0;
  if(!(a & 070))
    putW(0000400 + ((a & 7) << 9) + (b & 077) + (which << 6), CurPC);
  else if((a & 077) == 074)
    putW(0004000 + (b & 077) + (which << 6), CurPC);
  else
    erreur(12);
}

void Cchk(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  if(t != Word)
    erreur(15);
  a = op(Word, TRUE);
  if(!(a & Data))
    erreur(12);
  tstvirg();
  b = op(Word, FALSE);
  if(b & 070)
    erreur(12);
  putW(0040600 + ((b & 7) << 9) + (a & 077), CurPC);
}

void Cclr(int value)
{
  int t, a;

  pc += 2;
  t = get_size(Word);
  a = op(t, FALSE);
  if((a & (Alterable+Data)) != Alterable+Data)
    erreur(12);
  putW(0041000 + (t << 6) + (a & 077), CurPC);
}

void Ccmpm(int value)
{
  int t, a, b;

  t = get_size(Word);
  a = op(t, TRUE);
  tstvirg();
  b = op(t, FALSE);
  if((a & 070) != 030 || (b & 070) != 030)
    erreur(12);
  genW(0130410 + ((b & 7) << 9) + (t << 6) + (a & 7));
}

void Cdb(int value)
{
  int a, b;

  pc += 2;
  if(get_size(Word) != Word)
    erreur(15);
  b = op(Word, TRUE);
  if((b & 070))
    erreur(12);
  tstvirg();
  a = expression() - (CurPC + org) - 2;
  if(!a && cur_pass >= 2)
    erreur(9);
  if(a & 1)
    erreur(23);
  if(value == 36)
    value = 38;
  tstw(a);
  if(a >= 32768)
    erreur(9);
  putW(0050310 + ((value - 37) << 8) + (b & 7), CurPC);
  genW(a);
}

void Cdivs_divu(int value)
{
  int a, b;

  pc += 2;
  if(get_size(Word) != Word)
    erreur(15);
  a = op(Word, FALSE);
  if(!(a & Data))
    erreur(12);
  tstvirg();
  b = op(Word, FALSE);
  if(b & 070)
    erreur(12);
  putW(0100300 + ((b & 7) << 9) + ((54 - value) << 8) + (a & 077), CurPC);
}

void Cexg(int value)
{
  int a, b;

  if( get_size(Long) != Long)
    erreur(13);
  a = op(Long, TRUE);
  tstvirg();
  b = op(Long, FALSE);
  if((a & 060) || (b & 060))
    erreur(12);
  if((a & 010) != (b & 010))
    if(a & 010)
      genW(0140610 + ((b & 7) << 9) + (a & 7));
    else
      genW(0140610 + ((a & 7) << 9) + (b & 7));
  else
    if(a & 010)
      genW(0140510 + ((b & 7) << 9) + (a & 7));
    else
      genW(0140500 + ((a & 7) << 9) + (b & 7));
}
void Cext(int value)
{
  int t, a;

  t = get_size(Word);
  if(t == Byte)
    erreur(11);
  a = op(t, TRUE);
  if(a & 070)
    erreur(12);
  if(t == Word)
    genW(0044200 + (a & 7));
  else
    genW(0044300 + (a & 7));
}

void Cillegal(int value)
{
  genW(0x4afc);
}

void Cjmp(int value)
{
  int a;

  pc += 2;
  a = op(Word, TRUE);
  if(!(a & Control))
    erreur(12);
  putW(0047300 + (a & 077), CurPC);
}

void Cjsr(int value)
{
  int a;

  pc += 2;
  a = op(Word, TRUE);
  if(!(a & Control))
    erreur(12);
  putW(0047200 + (a & 077), CurPC);
}

void Clea(int value)
{
  int a, b;

  pc += 2;
  if(get_size(Long) != Long)
    erreur(13);
  a = op(Word, TRUE);
  if(!(a & Control))
    erreur(12);
  tstvirg();
  b = op(Word, FALSE);
  if((b & 070) != 010)
    erreur(12);
  putW(0040700 + ((b & 7) << 9) + (a & 077), CurPC);
}

void Clink(int value)
{
  int a, b;

  pc += 2;
  a = op(Word, TRUE);
  if((a & 070) != 010)
    erreur(12);
  tstvirg();
  b = op(Word, FALSE);
  if((b & 077) != 074)
    erreur(12);
  putW(0047120 + (a & 7), CurPC);
}

void Cmove(int value)
{
  int t, a, n;
  int m1, m2;

  t = get_size(Word);
  pc += 2;
  if(symbol_type == WORD_TYPE)
    {
      if(streql(cur_string, "sr"))
	{
	  if(t == Byte || t == Long)
	    erreur(13);
	  get_symbol();
	  tstvirg();
	  a = op(t, FALSE);
	  putW(0040300 + (a & 077), CurPC);
	  return;
	}
      else if(streql(cur_string, "ccr"))
	{
	  if(t == Long)
	    erreur(13);
	  get_symbol();
	  tstvirg();
	  a = op(t, FALSE);
	  putW(0041300 + (a & 077), CurPC);
	  return;
	}
      else if(streql(cur_string, "usp"))
	{
	  if(t == Byte)
	    erreur(11);
	  get_symbol();
	  tstvirg();
	  a = op(t, FALSE);
	  if((a & 070) != 010)
	    erreur(12);
	  putW(0047150 + (a & 7), CurPC);
	  return;
	}
    }
  m1 = op(t, TRUE);
  tstvirg();
  if(symbol_type == WORD_TYPE)
    {
      if(streql(cur_string, "sr"))
	{
	  if(t == Byte || t == Long)
	    erreur(13);
	  get_symbol();
	  putW(0043300 + (m1 & 077), CurPC);
	  return;
	}
      else if(streql(cur_string, "ccr"))
	{
	  if(t == Long)
	    erreur(13);
	  get_symbol();
	  putW(0042300 + (m1 & 077), CurPC);
	  return;
	}
      else if(streql(cur_string, "usp"))
	{
	  if(t == Byte)
	    erreur(11);
	  get_symbol();
	  if((m1 & 070) != 010)
	    erreur(12);
	  putW(0047140 + (m1 & 7), CurPC);
	  return;
	}
    }
  m2 = op(t, FALSE);
  if(!(m2 & Alterable))
    erreur(12);
  if(opt && cur_pass >= 2 && ((t == Long && (m1 & 077) == 074 && !(m2 & 070) && (n = getL(CurPC + 2)) >= -128L && n < 128L) ||
			      (t == Word && (m1 & 077) == 074 && !(m2 & 070) && (n = getN(CurPC + 2)) >= -128L && n < 128L)))
    opt("MOVEQ possible");
  putW((tt(t, 1, 3, 2) << 12) + ((m2 & 7) << 9) + ((m2 & 070) << (6 - 3)) +
       (m1 & 077), CurPC);
}

void Cmovea(int value)
{
  int t;
  int m1, m2;

  t = get_size(Word);
  pc += 2;
  m1 = op(t, TRUE);
  tstvirg();
  m2 = op(t, FALSE);
  if((m2 & 070) != 010)
    erreur(12);
  putW((tt(t, 1, 3, 2) << 12) + ((m2 & 7) << 9) + ((m2 & 070) << (6 - 3)) +
       (m1 & 077), CurPC);
}

void Cmovem(int value)
{
  int t, a, b, n, c;

  pc += 4;
  t = get_size(Word);
  if(t == Byte)
    erreur(11);
  if(isreg(cur_string))
    {
      b = reglist();
      tstvirg();
      a = op(t, FALSE);
      if((a & 070) == 030 ||
	 ((a & (Control + Alterable)) != Control + Alterable &&
	  (a & 070) != 040)
	 )
	erreur(12);
      if((a & 070) == 040)
	{
	  c=0;
	  for(n = 0 ; n < 16 ; n++)
	    {
	      c = (c << 1) + (b & 1);
	      b >>= 1;
	    }
	}
      else
	{c = b;}
      /*		t = 0044200 + (tt(t, 0, 0, 1) << 6) + (a & 077);
			putW(t, CurPC);
			fprintf(ferr,"{t:%d}",t);*/
      putW(0044200 + (tt(t, 0, 0, 1) << 6) + (a & 077), CurPC);
      putW(c, CurPC + 2);
      /*		fprintf(ferr,"{c:%d}",c);*/
    }
  else
    {
      a = op(t, TRUE);
      tstvirg();
      if((a & 070) == 040 || (!(a & Control) && (a & 070) != 030))
	erreur(12);
      putW(0046200 + (tt(t, 0, 0, 1) << 6) + (a & 077), CurPC);
      putW(reglist(), CurPC + 2);
    }
}

void Cmovep(int value)
{
  int t, a, b;

  pc += 2;
  t = get_size(Word);
  if(t == Byte)
    erreur(11);
  if(symbol_type == WORD_TYPE && isreg(cur_string))
    {
      a = cur_reg;
      if(a & 070)
	erreur(12);
      get_symbol();
      tstvirg();
      b = op(t, FALSE);
      if((b & 070) == 020)
	genW(0);
      else if((b & 070) != 050)
	erreur(12);
      putW(0000610 + ((a & 7) << 9) + (tt(t, 0, 0, 1) << 6) + (b & 7), CurPC);
    }
  else
    {
      b = op(t, TRUE);
      if((b & 070) == 020)
	genW(0);
      else if((b & 070) != 050)
	erreur(12);
      tstvirg();
      if(symbol_type != WORD_TYPE || !isreg(cur_string))
	erreur(12);
      a = cur_reg;
      if(a & 070)
	erreur(12);
      get_symbol();
      putW(0000410 + ((a & 7) << 9) + (tt(t, 0, 0, 1) << 6) + (b & 7), CurPC);
    }
}

void Cmoveq(int value)
{
  int a, b=0;


  if( get_size(Long) != Long)
    erreur(13);

  if(symbol_type != '#')
    erreur(12);

  get_symbol();

  a = expression();

  tstvirg();
  if(symbol_type != WORD_TYPE || !isreg(cur_string) ||
     (b = cur_reg) & 070)
    erreur(12);
  get_symbol();
  tstb(a);
  if(a>=128 && cur_pass == 2)
    warning("sign extend");
  genW(0070000 + ((b & 7) << 9) + (a & 0xff));
}

void Cmuls_mulu(int value)
{
  int a, b;

  pc += 2;
  if(get_size(Word) != Word)
    erreur(15);
  a = op(Word, TRUE);
  if(!(a & Data))
    erreur(12);
  tstvirg();
  b = op(Word, FALSE);
  if(b & 070)
    erreur(12);
  putW(0140300 + ((b & 7) << 9) + ((72 - value) << 8) + (a & 077), CurPC);
}

void Cnbcd(int value)
{
  int a;

  pc += 2;
  if(get_size(Byte) != Byte)
    erreur(13);
  a = op(Byte, TRUE);
  if((a & (Data + Alterable)) != Data + Alterable)
    erreur(12);
  putW(0044000 + (a & 077), CurPC);
}

void Cneg(int value)
{
  int t, a;

  pc += 2;
  t = get_size(Word);
  a = op(t, FALSE);
  if((a & (Alterable+Data)) != (Alterable+Data))
    erreur(12);
  putW(0042000 + (t << 6) + (a & 077), CurPC);
}

void Cnegx(int value)
{
  int t, a;

  pc += 2;
  t = get_size(Word);
  a = op(t, FALSE);
  if((a & (Alterable+Data)) != (Alterable+Data))
    erreur(12);
  putW(0040000 + (t << 6) + (a & 077), CurPC);
}

void Cnop(int value)
{
  genW(0x4e71);
}

void Cnot(int value)
{
  int t, a;

  pc += 2;
  t = get_size(Word);
  a = op(t, FALSE);
  if((a & (Alterable+Data)) != (Alterable+Data))
    erreur(12);
  putW(0043000 + (t << 6) + (a & 077), CurPC);
}

void Cpea(int value)
{
  int a;

  pc += 2;
  if( get_size(Long) != Long)
    erreur(13);
  a = op(Long, TRUE);
  if(!(a & Control))
    erreur(12);
  putW(0044100 + (a & 077), CurPC);
}


void Creset(int value)
{
  genW(0x4e70);
}

void Crte(int value)
{
  genW(0x4e73);
}

void Crtr(int value)
{
  genW(0x4e77);
}

void Crts(int value)
{
  genW(0x4e75);
}

void Cs(int value)
{
  int a;

  pc += 2;
  if(get_size(Byte) != Byte)
    erreur(13);
  a = op(Byte, FALSE);
  if((a & (Data + Alterable)) != Data + Alterable)
    erreur(12);
  putW(0050300 + ((value - 90) << 8) + (a & 077), CurPC);
}

void Cstop(int value)
{
  int a;

  if(symbol_type != '#')
    erreur(12);
  get_symbol();
  a = expression();
  tstw(a);
  genW(0x4e72);
  genW(a);
}

void Cswap(int value)
{
  if(get_size(Word) != Word)
    erreur(13);
  if(symbol_type != WORD_TYPE || !isreg(cur_string) ||
     (cur_reg & 8))
    erreur(12);
  genW(0044100 + cur_reg);
  get_symbol();
}

void Ctas(int value)
{
  int a;

  pc += 2;
  if(get_size(Byte) != Byte)
    erreur(13);
  a = op(Byte, FALSE);
  if((a & (Data + Alterable)) != Data + Alterable)
    erreur(12);
  putW(045300 + (a & 077), CurPC);
}

void Ctrap(int value)
{
  int a;

  if(symbol_type != '#')
    erreur(12);
  get_symbol();
  a = expression();
  tstb(a);
  genW(0047100 + (a & 15));
}

void Ctrapv(int value)
{
  genW(0x4e76);
}

void Ctst(int value)
{
  int t, a;

  pc += 2;
  t = get_size(Word);
  a = op(t, FALSE);
  if((a & (Alterable+Data)) != (Alterable+Data))
    erreur(12);
  putW(0045000 + (t << 6) + (a & 077), CurPC);
}

void Cunlk(int value)
{
  int a=-1;
  if(symbol_type != WORD_TYPE || !isreg(cur_string) ||
     !((a = cur_reg) & 070))
    erreur(12);
  if(a==-1)	internal_error("Cunlk");
  genW(0x4e58 + (a & 7));
}


void Fdcb(int value)
{
  do
    {
      if(symbol_type == STRING_TYPE && strlen(cur_string) > 1)
	{
	  char *s = cur_string;
	  while(*s)
	    genB(*s++);
	  get_symbol();
	}
      else
	genB(expression());
      if(symbol_type == ',')
	get_symbol();
      else
	break;
    }
  while(TRUE);
}

void Fdcw(int value)
{
  do
    {
      genW(expression());
      if(symbol_type == ',')
	get_symbol();
      else
	break;
    }
  while(TRUE);
}

void Fdcl(int value)
{
  do
    {
      genL(expression());
      if(symbol_type == ',')
	get_symbol();
      else
	break;
    }
  while(TRUE);
}

void Fdcv(int value)
{
  do
    {
      gen24(expression());
      if(symbol_type == ',')
	get_symbol();
      else
	break;
    }
  while(TRUE);
}

void Fdc(int value)
{
  int size = get_size(WS);
  switch(size)
    {
    case BS:
      Fdcb(0);
      break;

    case WS:
      Fdcw(0);
      break;

    case LS:
      Fdcl(0);
      break;

    case VS:
      Fdcv(0);
      break;

    }
}

void Fdss(int size)
{
  int n, a;

  n = expression();
  if(notdef)
    error(error_list[16]);
  if(symbol_type != ',')
    {
      n *= size;
      while(n-- > 0) genB(0);
    }
  else
    {
      get_symbol();
      a = expression();
      if(size == 1)
	{
	  tstb(a);
	  while(n-- > 0)
	    genB(a);
	}
      else if(size == 2)
	{
	  tstw(a);
	  while(n-- > 0)
	    genW(a);
	}
      else
	while(n-- > 0)
	  genL(a);
    }
}

void Fds()
{
  int size = get_size(WS);
  Fdss(size_table[size]);
}

void Fprint()
{
  int aff;
  aff = cur_pass > 1 && (!opt_relocatable || cur_pass > 2);
  if(symbol_type == STRING_TYPE)
    {
      if(aff)
	printf("\015--> '%s'\n", cur_string);
      get_symbol();
    }
  else
    {
      int n = expression();
      if(aff)
	printf("\015-->%d (%x)\n", n, n);
    }
  if(!aff)
    last_pass = FALSE;
}

void Frepeat()
{
  position pos;
  int	n;
  int	old_n_macro = cur_n_macro;

  pos = curpos;
  if(symbol_type != ';' && symbol_type)
    {
      n = expression();
      if(notdef)
	error(error_list[16]);
      if(n > 0)
	{
	  while(n--)
	    {
	      n_macro++;
	      cur_n_macro = n_macro;

	      curpos = pos;
	      assemble_block();
	    }
	}
      else
	skip_block();
    }
  else
    {
      do
	{
	  n_macro++;
	  cur_n_macro = n_macro;

	  curpos = pos;
	  assemble_block();
	  get_line();
	  if(symbol_type != WORD_TYPE || strcmp(cur_string, "UNTIL"))
	    {
	      error_expected("UNTIL");
	      break;
	    }
	  get_symbol();
	  n = expression();
	  if(notdef)
	    error(error_list[16]);
	}
      while(!n);
    }

  cur_n_macro = old_n_macro;
}

void Fwhile()
{
  position pos = lastpos;
  int n = expression();

  if(notdef)
    error(error_list[16]);
  if(n)
    {
      assemble_block();
      curpos = pos;
    }
  else
    skip_block();
}

void Fif()
{
  int n = expression();
  position p;

  if(notdef)
    error(error_list[16]);
  if(n)
    assemble_block();
  else
    skip_block();
  p = curpos;
  get_line();
  if(symbol_type == WORD_TYPE && !strcmp(cur_string, "ELSE"))
    {
      if(n)
	skip_block();
      else
	assemble_block();
    }
  else
    {
      curpos = p;
      symbol_type = 0;
    }
}

void Fincbin()
{
  char	name[256];


  if(symbol_type != STRING_TYPE)
    error_expected("string");
  else
    {
      int	size;
      FILE	*fd;
      sprintf(name,"%s%s",cur_dir,cur_string);
      size = search_file(name);
      if(!size)
	error(error_list[34]);
      else
	{
	  if(pc+size > buffer_size*1024)
	    fatal_error("code buffer overflow (use -b to iuncrease)");
	  if(cur_pass > 1)
	    {
	      fd = fopen(name, "rb");
	      fread(code+pc, 1, size, fd);
	      fclose(fd);
	    }
	  pc += size;
	  if(pc > max_pc)
	    max_pc = pc;
	}
      get_symbol();
    }
}

void Finclude()
{
  char		name[256];
  position pos, old_pos;

  if(symbol_type != STRING_TYPE)
    error_expected("string");
  else
    {
      old_pos = curpos;
      strcpy(name, cur_string);
      if(!new_position(&pos, name))
	fatal_error("can't open <%s>",name);
      pass(&pos);
      curpos = old_pos;
      symbol_type = END_OF_LINE;
    }
}

void Forg()
{
  if(opt_relocatable)
    get_symbol();
  else
    org = expression()-pc;

  if(opt_relwarn && cur_pass==1)
    warning(warn_list[4]);
}

void Fdefault()
{
  if(symbol_type == WORD_TYPE)
    {
      if(!strcmp(cur_string, "BYTE"))
	default_size = BS;
      else if(!strcmp(cur_string, "WORD"))
	default_size = WS;
      else
	error(error_list[26]);
      get_symbol();
    }
  else
    error_expected("'WORD' or 'BYTE'");
}

void Feven()
{
  if(pc & 1)
    pc++;
}
void Falign()
{
  int n = expression();

  if(pc % n)
    {
      n = n - (pc%n);
      pc += n;
    }
}

void Frsreset()
{
  cur_rs = 0;
}
void Frsset()
{
  cur_rs = expression();
  if(notdef)
    error(error_list[16]);
}

void Fload()
{
  load_adr = expression();
}
void Frun()
{
  run_adr = expression();
}

void Floadsymbol()
{
  FILE	*fd;
  word	*w;

  if(symbol_type != STRING_TYPE)
    {
      error_expected("string");
      return;
    }
  get_symbol();

  if(cur_pass > 1)
    return;

  fd = fopen(cur_string, "rb");
  if(!fd)
    {
      error("%s <%s>", error_list[20],cur_string);
      return;
    }

  while(!feof(fd))
    {
      char c[128], *s;

      s = c;
      while( (*s++ = fgetc(fd)) );
      if(s-1 == c)
	break;
      w = (word *) malloc(sizeof(word));
      if(!w)
	memory_error("word");
      w->name = strsav(c);
      w->type = LABEL;
      n_label++;
      w->pd.value = fgetc(fd);
      w->pd.value += (fgetc(fd) << 8);
      w->pd.value += (fgetc(fd) << 16);
      w->pd.value += (fgetc(fd) << 24);
      w->dd.value2 = w->pd.value;
      put_word(w);
    }
  fclose(fd);

  /*	if(opt_relocatable)
	{
	chg_ext(cur_string, ".SY2");
	fd = fopen(cur_string, "rb");
	if(!fd)
	{
	char a[128];

	sprintf(a, "impossible d'ouvrir %s", cur_string);
	error(a);
	return;
	}

	while(!feof(fd))
	{
	char c[128], *s;

	s = c;
	while( (*s++ = fgetc(fd)) );
	if(s-1 == c)
	break;
	w = search(c);
	if(!w)
	memory_error();
	w->dd.value2 = fgetc(fd);
	w->dd.value2 += (fgetc(fd) << 8);
	w->dd.value2 += (fgetc(fd) << 16);
	w->dd.value2 += (fgetc(fd) << 24);
	}
	fclose(fd);
	} */
}

void Floadsymbol2()
{
  FILE	*fd;
  word	*w;

  if(symbol_type != STRING_TYPE)
    {
      error_expected("string");
      return;
    }
  get_symbol();

  if(cur_pass > 1)
    return;

  fd = fopen(cur_string, "rb");
  if(!fd)
    {
      error("%s <%s>",error_list[20], cur_string);
      return;
    }

  while(!feof(fd))
    {
      char c[128], *s;

      s = c;
      while( (*s++ = fgetc(fd)) );
      if(s-1 == c)
	break;
      w = (word *) malloc(sizeof(word));
      if(!w)
	memory_error("word");
      w->name = strsav(c);
      w->type = LABEL;
      n_label++;
      w->pd.value = fgetc(fd);
      w->pd.value += (fgetc(fd) << 8);
      w->pd.value += (fgetc(fd) << 16);
      w->pd.value += (fgetc(fd) << 24);
      put_word(w);
    }
  fclose(fd);

  if(opt_relocatable)
    {
      chg_ext(cur_string, ".SY2");
      fd = fopen(cur_string, "rb");
      if(!fd)
	{
	  error("%s <%s>", error_list[20], cur_string);
	  return;
	}

      while(!feof(fd))
	{
	  char c[128], *s;

	  s = c;
	  while( (*s++ = fgetc(fd)) );
	  if(s-1 == c)
	    break;
	  w = search(c);
	  if(!w)
	    memory_error("search");
	  w->dd.value2  = (unsigned char)fgetc(fd);
	  w->dd.value2 += (unsigned char)fgetc(fd)<<8;
	  w->dd.value2 += (unsigned char)fgetc(fd)<<16;
	  w->dd.value2 += (unsigned char)fgetc(fd)<<24;
	}
      fclose(fd);
    }
}

void Frelocation()
{
  if(symbol_type == WORD_TYPE && streql(cur_string, "on"))
    {
      opt_relocatable = TRUE;
      if(cur_pass == 1)
	{
	  org = 0;
	  if(pc != 0)
	    fatal_error("RELOCATION must be at top");
	}
    }
  else if(symbol_type == WORD_TYPE && streql(cur_string, "off"))
    {
      if(opt_relocatable)
	fatal_error("RELOCATION directif changed");
      opt_relocatable = FALSE;
    }
  else
    error_expected("ON or OFF");

  get_symbol();
}

void Fbss()
{
  bss = pc;
}


struct slow_table {
  char	*s;
  void	(*function)();
  int	value;
}

opcode_list[] =
  {
    /* 68K mnemonics */
    { "abcd",		Cabcd_sbcd			,0  },
    { "add",		Cadd_cmp_sub		,1  },
    { "adda",		Cadda_cmpa_suba	,2  },
    { "addi",		Caddi_cmpi_subi	,3  },
    { "addq",		Caddq_subq			,4  },
    { "addx",		Caddx_subx			,5  },
    { "and",		Cand_or					,6   },
    { "andi",		Candi_eori_ori	,7   },
    { "asl",		Cdecale					,8   },
    { "asr",		Cdecale					,9   },
    { "bra",		Cb							,10  },
    { "bsr",		Cb							,11  },
    { "bhi",		Cb							,12  },
    { "bls",		Cb							,13  },
    { "bcc",		Cb							,14  },
    { "bcs",		Cb							,15  },
    { "bne",		Cb							,16  },
    { "beq",		Cb							,17  },
    { "bvc",		Cb							,18  },
    { "bvs",		Cb							,19  },
    { "bpl",		Cb							,20  },
    { "bmi",		Cb							,21  },
    { "bge",		Cb							,22  },
    { "blt",		Cb							,23  },
    { "bgt",		Cb							,24  },
    { "ble",		Cb							,25  },
    { "blo",		Cb							,15  },
    { "bhs",		Cb							,14  },
    { "bchg",		Cbchg_bclr_bset_btst	,26  },
    { "bclr",		Cbchg_bclr_bset_btst	,27  },
    { "bset",		Cbchg_bclr_bset_btst	,28  },
    { "btst",		Cbchg_bclr_bset_btst	,29  },
    { "chk",		Cchk						,30  },
    { "clr",		Cclr						,31  },
    { "cmp",		Cadd_cmp_sub			,32  },
    { "cmpa",		Cadda_cmpa_suba		,33  },
    { "cmpi",		Caddi_cmpi_subi		,34  },
    { "cmpm",		Ccmpm						,35  },
    { "dbra",		Cdb						,36  },
    { "dbt",		Cdb						,37  },
    { "dbf",		Cdb						,38  },
    { "dbhi",		Cdb						,39  },
    { "dbls",		Cdb						,40  },
    { "dbcc",		Cdb						,41  },
    { "dbcs",		Cdb						,42  },
    { "dbne",		Cdb						,43  },
    { "dbeq",		Cdb						,44  },
    { "dbvc",		Cdb						,45  },
    { "dbvs",		Cdb						,46  },
    { "dbpl",		Cdb						,47  },
    { "dbmi",		Cdb						,48  },
    { "dbge",		Cdb						,49  },
    { "dblt",		Cdb						,50  },
    { "dbgt",		Cdb						,51  },
    { "dble",		Cdb						,52  },
    { "divs",		Cdivs_divu				,53  },
    { "divu",		Cdivs_divu				,54  },
    { "eor",		Ceor						,55  },
    { "eori",		Candi_eori_ori			,56  },
    { "exg",		Cexg						,57  },
    { "ext",		Cext						,58  },
    { "illegal",	Cillegal					,59  },
    { "jmp",		Cjmp						,60  },
    { "jsr",		Cjsr						,61  },
    { "lea",		Clea						,62  },
    { "link",		Clink						,63  },
    { "lsl",		Cdecale					,64  },
    { "lsr",		Cdecale					,65  },
    { "move",		Cmove						,66  },
    { "movea",		Cmovea					,67  },
    { "movem",		Cmovem					,68  },
    { "movep",		Cmovep					,69  },
    { "moveq",		Cmoveq					,70  },
    { "muls",		Cmuls_mulu				,71  },
    { "mulu",		Cmuls_mulu				,72  },
    { "nbcd",		Cnbcd						,73  },
    { "neg",		Cneg						,74  },
    { "negx",		Cnegx						,75  },
    { "nop",		Cnop						,76  },
    { "not",		Cnot						,77  },
    { "or",			Cand_or					,78  },
    { "ori",		Candi_eori_ori			,79  },
    { "pea",		Cpea						,80  },
    { "reset",		Creset					,81  },
    { "rol",		Cdecale					,82  },
    { "ror",		Cdecale					,83  },
    { "roxl",		Cdecale					,84  },
    { "roxr",		Cdecale					,85  },
    { "rte",		Crte						,86  },
    { "rtr",		Crtr						,87  },
    { "rts",		Crts						,88  },
    { "sbcd",		Cabcd_sbcd				,89  },
    { "st",			Cs							,90  },
    { "sf",			Cs							,91  },
    { "shi",		Cs							,92  },
    { "sls",		Cs							,93  },
    { "scc",		Cs							,94  },
    { "scs",		Cs							,95  },
    { "sne",		Cs							,96  },
    { "seq",		Cs							,97  },
    { "svc",		Cs							,98  },
    { "svs",		Cs							,99  },
    { "spl",		Cs							,100  },
    { "smi",		Cs							,101  },
    { "sge",		Cs							,102  },
    { "slt",		Cs							,103  },
    { "sgt",		Cs							,104  },
    { "sle",		Cs							,105  },
    { "stop",		Cstop						,106  },
    { "sub",		Cadd_cmp_sub			,107  },
    { "suba",		Cadda_cmpa_suba		,108  },
    { "subi",		Caddi_cmpi_subi		,109  },
    { "subq",		Caddq_subq				,110  },
    { "subx",		Caddx_subx				,111  },
    { "swap",		Cswap						,112  },
    { "tas",		Ctas						,113  },
    { "trap",		Ctrap						,114  },
    { "trapv",		Ctrapv					,115  },
    { "tst",		Ctst						,116  },
    { "unlk",		Cunlk						,117 },

    /* Data storage facilities */
    { "dc",		Fdc,		0 },
    { "ds",		Fds,		0 },
    { "dcb",	Fds,		0 },
    /*
      {	"dcb",	Fdcb,		0 },
      { "dcw",	Fdcw,		0 },
      { "dcl",	Fdcl,		0 },
      { "dcv",	Fdcv,		0 },
      {	"dsb",	Fdss,		1 },
      { "dsw",	Fdss,		2 },
      { "dsl",	Fdss,		4 },
      { "dsv",	Fdss,		3,}, */

    /* Assembler control directives */
    { "PRINT",	Fprint,	0 },
    { "REPEAT",Frepeat,	0 },
    { "RPT",	Frepeat,	0 },
    { "WHILE",	Fwhile,	0 },
    { "IF",		Fif,		0 },
    { "INCBIN",Fincbin,	0 },
    { "INCLUDE",Finclude, 0 },
    { "ORG",	Forg,		0 },
    { "DEFAULT",Fdefault,0 },
    { "EVEN",	Feven,	0 },
    { "ALIGN",	Falign,	0 },
    { "RSRESET",Frsreset, 0 },
    { "RSSET",	Frsset,	0 },
    { "LOAD",	Fload,	0 },
    { "RUN",	Frun,		0 },
    { "LS",		Floadsymbol, 0 },
    { "LST",	Floadsymbol2, 0 },
    { "RELOCATION", Frelocation, 0 },
    { "BSS", Fbss, 0 },
    { 0, 0, 0 }
  },

  register_list[] =
    {
      { "sp",		0,	15 },
      { 0, 0, 0 }
    };

void make_fast_table(struct fast_table **ft, struct slow_table *st)
{
  struct fast_table *opt=NULL, **popt;
  uchar c;
  int n;
  for(n=0; st[n].s; n++)
    {
      char *s;
      s=st[n].s;
      popt = ft;
      for(c=*s++; isalpha(c); c=*s++)
	{
	  c = toupper(c)-'A';
	  opt = popt[c];
	  if(!opt)
	    {
	      int i;
	      opt = popt[c] = (struct fast_table *) malloc(sizeof(struct fast_table));
	      if(opt==NULL)
		memory_error("struct fast_table");
	      opt->function = 0;
	      for(i=0 ; i<26 ; i++)
		opt->letter[i] = 0;
	    }
	  popt = opt->letter;
	}
      if(c)
	internal_error("make_fast_table : slow table corrupt");

      if(opt==NULL)
	internal_error("make_fast_table : opt==NULL");
      opt->function	= st[n].function;
      opt->value 		= st[n].value;
    }
}

void init_opcodes()
{
  memset(opcode_table,0,sizeof(opcode_table));
  make_fast_table(opcode_table, opcode_list);

  memset(register_table,0,sizeof(register_table));
  make_fast_table(register_table, register_list);
}

