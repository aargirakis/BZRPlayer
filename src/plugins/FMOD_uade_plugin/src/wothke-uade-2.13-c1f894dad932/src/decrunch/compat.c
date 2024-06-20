#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "compat.h"

#ifdef NO_MKSTEMP

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int mkstemp(char *template)
{
    size_t l;
    int i;
    int fd;

    l = strlen(template);
    if (l < 6 || strcmp(&template[l - 6], "XXXXXX") != 0) {
	errno = EINVAL;
	return -1;
    }

    strcpy(&template[l - 6], "aaaaaa");

    while (1) {
	fd = open(template, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd >= 0) {
	    fprintf(stderr, "Using mkstemp() replacement: %s\n", template);
	    return fd;
	}

	/* Hahaa, direct brute force method! You can slow this down by
	   creating millions of files as this pattern is 100% predictable
	   for others. */
	for (i = 0; i < 6; i++) {
	    if (template[l - 6 + i] == 'z') {
		template[l - 6 + i] = 'a';
	    } else {
		template[l - 6 + i] += 1;
		break;
	    }
	}

	/* Theoretical check, will never happen */
	if (strcmp(&template[l - 6], "zzzzzz") == 0) {
	    errno = EEXIST;
	    return -1;
	}
    }

    return -1;
}
#endif
