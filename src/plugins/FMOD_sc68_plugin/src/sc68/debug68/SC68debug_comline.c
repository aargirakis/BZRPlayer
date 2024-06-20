/*
 *                       sc68 - debug68 command line
 *         Copyright (C) 2001 Benjamin Gerard <ben@sashipa.com>
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "debug68/SC68debug_comline.h"

#ifdef HAVE_READLINE_READLINE_H
# include <readline/readline.h>
#else
# undef HAVE_READLINE_HISTORY_H
/* A very simple readline() replacement :) */
static char *readline (const char *prompt)
{
  char tmp[1024], *s;
  if (prompt) {
    fputs(prompt, stdout);
  }
  s = fgets(tmp, sizeof(tmp)-1, stdin);
  if (s) {
    s[sizeof(tmp)-1] = 0;
    s = strdup(s);
  }
  return s;
}
#endif

#ifdef HAVE_READLINE_HISTORY_H
# include <readline/history.h>
#else
/* ... and an even more simple add_history() replacement :) */
static void add_history(const char *s) {}
#endif

#define MAX_COMARG 15

static char * killspace(char *s)
{
  while (*s && !isgraph(*s))
    s++;
  return s;
}

/* Get word start & end. Could be inside quote. 
 * Returns wordend or 0 is no word.
 */
static char * word(char * word, char ** wordstart)
{
  word = killspace(word);
  if (!word[0]) {
    wordstart = 0;
    return 0;
  } else if (word[0] == '"') {
    *wordstart = ++word;
    for (; *word && *word != '"'; word++);
  } else {
    *wordstart = word;
    for (; *word && !isspace(*word); word++);
  }
  if (*word) {
    *word++ = 0;
  }
  return word;
}

static int dispatch_word(char ** here, int max_args, char *str)
{
  int i;
  for (i = 0; i < max_args && str; i++) {
    str = word(str, here + i);
  }
  return i - 1;
}


  /** Free command line. */
void SC68comline_free(debug68_comline_t * comline)
{
  if (comline) {
    if (comline->comline) {
      free(comline->comline);
    }
    memset(comline,0,sizeof(*comline));
  }
}


int SC68comline_read(const char * prompt, debug68_comline_t * comline)
{
  const int max_coms = sizeof(comline->coms)/sizeof(*comline->coms)-1;
  char * enter;

  if (!comline) {
    return 0;
  }

  SC68comline_free(comline);

  comline->comline = readline(prompt);
  if (!comline->comline) {
    return 0;
  }

  /* Remove trailing '/n' */
  if (enter = strchr(comline->comline,'\n'), enter) {
    *enter = 0;
  }

  enter = killspace(comline->comline);
  /* No command line */
  if (!*enter) {
    SC68comline_free(comline);
    return 0;
  }

  /* Add to history before word parsing. */
  add_history(enter);

  /* Process */
  comline->na =
    dispatch_word(comline->coms, max_coms, enter);

  if (comline->na <= 0) {
    SC68comline_free(comline);
    return 0;
  }


  return comline->na;
}
