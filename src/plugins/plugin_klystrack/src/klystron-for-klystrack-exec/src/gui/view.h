#pragma once

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

#define SEPARATOR_HEIGHT 6
#define ELEMENT_MARGIN 4

#include "gfx/gfx.h"

typedef struct
{
	const SDL_Rect position;
	
	/*
	This is a combined drawing and mouse event handler.
	*/
	
	void (*handler)(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
	void *param;
	/*
	When clicked the focus set to the following or if the param equals -1 it will leave as it is
	*/
	int focus;
} View;

void draw_view(GfxDomain *dest, const View* views, const SDL_Event *event);

void adjust_rect(SDL_Rect *rect, int margin);
void copy_rect(SDL_Rect *dest, const SDL_Rect *src);
void update_rect(const SDL_Rect *parent, SDL_Rect *rect);
void clip_rect(SDL_Rect *rect, const SDL_Rect *limits);
void center_rect(const SDL_Rect *parent, SDL_Rect *rect);
