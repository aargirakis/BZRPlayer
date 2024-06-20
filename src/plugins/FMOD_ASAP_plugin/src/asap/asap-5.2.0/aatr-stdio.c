/*
 * aatr-stdio.c - another ATR file extractor
 *
 * Copyright (C) 2012-2019  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include "aatr-stdio.h"
#include "aatr.c"

typedef struct {
	AATR base;
	FILE *fp;
} AATRStdio;

static bool AATRStdio_Read(const AATR *self, int offset, uint8_t *buffer, int length)
{
	FILE *fp = ((AATRStdio *) self)->fp;
	return fseek(fp, offset, SEEK_SET) == 0
		&& fread(buffer, length, 1, fp) == 1;
}

AATR *AATRStdio_New(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return NULL;
	AATRStdio *self = (AATRStdio *) malloc(sizeof(AATRStdio));
	if (self == NULL) {
		fclose(fp);
		return NULL;
	}
	static const AATRVtbl vtbl = { AATRStdio_Read };
	self->base.vtbl = &vtbl;
	self->fp = fp;
	if (!AATR_Open(&self->base)) {
		free(self);
		fclose(fp);
		return NULL;
	}
	return &self->base;
}

void AATRStdio_Delete(AATR *self)
{
	FILE *fp = ((AATRStdio *) self)->fp;
	free(self);
	fclose(fp);
}

#if 0
int main(int argc, char **argv)
{
	AATR *disk = AATRStdio_New("C:\\0\\a8\\SV2K12_STUFF_AtariDOS.atr");
	AATRRecursiveLister *lister = AATRRecursiveLister_New();
	AATRFileStream *stream = AATRFileStream_New();
	if (disk == NULL || lister == NULL || stream == NULL)
		return 1;
	AATRRecursiveLister_Open(lister, disk);
	for (;;) {
		const char *current_filename = AATRRecursiveLister_NextFile(lister);
		if (current_filename == NULL)
			break;
		AATRFileStream_Open(stream, AATRRecursiveLister_GetDirectory(lister));
		printf("%s (%d)\n", current_filename, AATRFileStream_GetLength(stream));
	}
	return 0;
}
#endif
