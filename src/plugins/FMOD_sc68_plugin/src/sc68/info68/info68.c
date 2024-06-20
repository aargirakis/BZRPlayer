/*
 *                   info68 - Get sc68 file information
 *         Copyright (C) 2001 Ben(jamin) Gerard <ben@sashipa.com>
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
 
/* $Id: info68.c,v 2.3 2003/09/24 19:31:08 benjihan Exp $ */

#include <config68.h>

/* Standard includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

/* sc68 includes */
#include "file68/error68.h"
#include "file68/alloc68.h"
#include "file68/file68.h"
#include "file68/string68.h"

#define BUILD_DATE  __DATE__

static int error(const char *format, ...)
{
  va_list list;
  va_start(list, format);
  vfprintf(stderr, format, list);
  va_end(list);
  return -1;
}

static int spool_error(void)
{
  int i = 0;
  const char * err = SC68error_get();
  while(err) {
    error("  (%d) \"%s\"\n", ++i, err);
    err = SC68error_get();
  }
  return i;
}

static int file_error(const char *filename)
{
  error ("Bad or missing sc68 input file \"%s\"\n", filename);
  spool_error();
  return 2;
}

static int command_error(const char *command)
{
  error ("Bad print command \"%s\" (skipped)\n", command);
  return 3;
}

static int tracklist_error(const char *command)
{
  error("Bad track-list \"%s\"\n", command);
  return 4;
}

static int Message(void)
{
  puts(
    "\n"
    "info68: Get and format information from sc68 files.\n"
    "\n"
    "          by Ben(jamin) Gerard / sashipa\n"
    "          (c)sashipa [" BUILD_DATE "]\n"
    "\n"
    "usage: info68 [option ... ] sc68-file [ command ...] \n"
    "\n"
    "option:\n"
    "\n"
    "  --help         : display this message and exit.\n"
    "  --version\n"
    "  --build        : display build date and exit.\n"
    "  -o file\n"
    "  --output file  : Change output to file (- is stdout)\n"
    "\n"
    "command:\n"
    "\n"
    "  -#        : number of tracks\n"
    "  -?        : default track\n"
    "  -N        : disk name\n"
    "  -A        : default track author name\n"
    "  -C        : default track composer name\n"
    "  -T        : disk time in sec\n"
    "  -Y        : formated disk time. Format \"TT MM:SS\"\n"
    "  -H        : all tracks ORed hardware flags (see -h)\n"
    "\n"
    " track-list:\n"
    "\n"
    "  -DIGIT[[,DIGIT]|[-DIGIT]]\n"
    "            : track-list executes following track-commands for all listed\n"
    "              tracks until another track-list is encountered or end of\n"
    "              command line.\n"
    "              Track are base 1 numbered. 0 is replaced by the number of tracks.\n"
    "              This command is a kind of loop.\n"
    "              e.g: \"-1,5,[4-6]\" works with tracks 1,5,4,5 and 6 in this order.\n"
    "\n"
    " track-commands [apply to current track]:\n"
    "\n"
    "  -%        : track number\n"
    "  -n        : track name\n"
    "  -a        : author name\n"
    "  -c        : composer name\n"
    "  -r        : replay name\n"
    "  -t        : time in sec\n"
    "  -y        : formated time. Format \"TT MM:SS\"\n"
    "  -f        : replay frequency\n"
    "  -@        : load address\n"
    "  -h        : hardware flags [YSA] uppercase means activated\n"
    "                 Y:YM-2149,  S:STE  A:Amiga\n"
    "\n"
    " misc commands\n"
    "\n"
    "  -L        : Display a newline character\n"
    "  --STRING  : Display \"-STRING\"\n"
    "  STRING    : Display \"STRING\"\n"
    "\n");

  return 1;
}

static const char *HWflags(int f)
{
  static char flags[] = "YSA";
  flags[0] = (f & SC68_YM)    ? 'Y' : 'y';
  flags[1] = (f & SC68_STE)   ? 'S' : 's';
  flags[2] = (f & SC68_AMIGA) ? 'A' : 'a';
  return flags;
}

static void Puts(FILE *out, const char *s)
{
  fputs(s, out);
}

static void PutI(FILE *out, int v)
{
  fprintf(out,"%d", v);
}

static void PutX(FILE *out, int v)
{
  fprintf(out,"%x", v);
}

static int ReadTrackNumber(char **ps, int max)
{
  int n;
  if (!isdigit(**ps)) {
    return -1;
  }
  n = strtol(*ps, ps, 10);
  if (n<0 || n>max) {
    return -1;
  } else if (n==0) {
    n = max;
  }
  return n - 1;
}

static int ReadTrackList(char **trackList, int max, int *from, int *to)
{
  int fromTrack, toTrack;
  char *t = *trackList;

  if (t) {
    /* Skip comma ',' */
    while(*t == ',') {
      ++t;
    }
  }

  /* Done with this list. */
  if (!t || !*t) {
    *trackList = 0;
    return 0;
  }

  *trackList = t;
  fromTrack = ReadTrackNumber(trackList, max);
  if (fromTrack < 0) {
    return -1;
  }

  switch(**trackList) {
  case ',': case 0:
    toTrack = fromTrack;
    break;
  case '-':
    (*trackList)++;
    toTrack = ReadTrackNumber(trackList, max);
    if (toTrack < 0) {
      return -2;
    }
    break;
  default:
    return -1;
  }

  *from = fromTrack;
  *to   = toTrack;

  return 1;
}

