#include "editor.h"
#include "export.h"
#include "gfx/gfx.h"
#include "gfx/font.h"
#include "gui/toolutil.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <strings.h>

#define LIBCONFIG_STATIC

#include <libconfig.h>

#define MAGICK_LAYER MAX_LAYERS
#define BRUSH_LAYER MAX_LAYERS+1
#define CLIPBOARDSIZE 32

enum
{
	EM_LEVEL,
	EM_EVENTS
};

Level level;
int screen_width = 640, screen_height = 480, screen_scale;
int pf_width = 640, pf_height = 480;
int saved_layer = 0;
int current_layer = 0;
int selected_tile = 0;
int selected_param = 0;
int scroll_x = 0, scroll_y = 0;
int drag_x = -1, drag_y = -1, drag_event = -1, selected_event = -1;
int edit_mode = EM_LEVEL;
int ev_drag_start_x, ev_drag_start_y, drag_start_x, drag_start_y;
Uint32 bg_color = 0;
GfxDomain *domain = NULL;
Font font;
GfxSurface *gfx = NULL;

typedef enum { EVPAR_INT, EVPAR_ENUM } EvParType;

typedef struct
{
	const char *name;
	EvParType type;
	const char **enums;
	int int_min, int_max; 
	int default_val;
} EvParDesc;

typedef struct
{
	const char *name;
	Uint32 color;
	EvParDesc param[EV_PARAMS];
} EvDesc;

static EvDesc *ev_desc = NULL;
static int n_ev_desc = 0;

static const char *tileset = "";
TileDescriptor *descriptor;
config_t cfg;

void load_dialog()
{
	FILE *f = open_dialog("rb", "Load level", "lev", domain, gfx, &font, &font, NULL);
	if (f)
	{
	
		level_load(&level, f);
		fclose(f);
		
		for (int i = 0 ; i < MAX_LAYERS+2 ; ++i)
		{
			level.layer[i].tiles = descriptor;
		}
	}
}


void load_level(const char *path)
{
	FILE *f = fopen(path, "rb");
	if (f)
	{
	
		level_load(&level, f);
		fclose(f);
		
		for (int i = 0 ; i < MAX_LAYERS+2 ; ++i)
		{
			level.layer[i].tiles = descriptor;
		}
	}
}


void load_defs(const char *fn)
{
	FILE *f = fn ? fopen(fn, "r") : open_dialog("r", "Load project defs", "cfg", domain, gfx, &font, &font, NULL);
	if (f)
	{
		int r = config_read(&cfg, f);
		
		fclose(f);
		
		if (r)
		{
			if (!config_lookup_string(&cfg, "tileset", &tileset))
				warning("No tileset defined");
			if (!config_lookup_int(&cfg, "bg_color", (Sint32*)&bg_color))
				warning("No bg_color defined");
			config_lookup_int(&cfg, "screen.width", &screen_width);
			config_lookup_int(&cfg, "screen.height", &screen_height);
			config_lookup_int(&cfg, "screen.scale", &screen_scale);
			config_lookup_int(&cfg, "playfield.width", &pf_width);
			config_lookup_int(&cfg, "playfield.height", &pf_height);
				
			config_setting_t *e = config_lookup(&cfg, "events");
			if (e)
			{
				n_ev_desc = config_setting_length(e);
				ev_desc = calloc(n_ev_desc, sizeof(ev_desc[0]));
				
				for (int i = 0 ; i < n_ev_desc ; ++i)
				{
					config_setting_t *elem = config_setting_get_elem(e, i);
					config_setting_lookup_string(elem, "name", &ev_desc[i].name);
					config_setting_t *params = config_setting_get_member(elem, "params");
					
					EvDesc *ed = &ev_desc[i];
					
					ed->color = 0x00ff00;
					
					config_setting_lookup_int(elem, "color", (Sint32*)&ev_desc[i].color);
					
					if (params && config_setting_type(params) == CONFIG_TYPE_LIST) 
					{
						int n_p = config_setting_length(params);
						
						for (int i = 0 ; i < n_p && i < EV_PARAMS - 3 ; ++i)
						{
							EvParDesc *p = &ed->param[i + 1];
							
							config_setting_t *elem = config_setting_get_elem(params, i);
							config_setting_lookup_string(elem, "name", &p->name);
							config_setting_t *enums = config_setting_get_member(elem, "enum");
							
							if (enums && config_setting_type(enums) == CONFIG_TYPE_ARRAY)
							{
								p->type = EVPAR_ENUM;
								p->int_min = 0;
								p->int_max = config_setting_length(enums);
								p->enums = calloc(p->int_max, sizeof(p->enums[0]));
								for (int e = 0 ; e < p->int_max ; ++e)
								{
									p->enums[e] = config_setting_get_string_elem(enums, e); 
								}
								
								p->int_max--;
							}
							else
							{
								p->type = EVPAR_INT;
								config_setting_t *range = config_setting_get_member(elem, "range");
								if (range && config_setting_type(range) == CONFIG_TYPE_ARRAY)
								{
									p->int_min = config_setting_get_int_elem(range, 0); 
									p->int_max = config_setting_get_int_elem(range, 1); 
								}
								else
								{
									p->int_min = 0; 
									p->int_max = 65535;
								}
							}
						}
					}
				}
			}
			else
			{
				warning("No events defined");
			}
		}
		else
		{
			warning("Could not read config: (%d) %s", config_error_line(&cfg), config_error_text(&cfg));
		}
	}
	else
	{
		warning("cfgfile not found");
	}
}


int save_dialog()
{
	FILE *f = open_dialog("wb", "Save level", "lev", domain, gfx, &font, &font, NULL);
	if (f)
	{
		level.n_layers = MAX_LAYERS;
	
		level_save(&level, f);
		fclose(f);
		
		return 1;
	}
	else
	{
		return 0;
	}
}

