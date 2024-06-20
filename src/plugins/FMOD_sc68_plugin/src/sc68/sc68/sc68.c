/*
 *     sc68 - atari st and amiga music emulator - command line player
 *      Copyright (C) 1998-2001 Ben(jamin) Gerard <ben@sasghipa.com>
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
 */


/* generated config include */
#include <config68.h>

/* Standard Includes */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* sc68 includes */
#include "api68/api68.h"

static api68_t * sc68 = 0;

/** Display version number. */
static int Version(void)
{
  puts(PACKAGE68 " version " VERSION68 "\n");
  return 0;
}

/** Display usage message. */
static int Usage(void)
{
  puts(
    "sc68 - Atari ST and Amiga music emulator\n"
    "Copyright (C) 1998-2003 Benjamin Gerard <ben@sasghipa.com>\n"
    "\n"
    "This program is free software.\n"
    "\n"
    "usage : sc68 [options] <URL>\n"
    "\n"
    "options:\n"
    "  --help             : Display this message and exit\n"
    "  --version          : Display sc68 version x.y.z and exit\n"
    "  --quiet            : Do not display music info\n"
    "  --track=#[,#]      : Choose track to play [0=all tracks]\n"
    "                       and number of loop [0=infinite]\n"
    "  --sc68_data=path   : Set shared resource path (prior to ${SC68_DATA})\n"
    "  --sc68_user=path   : Set user resource path (prior to ${SC68_USER})\n"
    "  --sc68_music=path  : Set local music path (prior to ${SC68_USER})\n"
    "  --sc68_rmusic=path : Set remote music path (prior to ${SC68_USER})\n"
    "\n"
    "URL:\n"
    "  - or \n"
    "  stdin://           : Read file fron standard input.\n"
    "  path or\n"
    "  file://path or \n"
    "  local://path       : Local sc68 music file.\n"
    "  http://path or \n"
    "  ftp://path or\n"
    "  ...                : Load nusic file via given protocol (see curl)\n"
    "  sc68://author/hw/title[/#track]\n"
    "  Access sc68 music database. The music file is searched first in local\n"
    "  music path and if not found in remote music path.\n"
    "\n"
    );
  return 1;
}

static int quiet = 0;

void Message(const char * fmt, ...)
{
  va_list list;

  if (quiet) {
    return;
  }
  va_start(list, fmt);
  vfprintf(stderr, fmt, list);
  va_end(list);
}

/** Display to output debug statcked error messages.
 */
static void spool_error_message(void)
{
  const char * s;

  if (s = api68_error(), s) {
    fprintf(stderr, "Stacked Error Message:\n");
    do {
      fprintf(stderr, "%s\n", s);
    } while (s = api68_error(), s != NULL);
  }
}

static void DisplayInfo(int track)
{
  api68_music_info_t info;
  if (!api68_music_info(sc68,&info,track,0)) {
    Message("Track      : %d/%d\n",info.track, info.tracks);
    Message("Title      : %s\n",info.title);
    Message("Author     : %s\n",info.author);
    Message("Composer   : %s\n",info.composer);
    Message("Replay     : %s\n",info.replay);
    Message("Hardware   : %s\n",info.hwname);
    Message("Start time : %u:%02u\n",
	    info.start_ms/60000u, (info.start_ms/1000u)%60u);
    Message("Duration   : %s\n", info.time);
  }
}

/* track:  0:all -1:default */
static int PlayLoop(FILE * out, int track, int loop)
{
  static char buffer[512 * 4];
  int all = 0;
  int code;

  if (track == -1) {
    track = 0;
  } else if (track == 0) {
    track = 1;
    all = 1;
  }

  api68_play(sc68, track, loop);

  code = api68_process(sc68, 0, 0);
  if (!(code & API68_END) && (code & API68_CHANGE)) {
    DisplayInfo(-1);
  }

  while ( ! (code & API68_END) ) {
    code = api68_process(sc68, buffer, sizeof(buffer) >> 2);
    if (code == API68_MIX_ERROR) {
      break;
    }
    if (code & API68_LOOP) {
      Message("Loop: #%d\n", api68_play(sc68, -1, 1));
    }

    if (code & API68_CHANGE) {
      if (!all) {
	break;
      }
      DisplayInfo(-1);
    }

    /* Send audio PCM to stdout. */
    /* $$$ ben: hack for MAcOS debugin */
/*     if (0) { */
/*       int i; */
/*       for (i=0; i<sizeof(buffer); ++i) { */
/* 	printf("%02X",buffer[i]); */
/*       } */
/*     } */
    if (1) {
      if (fwrite(buffer, sizeof(buffer), 1, out) != 1) {
	perror("sc68");
	return -1;
      }
    }
  }

  return -(code == API68_MIX_ERROR);
}

static int IsIntParam(const char *parm,
		      const char *what,
		      int * res, int * res2)
{
  int cnt = 0;
  if (strstr(parm, what) == parm) {
    int len = strlen(what);
    if (isdigit(parm[len])) {
      char *e;
      *res = strtol(parm+len, &e, 0);
      cnt = 1;
      if (res2 && *e==',' && isdigit(e[1])) {
	*res2 = strtol(e+1,&e, 0);
	cnt = 2;
      }
    }
  }
  return cnt;
}

int main(int na, char **a)
{
  int i;
  int help = 0;
  char * fname = 0;
  int track = -1;
  int loop = -1;
  int err = 1;
  api68_init_t init68;

  /* Scan help and info options */
  for (i=1; i<na; ++i) {
    if (!strcmp(a[i],"--")) {
      break;
    } else if (!strcmp(a[i],"--help")) {
      help = 1;
      break;
    } else if (!strcmp(a[i],"--version")) {
      return Version();
    } else if (!strcmp(a[i],"--quiet")) {
      quiet = 1;
    }
  }
  if (na<2 || help) {
    return Usage();
  }

  /* Initialize sc68 api. */
  memset(&init68, 0, sizeof(init68));
  init68.alloc = malloc;
  init68.free = free;
  init68.argc = na;
  init68.argv = a;
#ifdef _DEBUG
  init68.debug = (debugmsg68_t)vfprintf;
  init68.debug_cookie = stderr;
#endif
  sc68 = api68_init(&init68);
  if (!sc68) {
    goto error;
  }

  /* */
  na = init68.argc;
  for (i=1; i<na; ++i) {
    if (!strcmp(a[i],"--")) {
      break;
    } else if (!IsIntParam(a[i],"--track=", &track, &loop)) {
      if (fname) {
	fprintf(stderr, "Invalid parameters \"%s\"\n", a[i]);
	return 2;
      } else {
	fname = a[i];
      }
    }
  }
  if (!fname && i < na) {
    fname = a[i];
  }

  if (!fname) {
    fprintf (stderr, "Missing input file. try --help\n");
    goto error;
  }
  if (!strcmp(fname,"-")) {
    fname = "stdin://";
  }
  
  /* Verify sc68 file. */
#if 0  /* Mess with stdin  */
  if (api68_verify_file(fname) < 0) {
    goto error;
  }
#endif
  
  if (api68_load_file(sc68, fname)) {
    goto error;
  }
    
  /** @todo  Set out stream mode to binary. */

  /* Loop */
  if (PlayLoop(stdout, track, loop) < 0) {
    goto error;
  }

  err = 0;

 error:
  api68_shutdown(sc68);
  if (err) {
    spool_error_message();
    return -1;
  }

  return 0;
}
