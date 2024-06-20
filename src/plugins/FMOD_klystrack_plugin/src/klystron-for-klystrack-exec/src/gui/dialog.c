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

#include "dialog.h"
#include "gui/bevel.h"
#include "gui/mouse.h"
#include "gui/view.h"
#include "gfx/font.h"
#include "gui/bevdefs.h"
#include <string.h>


static void flip(void *bits, void *mask, void *unused)
{
	*CASTTOPTR(Uint32,bits) ^= CASTPTR(Uint32,mask);
}


int checkbox(GfxDomain *dest, const SDL_Event *event, const SDL_Rect *area, GfxSurface *gfx, const Font * font,  int offset, int offset_pressed, int decal, const char* _label, Uint32 *flags, Uint32 mask)
{
	SDL_Rect tick, label;
	copy_rect(&tick, area);
	copy_rect(&label, area);
	tick.h = tick.w = 8;
	label.w -= tick.w + 4;
	label.x += tick.w + 4;
	label.y += 1;
	label.h -= 1;
	int pressed = button_event(dest, event, &tick, gfx, offset, offset_pressed, (*flags & mask) ? decal : -1, flip, flags, MAKEPTR(mask), 0);
	font_write(font, dest, &label, _label);
	pressed |= check_event(event, &label, flip, flags, MAKEPTR(mask), 0);
	
	return pressed;
}


static void delegate(void *p1, void *p2, void *p3)
{
	set_motion_target(NULL, p3);
	
	if (p1)
	{
		((void(*)(void*,void*,void*))p1)(((void **)p2)[0], ((void **)p2)[1], ((void **)p2)[2]);
	}
}


int button_event(GfxDomain *dest, const SDL_Event *event, const SDL_Rect *area, GfxSurface *gfx, int offset, int offset_pressed, int decal, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	Uint32 mask = ((Uint32)area->x << 16) | (Uint32)area->y | 0x80000000;
	void *p[3] = { param1, param2, param3 };
	int pressed = check_event(event, area, delegate, action, p, MAKEPTR(mask));
	pressed |= check_drag_event(event, area, NULL, MAKEPTR(mask)) << 1;
	button(dest, area, gfx, pressed ? offset_pressed : offset, decal);
	
	return pressed;
}


int button_text_event(GfxDomain *dest, const SDL_Event *event, const SDL_Rect *area, GfxSurface *gfx, const Font *font, int offset, int offset_pressed, const char * label, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	Uint32 mask = ((Uint32)area->x << 16) | (Uint32)area->y | 0x80000000;
	void *p[3] = { param1, param2, param3 };
	int pressed = check_event(event, area, delegate, action, p, MAKEPTR(mask));
	pressed |= check_drag_event(event, area, NULL, MAKEPTR(mask)) << 1;
	button_text(dest, area, gfx, pressed ? offset_pressed : offset, font, label);
	
	return pressed;
}


int spinner(GfxDomain *dest, const SDL_Event *event, const SDL_Rect *_area, GfxSurface *gfx, int param)
{
	int plus, minus;
	SDL_Rect area;
	copy_rect(&area, _area);
	area.w /= 2;
	minus = button_event(dest, event, &area, gfx, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_MINUS, NULL, MAKEPTR(0x80000000 | ((Uint32)area.x << 16 | area.y)), 0, NULL) & 1;

	area.x += area.w;
	plus = button_event(dest, event, &area, gfx, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_PLUS, NULL, MAKEPTR(0x81000000 | ((Uint32)area.x << 16 | area.y)), 0, NULL) & 1;
	
	return plus ? +1 : (minus ? -1 : 0);
}


int generic_edit_text(SDL_Event *e, char *edit_buffer, size_t edit_buffer_size, int *editpos)
{
	switch (e->type)
	{
		case SDL_TEXTINPUT:
			if (*editpos < (edit_buffer_size - 1) && strlen(edit_buffer) < edit_buffer_size - 1)
			{
				memmove(&edit_buffer[*editpos + 1], &edit_buffer[*editpos], edit_buffer_size - *editpos - 1);
				edit_buffer[*editpos] = e->text.text[0];
				clamp(*editpos, +1, 0,edit_buffer_size-1);
			}
			break;
		case SDL_TEXTEDITING:
			break;
		
	
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_ESCAPE:
				return -1;
			break;
			
			case SDLK_HOME:
				*editpos = 0;
			break;
			
			case SDLK_END:
				*editpos = strlen(edit_buffer);
			break;
			
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				return 1;
			break;
		
			case SDLK_BACKSPACE:
				if (*editpos > 0) --*editpos;
				/* Fallthru */
			case SDLK_DELETE:
				memmove(&edit_buffer[*editpos], &edit_buffer[*editpos + 1], edit_buffer_size - *editpos);
				edit_buffer[edit_buffer_size - 1] = '\0';
			break;
		
			case SDLK_LEFT:
			{
				if (*editpos > 0) --*editpos;
			}
			break;
			
			case SDLK_RIGHT:
			{ 
				if (*editpos < strlen(edit_buffer) && *editpos < edit_buffer_size - 1) ++*editpos;
			}
			break;
		
			default:
			{
				/**/
			}
			break;
		}
		
		break;
	}
	
	return 0;
}