void draw_rect(GfxDomain *s, int x1, int y1, int x2, int y2, Uint32 color)
{
	
	if (x1 > x2) { int temp = x2; x2 = x1; x1 = temp; }
	if (y1 > y2) { int temp = y2; y2 = y1; y1 = temp; }

	{
		SDL_Rect rect = {x1, y1, x2-x1, 1};
		gfx_rect(s, &rect, color);
	}
	
	{
		SDL_Rect rect = {x1, y2, x2-x1, 1};
		gfx_rect(s, &rect, color);
	}
	
	{
		SDL_Rect rect = {x1, y1, 1, y2-y1};
		gfx_rect(s, &rect, color);
	}
	
	{
		SDL_Rect rect = {x2, y1, 1, y2-y1};
		gfx_rect(s, &rect, color);
	}
}


void vector(GfxDomain *screen, int x0, int y0, int x1, int y1, Uint32 color)
{
	gfx_line(screen, x0, y0, x1, y1, color);
	
	int dx = (x0 - x1);
	int dy = (y0 - y1);
	int d = sqrt(dx*dx+dy*dy);
	if (d == 0) return;
	dx = dx*16/d;
	dy = dy*16/d;
	
	gfx_line(screen, x1, y1, x1 + dy + dx, y1 - dx + dy, color);
	gfx_line(screen, x1, y1, x1 - dy + dx, y1 + dx + dy, color);
}


void draw(GfxDomain *screen, int mouse_x, int mouse_y, int draw_all)
{
	if (draw_all)
	{
		for (int i = 0 ; i < MAX_LAYERS ; ++i)
		{
			if (level.layer[i].flags & BG_PARALLAX)
				bg_draw(screen, NULL, &level.layer[i], scroll_x / my_max(1, level.layer[i].prx_mlt_x), scroll_y / my_max(1, level.layer[i].prx_mlt_y));
			else
				bg_draw(screen, NULL, &level.layer[i], scroll_x, scroll_y);
		}
	}
	else
	{
		int _scroll_x = scroll_x;
		int _scroll_y = scroll_y;
		int _drag_x = drag_x;
		int _drag_y = drag_y;
	
		if (current_layer >= MAGICK_LAYER)
		{
			_scroll_x = my_max(0, my_min(scroll_x, level.layer[current_layer].w * CELLSIZE - screen_width));
			_scroll_y = my_max(0, my_min(scroll_y, level.layer[current_layer].h * CELLSIZE - screen_height));
		}
		
		mouse_x = (mouse_x + ((_scroll_x) & (CELLSIZE-1))) / CELLSIZE;
		mouse_y = (mouse_y + ((_scroll_y) & (CELLSIZE-1))) / CELLSIZE;
		_drag_x = (_drag_x + ((_scroll_x) & (CELLSIZE-1))) / CELLSIZE;
		_drag_y = (_drag_y + ((_scroll_y) & (CELLSIZE-1))) / CELLSIZE;
		
		bg_draw(screen, NULL, &level.layer[current_layer], _scroll_x, _scroll_y);
		draw_rect(screen, -_scroll_x-1, -_scroll_y-1, -_scroll_x + level.layer[current_layer].w*CELLSIZE, -_scroll_y + level.layer[current_layer].h*CELLSIZE, 0xffffff);
		
		if (edit_mode == EM_LEVEL) 
		{
			if (drag_x != -1)
			{
				draw_rect(screen, (_drag_x)*CELLSIZE - ((_scroll_x) & (CELLSIZE-1)), (_drag_y)*CELLSIZE - ((_scroll_y) & (CELLSIZE-1)), 
					(mouse_x+1)*CELLSIZE - ((_scroll_x) & (CELLSIZE-1)), (mouse_y+1)*CELLSIZE - ((_scroll_y) & (CELLSIZE-1)), 0xffffff);
			}
			else
			{
				draw_rect(screen, (mouse_x)*CELLSIZE - ((_scroll_x) & (CELLSIZE-1)), (mouse_y)*CELLSIZE - ((_scroll_y) & (CELLSIZE-1)), 
					(mouse_x+level.layer[BRUSH_LAYER].w)*CELLSIZE - ((_scroll_x) & (CELLSIZE-1)), (mouse_y+level.layer[BRUSH_LAYER].h)*CELLSIZE - ((_scroll_y) & (CELLSIZE-1)), 0xffffff);
			}
		}
		else
		{
			for (int i = 0 ; i < level.n_events ; ++i)
			{
				for (int s = 0 ; s < 4 ; s+=2)
				draw_rect(screen, s + level.event[i].x * CELLSIZE - _scroll_x, s + level.event[i].y * CELLSIZE - _scroll_y, 
				  level.event[i].w * CELLSIZE + level.event[i].x * CELLSIZE - _scroll_x - s, level.event[i].h * CELLSIZE + level.event[i].y * CELLSIZE - _scroll_y - s, i == selected_event ? 0xffffff : ev_desc[level.event[i].param[0]].color);
				  
				if (level.event[i].param[EV_NEXT] != -1 && level.event[i].param[EV_NEXT] < level.n_events)
				{
					vector(screen, level.event[i].x * CELLSIZE + level.event[i].w*CELLSIZE/2 - _scroll_x, level.event[i].y * CELLSIZE + level.event[i].h*CELLSIZE/2 - _scroll_y, 
						level.event[level.event[i].param[EV_NEXT]].x * CELLSIZE + level.event[level.event[i].param[EV_NEXT]].w*CELLSIZE/2 - _scroll_x, level.event[level.event[i].param[EV_NEXT]].y * CELLSIZE + level.event[level.event[i].param[EV_NEXT]].h*CELLSIZE/2 - _scroll_y, 0xffffff);
				}
				
				if (level.event[i].param[EV_TRGPARENT] != -1 && level.event[i].param[EV_TRGPARENT] < level.n_events)
				{
					vector(screen, 	level.event[level.event[i].param[EV_TRGPARENT]].x * CELLSIZE - _scroll_x, level.event[level.event[i].param[EV_TRGPARENT]].y * CELLSIZE - _scroll_y, 
						level.event[i].x * CELLSIZE - _scroll_x, level.event[i].y * CELLSIZE - _scroll_y, 0xff0000);
				}
			}
		}
	}
	
	if (current_layer < MAGICK_LAYER) 
		draw_rect(screen, screen->screen_w / 2 - pf_width / 2, screen->screen_h / 2 - pf_height / 2, screen->screen_w / 2 + pf_width / 2, screen->screen_h / 2 + pf_height / 2, 0xa0a040);
}


