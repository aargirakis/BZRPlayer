/* decrunch.c
 *
 * based on load.c from:
 * Extended Module Player
 *
 * Copyright (C) 1996-1999 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * CHANGES: (modified for uade by mld)
 * removed all xmp related code)
 * added "custom" labels of pp20 files
 * added support for external unrar decruncher
 * added support for the external XPK Lib for Unix (the xType usage *g*)
 *
 * TODO:
 * real builtin support for XPK lib for Unix
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>

#include "decrunch.h"
#include "ppdepack.h"
#include "unsqsh.h"
#include "mmcmp.h"
#include "s404_dec.h"
#include "compat.h"

/*
size_t atomic_fread(void *dst, FILE *f, size_t count)
{
    uint8_t *p = (uint8_t *) dst;
    size_t left = count;

    while (left > 0) {

	ssize_t nread = fread(p, 1, left, f);

	if (nread <= 0) {
	    fprintf(stderr, "atomic_fread() failed: %s\n", strerror(errno));
	    return 0;
	}

	left -= nread;
	p += nread;
    }

    return count;
}


size_t atomic_fwrite(FILE *f, void *src, size_t count)
{
    uint8_t *p = (uint8_t *) src;
    size_t left = count;

    while (left > 0) {
	ssize_t nwrite = fwrite(p, 1, left, f);
	if (nwrite <= 0) {
	    fprintf(stderr, "atomic_fwrite() failed: %s\n", strerror(errno));
	    return 0;
	}
	left -= nwrite;
	p += nwrite;
    }

    return count;
}

static int read_packed_data_to_memory(uint8_t **buf, size_t *nbytes,
				      uint8_t *header, FILE *in)
{
    if (in == stdin) {
	size_t bsize = 4096;

	*buf = malloc(bsize);
	if (*buf == NULL)
	    return -1;

        // header[] contains nbytes of data initially
	memcpy(*buf, header, *nbytes);

	while (1) {
	    size_t n;

	    if (*nbytes == bsize) {
		uint8_t *newbuf;

		bsize *= 2;

		newbuf = realloc(*buf, bsize);
		if (newbuf == NULL)
		    return -1;

		*buf = newbuf;
	    }

	    n = fread(&(*buf)[*nbytes], 1, bsize - *nbytes, in);

	    if (n <= 0)
		break;

	    *nbytes += n;
	}

    } else {

	fseek(in, 0, SEEK_END);
	*nbytes = ftell(in);
	fseek(in, 0, SEEK_SET);

	*buf = malloc(*nbytes);
	if (*buf == NULL)
	    return -1;

	if (atomic_fread(*buf, in, *nbytes) == 0)
	    return -1;
    }

    return 0;
}
*/


static struct decruncher *check_header(uint8_t b[16])
{
    struct decruncher *decruncher = NULL;

    if ((b[0] == 'P' && b[1] == 'X' && b[2] == '2' && b[3] == '0') ||
	(b[0] == 'P' && b[1] == 'P' && b[2] == '2' && b[3] == '0')) {
	decruncher = &decruncher_pp;

    } else if (b[0] == 'X' && b[1] == 'P' && b[2] == 'K' && b[3] == 'F' &&
	       b[8] == 'S' && b[9] == 'Q' && b[10] == 'S' && b[11] == 'H') {
	decruncher = &decruncher_sqsh;

    } else if (b[0] == 'z' && b[1] == 'i' && b[2] == 'R' && b[3] == 'C' &&
	       b[4] == 'O' && b[5] == 'N' && b[6] == 'i' && b[7] == 'a') {
	decruncher = &decruncher_mmcmp;

    } else if (b[0] == 'S' && b[1] == '4' && b[2] == '0' && b[3] == '4' &&
	       b[4] < 0x80 && b[8] < 0x80 && b[12] < 0x80) {
	decruncher = &decruncher_s404;
    }

    return decruncher;
}


int decrunch(uint8_t *in, int inlen, uint8_t **unpacked, size_t *unpackedlen)
{
    uint8_t b[16];

    if (inlen < sizeof b)
		return -1;

    struct decruncher *decruncher;
	memcpy(&b[0], in, sizeof b);

    decruncher = check_header(b);
    if (decruncher == NULL) {
	//	fprintf(stderr, "failed to decrunch due to header check\n");
		*unpacked = 0;
		*unpackedlen = 0;
		return -1;
	}

	int  res = decruncher->decrunch(in, inlen, unpacked, unpackedlen);

    if (res < 0) {
		// error
		fprintf(stderr, "failed to decrunch using %s\n", decruncher->name);
	} else {
	//	fprintf(stderr, "successfully decrunched using %s\n", decruncher->name);
	}

	return res;
}

