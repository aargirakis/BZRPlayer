#ifndef OBJHDR_H
#define OBJHDR_H

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
#include "sllhdr.h"
#include "gfxsurf.h"
#include "macros.h"

enum // OBJFLAGS
{
	OBJ_COL_NORMAL = 0,
	OBJ_COL_PIXEL = 1,
	OBJ_COL_RECT = 2,
	OBJ_DRAW_RECT = 4,
	OBJ_ANIM_FINISHED = 8,
	OBJ_RELATIVE_CHILDREN = 16,
	OBJ_COL_DISABLE = 32,
	OBJ_ANIM_LOOPED = 64,
	OBJ_ANIM_NEXT_FRAME = 128
};

#define OBJ_COL_FLAGS (OBJ_COL_NORMAL|OBJ_COL_PIXEL|OBJ_COL_RECT|OBJ_COL_DISABLE)

enum // ANIM
{
	ANIM_JUMP = -1,
	ANIM_END = -2
};

typedef struct
{
	int frame;
	int delay;
} AnimFrame;

#define OBJHDR_ANIM_SPEED_NORMAL 256

#define OBJHDR \
	SLLHDR;\
	Uint32 objid; \
	Uint32 objflags;\
	Sint32 x, y;\
	Sint32 w, h;\
	Sint32 _yofs;\
	SDL_Rect colrect;\
	GfxSurface *surface;\
	const AnimFrame *anim;\
	Sint32 current_frame, anim_frame, frame_delay, anim_speed_fine;\
	

typedef struct
{
	OBJHDR;
} ObjHdr;

struct GfxDomain_t;

// rect collision
int objhdr_check_collision(const ObjHdr *a, const ObjHdr *b);
const ObjHdr * objhdr_check_collision_chained(const ObjHdr *a, const ObjHdr *head);
const ObjHdr * objhdr_check_collision_chained2(const ObjHdr *head1, const ObjHdr *head2);
void objhdr_draw(struct GfxDomain_t *destination, const ObjHdr *object, int xofs, int yofs);
void objhdr_draw_chained(struct GfxDomain_t *destination, const ObjHdr *head, int xofs, int yofs);
void objhdr_set_animation(ObjHdr *obj, const AnimFrame *anim, int anim_speed_fine);
void objhdr_advance_animation(ObjHdr *obj);
void objhdr_advance_animation_chained(ObjHdr *head);

#define objhdr_walk(ptr,head,block) sllhdr_walk(ptr,head,block)
#define objhdr_insert_after(queue, obj) sllhdr_insert_after(queue, obj)
#define objhdr_insert_before(queue, obj) sllhdr_insert_before(queue, obj)
#define objhdr_update_list(ptr, unused_head, in_use_head, foreach, kill_if, on_death) sllhdr_update_list(ObjHdr, ptr, unused_head, in_use_head, foreach, kill_if, on_death)
	
#endif