void set_flags(int flip)
{
	if (current_layer < MAGICK_LAYER)
		level.layer[current_layer].flags ^= flip;
}


void resize_layer(int w, int h)
{
	if (current_layer == MAGICK_LAYER) return;
	
	Background *layer = &level.layer[current_layer];
	
	if (w <= 0 || h <= 0)
	{
		layer->w = my_max(0, layer->w);
		layer->h = my_max(0, layer->h);
		if (layer->data) free (layer->data);
		layer->data = NULL;
	}

	BgCell * temp = layer->data;
	
	layer->data = malloc(sizeof(BgCell)*w*h);
	memset(layer->data, 0, sizeof(BgCell)*w*h);
	
	if (temp)
	{
		for (int y = 0 ; y < h && y < layer->h ; ++y)
		{
			for (int x = 0 ; x < w && x < layer->w ; ++x)
			{
				memcpy(&layer->data[w*y+x], &temp[x+y*layer->w], sizeof(layer->data[0]));
			}
		}
		free(temp);
	}
	
	layer->w = w;
	layer->h = h;
	
	
	
}

void get_tile(int x, int y)
{
	if (current_layer != MAGICK_LAYER)
	{
		x += scroll_x;
		y += scroll_y;
	}

	x /= CELLSIZE;
	y /= CELLSIZE;
	
	if (saved_layer == BRUSH_LAYER || current_layer == BRUSH_LAYER)
	{
		if (!(x >= level.layer[current_layer].w || x < 0 || y >= level.layer[current_layer].h || y < 0));
			selected_tile = level.layer[current_layer].data[x + y*level.layer[current_layer].w].tile;
			
		return;
	}
	
	if (level.layer[current_layer].flags & BG_REPEAT_X)
		x = (x % level.layer[current_layer].w + level.layer[current_layer].w) % level.layer[current_layer].w;
	
	if (level.layer[current_layer].flags & BG_REPEAT_Y)
		y = (y % level.layer[current_layer].h + level.layer[current_layer].h) % level.layer[current_layer].h;
		
	if (!(x >= level.layer[current_layer].w || x < 0 || y >= level.layer[current_layer].h || y < 0))
	{
		int temp = current_layer;
		current_layer = BRUSH_LAYER;
		resize_layer(1,1);
		current_layer = temp;
		selected_tile = level.layer[BRUSH_LAYER].data[0].tile = level.layer[current_layer].data[x+y*level.layer[current_layer].w].tile;
	}
}


void set_tile(int ax, int ay)
{
	if (current_layer == MAGICK_LAYER) return;
	
	if (current_layer == BRUSH_LAYER)
	{
		ax /= CELLSIZE;
		ay /= CELLSIZE;
	
		if (!(ax >= level.layer[current_layer].w || ax < 0 || ay >= level.layer[current_layer].h || ay < 0))
			level.layer[current_layer].data[ax + ay*level.layer[current_layer].w].tile = selected_tile;
			
		return;
	}
	
	
	ax += scroll_x;
	ay += scroll_y;
	
	int fx = ax < 0;
	int fy = ay < 0;
	
	
	ax /= CELLSIZE;
	ay /= CELLSIZE;
	
	for (int y = 0 ; y < level.layer[BRUSH_LAYER].h ; ++y)
	{
		for (int x = 0 ; x < level.layer[BRUSH_LAYER].w ; ++x)
		{
			int _x = x+ax-fx, _y = y+ay-fy;
			
			if (level.layer[current_layer].flags & BG_REPEAT_X)
				_x = (_x % level.layer[current_layer].w + level.layer[current_layer].w) % level.layer[current_layer].w;
			
			if (level.layer[current_layer].flags & BG_REPEAT_Y)
				_y = (_y % level.layer[current_layer].h + level.layer[current_layer].h) % level.layer[current_layer].h;
			
			if (!(_x >= level.layer[current_layer].w || _x < 0 || _y >= level.layer[current_layer].h || _y < 0))
				level.layer[current_layer].data[_x + _y*level.layer[current_layer].w].tile = level.layer[BRUSH_LAYER].data[x+y*level.layer[BRUSH_LAYER].w].tile;
		}
	}
}


int has_pixels(TileDescriptor *desc)
{
	my_lock(desc->surface->surface);
	
	int result = 0;
	
#if SDL_VERSION_ATLEAST(1,3,0)
	Uint32 key;
	SDL_GetColorKey(desc->surface->surface, &key);
#else	
	const Uint32 key = desc->surface->surface->format->colorkey;
#endif
	
	for (int y = 0 ; y < desc->rect.h ; ++y)
	{
		Uint8 *p = (Uint8 *)desc->surface->surface->pixels + ((int)desc->rect.y + y) * desc->surface->surface->pitch + (int)desc->rect.x * desc->surface->surface->format->BytesPerPixel;
		
		for (int x = 0 ; x < desc->rect.w ; ++x)
		{
			//printf("%08x", *(Uint32*)p);
			if ((*((Uint32*)p)&0xffffff) != key)
			{
				++result;
			}
			
			p+=desc->surface->surface->format->BytesPerPixel;
		}
	}
	
	my_unlock(desc->surface->surface);
	
	return result;
}


