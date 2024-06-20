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

#include "menu.h"
#include "gui/bevel.h"
#include "gfx/font.h"
#include "gui/view.h"
#include "shortcuts.h"
#include "bevdefs.h"
#include <string.h>

#define SC_SIZE 64

enum { ZONE, DRAW };

static void (*menu_close_hook)(void)  = NULL;
static const Menu *current_menu = NULL;
static const Menu *current_menu_action = NULL;
static const KeyShortcut *shortcuts = NULL;
static GfxSurface *menu_gfx = NULL;
static const Font * menu_font, *shortcut_font, *header_font;
static const Font * menu_font_selected, *shortcut_font_selected, *header_font_selected;

void open_menu(const Menu *mainmenu, const Menu *action, void (*close_hook)(void), const KeyShortcut *_shortcuts, 
	const Font *headerfont, const Font *headerfont_selected, 
	const Font *menufont, const Font *menufont_selected, 
	const Font *shortcutfont, const Font *shortcutfont_selected, GfxSurface *gfx)
{
	current_menu = mainmenu;
	current_menu_action = action;
	menu_close_hook = close_hook;
	shortcuts = _shortcuts;
	menu_gfx = gfx;
	header_font = headerfont;
	header_font_selected = headerfont_selected; 
	menu_font = menufont; 
	menu_font_selected = menufont_selected;
	shortcut_font = shortcutfont;
	shortcut_font_selected = shortcutfont_selected;
}


const Menu * get_current_menu()
{
	return current_menu;
}


const Menu * get_current_menu_action()
{
	return current_menu_action;
}

void close_menu()
{
	if (current_menu_action == NULL)
	{
		if (menu_close_hook) menu_close_hook();
	}
	else
	{
		if (menu_close_hook) menu_close_hook();
		if (current_menu_action->action == MENU_CHECK || current_menu_action->action == MENU_CHECK_NOSET)
		{
			if (current_menu_action->action == MENU_CHECK) *(int*)(current_menu_action->p1) ^= CASTPTR(int,current_menu_action->p2);
			
			if (current_menu_action->p3)
				((void *(*)(void*,void*,void*))(current_menu_action->p3))(0,0,0);
		}
		else
		{
			current_menu_action->action(current_menu_action->p1, current_menu_action->p2, current_menu_action->p3);
		}
		
		current_menu = NULL;
		current_menu_action = NULL;
	}
}


static int get_menu_item_width(const Menu *item)
{
	return strlen(item->text) + 1;
}


static const char * get_shortcut_key(const Menu *item)
{
	if (!shortcuts) return NULL;

	for (int i = 0 ; shortcuts[i].action ; ++i)
	{
		if (((item->action == MENU_CHECK || item->action == MENU_CHECK_NOSET) && (void*)shortcuts[i].action == item->p3) ||
			(shortcuts[i].action == item->action &&
			(void*)shortcuts[i].p1 == item->p1 &&
			(void*)shortcuts[i].p2 == item->p2 &&
			(void*)shortcuts[i].p3 == item->p3))
		{
			return get_shortcut_string(&shortcuts[i]);
		}
		else if (item->submenu)
		{
			return "½";
		}
	}

	return NULL;
}


// Below is a two-pass combined event and drawing menu routine. It is two-pass (as opposed to other combined drawing
// handlers otherwhere in the project) so it can handle overlapping zones correctly. Otherwhere in the app there simply
// are no overlapping zones, menus however can overlap because of the cascading submenus etc.

