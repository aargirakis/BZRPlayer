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

#include "bevdefs.h"
#include "bevel.h"
#include "view.h"

#define BORDER 4
#define SIZE_MINUS_BORDER (BEV_SIZE - BORDER)

void bevelex(GfxDomain *screen, const SDL_Rect *area, GfxSurface *gfx, int offset, int bevel_flags)
{
	/* Center */
	if (!(bevel_flags & BEV_F_DISABLE_CENTER))
	{
		if (!(bevel_flags & BEV_F_STRETCH_CENTER))
		{
			for (int y = BORDER ; y < area->h - BORDER ; y += BEV_SIZE / 2)
			{
				for (int x = BORDER ; x < area->w - BORDER ; x += BEV_SIZE / 2)
				{
					SDL_Rect src = { BORDER + offset * BEV_SIZE, BORDER, my_min(BEV_SIZE / 2, area->w - x - BORDER), my_min(BEV_SIZE / 2, area->h - y - BORDER) };
					SDL_Rect dest = { x + area->x, y + area->y, my_min(BEV_SIZE / 2, area->w - x - BORDER), my_min(BEV_SIZE / 2, area->h - y - BORDER) };
					my_BlitSurface(gfx, &src, screen, &dest);
				}
			}
		}
		else
		{
			SDL_Rect src = { BORDER + offset * BEV_SIZE, BORDER, BEV_SIZE - 2 * BORDER, BEV_SIZE - 2 * BORDER };
			SDL_Rect dest = { area->x + BORDER, area->y + BORDER, area->w - 2 * BORDER, area->h - 2 * BORDER };
			my_BlitSurface(gfx, &src, screen, &dest);
		}
	}
	
	/* Sides */
	if (!(bevel_flags & BEV_F_STRETCH_BORDERS))
	{
		for (int y = BORDER ; y < area->h - BORDER ; y += BEV_SIZE / 2)
		{	
			{
				SDL_Rect src = { offset * BEV_SIZE, BORDER, BORDER, my_min(BEV_SIZE / 2, area->h - BORDER - y) };
				SDL_Rect dest = { area->x, y + area->y, BORDER, my_min(BEV_SIZE / 2, area->h - BORDER - y) };
				my_BlitSurface(gfx, &src, screen, &dest);
			}
			
			{
				SDL_Rect src = { SIZE_MINUS_BORDER + offset * BEV_SIZE, BORDER, BORDER, my_min(BEV_SIZE / 2, area->h - BORDER - y) };
				SDL_Rect dest = { area->x + area->w - BORDER, y + area->y, BORDER, my_min(BEV_SIZE / 2, area->h - BORDER - y) };
				my_BlitSurface(gfx, &src, screen, &dest);
			}
		}
		
		for (int x = BORDER ; x < area->w - BORDER ; x += BEV_SIZE / 2)
		{	
			{
				SDL_Rect src = { BORDER + offset * BEV_SIZE, 0, my_min(BEV_SIZE / 2, area->w - BORDER - x), BORDER };
				SDL_Rect dest = { area->x + x, area->y, my_min(BEV_SIZE / 2, area->w - BORDER - x), BORDER };
				my_BlitSurface(gfx, &src, screen, &dest);
			}
			
			{
				SDL_Rect src = { BORDER + offset * BEV_SIZE, SIZE_MINUS_BORDER, my_min(BEV_SIZE / 2, area->w - BORDER - x), BORDER };
				SDL_Rect dest = { x + area->x, area->y + area->h - BORDER, my_min(BEV_SIZE / 2, area->w - BORDER - x), BORDER };
				my_BlitSurface(gfx, &src, screen, &dest);
			}
		}
	}
	else
	{
		{
			SDL_Rect src = { offset * BEV_SIZE, BORDER, BORDER, BEV_SIZE / 2 };
			SDL_Rect dest = { area->x, BORDER + area->y, BORDER, area->h - 2 * BORDER };
			my_BlitSurface(gfx, &src, screen, &dest);
		}
		
		{
			SDL_Rect src = { SIZE_MINUS_BORDER + offset * BEV_SIZE, BORDER, BORDER, BEV_SIZE / 2 };
			SDL_Rect dest = { area->x + area->w - BORDER, BORDER + area->y, BORDER, area->h - 2 * BORDER };
			my_BlitSurface(gfx, &src, screen, &dest);
		}
		{
			SDL_Rect src = { BORDER + offset * BEV_SIZE, 0, BEV_SIZE / 2, BORDER };
			SDL_Rect dest = { area->x + BORDER, area->y, area->w - 2 * BORDER, BORDER };
			my_BlitSurface(gfx, &src, screen, &dest);
		}
		
		{
			SDL_Rect src = { BORDER + offset * BEV_SIZE, SIZE_MINUS_BORDER, BEV_SIZE / 2, BORDER };
			SDL_Rect dest = { BORDER + area->x, area->y + area->h - BORDER, area->w - 2 * BORDER, BORDER };
			my_BlitSurface(gfx, &src, screen, &dest);
		}
	}
	
	/* Corners */
	{
		SDL_Rect src = { offset * BEV_SIZE, 0, BORDER, BORDER };
		SDL_Rect dest = { area->x, area->y, BORDER, BORDER };
		my_BlitSurface(gfx, &src, screen, &dest);
	}
	
	{
		SDL_Rect src = { SIZE_MINUS_BORDER + offset * BEV_SIZE, 0, BORDER, BORDER };
		SDL_Rect dest = { area->x + area->w - BORDER, area->y, BORDER, BORDER };
		my_BlitSurface(gfx, &src, screen, &dest);
	}
	
	{
		SDL_Rect src = { SIZE_MINUS_BORDER + offset * BEV_SIZE, SIZE_MINUS_BORDER, BORDER, BORDER };
		SDL_Rect dest = { area->x + area->w - BORDER, area->y + area->h - BORDER, BORDER, BORDER };
		my_BlitSurface(gfx, &src, screen, &dest);
	}
	
	{
		SDL_Rect src = { offset * BEV_SIZE, SIZE_MINUS_BORDER, BORDER, BORDER };
		SDL_Rect dest = { area->x, area->y + area->h - BORDER, BORDER, BORDER };
		my_BlitSurface(gfx, &src, screen, &dest);
	}
}


