/**
 * @ingroup   as68_devel
 * @file      word.c
 * @author    Penne Vincent
 * @date      1993
 * @brief     68000 macro assembler - word hash functions
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "as68.h"

word *word_table[256];

int checksum(char *s)
{
	int c = 0;
	while(*s)
		c = c << 1 ^ (*s++&255);
	return c&255;
}

word *search(char *s)
{
	word *w = word_table[checksum(s)];
	while(w)
	{
		if(!strcmp(s, w->name))
			return w;
		w = w->next;
	}
	return 0L;
}

void put_word(word *w)
{
	int chk = checksum(w->name);
	w->next = word_table[chk];
	word_table[chk] = w;
}

void delete_word(word *ww)
{
	int chk = checksum(ww->name);
	word *w = word_table[chk], **ow = &word_table[chk];

	while(w)
	{
		if(w == ww)
		{
			*ow = w->next;
			return;
		}
		ow = &w->next;
		w = w->next;
	}
}

char *strsav(char *s)
{
	char *ss = (char *) malloc(strlen(s)+1);
	if(ss==NULL)
		memory_error("strsav");
	strcpy(ss, s);
	return ss;
}

void delete_word_list(word *w)
{
	word *nw;
	while(w!=NULL)
	{
		nw = w->next;
		free(w);
		w = nw;
	}
}

struct fast_table *look_in_fast_table(char *s, struct fast_table **pft)
{
	uchar c;
	struct fast_table *ft=NULL;

	if(pft==NULL || s==NULL)
		return NULL;

	while(c=*s++, isalpha(c))
	{
		c = toupper(c)-'A';
		ft = pft[c];
		if(!ft)
			return NULL;
		pft = &(ft->letter[0]);
	}
	if(c || ft==NULL)
		return NULL;

	if(ft->function)
		return ft;
	else
		return NULL;
}
