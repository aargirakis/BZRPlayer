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

#include "objhdr.h"
#include <stdio.h>
#include <assert.h>
#include "gfx.h"

static inline int surf_alpha_both(const int *a, const int *b, int ax, int ay, int bx, int by, int w, int h, int stride_a, int stride_b)
{
	for (int y = 0 ; y < h ; ++y)
	{
		const int *pa = a + (ay + y) * stride_a + ax;
		const int *pb = b + (by + y) * stride_b + bx;
		
		for (int x = 0 ; x < w ; ++x)
		{
			if (*pa && *pb)
			{
				return 1;
			}
			++pa;
			++pb;
		}
	}
	
	return 0;
}


static inline int surf_alpha_a(const int *a, int ax, int ay, int w, int h, int stride)
{
	for (int y = ay ; y < h+ay ; ++y)
	{
		const int *pa = a + y * stride + ax;
		
		for (int x = 0 ; x < w ; ++x)
		{
			if (*pa)
			{
				return 1;
			}
			++pa;
		}
	}
	
	return 0;
}


static int objhdr_check_collision_internal(const ObjHdr *a, const ObjHdr *b, int axoff, int ayoff, int bxoff, int byoff)
{
	if ((a->objflags & OBJ_COL_DISABLE) || (b->objflags & OBJ_COL_DISABLE)) return 0;

	int a_left = a->x - axoff, a_right = a_left + a->w - 1; 
	int a_top = a->y - ayoff, a_bottom = a_top + a->h - 1; 
	int b_left = b->x - bxoff, b_right = b_left + b->w - 1; 
	int b_top = b->y - byoff, b_bottom = b_top + b->h - 1; 
	
	if (a->objflags & OBJ_COL_RECT)
	{
		a_left =  a->x + a->colrect.x - axoff;
		a_right = a_left + a->colrect.w - 1; 
		a_top = a->y + a->colrect.y - ayoff;
		a_bottom = a_top + a->colrect.h - 1; 
	}
	
	if (b->objflags & OBJ_COL_RECT)
	{
		b_left = b->x + b->colrect.x - bxoff;
		b_right = b_left + b->colrect.w - 1; 
		b_top = b->y + b->colrect.y - ayoff;
		b_bottom = b_top + b->colrect.h - 1; 
	}
	
	if(b_right < a_left)	return 0;	//just checking if their
	if(b_left > a_right)	return 0;	//bounding boxes even touch

	if(b_bottom < a_top)	return 0;
	if(b_top > a_bottom)	return 0;
	
	if (!(a->objflags & OBJ_COL_PIXEL) && !(b->objflags & OBJ_COL_PIXEL))
		return 1;
	
	int a_xofs, a_yofs;
	int b_xofs, b_yofs;
	
	if (a_left < b_left) 
	{
		b_xofs = 0;
		a_xofs = b_left-a_left;
	}
	else
	{
		b_xofs = a_left-b_left;
		a_xofs = 0;
	}
	
	if (a_top < b_top) 
	{
		b_yofs = 0;
		a_yofs = b_top-a_top;
	}
	else
	{
		b_yofs = a_top-b_top;
		a_yofs = 0;
	}
	
	int h = my_min(a_bottom - a_top - a_yofs+1, b_bottom - b_top - b_yofs+1),
		w = my_min(a_right - a_left - a_xofs+1, b_right - b_left - b_xofs+1);
		
	if (a->objflags & OBJ_COL_PIXEL && b->objflags & OBJ_COL_PIXEL)
		return surf_alpha_both(a->surface->mask, b->surface->mask, a_xofs + a->current_frame*a->w, a_yofs + a->_yofs, b_xofs + b->current_frame*b->w, b_yofs + b->_yofs, w, h, a->surface->surface->w, b->surface->surface->w);
	else if (a->objflags & OBJ_COL_PIXEL && !(b->objflags & OBJ_COL_PIXEL))
		return surf_alpha_a(a->surface->mask, a_xofs + a->current_frame*a->w, a_yofs + a->_yofs, w, h, a->surface->surface->w);
	else if (b->objflags & OBJ_COL_PIXEL && !(a->objflags & OBJ_COL_PIXEL))
		return surf_alpha_a(b->surface->mask, b_xofs + b->current_frame*b->w, b_yofs + b->_yofs , w, h, b->surface->surface->w);
	else return 1;
}