static void draw_submenu(GfxDomain *menu_dest, const SDL_Event *event, const Menu *items, const Menu *child, SDL_Rect *child_position, int pass)
{
	SDL_Rect area = { 0, 0, menu_dest->screen_w, shortcut_font->h + 4 + 1 };
	SDL_Rect r;
	const Font *font = NULL;
	int horiz = 0;
	
	/* In short: this first iterates upwards the tree until it finds the main menu (FILE, SHOW etc.)
	Then it goes back level by level updating the collision/draw zones
	*/
	
	if (items)
	{
		if (items[0].parent != NULL)
		{
			draw_submenu(menu_dest, event, items[0].parent, items, &area, pass);
			
			font = menu_font;
			
			area.w = area.h = 0;
			
			const Menu * item = items;
		
			for (; item->text ; ++item)
			{
				area.w = my_max(get_menu_item_width(item), area.w);
				if (item->text[0])
					area.h += font->h + 1;
				else
					area.h += SEPARATOR_HEIGHT;
			}
			
			area.w = area.w * font->w;
			area.x += 3;
			area.y += 4;
			
			area.w += SC_SIZE;
			
			if (area.w + area.x > menu_dest->screen_w)
				area.x -= area.w + area.x - menu_dest->screen_w + 2;
			
			copy_rect(&r, &area);
			
			SDL_Rect bev;
			copy_rect(&bev, &area);
			adjust_rect(&bev, -6);
			
			if (pass == DRAW) bevel(menu_dest, &bev, menu_gfx, BEV_MENU);
			
			r.h = font->h + 1;
		}
		else
		{
			if (pass == DRAW) bevel(menu_dest, &area, menu_gfx, BEV_MENUBAR);
			
			copy_rect(&r, &area);
			adjust_rect(&r, 2);
			
			font = header_font;
			
			horiz = 1;
			
			r.h = font->h;
		}
		
		const Menu * item = items;
		
		for (; item->text ; ++item)
		{
			if (item->text[0])
			{
				if (horiz) font = header_font; else font = menu_font;
			
				const char * sc_text = get_shortcut_key(item);
				int bg = 0;
				
				if (horiz) r.w = font->w * get_menu_item_width(item) + 8;
				
				if (event->type == SDL_MOUSEMOTION && pass == ZONE)
				{
					if ((event->button.x >= r.x) && (event->button.y >= r.y) 
						&& (event->button.x < r.x + r.w) && (event->button.y < r.y + r.h))
					{
						if (item->submenu)
						{
							current_menu = item->submenu;
							current_menu_action = NULL;
							bg = 1;
						}
						else if (item->action)
						{
							current_menu_action = item;
							current_menu = items;
							bg = 1;
						}
					}
					else if (current_menu_action && item == current_menu_action)
					{
						current_menu_action = NULL;
					}
				}
				
				if (item->submenu == child && child)
				{
					copy_rect(child_position, &r);
					if (horiz) child_position->y += r.h;
					else { child_position->x += r.w + 4; child_position->y -= 4; } 
					bg = 1;
				}
				
				int selected = 0;
				
				if ((pass == DRAW) && (bg || (current_menu_action == item && current_menu_action)))
				{
					SDL_Rect bar;
					copy_rect(&bar, &r);
					adjust_rect(&bar, -1);
					bar.h --;
					bevel(menu_dest, &bar, menu_gfx, BEV_MENU_SELECTED);
					
					font = horiz ? header_font_selected : menu_font_selected;
					selected = 1;
				}
				
				if (pass == DRAW) 
				{
					SDL_Rect text;
					copy_rect(&text, &r);
					text.x += font->w;
					text.w -= font->w;
					font_write(font, menu_dest, &text, item->text);
					
					char tick_char[2] = { 0 };
					
					if ((item->action == MENU_CHECK || item->action == MENU_CHECK_NOSET) && (*(int*)item->p1 & CASTPTR(int,item->p2)))
						*tick_char = '§';
					else if (item->flags & MENU_BULLET)
						*tick_char = '^';
					
					if (tick_char[0] != 0)
					{
						SDL_Rect tick;
						copy_rect(&tick, &r);
						tick.y = r.h / 2 + r.y - shortcut_font->h / 2;
						font_write(selected ? shortcut_font_selected : shortcut_font, menu_dest, &tick, tick_char);
					}
				}
				
				if (pass == DRAW && !horiz && sc_text) 
				{
					r.x += r.w;
					int tmpw = r.w, tmpx = r.x, tmpy = r.y;
					r.w = SC_SIZE;
					r.x -= strlen(sc_text) * shortcut_font->w;
					r.y = r.h / 2 + r.y - shortcut_font->h / 2;
					font_write(selected ? shortcut_font_selected : shortcut_font, menu_dest, &r, sc_text);
					r.x = tmpx;
					r.y = tmpy;
					update_rect(&area, &r);
					r.w = tmpw;
				}
				else update_rect(&area, &r);
			}
			else
			{
				separator(menu_dest, &area, &r, menu_gfx, BEV_SEPARATOR);
			}
		}
	}
}


void draw_menu(GfxDomain *dest, const SDL_Event *e)
{
	draw_submenu(dest, e, current_menu, NULL, NULL, ZONE);
	draw_submenu(dest, e, current_menu, NULL, NULL, DRAW);
}
