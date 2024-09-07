#ifndef FONT_H
#define FONT_H

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


#include "tiledescriptor.h"
#include "../util/bundle.h"
#include "gfx.h"

#include <stdio.h>

typedef struct
{
	char *charmap;
	TileDescriptor *tiledescriptor;
	const TileDescriptor *ordered_tiles[256];
	GfxSurface * surface;
	int w, h;
	int char_spacing, space_width;
} Font;

int font_load(GfxDomain *domain, Font *font, Bundle *b, char *name);
int font_load_file(GfxDomain *domain, Font *font, char *filename);
int font_load_RW(GfxDomain *domain, Font *font, SDL_RWops *rw);
void font_create(Font *font, GfxSurface *tiles, const int w, const int h, const int char_spacing, const int space_width, char *charmap);
void font_destroy(Font *font);
void font_set_color(Font *font, Uint32 rgb);
void font_write_cursor(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 *cursor, SDL_Rect *bounds, const char * text);
void font_write_va(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 * cursor, SDL_Rect *bounds, const char * text, va_list va);
void font_write_cursor_args(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 *cursor, SDL_Rect *bounds, const char * text, ...) __attribute__ ((format (printf, 6, 7)));
void font_write(const Font *font, GfxDomain *dest, const SDL_Rect *r, const char * text);
void font_write_args(const Font *font, GfxDomain *dest, const SDL_Rect *r, const char * text, ...) __attribute__ ((format (printf, 4, 5)));
int font_text_width(const Font *font, const char *text);

#endif
