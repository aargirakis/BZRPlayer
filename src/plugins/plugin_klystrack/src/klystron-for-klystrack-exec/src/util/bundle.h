#ifndef BUNDLE_H
#define BUNDLE_H

/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/


#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>
#include "SDL_rwops.h"

#define BND_SIG "bnd!"
#define BND_MAX_NAME_SIZE 256
#define bnd_cycle_key(key) (key = ((key << 2) ^ (key * 13)) | 1)

typedef struct
{
	char * name;
	Uint32 offset, size;
} BundleFile;

typedef struct
{
	Uint32 flags;
	SDL_RWops *handle;
	BundleFile *file;
	Uint32 n_files;
	bool close_handle;
} Bundle;

#define BND_FLAG_CRYPTED 1

int bnd_open(Bundle *bundle, const char * filename);
int bnd_open_RW(Bundle *bundle, SDL_RWops * rw);
int bnd_open_file(Bundle *bundle, FILE *f, const char * filename);
void bnd_free(Bundle *bundle);
int bnd_exists(const Bundle *bundle, const char *filename);
SDL_RWops *SDL_RWFromBundle(Bundle *bundle, const char *filename);

#endif
