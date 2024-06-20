#ifndef BACKGROUND_H
#define BACKGROUND_H

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


#include "bgcell.h"
#include "objhdr.h"
#include "tiledescriptor.h"
#include "SDL.h"

enum
{
	BG_REPEAT_X = 1,
	BG_REPEAT_Y = 2,
	BG_PARALLAX = 4
};

typedef struct
{
	int flags;
	int w, h;
	int prx_mlt_x, prx_mlt_y;
	int off_x, off_y;
	TileDescriptor *tiles;
	BgCell *data;
} Background;

struct GfxDomain_t;

int bg_check_collision(const Background *bg, const ObjHdr *object, ObjHdr *collided_tile);
const ObjHdr * bg_check_collision_chained(const Background *bg, const ObjHdr *head, ObjHdr *collided_tile);
void bg_draw(struct GfxDomain_t *surface, const SDL_Rect * dest, const Background *bg, int xofs, int yofs);
int bg_create_tile_objhdr(ObjHdr* object_array, const Background *bg, int x, int y, int w, int h, int zero_src, TileDescriptor *tiles);
void bg_create(Background *bg, int w, int h);
void bg_destroy(Background *bg);

#endif
