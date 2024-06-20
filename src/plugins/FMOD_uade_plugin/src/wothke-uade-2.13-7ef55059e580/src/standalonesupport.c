/* Standalone/Emscripten support tools for uadecore.

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


#ifdef EMSCRIPTEN
extern int uade_request_file(const char *filename); // must be implemented on JavaScript side (also see callback.js) 
extern long uade_request_file_size(const char *filename);
#endif

struct AFILE open_stat;	// since all loading is sequential we can use this shared struct

char virt_fs_path[512];

struct AFILE * uade_fopen(const char *filename, const char *mode) {
// every file access comes through this API... return NULL to signal "file not ready" error .. 
#ifdef EMSCRIPTEN
	FILE *f; 
	int status= uade_request_file(filename);
//	fprintf(stderr, "uade_request_file response for %s: %d\n", filename, status);

	if (status < 0) {	// file not ready..
		f= 0;
//		fprintf(stderr, "file not ready: [%s]\n", filename);	
	} else if (status > 0) {
//		fprintf(stderr, "error: file does not exist: /%s\n", filename);	
		f= 0;
	} else {	
		snprintf(virt_fs_path, 512, "/%s", filename);	
		f =fopen(virt_fs_path, mode);	// in Emscripten this will use the virtual "in-memory FS"
//		fprintf(stderr, "loading of: [%s] %s\n", virt_fs_path, f?"succeeded":"failed (must NEVER happen!)");	
	}
	open_stat.async_status= status;
	open_stat.file= f;
#else
	open_stat.async_status= 0;
	open_stat.file= fopen(filename, mode);
#endif
	return &open_stat;
}

long uade_get_file_size(const char *filename) {
	// hack: this is actually only called after the file has been previously opened
	// i.e. in our JavaScript version we already cache the complete file data at this
	// point and error handling is not needed..
#ifndef EMSCRIPTEN
	struct stat s;
	FILE *f; 
	f= uade_fopen(filename, "r")->file;
	fstat(fileno(f), &s);
	fclose(f);
	return s.st_size;
#else
	return uade_request_file_size(filename);
#endif
	
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
    fprintf(stderr, "uade: error: amiga tried to open a very long filename\nplease REPORT THIS!\n");
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
      fprintf(stderr, "uade: open_amiga_file: unknown amiga volume (%s)\n", aname);
      return NULL;
    }	
    /* fprintf(stderr, "uade: opening from dir %s\n", dirname); */
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
  

//fprintf(stderr, "amiga out: opening [%s]\n", dirname);

  struct AFILE *uo= uade_fopen(dirname, "r");
  
  if (uo->file == 0) {
	// when this happens it might be a misconfiguration - in the JavaScript context
	// an async load may habe been triggered but we cannot block and wait for the result here.
	// because this occurs in the middle of the Amiga emulation we will not be able to
	// to resume processing at the correct place once we get the file.. rather we'll terminate
	// the emulation now and the JavaScript part may restart from scratch once it gets 
	// the needed file (because all the loading will take place in the "initial" phase of the
	// playback, this repeated trial&error approach should not be a problem)
  
	if (uo->async_status < 0) {
		last_file_not_ready=1;
	}
	quit_program = 1;
    uade_reboot = 1;
	
  }
  return uo;
}


void uade_portable_initializations(void)
{
}