void button(GfxDomain *screen, const SDL_Rect *area, GfxSurface *gfx, int offset, int decal)
{
	bevelex(screen, area, gfx, offset, BEV_F_STRETCH_ALL);
	
	if (decal >= 0)
	{
		SDL_Rect src = { decal * BEV_SIZE, BEV_SIZE, BEV_SIZE, BEV_SIZE };
		SDL_Rect dest = { area->x + area->w / 2 - BEV_SIZE / 2, area->y + area->h / 2 - BEV_SIZE / 2, BEV_SIZE, BEV_SIZE };
		my_BlitSurface(gfx, &src, screen, &dest);
	}
}


void button_text(GfxDomain *screen, const SDL_Rect *area, GfxSurface *gfx, int offset, const Font *font, const char *label)
{
	bevelex(screen, area, gfx, offset, BEV_F_STRETCH_ALL);
	
	SDL_Rect dest = { area->x + area->w / 2 - font_text_width(font, label) / 2, area->y + area->h / 2 - font->h / 2, 1000, 1000 };
	font_write(font, screen, &dest, label);
}


void separator(GfxDomain *dest, const SDL_Rect *parent, SDL_Rect *rect, GfxSurface *gfx, int offset)
{
	while (rect->x > parent->x) update_rect(parent, rect);
	
	SDL_Rect r;
	copy_rect(&r, rect);
	
	rect->x = parent->x;
	rect->y += SEPARATOR_HEIGHT;
	
	r.y += SEPARATOR_HEIGHT/3;
	r.h = SEPARATOR_HEIGHT/2;
	r.w = parent->w;
	
	bevel(dest, &r, gfx, offset);
}


void bevel(GfxDomain *screen, const SDL_Rect *area, GfxSurface *gfx, int offset)
{
	bevelex(screen, area, gfx, offset, BEV_F_NORMAL);
}
