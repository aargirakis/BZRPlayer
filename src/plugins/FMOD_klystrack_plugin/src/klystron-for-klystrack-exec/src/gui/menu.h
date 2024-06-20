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

struct menu_t
{
	int flags;
	const struct menu_t *parent;
	const char * text;
	const struct menu_t *submenu;
	void (*action)(void*, void*, void *);
	void *p1, *p2, *p3;
} menu_t;

enum { MENU_BULLET = 1 };
#define MENU_CHECK (void*)1
#define MENU_CHECK_NOSET (void*)2

typedef struct menu_t Menu;

#include "SDL.h"
#include "shortcuts.h"
#include "gfx/font.h"
#include "gfx/gfx.h"

void open_menu(const Menu *mainmenu, const Menu *action, void (*close_hook)(void), const KeyShortcut *_shortcuts, 
	const Font *headerfont, const Font *headerfont_selected, 
	const Font *menufont, const Font *menufont_selected, 
	const Font *shortcutfont, const Font *shortcutfont_selected, GfxSurface *gfx);
void close_menu();
void draw_menu(GfxDomain *dest, const SDL_Event *e);
const Menu * get_current_menu_action();
const Menu * get_current_menu();