int main(int na, char **a)
{
  int i, j;
  int diskHW;
  disk68_t *d = 0;
  music68_t *m = 0;
  int curTrack;
  char *trackList;
  int loopArg = 0;
  int toTrack;
  FILE *out = stdout;

  SC68set_alloc(malloc);
  SC68set_free(free);

  if (na < 2) {
    error("Missing argument: try --help\n");
    return 1;
  }

  /* Scan parameters for --help, --version or --build option */
  for (i=1; i<na; ++i) {
    if (!strcmp(a[i],"--help")) {
      return Message();
    } else if (!strcmp(a[i],"--version") || !strcmp(a[i],"--build")) {
      printf("info68 [build %s]\n", BUILD_DATE);
      return 1;
    }
  }

  /* Search if specific output is request */
  i = 1;
  if (!strcmp(a[i], "-o") || !strcmp(a[i], "--output")) {
    if (na < 4) {
      error("Not enought parameter to specify an output file.\n");
      return 5;
    }
    if (strcmp(a[++i],"-")) {
      out = fopen(a[i],"wt");
      if (!out) {
        error("Can't create output file \"%s\" : %s\n", a[i], strerror(errno));
        return 6;
      }
      ++i;
    }
  }

  /* Load input file */
  d = SC68file_load_file(a[i]);
  if (!d) {
    return file_error(a[i]);
  }

  /* Setup variable */
  toTrack= curTrack = d->default_six;
  diskHW = 0;
  for (j=0; j<d->nb_six; ++j) {
    diskHW |= *(int*)&d->mus[j].flags;
  }
  trackList = 0;

  for (++i; i<=na; ++i) {

    if (i == na || isdigit(a[i][1])) {
      int res;
      /* Entering track-list */

      if (trackList) {
        /* Already in trackList, finish this one. */
        if (curTrack < toTrack) {
          curTrack++;
          res = 1;
        } else {
          res = ReadTrackList(&trackList, d->nb_six, &curTrack , &toTrack);
        }
        if (res < 0) {
          return tracklist_error(trackList);
        } else if (res > 0) {
          /* Must loop */
          i = loopArg;
          continue;
        }
      }
      if (i==na) {
        /* All is over capt'n */
        break;
      }

      /* Previous list is over, start new one */
      trackList = a[i]+1;
      loopArg = i; /* $$$ or i+1 */

      /* get From track parm */
      res = ReadTrackList(&trackList, d->nb_six, &curTrack , &toTrack);
      if (res < 0) {
        return tracklist_error(trackList);
      } else if (!res) {
        /* This can't be becoz we check that almost one digit was there above */
        error("%s(%d) : Internal bug error;"
	      " program should not reach this point\n", __FILE__, __LINE__);
        return 0x666;
      }
      /*curTrack = toTrack;*/
      continue;
    } else if (a[i][0] != '-') {
      Puts(out,a[i]);
      continue;
    }


    m = d->mus + curTrack;
    switch (a[i][1]) {
    case '-':
      /* Escape sequence '--' */
      Puts(out,a[i]+1);
      break;

    case 'L':
      Puts(out,"\n");
      break;

    /* DISK commands */
    case '#':
      PutI(out,d->nb_six);
      break;
    case '?':
      PutI(out,d->default_six+1);
      break;
    case 'N':
      Puts(out,d->name);
      break;
    case 'A':
      Puts(out,d->mus[d->default_six].author);
      break;
    case 'C':
      Puts(out,d->mus[d->default_six].composer);
      break;
    case 'T':
      PutI(out,d->time_ms/1000u);
      break;
    case 'Y':
      Puts(out,SC68time_str(0, d->nb_six, d->time_ms/1000u));
      break;
    case 'H':
      Puts(out,HWflags(diskHW));
      break;

    case '%':
      PutI(out,curTrack+1);
      break;
    case 'n':
      Puts(out,m->name);
      break;
    case 'a':
      Puts(out,m->author);
      break;
    case 'c':
      Puts(out,m->composer);
      break;
    case 'r':
      Puts(out,m->replay ? m->replay : "internal");
      break;
    case 't':
      PutI(out,m->time_ms/1000u);
      break;
    case 'y':
      Puts(out,SC68time_str(0, curTrack+1, m->time_ms/1000u));
      break;
    case 'h':
      Puts(out,HWflags(*(int*)&m->flags));
      break;
    case 'f':
      PutI(out,m->frq);
      break;
    case '@':
      PutX(out,m->a0);
      break;

    default:
      /* Not fatal, command is skipped */
      command_error(a[i]);
    }
  }

  if (out && out != stdout) {
    fclose(out);
  }

  SC68free(d);
  return 0;
}