void init_magic_layer(int w, int h, TileDescriptor *desc, int tiles)
{
	level.layer[BRUSH_LAYER].w = 1;
	level.layer[BRUSH_LAYER].h = 1;
	level.layer[BRUSH_LAYER].data = malloc(level.layer[BRUSH_LAYER].w*level.layer[BRUSH_LAYER].h*sizeof(level.layer[BRUSH_LAYER].data[0]));
	memset(level.layer[BRUSH_LAYER].data, 0, level.layer[BRUSH_LAYER].w*level.layer[BRUSH_LAYER].h*sizeof(level.layer[BRUSH_LAYER].data[0]));
	level.layer[BRUSH_LAYER].data[0].tile = selected_tile;

	level.layer[MAGICK_LAYER].w = w / CELLSIZE;
	level.layer[MAGICK_LAYER].h = h / CELLSIZE;
	level.layer[MAGICK_LAYER].data = malloc(level.layer[MAGICK_LAYER].w*level.layer[MAGICK_LAYER].h*sizeof(level.layer[MAGICK_LAYER].data[0]));
	memset(level.layer[MAGICK_LAYER].data, 0, level.layer[MAGICK_LAYER].w*level.layer[MAGICK_LAYER].h*sizeof(level.layer[MAGICK_LAYER].data[0]));
	
	for (int i = 0 ; i < tiles ; ++i)
	{
		int x = desc[i].rect.x / CELLSIZE;
		int y = desc[i].rect.y / CELLSIZE;
		level.layer[MAGICK_LAYER].data[x+y*level.layer[MAGICK_LAYER].w].tile = has_pixels(&desc[i]) ? i+1 : 0;
	}
}

void clear_layer()
{
	if (current_layer >= MAGICK_LAYER) return;
	memset(level.layer[current_layer].data, 0, level.layer[current_layer].w*level.layer[current_layer].h*sizeof(level.layer[current_layer].data[0]));
}


void get_brush(int x1, int y1, int x2, int y2)
{
	if (current_layer == BRUSH_LAYER) return;

	if (current_layer < MAGICK_LAYER)
	{
		x1 += scroll_x;
		y1 += scroll_y;
		x2 += scroll_x;
		y2 += scroll_y;
	}
	
	x1 /= CELLSIZE;
	y1 /= CELLSIZE;
	x2 /= CELLSIZE;
	y2 /= CELLSIZE;
	
	if (x1 > x2) { int temp = x2; x2 = x1; x1 = temp; }
	if (y1 > y2) { int temp = y2; y2 = y1; y1 = temp; }
	
	free(level.layer[BRUSH_LAYER].data);
	level.layer[BRUSH_LAYER].w = x2 - x1 + 1;
	level.layer[BRUSH_LAYER].h = y2 - y1 + 1;
	level.layer[BRUSH_LAYER].data = malloc(level.layer[BRUSH_LAYER].w * level.layer[BRUSH_LAYER].h * sizeof(level.layer[BRUSH_LAYER].data[0]));
	memset(level.layer[BRUSH_LAYER].data, 0, level.layer[BRUSH_LAYER].w * level.layer[BRUSH_LAYER].h * sizeof(level.layer[BRUSH_LAYER].data[0]));
	
	for (int y = 0 ; y < level.layer[BRUSH_LAYER].h ; ++y)
	{
		for (int x = 0 ; x < level.layer[BRUSH_LAYER].w ; ++x)
		{
			int _x = x + x1;
			int _y = y + y1;
			
			if (level.layer[current_layer].flags & BG_REPEAT_X)
				_x = (_x % level.layer[current_layer].w + level.layer[current_layer].w) % level.layer[current_layer].w;
			else if (_x >= level.layer[current_layer].w) break;
			
			if (level.layer[current_layer].flags & BG_REPEAT_Y)
				_y = (_y % level.layer[current_layer].h + level.layer[current_layer].h) % level.layer[current_layer].h;
			else if (_y >= level.layer[current_layer].h) break;
		
			level.layer[BRUSH_LAYER].data[x+y*level.layer[BRUSH_LAYER].w].tile = 
				level.layer[current_layer].data[_x+_y*level.layer[current_layer].w].tile;
		}
	}
}

void swap(void *a, void *b, size_t size)
{
	void * ptr = malloc(size);
	memcpy(ptr, a, size);
	memcpy(a, b, size);
	memcpy(b, ptr, size);
	free(ptr);
}


int get_event(int x, int y)
{
	x += scroll_x;
	y += scroll_y;
	
	if (x < 0) x-=CELLSIZE;
	if (y < 0) y-=CELLSIZE;
		
	x /= CELLSIZE;
	y /= CELLSIZE;
	
	// check for clicks on border
	
	const int border = 1;
	
	for (int i = 0 ; i < level.n_events ; ++i)
	{
		if (((int)level.event[i].x <= x && (int)level.event[i].y <= y && (int)level.event[i].x+(int)level.event[i].w > x && (int)level.event[i].y+(int)level.event[i].h > y ) &&
			!((int)level.event[i].x + border <= x && (int)level.event[i].y + border <= y && (int)level.event[i].x+(int)level.event[i].w - border > x && (int)level.event[i].y+(int)level.event[i].h - border > y ))
			return i;
	}
	
	// check for clicks inside the full area
		
	for (int i = 0 ; i < level.n_events ; ++i)
	{
		if ((int)level.event[i].x <= x && (int)level.event[i].y <= y && (int)level.event[i].x+(int)level.event[i].w > x && (int)level.event[i].y+(int)level.event[i].h > y )
			return i;
	}
	
	return -1;
}


