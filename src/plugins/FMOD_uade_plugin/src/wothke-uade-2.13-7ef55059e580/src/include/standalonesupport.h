#ifndef _UADE_STANDALONESUPPORT_H_
#define _UADE_STANDALONESUPPORT_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

// wrapper for FILE used to deal with additonal status needed for async loading..
struct AFILE {
  FILE* file;
  int async_status;
};


#define die(fmt, args...) do { fprintf(stderr, "uade: " fmt, ## args); exit(1); } while(0)

#define dieerror(fmt, args...) do { fprintf(stderr, "uade: " fmt ": %s\n", ## args, strerror(errno)); exit(1); } while(0)


struct AFILE *uade_fopen(const char *filename, const char *mode);
struct AFILE *uade_open_amiga_file(char *aname, const char *playerdir);
int is_amiga_file_not_ready(void);
void uade_portable_initializations(void);
long uade_get_file_size(const char *filename);
#endif