int objhdr_check_collision(const ObjHdr *a, const ObjHdr *b)
{
	if (!(a->objflags & OBJ_RELATIVE_CHILDREN) && !(b->objflags & OBJ_RELATIVE_CHILDREN))
		return objhdr_check_collision_internal(a, b, 0, 0, 0, 0);
	else if ((a->objflags & OBJ_RELATIVE_CHILDREN) && !(b->objflags & OBJ_RELATIVE_CHILDREN))
		return objhdr_check_collision_chained(b, a) != NULL;
	else if (!(a->objflags & OBJ_RELATIVE_CHILDREN) && (b->objflags & OBJ_RELATIVE_CHILDREN))
		return objhdr_check_collision_chained(a, b) != NULL;
	else if ((a->objflags & OBJ_RELATIVE_CHILDREN) && (b->objflags & OBJ_RELATIVE_CHILDREN))
		return objhdr_check_collision_chained2(a, b) != NULL;
	else
		return 0;
}


const ObjHdr * objhdr_check_collision_chained(const ObjHdr *a, const ObjHdr *head)
{
	const ObjHdr *ptr;
	
	if (!(head->objflags & OBJ_RELATIVE_CHILDREN))
	{
		objhdr_walk(ptr, head, if (a != ptr && objhdr_check_collision_internal(a, ptr, 0, 0, 0, 0)) return ptr;);
	}
	else
	{
		if (objhdr_check_collision_internal(a, head, 0, 0, 0, 0)) return head;
		
		objhdr_walk(ptr, head->next, if (a != ptr && objhdr_check_collision_internal(a, ptr, 0, 0, -head->x, -head->y)) return ptr;);
	}
	
	return NULL;
}


const ObjHdr * objhdr_check_collision_chained2(const ObjHdr *head1, const ObjHdr *head2)
{
	const ObjHdr *ptr1, *ptr2;
	
	if (!head1 || !head2) return NULL;
	
	if (!(head1->objflags & OBJ_RELATIVE_CHILDREN) && !(head2->objflags & OBJ_RELATIVE_CHILDREN))
	{
		objhdr_walk(ptr1, head1, objhdr_walk(ptr2, head2, if (ptr1 != ptr2 && objhdr_check_collision_internal(ptr1, ptr2, 0, 0, 0, 0)) return ptr2;));
	}
	else if ((head1->objflags & OBJ_RELATIVE_CHILDREN) && !(head2->objflags & OBJ_RELATIVE_CHILDREN))
	{
		if (objhdr_check_collision_internal(head1, head2, 0, 0, 0, 0)) return head2;
		
		objhdr_walk(ptr1, head1->next, objhdr_walk(ptr2, head2, if (ptr1 != ptr2 && objhdr_check_collision_internal(ptr1, ptr2, -head1->x, -head1->y, 0, 0)) return ptr2;));
	}
	else if (!(head1->objflags & OBJ_RELATIVE_CHILDREN) && (head2->objflags & OBJ_RELATIVE_CHILDREN))
	{
		if (objhdr_check_collision_internal(head1, head2, 0, 0, 0, 0)) return head2;
		
		objhdr_walk(ptr1, head1, objhdr_walk(ptr2, head2->next, if (ptr1 != ptr2 && objhdr_check_collision_internal(ptr1, ptr2, 0, 0, -head2->x, -head2->y)) return ptr2;));
	}
	else if ((head1->objflags & OBJ_RELATIVE_CHILDREN) && (head2->objflags & OBJ_RELATIVE_CHILDREN))
	{
		if (objhdr_check_collision_internal(head1, head2, 0, 0, 0, 0)) return head2;
		
		objhdr_walk(ptr1, head1->next, objhdr_walk(ptr2, head2->next, if (ptr1 != ptr2 && objhdr_check_collision_internal(ptr1, ptr2, -head1->x, -head1->y, -head2->x, -head2->y)) return ptr2;));
	}
	
	return NULL;
}