void move_ev_pos(int x, int y)
{
	if (drag_event == -1) return;

	level.event[drag_event].x+=x;
	level.event[drag_event].y+=y;
}


void add_event(int x, int y)
{
	level.event = realloc(level.event, (level.n_events + 1) * sizeof(*level.event));
	
	x += scroll_x;
	y += scroll_y;
	
	x /= CELLSIZE;
	y /= CELLSIZE;
	
	level.event[level.n_events].x = x;
	level.event[level.n_events].y = y;
	level.event[level.n_events].w = 1;
	level.event[level.n_events].h = 1;
	
	
	if (drag_event != -1)
	{
		level.event[level.n_events].w = level.event[drag_event].w;
		level.event[level.n_events].h = level.event[drag_event].h;
		memcpy(&level.event[level.n_events].param, &level.event[drag_event].param, sizeof(level.event[drag_event].param));
	}
	else
	{
		if (n_ev_desc > 0)
		{
			for (int i = 0; i < EV_PARAMS ; ++i)
				level.event[level.n_events].param[i] = ev_desc[0].param[i].default_val;
		}	
		level.event[level.n_events].param[EV_NEXT] = -1;
		level.event[level.n_events].param[EV_TRGPARENT] = -1;
	}
	
	selected_event = level.n_events;
	
	++level.n_events;
}


void delete_event()
{
	if (selected_event == -1) return;
	
	memcpy(&level.event[selected_event], &level.event[selected_event+1], (level.n_events - selected_event) * sizeof(*level.event));
	
	--level.n_events;
	
	for (int i = 0 ; i < level.n_events ; ++i)
	{
		if (level.event[i].param[EV_NEXT] == selected_event)
		{
			level.event[i].param[EV_NEXT] = -1;
		}
		else if (level.event[i].param[EV_NEXT] > selected_event)
		{
			--level.event[i].param[EV_NEXT];
		}
		
		if (level.event[i].param[EV_TRGPARENT] == selected_event)
		{
			level.event[i].param[EV_TRGPARENT] = -1;
		}
		else if (level.event[i].param[EV_TRGPARENT] > selected_event)
		{
			--level.event[i].param[EV_TRGPARENT];
		}
	}
	
	if (selected_event >= level.n_events) --selected_event;
}


void shift_layer(int dx, int dy)
{
	int w = level.layer[current_layer].w;
	int h = level.layer[current_layer].h;
	
	BgCell *temp = malloc(sizeof(BgCell)*w*h);
	
	for (int y = 0 ; y < h ; ++y)
		for (int x = 0 ; x < w ; ++x)
		{
			memcpy(&temp[(x + dx + w) % w + ((y + dy + h) % h) * w], &level.layer[current_layer].data[x+y*w], sizeof(BgCell));
		}
		
	memcpy(level.layer[current_layer].data, temp, w * h * sizeof(BgCell));
	
	free(temp);
}


void shift_events(int dx,int dy)
{
	for (int i = 0 ; i < level.n_events ; ++i)
	{
		level.event[i].x += dx;
		level.event[i].y += dy;
	}
}


void double_layer()
{
	int w = level.layer[current_layer].w;
	int h = level.layer[current_layer].h;
	
	BgCell *temp = malloc(sizeof(BgCell) * (w * 2) * (h * 2));
	
	for (int y = 0 ; y < h * 2 ; ++y)
		for (int x = 0 ; x < w * 2 ; ++x)
		{
			memcpy(&temp[x + y * (w * 2)], &level.layer[current_layer].data[(x / 2) + (y / 2) * w], sizeof(BgCell));
		}
		
	BgCell *temp2 = level.layer[current_layer].data;
	level.layer[current_layer].data = temp;
	
	level.layer[current_layer].w *= 2;
	level.layer[current_layer].h *= 2;
	
	free(temp2);
}


void insert_rowcol(int sx, int sy, int dx, int dy)
{
	debug("Inserting row/col at %d,%d", sx, sy);
	resize_layer(level.layer[current_layer].w + dx, level.layer[current_layer].h + dy);
	
	for (int x = level.layer[current_layer].w - 1 ; x >= sx + dx ; --x)
	{
		for (int y = 0 ; y < level.layer[current_layer].h ; ++y)
		{
			level.layer[current_layer].data[x + y * level.layer[current_layer].w].tile = 
				level.layer[current_layer].data[x + y * level.layer[current_layer].w - dx].tile;
		}
	}
	
	for (int x = 0 ; x < level.layer[current_layer].w ; ++x)
	{
		for (int y = level.layer[current_layer].h - 1 ; y >= sy + dy ; --y)
		{
			level.layer[current_layer].data[x + y * level.layer[current_layer].w].tile = 
				level.layer[current_layer].data[x + (y - dy) * level.layer[current_layer].w].tile;
		}
	}
	
}


void resize_event(int dx,int dy)
{
	if (selected_event == -1) return;
	
	if (dx < 0 && level.event[selected_event].w > 1) level.event[selected_event].w += dx;
	if (dy < 0 && level.event[selected_event].h > 1) level.event[selected_event].h += dy;
	if (dx > 0 && level.event[selected_event].w < 65535) level.event[selected_event].w += dx;
	if (dy > 0 && level.event[selected_event].h < 65535) level.event[selected_event].h += dy;
}

