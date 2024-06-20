/* Standalone/EMSCRIPTEN support tools for uadecore.

   Replaces the implementations from unixsupport.c

   Copyright 2014 (C) Juergen Wothke
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

#include "uae.h"
#include "uade.h"
#include "unixatomic.h"


extern int ems_request_file(const char *filename); // must be implemented on JavaScript side (also see callback.js)
extern long ems_request_file_size(const char *filename);
extern long ems_cache_file(const char *filename, uint8_t *buf, size_t len);

struct AFILE open_stat;	// since all loading is sequential we can use this shared struct

char virt_fs_path[512];

int uade_write_file(const char *filename, uint8_t *buf, size_t len) {
	// overwrite existing file
	FILE *f= fopen(filename, "w+");	// overwrite original file

	len = fwrite(buf, 1, len, f);
	fflush(f);
	fclose(f);

	// since player may look in the cache instead of looking for a
	// file, put it in the cache as well
	ems_cache_file(filename, buf, len);

	return len;
}


struct AFILE * uade_fopen(const char *filename, const char *mode) {
	// every file access comes through this API... return NULL to signal "file not ready" error ..
	FILE *f;

	int status= ems_request_file(filename);

	if (status < 0) {	// file not ready..
		f= 0;
	} else if (status > 0) {
		f= 0;
	} else {
		f = fopen(filename, mode);	// via Emscripten's virtual "in-memory FS"

		if (f == NULL) fprintf(stdout, "warning: file not found %s\n", filename);
	}
	open_stat.async_status= status;
	open_stat.file= f;
	return &open_stat;
}

long uade_get_file_size(const char *filename) {
	long size= ems_request_file_size(filename);	// just in case
	if (size<0) return size;
	
	struct stat s;
	FILE *f = uade_fopen(filename, "r")->file;
	fstat(fileno(f), &s);
	fclose(f);

	return s.st_size;
}

static int last_file_not_ready= 0; 	// additional status signalling if the last file could not be
									// "found" - which in the JavaScript environment will mean.. async load is still in progress

int is_amiga_file_not_ready(void) {
	return last_file_not_ready;
}

extern void emsCopyPath(char *dest, int maxsize, char*src);

/* opens file in amiga namespace; replaces impl from unixsupport.c (which isn't used!) */
struct AFILE * uade_open_amiga_file(char *aname, const char *playerdir)
{
  char *separator;
  char *ptr;
  char copy[PATH_MAX];
  char dirname[PATH_MAX];
  char remainder[PATH_MAX];
  char fake[PATH_MAX];
  char real[PATH_MAX];
  int len, len2;
  DIR *dir;
  FILE *file;

  last_file_not_ready= 0;

  if (strlcpy(copy, aname, sizeof(copy)) >= sizeof(copy)) {
    fprintf(stdout, "uade: error: amiga tried to open a very long filename\nplease REPORT THIS!\n");
    return NULL;
  }
  ptr = copy;

// example aname: "ENV:EaglePlayer/ahx"   -> which translates to "players/ENV/EaglePlayer/"

  if ((separator = strchr(ptr, (int) ':'))) {
    len = (int) (separator - ptr);
    memcpy(dirname, ptr, len);
    dirname[len] = 0;

	len2=strlen(ptr)-(len+1);
	memcpy(remainder, ptr+len+1, len2);
	remainder[len2] = 0;

	if (!strcasecmp(dirname, "ENV")) {
	  snprintf(dirname, sizeof(dirname), "%s/players/ENV/%s", playerdir, remainder);  	// we only have the EaglePlayer..
	} else if (!strcasecmp(dirname, "S")) {
      snprintf(dirname, sizeof(dirname), "%s/players/S/%s", playerdir, remainder);
    } else {
      fprintf(stdout, "uade: open_amiga_file: unknown amiga volume (%s)\n", aname);
      return NULL;
    }
    /* fprintf(stdout, "uade: opening from dir %s\n", dirname); */
  } else {
	if (!strncasecmp(ptr, "AYPlayers", 9)) {
		// HACK: special case of a player ("PlayAY") trying to load additional files (e.g. "AYPlayers/ZXAYEMUL")..
		// note: there is currently NO general solution for this kind of scenario (regular PC apps
		// would probably search the PATH but that option is not available here)
		snprintf(dirname, sizeof(dirname), "%s/players/%s", playerdir, ptr);	// redirect to players folder
	} else {
		emsCopyPath(dirname, sizeof(dirname), aname);
	//	snprintf(dirname, sizeof(dirname), aname);	// e.g. railroad tycoon.dl
	}
  }

  struct AFILE *uo= uade_fopen(dirname, "r");

  if (uo->file == 0) {
	// when this happens it might be a misconfiguration - in the JavaScript context
	// an async load may have been triggered but we cannot block and wait for the result here.
	// because this occurs in the middle of the Amiga emulation we will not be able to
	// to resume processing at the correct place once we get the file.. rather we'll terminate
	// the emulation now and the JavaScript part may restart from scratch once it gets
	// the needed file (because all the loading will take place in the "initial" phase of the
	// playback, this repeated trial&error approach should not be a problem)

	if (uo->async_status < 0) {
		last_file_not_ready=1;

		quit_program = 1;
		uade_reboot = 1;

	} else {
		// testcase: MusicMaker player actually expects to look for uncompressed
		// instrument file and it is OK/expected to get a 0 result.. it will then
		// try to find compressed files as a fallback
	}
  }
  return uo;
}


void uade_portable_initializations(void)
{
}


// --- below APIs are used in UADE - but probably not actually relevant

	// (RTC information is obviously useless in the "batch" processing
	// mode used here..)
	// todo: find testcase where this might be relevant
	
	
extern uint64_t getCurrentMicros();
	
//#define USE_OLD_TIMEHANDLER

#ifdef USE_OLD_TIMEHANDLER

#include <time.h>
#include <sys/time.h>

time_t ems_time(time_t *timer) {
	return time(timer);
}

int ems_gettimeofday( struct timeval *tv, struct tzp *tz) {
	return gettimeofday(tv, tz);
}
#else
time_t ems_time(time_t *timer) {
	uint64_t micros = getCurrentMicros();
	return micros / 1000000;	// in secs
}

int ems_gettimeofday( struct timeval *tv, struct tzp *tz) {
	uint64_t micros = getCurrentMicros();
	tv->tv_sec = micros / 1000000;
	tv->tv_usec = micros % 1000000;
	return 0;
}
#endif


