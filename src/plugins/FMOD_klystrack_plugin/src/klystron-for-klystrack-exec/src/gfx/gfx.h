#ifndef GFX_H
#define GFX_H

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
#include "SDL_rwops.h"
#include "tiledescriptor.h"
#include "gfxsurf.h"
#include <stdbool.h>

#ifdef USESDL_GPU
#include "SDL_gpu.h"
#endif

enum
{
	GFX_KEYED = 1,
	GFX_ALPHA = 2,
	GFX_TRANS_HALF = 4,
	GFX_COL_MASK = 8
};

enum
{
	GFX_DOMAIN_DISABLE_RENDER_TO_TEXTURE = 1
};

typedef enum
{
	GFX_SCALE_NEAREST,
	GFX_SCALE_SCANLINES
} GfxScaleType;

#ifdef WIN32
typedef Uint64 FramerateTimer;
#else
typedef Uint32 FramerateTimer;
#endif

struct GfxDomain_t
{
#ifdef USESDL_GPU
	GPU_Target *screen;
	SDL_Rect clip;
#else
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *scale_texture, *scanlines_texture;
	bool render_to_texture;
#endif
	int screen_w, screen_h;
	int window_w, window_h, window_min_w, window_min_h;
	int scale, fullscreen, fps;
	GfxScaleType scale_type;
	FramerateTimer start_time, dt, clock_resolution, accumulator;
	int flags;
	Uint8 texmod_r, texmod_g, texmod_b;
#ifdef DEBUG
	int calls_per_frame;
#endif
};

typedef struct GfxDomain_t GfxDomain;

GfxSurface* gfx_load_surface(GfxDomain *domain, const char* filename, const int flags);
GfxSurface* gfx_load_surface_RW(GfxDomain *domain, SDL_RWops *rw, const int flags);
int * gfx_build_collision_mask(SDL_Surface *s);
GfxSurface * gfx_create_surface(GfxDomain *domain, int w, int h);
void gfx_free_surface(GfxSurface *surface);
void gfx_blit_2x(SDL_Surface *dest, SDL_Surface *src);
void gfx_blit_2x_resample(SDL_Surface *dest, SDL_Surface *src);
void gfx_blit_3x(SDL_Surface *dest, SDL_Surface *src);
void gfx_blit_3x_resample(SDL_Surface *dest, SDL_Surface *src);
void gfx_blit_4x(SDL_Surface *dest, SDL_Surface *src);
void gfx_raster(SDL_Surface *dest, const SDL_Rect *rect, const Uint32 *colors, int len);
void gfx_generate_raster(Uint32 *dest, const Uint32 from, const Uint32 to, int len);
TileDescriptor *gfx_build_tiledescriptor(GfxSurface *tiles, const int cellwidth, const int cellheight, int *out_n_tiles);
void gfx_line(GfxDomain *dest, int x0, int y0, int x1, int y1, Uint32 color);
void gfx_circle_inverted(SDL_Surface *dest, const int xc, const int yc, const int r, const Uint32 color);
void gfx_circle(SDL_Surface *dest, const int xc, const int yc, const int r, const Uint32 color);
void gfx_clear(GfxDomain *domain, Uint32 color);
void gfx_blit(GfxSurface *src, SDL_Rect *srcrect, GfxDomain *domain, SDL_Rect *dest);
void gfx_rect(GfxDomain *domain, SDL_Rect *dest, Uint32 rgb);
void gfx_surface_set_color(GfxSurface *surf, Uint32 color);
void gfx_update_texture(GfxDomain *domain, GfxSurface *surface);
void gfx_convert_mouse_coordinates(GfxDomain *domain, int *x, int *y);

GfxDomain * gfx_create_domain(const char *title, Uint32 window_flags, int window_w, int window_h, int scale);
void gfx_domain_update(GfxDomain *domain, bool resize_window);
void gfx_domain_flip(GfxDomain *domain);
void gfx_domain_free(GfxDomain *domain);
int gfx_domain_is_next_frame(GfxDomain *domain);

void gfx_domain_set_clip(GfxDomain *domain, const SDL_Rect *rect);
void gfx_domain_get_clip(GfxDomain *domain, SDL_Rect *rect);

//
void my_BlitSurface(GfxSurface *src, SDL_Rect *src_rect, GfxDomain *dest, SDL_Rect *dest_rect);

#endif