void objhdr_draw(GfxDomain *destination, const ObjHdr *object, int xofs, int yofs)
{
	if (object->objflags & OBJ_RELATIVE_CHILDREN)
		objhdr_draw_chained(destination, object, xofs, yofs);
	else
	{

		SDL_Rect dest = {object->x - xofs, object->y - yofs, object->w, object->h};

#ifdef DEBUG	
		if (object->surface == NULL)
		{
			gfx_rect(destination, &dest, 0xffffffff);
		}
		else 
#endif	
		if (object->objflags & OBJ_DRAW_RECT)
		{
			gfx_rect(destination, &dest, *(Uint32*)&object->surface);
		}
		else
		{
			if (object->surface != NULL)
			{
				SDL_Rect src = { object->current_frame * object->w, object->_yofs, object->w, object->h };
				my_BlitSurface(object->surface , &src, destination, &dest);
			}
		}
		
#ifdef DEBUG
		if (object->objflags & OBJ_COL_RECT)
		{
			SDL_Rect dest = {object->x - xofs + object->colrect.x, object->y - yofs + object->colrect.y, object->colrect.w, object->colrect.h};
			
			gfx_rect(destination, &dest, 0xff0000);
		}
#endif
	}
}


void objhdr_draw_chained(GfxDomain *destination, const ObjHdr *head, int xofs, int yofs)
{
	const ObjHdr *ptr;
	
	if (!head) return;
	
	if (head->objflags & OBJ_RELATIVE_CHILDREN)
	{
		//objhdr_draw(destination, head, xofs, yofs);
		
		if (head->next)
		{
			xofs -= head->x;
			yofs -= head->y;
			objhdr_walk(ptr, head->next, objhdr_draw(destination, ptr, xofs, yofs));
		}
	}
	else
	{
		objhdr_walk(ptr, head, objhdr_draw(destination, ptr, xofs, yofs));
	}
}


void objhdr_set_animation(ObjHdr *obj, const AnimFrame *anim, int anim_speed_fine)
{
	obj->anim = anim;
	obj->anim_frame = 0;
	obj->anim_speed_fine = anim_speed_fine;
	if (anim)
	{
		obj->current_frame = obj->anim[0].frame;
		obj->frame_delay = obj->anim[0].delay * OBJHDR_ANIM_SPEED_NORMAL;
	}
}


void objhdr_advance_animation(ObjHdr *obj)
{
	if (obj->anim == NULL) return;
	
	obj->objflags &= ~(OBJ_ANIM_FINISHED|OBJ_ANIM_LOOPED|OBJ_ANIM_NEXT_FRAME);
	
	if (obj->frame_delay <= 0)
	{
		obj->objflags |= OBJ_ANIM_NEXT_FRAME;
		++obj->anim_frame;
		switch (obj->anim[obj->anim_frame].frame) 
		{
			case ANIM_JUMP:
			{
				obj->anim_frame = obj->anim[obj->anim_frame].delay;
				obj->objflags |= OBJ_ANIM_LOOPED;
			}
			break;
			
			case ANIM_END:
			{
				obj->anim_frame = 0;
				obj->objflags |= OBJ_ANIM_FINISHED;
			}
			break;
		}
		
		obj->current_frame = obj->anim[obj->anim_frame].frame;
		obj->frame_delay += obj->anim[obj->anim_frame].delay * OBJHDR_ANIM_SPEED_NORMAL;
	}
	
	obj->frame_delay -= obj->anim_speed_fine;
}


void objhdr_advance_animation_chained(ObjHdr *head)
{
	ObjHdr *ptr;
	objhdr_walk(ptr, head,	objhdr_advance_animation(ptr));
}
