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


#include "background.h"
#include "gfx.h"
#include <stdlib.h>
#include <string.h>

int bg_check_collision(const Background *bg, const ObjHdr *object, ObjHdr *collided_object)
{
	int right = (object->x + object->w) / CELLSIZE, bottom = (object->y + object->h) / CELLSIZE;
	int top = (object->y) / CELLSIZE-1, left = (object->x) / CELLSIZE-1;
	
	ObjHdr _hdr;
	ObjHdr *hdr = (collided_object != NULL) ? collided_object : &_hdr;
	
	hdr->w = CELLSIZE;
	hdr->h = CELLSIZE;
	hdr->y = top * CELLSIZE;
	 
	for (int y = top ; y <= bottom ; ++y)
	{
		hdr->x = left * CELLSIZE;
		
		int _y = y;
		
		if (bg->flags & BG_REPEAT_Y) _y = ((y % bg->h)+bg->h) % bg->h;
		else if (y >= bg->h) break;
		
		//printf("%d %d\n", hdr.x, object->x);
		
		
		if (_y >= 0) 
		{
			for (int x = left ; x <= right ; ++x)
			{
				int _x = x;
				
				if (bg->flags & BG_REPEAT_X) _x = ((x % bg->w)+bg->w) % bg->w;
				else if (x >= bg->w) break;
				
				if (_x >= 0)
				{
					if ( bg->data[_x+_y*bg->w].tile && !(bg->tiles[bg->data[_x+_y*bg->w].tile-1].flags & TILE_COL_DISABLE)) 
					{
						hdr->objflags = bg->tiles[bg->data[_x+_y*bg->w].tile-1].flags;
						hdr->surface = bg->tiles[bg->data[_x+_y*bg->w].tile-1].surface;
						hdr->current_frame = bg->tiles[bg->data[_x+_y*bg->w].tile-1].rect.x / CELLSIZE;
						hdr->_yofs = bg->tiles[bg->data[_x+_y*bg->w].tile-1].rect.y;
						
						if (objhdr_check_collision(object, hdr))
						{
							return 1;
						}
					}
				}
				
				hdr->x += CELLSIZE;
			}
		}
		
		hdr->y += CELLSIZE;
	}

	return 0;
}

void bg_draw(GfxDomain *surface, const SDL_Rect * dest, const Background *bg, int xofs, int yofs)
{
	int sx = 0;
	int sy = 0;
	
	const SDL_Rect def = {0, 0, surface->screen_w, surface->screen_h};
	
	if (dest == NULL)
		dest = &def;
		
	for (int y = (yofs) / CELLSIZE - 1; sy <= dest->h + 2 * CELLSIZE ; ++y, sy += CELLSIZE)
	{
		sx = 0;
				
		int _y = y;
		
		if (bg->flags & BG_REPEAT_Y) _y = ((y % bg->h)+bg->h) % bg->h;
			else if (y < 0) continue;
			else if (y >= bg->h) break;
				
		for (int x = (xofs) / CELLSIZE - 1 ; sx <= dest->w + 2 * CELLSIZE ; ++x, sx += CELLSIZE)
		{
			int _x = x;
			
			if (bg->flags & BG_REPEAT_X) _x = ((x % bg->w)+bg->w) % bg->w;
				else if (x < 0) continue;
				else if (x >= bg->w) break;
		
			BgCell * cell = &bg->data[_y*bg->w + _x];
			
			if ( cell->tile ) 
			{
				SDL_Rect rect = { sx - (xofs % (CELLSIZE)) + dest->x - CELLSIZE, sy - (yofs % (CELLSIZE)) + dest->y - CELLSIZE, CELLSIZE, CELLSIZE };
				my_BlitSurface(bg->tiles[cell->tile-1].surface, &bg->tiles[cell->tile-1].rect, surface, &rect);
				// SDL_FillRect(surface, &rect, 0x808080);
			}
		
		}
	}

}


const ObjHdr * bg_check_collision_chained(const Background *bg, const ObjHdr *head, ObjHdr *collided_tile)
{
	const ObjHdr *ptr;
	objhdr_walk(ptr, head, if (bg_check_collision(bg, ptr, collided_tile)) return ptr);
	return NULL;
}


int bg_create_tile_objhdr(ObjHdr* object_array, const Background *bg, int x, int y, int w, int h, int zero_src, TileDescriptor *tiles)
{
	int items = 0;
	ObjHdr *obj = object_array;
	
	if (!tiles)
		tiles = bg->tiles;
	
	for (int _y = y ; _y < h + y ; ++_y)
	{
		for (int _x = x ; _x < w + x ; ++_x)
		{
			if (bg->data[_x + _y * bg->w].tile)
			{
				if (obj != NULL)
				{
					obj->objflags = tiles[bg->data[_x + _y * bg->w].tile-1].flags & OBJ_COL_FLAGS;
					obj->x = (_x - x) * CELLSIZE;
					obj->y = (_y - y) * CELLSIZE;
					obj->w = obj->h = CELLSIZE;
					obj->anim = NULL;
					obj->current_frame = tiles[bg->data[_x + _y * bg->w].tile-1].rect.x / CELLSIZE;
					obj->_yofs = tiles[bg->data[_x + _y * bg->w].tile-1].rect.y;
					obj->surface = tiles[bg->data[_x + _y * bg->w].tile-1].surface;
					obj = obj->next;
					if (zero_src) bg->data[_x + _y * bg->w].tile = 0;
				}
				++items;
			}
		}
	}
	
	if (items > 0) object_array[items-1].next = NULL;
	
	return items;
}


void bg_create(Background *bg, int w, int h)
{
	memset(bg, 0, sizeof(*bg));
	bg->data = calloc(w * h, sizeof(bg->data[0]));
	bg->w = w;
	bg->h = h;
}


void bg_destroy(Background *bg)
{
	free(bg->data);
	memset(bg, 0, sizeof(*bg));
}
