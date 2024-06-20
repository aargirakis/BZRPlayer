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

#include "view.h"
#include "macros.h"
#include <string.h>

extern int event_hit;

void update_rect(const SDL_Rect *parent, SDL_Rect *rect)
{
	rect->x += rect->w + ELEMENT_MARGIN;
	
	if (rect->x + rect->w - ELEMENT_MARGIN >= parent->x + parent->w)
	{
		rect->x = parent->x;
		rect->y += rect->h;
	}
}


void adjust_rect(SDL_Rect *rect, int margin)
{
	rect->x += margin;
	rect->y += margin;
	rect->w -= margin * 2;
	rect->h -= margin * 2;
}


void copy_rect(SDL_Rect *dest, const SDL_Rect *src)
{
	memcpy(dest, src, sizeof(*dest));
}


void clip_rect(SDL_Rect *rect, const SDL_Rect *limits)
{
	int w = rect->w, h = rect->h;

	if (rect->x < limits->x) { w -= limits->x - rect->x; rect->x = limits->x; }
	if (rect->y < limits->y) { h -= limits->y - rect->y; rect->y = limits->y; }
	if (w + rect->x > limits->w + limits->x) { w = limits->w + limits->x - rect->x; }
	if (h + rect->y > limits->h + limits->y) { h = limits->h + limits->y - rect->y; }
	
	rect->w = my_max(0, w);
	rect->h = my_max(0, h);
}


void draw_view(GfxDomain *dest, const View* views, const SDL_Event *_event)
{
	SDL_Event event;
	memcpy(&event, _event, sizeof(event));
	for (int i = 0 ; views[i].handler ; ++i)
	{
		const View *view = &views[i];
		
		SDL_Rect area;
		area.x = view->position.x >= 0 ? view->position.x : dest->screen_w + view->position.x;
		area.y = view->position.y >= 0 ? view->position.y : dest->screen_h + view->position.y;
		area.w = *(Sint16*)&view->position.w > 0 ? *(Sint16*)&view->position.w : dest->screen_w + *(Sint16*)&view->position.w - view->position.x;
		area.h = *(Sint16*)&view->position.h > 0 ? *(Sint16*)&view->position.h : dest->screen_h + *(Sint16*)&view->position.h - view->position.y;

		int iter = 0;
		do
		{
			event_hit = 0;
			view->handler(dest, &area, &event, view->param);
			if (event_hit) 
			{
				event.type = SDL_USEREVENT + 1;
				++iter;
			}
		}
		while (event_hit && iter <= 1);
	}
}



void center_rect(const SDL_Rect *parent, SDL_Rect *rect)
{
	rect->x = (parent->w - rect->w) / 2;
	rect->y = (parent->h - rect->h) / 2;
}