#undef main

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE);
	atexit(SDL_Quit);
	
	domain = gfx_create_domain("Editor", 0, 640, 480, 1);
	domain->screen_w = 640;
	domain->screen_h = 480;
	domain->fps = 20;
	domain->scale = 1;
	gfx_domain_update(domain, true);
	
	gfx = gfx_load_surface(domain, "bevel.bmp", GFX_KEYED);
	font_load_file(domain, &font, "8x8.fnt");
	
	config_init(&cfg);
	
	load_defs(argc > 1 ? argv[1] : NULL);
	
	if (strcmp(tileset,"") == 0) 
	{
		fatal("no tileset specified");
		return 1;
	}
	
	if (argc > 2) load_level(argv[2]);
	
	domain->screen_w = screen_width;
	domain->screen_h = screen_height;
	domain->scale = screen_scale;
	gfx_domain_update(domain, true);
	
	GfxSurface *tiles = gfx_load_surface(domain, tileset, GFX_KEYED);
	
	if (!tiles)
	{
		fatal("tileset not found");
		return 2;
	}
	
	descriptor = gfx_build_tiledescriptor(tiles, CELLSIZE, CELLSIZE, NULL);
	
	for (int i = 0 ; i < MAX_LAYERS+2 ; ++i)
	{
		level.layer[i].tiles = descriptor;
	}
	
	level.n_layers = MAX_LAYERS;
	
	init_magic_layer(tiles->surface->w, tiles->surface->h, descriptor, (tiles->surface->w/CELLSIZE) * (tiles->surface->h/CELLSIZE));
		
	int done = 0;
	
	while (1)
	{
		SDL_Event e;
		
		int got_event = 0;
		
		while (SDL_PollEvent(&e))
		{
			got_event = 1;
			switch (e.type)
			{
				case SDL_QUIT: 
					done = 1; 
				break;
				
				case SDL_MOUSEMOTION:
				{
					if (e.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
					{
						scroll_x -= e.motion.x/domain->scale - drag_start_x;
						scroll_y -= e.motion.y/domain->scale - drag_start_y;
						drag_start_x = e.button.x / domain->scale;
						drag_start_y = e.button.y / domain->scale;
					}
					
			
					if (edit_mode == EM_LEVEL && e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT) && drag_x == -1)
					{
						set_tile(e.motion.x / domain->scale, e.motion.y / domain->scale);
					}
					else if (edit_mode == EM_EVENTS && e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT) && drag_event != -1)
					{
						move_ev_pos(e.motion.x / CELLSIZE / domain->scale - ev_drag_start_x, e.motion.y / CELLSIZE / domain->scale - ev_drag_start_y);
						ev_drag_start_x = e.motion.x / CELLSIZE / domain->scale;
						ev_drag_start_y = e.motion.y / CELLSIZE / domain->scale;
					}
				}
				break;
				
				case SDL_MOUSEBUTTONUP:
				{
					if (edit_mode == EM_LEVEL && drag_x != -1)
					{
						get_brush(drag_x, drag_y, e.button.x / domain->scale, e.button.y / domain->scale);
						drag_x = -1;
						selected_event = drag_event = -1;
					}
					else if (edit_mode == EM_EVENTS && drag_event != -1)
					{
						drag_event = -1;
					}
				}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					drag_start_x = e.button.x / domain->scale;
					drag_start_y = e.button.y / domain->scale;
					if (edit_mode == EM_LEVEL)
					{
						selected_event = drag_event = -1;
						if (e.button.button == (SDL_BUTTON_LEFT))
						{
							const Uint8 * keys = SDL_GetKeyboardState(NULL);
							if (keys[SDL_SCANCODE_LSHIFT]||keys[SDL_SCANCODE_RSHIFT])
							{
								drag_x = e.button.x / domain->scale;
								drag_y = e.button.y / domain->scale;
							}
							else
							{
								set_tile(e.button.x / domain->scale, e.button.y / domain->scale);
							}
						}
						if ((e.button.button == (SDL_BUTTON_LEFT) && current_layer == MAGICK_LAYER))
						{
							get_tile(e.button.x / domain->scale, e.button.y / domain->scale);
						}
					}
					else
					{
						const Uint8 * keys = SDL_GetKeyboardState(NULL);
						if (selected_event != -1 && (keys[SDL_SCANCODE_LCTRL]||keys[SDL_SCANCODE_RCTRL]))
						{
							level.event[selected_event].param[(keys[SDL_SCANCODE_LSHIFT]||keys[SDL_SCANCODE_RSHIFT])?EV_TRGPARENT:EV_NEXT] = get_event(e.button.x / domain->scale, e.button.y / domain->scale);
						}
						else
						{
							drag_event = selected_event = get_event(e.button.x / domain->scale, e.button.y / domain->scale);
							ev_drag_start_x = e.button.x / CELLSIZE / domain->scale;
							ev_drag_start_y = e.button.y / CELLSIZE / domain->scale;
						}
					}
				break;
				
				
				case SDL_KEYDOWN:
				
					if ((e.key.keysym.mod & KMOD_SHIFT) && (e.key.keysym.mod & KMOD_CTRL))
					{
						if (e.key.keysym.sym == SDLK_i)
						{
							int x,y;
							SDL_GetMouseState(&x, &y);
							insert_rowcol((x / domain->scale + scroll_x) / CELLSIZE, (y / domain->scale + scroll_y) / CELLSIZE, 1, 0);
						}
						else if (edit_mode == EM_EVENTS)
						{
							switch (e.key.keysym.sym)
							{
								default:break;
								case SDLK_UP:
								
								shift_events(0,-1);
								
								break;
								
								case SDLK_DOWN:
								
								shift_events(0,1);
								
								break;
								
								case SDLK_LEFT:
								
								shift_events(-1,0);
								
								break;
								
								case SDLK_RIGHT:
								
								shift_events(1,0);
								
								break;
							}
						} 
						else
						{
							switch (e.key.keysym.sym)
							{
								default:break;
								case SDLK_UP:
								
								shift_layer(0,-1);
								
								break;
								
								case SDLK_DOWN:
								
								shift_layer(0,1);
								
								break;
								
								case SDLK_LEFT:
								
								shift_layer(-1,0);
								
								break;
								
								case SDLK_RIGHT:
								
								shift_layer(1,0);
								
								break;
							}
						}
					}
					else if (e.key.keysym.mod & KMOD_CTRL)
					{
						if (e.key.keysym.sym == SDLK_F9)
							level_export(&level);
						else if (e.key.keysym.sym == SDLK_s)
							save_dialog();
						else if (e.key.keysym.sym == SDLK_o)
							load_dialog();
						else if (e.key.keysym.sym == SDLK_i)
						{
							int x,y;
							SDL_GetMouseState(&x, &y);
							insert_rowcol((x / domain->scale + scroll_x) / CELLSIZE, (y / domain->scale + scroll_y) / CELLSIZE, 0, 1);
						}
						else if (edit_mode == EM_EVENTS)
						{
							switch (e.key.keysym.sym)
							{
								default:break;
								case SDLK_UP:
								
								resize_event(0,-1);
								
								break;
								
								case SDLK_DOWN:
								
								resize_event(0,1);
								
								break;
								
								case SDLK_LEFT:
								
								resize_event(-1,0);
								
								break;
								
								case SDLK_RIGHT:
								
								resize_event(1,0);
								
								break;
							}
						}
						else
						{
							switch (e.key.keysym.sym)
							{
								case SDLK_UP:
								
								if (level.layer[current_layer].h > 0)
									resize_layer(level.layer[current_layer].w, level.layer[current_layer].h-1);
								
								break;
								
								case SDLK_DOWN:
								
								resize_layer(level.layer[current_layer].w, level.layer[current_layer].h+1);
								
								break;
								
								case SDLK_LEFT:
								
								if (level.layer[current_layer].w > 0)
									resize_layer(level.layer[current_layer].w-1, level.layer[current_layer].h);
								
								break;
								
								case SDLK_RIGHT:
								
								resize_layer(level.layer[current_layer].w+1, level.layer[current_layer].h);
								
								break;
								
								case SDLK_F10:
								clear_layer();
								break;
								
								default:
								break;
							}
						}
					}
					else 
					{
						if (edit_mode == EM_LEVEL)
						{
							switch (e.key.keysym.sym)
							{
								case SDLK_h:
									double_layer();
								break;
								
								case SDLK_PAGEUP:
								{
									if (current_layer > 0)
									{
										swap(&level.layer[current_layer], &level.layer[current_layer-1], sizeof(level.layer[current_layer]));
										--current_layer;
									}
								}
								break;
								
								case SDLK_PAGEDOWN:
								{
									if (current_layer < MAX_LAYERS - 1)
									{
										swap(&level.layer[current_layer], &level.layer[current_layer+1], sizeof(level.layer[current_layer]));
										++current_layer;
									}
								}
								break;
								
								case SDLK_e:
									edit_mode = edit_mode == EM_LEVEL ? EM_EVENTS : EM_LEVEL;
								break;
							
								case SDLK_1:
								case SDLK_2:
								case SDLK_3:
								case SDLK_4:
								case SDLK_5:
								case SDLK_6:
								case SDLK_7:
								case SDLK_8:
															
								current_layer = e.key.keysym.sym-SDLK_1;
								
								break;
								
								case SDLK_b:
															
								current_layer = BRUSH_LAYER;
								
								break;
								
								case SDLK_RALT:
								case SDLK_LALT:
									if (e.key.repeat)
										break;
									saved_layer = current_layer;
									current_layer = MAGICK_LAYER;
								break;
								
								case SDLK_UP:
								
								scroll_y -= 4;
								
								break;
								
								case SDLK_DOWN:
								
								scroll_y += 4;
								
								break;
								
								case SDLK_LEFT:
								
								scroll_x -= 4;
								
								break;
								
								case SDLK_RIGHT:
								
								scroll_x += 4;
								
								break;
								
								case SDLK_m:
								
								level.layer[current_layer].prx_mlt_x = (level.layer[current_layer].prx_mlt_x + 1) % 17;
								
								break;
								
								case SDLK_n:
								
								level.layer[current_layer].prx_mlt_y = (level.layer[current_layer].prx_mlt_y + 1) % 17;
								
								break;
								
								case SDLK_p:
								
								set_flags(BG_PARALLAX);
								
								break;
								
								case SDLK_x:
								
								set_flags(BG_REPEAT_X);
								
								break;
								
								case SDLK_y:
								
								set_flags(BG_REPEAT_Y);
								
								break;
								
								default: break;
							}
						}
						else
						{
							
							{
								switch (e.key.keysym.sym)
								{
									case SDLK_e:
										edit_mode = edit_mode == EM_LEVEL ? EM_EVENTS : EM_LEVEL;
									break;
									
									case SDLK_INSERT:
									{
										int x,y;
										SDL_GetMouseState(&x, &y);
										add_event(x / domain->scale, y / domain->scale);
									}
									break;
									
									case SDLK_DELETE:
									
									delete_event();
									
									break;
									
									
									case SDLK_PERIOD:
									case SDLK_COMMA:
										if (selected_event == -1) break;
										
										level.event[selected_event].param[selected_param] += e.key.keysym.sym == SDLK_COMMA ? -1 : 1;
										
										if (selected_param == 0)
										{
											level.event[selected_event].param[selected_param] = my_max(0, my_min(n_ev_desc - 1,level.event[selected_event].param[selected_param]));
											
											for (int i = 1 ; i < EV_PARAMS - 2 ; ++i)
												level.event[selected_event].param[i] = my_max(ev_desc[level.event[selected_event].param[0]].param[i].int_min, 
													my_min(ev_desc[level.event[selected_event].param[0]].param[i].int_max ,level.event[selected_event].param[i]));
										}
										else
										{
											if (selected_param != EV_NEXT && selected_param != EV_TRGPARENT)
												level.event[selected_event].param[selected_param] = my_max(ev_desc[level.event[selected_event].param[0]].param[selected_param].int_min, 
													my_min(ev_desc[level.event[selected_event].param[0]].param[selected_param].int_max ,level.event[selected_event].param[selected_param]));
										}
									break;
									
									case SDLK_PAGEUP:
										selected_param = (selected_param - 1) & (EV_PARAMS-1);
									break;
									
									case SDLK_PAGEDOWN:
										selected_param = (selected_param + 1) & (EV_PARAMS-1);
									break;
									
									case SDLK_UP:
									
									scroll_y -= 4;
									
									break;
									
									case SDLK_DOWN:
									
									scroll_y += 4;
									
									break;
									
									case SDLK_LEFT:
									
									scroll_x -= 4;
									
									break;
									
									case SDLK_RIGHT:
									
									scroll_x += 4;
									
									break;
									
									default: break;
								}
							}
						}
					}
				
				break;
				
				case SDL_KEYUP:
					if(edit_mode == EM_LEVEL)
					{
						switch (e.key.keysym.sym)
						{
							
							case SDLK_RALT:
							case SDLK_LALT:
								current_layer = saved_layer;
							break;
							
							default: break;
						}
					}
				break;
			}
		}
		
		if (got_event)
		{
			
			const Uint8 * keys = SDL_GetKeyboardState(NULL);
			int x,y;
			SDL_GetMouseState(&x, &y);
			
			int show_all_layers = keys[SDL_SCANCODE_A];
			
			gfx_rect(domain, NULL, bg_color);
			draw(domain, x / domain->scale, y / domain->scale, show_all_layers);
			
			static const char *layer_names[] =
			{
				"Tiles",
				"Brush"
			};
			
			
			char text[100], si[10];
			SDL_Rect textpos = {domain->screen_w - 400,0, 1000, 1000};
			
			if (edit_mode == EM_LEVEL)
			{
				//if (current_layer < MAGICK_LAYER)
				{
					sprintf(si, "%d", current_layer);
					sprintf(text, "[L %s] prx(%s) pos(%d,%d) size(%dx%d,%dx%d)\n", show_all_layers?"All":(current_layer>=MAGICK_LAYER?layer_names[current_layer-MAGICK_LAYER]:si), level.layer[current_layer].flags&BG_PARALLAX?"ON":"OFF", scroll_x/CELLSIZE, scroll_y/CELLSIZE, level.layer[current_layer].w, level.layer[current_layer].prx_mlt_x, level.layer[current_layer].h, level.layer[current_layer].prx_mlt_y);
					
					font_write(&font, domain, &textpos, text);
					textpos.y += font.h;
				}
			}
			else
			{
				if (selected_event == -1)
				{
					sprintf(text, "[EV] pos(%d,%d)\n", scroll_x/CELLSIZE, scroll_y/CELLSIZE);
					font_write(&font, domain, &textpos, text);
					textpos.y += font.h;
				}
				else
				{
					sprintf(text, "[EV:%02x(%d,%d %d,%d)] pos(%d,%d)\n", selected_event, level.event[selected_event].x, level.event[selected_event].y, level.event[selected_event].w, level.event[selected_event].h, scroll_x/CELLSIZE, scroll_y/CELLSIZE);
					font_write(&font, domain, &textpos, text);
					textpos.y += font.h;
					
					for (int i = 0 ; i < EV_PARAMS ; ++i)
					{
						char s = ' ';
						
						if (i == selected_param) s = '½';
						
						if (i == 0)
						{
							snprintf(text, 100, "%-9s: %s", "Type", ev_desc[level.event[selected_event].param[i]].name);
						}
						else if (i == EV_TRGPARENT)
						{
							snprintf(text, 100, "%-9s: %4d", "TrgParent", level.event[selected_event].param[i]);
						}
						else if (i == EV_NEXT)
						{
							snprintf(text, 100, "%-9s: %4d", "Next", level.event[selected_event].param[i]);
						}
						else
						{
							if (ev_desc[level.event[selected_event].param[0]].param[i].type == EVPAR_ENUM)
								snprintf(text, 100, "%-9s: %s", ev_desc[level.event[selected_event].param[0]].param[i].name, ev_desc[level.event[selected_event].param[0]].param[i].enums[level.event[selected_event].param[i]]);
							else
								snprintf(text, 100, "%-9s: %4d", ev_desc[level.event[selected_event].param[0]].param[i].name, level.event[selected_event].param[i]);
						}
					
						font_write_args(&font, domain, &textpos, "%c%s", s, text);
						textpos.y += font.h;
					}
				}
				
				
			}
				
			gfx_domain_flip(domain);
		}
		else
		{
			SDL_Delay(1);
		}
		
		if (done) 
		{
			int r = confirm_ync(domain, gfx, &font, "Save level?");
			
			if (r == 0) done = 0;
			if (r == -1) goto out;
			if (r == 1) { if (!save_dialog()) done = 0; else break; }
		}
	}
	
	out:
	
	font_destroy(&font);
	config_destroy(&cfg);
	free(ev_desc);
		
	return 0;
}
