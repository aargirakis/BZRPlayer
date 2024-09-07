/*
Copyright (c) 2009-2011 Tero Lindeman (kometbomb)

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

#include <assert.h>
#include "gfx.h"
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef USESDL_IMAGE
#include "SDL_image.h"
#endif


// gives the optimizer an opportunity to vectorize the second loop

#define vechelper(width, block)\
	{\
		const int w = width & 0xffffff00;\
		int x;\
		for (x = 0 ; x < w ; ++x)\
		{\
			block;\
		}\
		for ( ; x < width ; ++x)\
		{\
			block;\
		}\
	}

int * gfx_build_collision_mask(SDL_Surface *s)
{
	int * mask = malloc(sizeof(int) * s->w * s->h);

#if SDL_VERSION_ATLEAST(1,3,0)
	Uint32 key;
	SDL_GetColorKey(s, &key);
#else
	const Uint32 key = s->format->colorkey;
#endif

	my_lock(s);
	for (int y = 0 ; y < s->h ; ++y)
	{
		Uint8 *pa = (Uint8 *)s->pixels + y * s->pitch;

		for (int x = 0 ; x < s->w ; ++x)
		{
			int p = 0;

			switch (s->format->BytesPerPixel)
			{
				default:
				case 1: p = ((*((Uint8*)pa))) != key; break;
				case 2: p = ((*((Uint16*)pa))&0xffff) != key; break;
				case 3: p = ((*((Uint32*)pa))&0xffffff) != key; break;
				case 4: p = ((*((Uint32*)pa))&0xffffff) != key; break;
			}

			mask[x + y * s->w] = p;
			pa += s->format->BytesPerPixel;
		}
	}
	my_unlock(s);

	return mask;
}


GfxSurface* gfx_load_surface(GfxDomain *domain, const char* filename, const int flags)
{
	SDL_RWops * rw = SDL_RWFromFile(filename, "rb");

	if (rw)
	{
		GfxSurface * s = gfx_load_surface_RW(domain, rw, flags);
		return s;
	}
	else
	{
		warning("Opening image '%s' failed", filename);
		return NULL;
	}
}


GfxSurface* gfx_load_surface_RW(GfxDomain *domain, SDL_RWops *rw, const int flags)
{
#ifdef USESDL_IMAGE
	SDL_Surface* loaded = IMG_Load_RW(rw, 1);
#else
	SDL_Surface* loaded = SDL_LoadBMP_RW(rw, 1);
#endif

	if (!loaded)
	{
		warning("Loading surface failed: %s", SDL_GetError());
		return NULL;
	}

	GfxSurface *gs = calloc(1, sizeof(GfxSurface));

	if (flags & GFX_KEYED)
	{
		Uint32 c = SDL_MapRGB(loaded->format, 255, 0, 255);
		Uint8 r, g, b;
		SDL_GetRGB(c, loaded->format, &r, &g, &b);

		if (r == 255 && g == 0 && b == 255)
			SDL_SetColorKey(loaded, SDL_TRUE, c);

#ifdef USESDL_GPU
		SDL_Surface *conv = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ARGB8888, 0);
		SDL_FreeSurface(loaded);
		loaded = conv;
#endif
	}

	gs->surface = loaded;

	if (flags & GFX_COL_MASK) gs->mask = gfx_build_collision_mask(gs->surface);

	gs->flags = flags;

	gfx_update_texture(domain, gs);

	return gs;
}


void gfx_blit_2x(SDL_Surface *dest, SDL_Surface *src)
{

	for (int y = 0 ; y < src->h ; y ++)
	{
		unsigned int *dptr1 = dest->pixels+dest->pitch*(y*2);
		unsigned int *dptr2 = dest->pixels+dest->pitch*(y*2+1);
		unsigned int *sptr = src->pixels+src->pitch*y;
		for (int x = src->w ; x != 0  ; --x)
		{
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *(sptr++);
		}
	}
}


void gfx_blit_3x(SDL_Surface *dest, SDL_Surface *src)
{

	for (int y = 0 ; y < src->h ; y ++)
	{
		unsigned int *dptr1 = dest->pixels+dest->pitch*(y*3);
		unsigned int *dptr2 = dest->pixels+dest->pitch*(y*3+1);
		unsigned int *dptr3 = dest->pixels+dest->pitch*(y*3+2);
		unsigned int *sptr = src->pixels+src->pitch*y;
		for (int x = src->w ; x != 0  ; --x)
		{
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr3++) = *(sptr++);
		}
	}
}


void gfx_blit_4x(SDL_Surface *dest, SDL_Surface *src)
{

	for (int y = 0 ; y < src->h ; y ++)
	{
		unsigned int *dptr1 = dest->pixels+dest->pitch*(y*4);
		unsigned int *dptr2 = dest->pixels+dest->pitch*(y*4+1);
		unsigned int *dptr3 = dest->pixels+dest->pitch*(y*4+2);
		unsigned int *dptr4 = dest->pixels+dest->pitch*(y*4+3);
		unsigned int *sptr = src->pixels+src->pitch*y;
		for (int x = src->w ; x != 0  ; --x)
		{
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr1++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr2++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr3++) = *sptr;
			*(dptr4++) = *sptr;
			*(dptr4++) = *sptr;
			*(dptr4++) = *sptr;
			*(dptr4++) = *(sptr++);
		}
	}
}


void gfx_raster(SDL_Surface *dest, const SDL_Rect *rect, const Uint32 *colors, int len)
{
	for (int y = rect ? rect->y : 0, i = 0 ; y < (rect ? rect->h+rect->y : dest->h) && i < len ; ++i, ++y)
	{
		SDL_Rect r = {rect ? rect->x : 0, y, rect ? rect->w : dest->w, 1};
		SDL_FillRect(dest, &r, colors[i]);
	}
}


void gfx_generate_raster(Uint32 *dest, const Uint32 from, const Uint32 to, int len)
{
	for (int i = 0 ; i < len ; ++i)
	{
		int from_r = ((Uint8*)&from)[0];
		int from_g = ((Uint8*)&from)[1];
		int from_b = ((Uint8*)&from)[2];
		int to_r = ((Uint8*)&to)[0];
		int to_g = ((Uint8*)&to)[1];
		int to_b = ((Uint8*)&to)[2];

		*(dest++) = ((from_r + (to_r-from_r) * i / len) & 0xff)
					| (((from_g + (to_g-from_g) * i / len) & 0xff) << 8)
					| (((from_b + (to_b-from_b) * i / len) & 0xff) << 16)
					;
	}
}


static int has_pixels(TileDescriptor *desc)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	Uint32 key;
	SDL_GetColorKey(desc->surface->surface, &key);
#else
	const Uint32 key = desc->surface->surface->format->colorkey;
#endif

	my_lock(desc->surface->surface);

	int result = 0;

	for (int y = 0 ; y < desc->rect.h && y + desc->rect.y < desc->surface->surface->h ; ++y)
	{
		Uint8 *p = (Uint8 *)desc->surface->surface->pixels + ((int)desc->rect.y + y) * desc->surface->surface->pitch + (int)desc->rect.x * desc->surface->surface->format->BytesPerPixel;

		for (int x = 0 ; x < desc->rect.w && x + desc->rect.x < desc->surface->surface->w ; ++x)
		{
			Uint32 c = 0;

			switch (desc->surface->surface->format->BytesPerPixel)
			{
				case 1:
					c = *((Uint8*)p);
					break;

				case 2:
					c = *((Uint16*)p);
					break;

				case 3:
					c = *((Uint8*)p) | (*((Uint16*)&p[1]) << 8);
					break;

				default:
				case 4:
					c = *((Uint32*)p);
					break;
			}

			if ((c & 0xffffff) != key)
			{
				++result;
			}

			p+=desc->surface->surface->format->BytesPerPixel;
		}
	}

	my_unlock(desc->surface->surface);

	return result;
}


TileDescriptor *gfx_build_tiledescriptor(GfxSurface *tiles, const int cellwidth, const int cellheight, int *out_n_tiles)
{
	TileDescriptor *descriptor = calloc(sizeof(*descriptor), (tiles->surface->w/cellwidth)*(tiles->surface->h/cellheight));

	int n_tiles = 0;

	for (int i = 0 ; i < (tiles->surface->w/cellwidth)*(tiles->surface->h/cellheight) ; ++i)
	{
		descriptor[i].rect.x = i*cellwidth;
		descriptor[i].rect.y = 0;

		while (descriptor[i].rect.x >= tiles->surface->w)
		{
			descriptor[i].rect.y += cellheight;
			descriptor[i].rect.x -= tiles->surface->w;
		}

		descriptor[i].rect.w = cellwidth;
		descriptor[i].rect.h = cellheight;
		descriptor[i].surface = tiles;

		int pixels = has_pixels(&descriptor[i]);

		if (pixels == CELLSIZE*CELLSIZE || !(tiles->flags & GFX_COL_MASK))
			descriptor[i].flags = TILE_COL_NORMAL;
		else if (pixels > 0)
			descriptor[i].flags = TILE_COL_PIXEL;
		else descriptor[i].flags = TILE_COL_DISABLE;

		n_tiles++;
	}

	if (out_n_tiles)
		*out_n_tiles = n_tiles;

	return descriptor;
}


#define SWAP(x,y) {int t = x; x = y; y=t;}

void gfx_line(GfxDomain *dest, int x0, int y0, int x1, int y1, Uint32 color)
{
#ifdef USESDL_GPU
	SDL_Color c = {(color >> 16) & 255, (color >> 8) & 255, color & 255, 255};
	GPU_Line(dest->screen, x0, y0, x1, y1, c);
#else
	SDL_SetRenderDrawColor(dest->renderer, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
	SDL_RenderDrawLine(dest->renderer, x0, y0, x1, y1);
#endif

}


static void scale2x(SDL_Surface *src, SDL_Surface *dst)
{
	int looph, loopw;

	Uint8* srcpix = (Uint8*)src->pixels;
	Uint8* dstpix = (Uint8*)dst->pixels;

	const int srcpitch = src->pitch;
	const int dstpitch = dst->pitch;
	const int width = src->w;
	const int height = src->h;

	Uint32 *E0, *E1, *E2, *E3;
	Uint32 B, D, E, F, H;
	for(looph = 0; looph < height; ++looph)
	{
		E0 = (Uint32*)(dstpix + looph*2*dstpitch + 0*2*4);
		E1 = (Uint32*)(dstpix + looph*2*dstpitch + (0*2+1)*4);
		E2 = (Uint32*)(dstpix + (looph*2+1)*dstpitch + 0*2*4);
		E3 = (Uint32*)(dstpix + (looph*2+1)*dstpitch + (0*2+1)*4);

		for(loopw = 0; loopw < width; ++ loopw)
		{
			B = *(Uint32*)(srcpix + (my_max(0,looph-1)*srcpitch) + (4*loopw));
			D = *(Uint32*)(srcpix + (looph*srcpitch) + (4*my_max(0,loopw-1)));
			E = *(Uint32*)(srcpix + (looph*srcpitch) + (4*loopw));
			F = *(Uint32*)(srcpix + (looph*srcpitch) + (4*my_min(width-1,loopw+1)));
			H = *(Uint32*)(srcpix + (my_min(height-1,looph+1)*srcpitch) + (4*loopw));

			if (B != H && D != F)
			{
				*E0 = D == B ? D : E;
				*E1 = B == F ? F : E;
				*E2 = D == H ? D : E;
				*E3 = H == F ? F : E;
			}
			else
			{
				*E0 = E;
				*E1 = E;
				*E2 = E;
				*E3 = E;
			}


			E0 += 2;
			E1 += 2;
			E2 += 2;
			E3 += 2;
		}
	}

}


static void scale3x(SDL_Surface *src, SDL_Surface *dst)
{
	int looph, loopw;

	Uint8* srcpix = (Uint8*)src->pixels;
	Uint8* dstpix = (Uint8*)dst->pixels;

	const int srcpitch = src->pitch;
	const int dstpitch = dst->pitch;
	const int width = src->w;
	const int height = src->h;

	Uint32 *E0, *E1, *E2, *E3, *E4, *E5, *E6, *E7, *E8, A, B, C, D, E, F, G, H, I;
	for(looph = 0; looph < height; ++looph)
	{
		E0 = (Uint32*)(dstpix + looph*3*dstpitch + 0*3*4);
		E1 = (Uint32*)(dstpix + looph*3*dstpitch + (0*3+1)*4);
		E2 = (Uint32*)(dstpix + looph*3*dstpitch + (0*3+2)*4);
		E3 = (Uint32*)(dstpix + (looph*3+1)*dstpitch + 0*3*4);
		E4 = (Uint32*)(dstpix + (looph*3+1)*dstpitch + (0*3+1)*4);
		E5 = (Uint32*)(dstpix + (looph*3+1)*dstpitch + (0*3+2)*4);
		E6 = (Uint32*)(dstpix + (looph*3+2)*dstpitch + 0*3*4);
		E7 = (Uint32*)(dstpix + (looph*3+2)*dstpitch + (0*3+1)*4);
		E8 = (Uint32*)(dstpix + (looph*3+2)*dstpitch + (0*3+2)*4);

		for(loopw = 0; loopw < width; ++ loopw)
		{
				A = *(Uint32*)(srcpix + (my_max(0,looph-1)*srcpitch) + (4*my_max(0,loopw-1)));
				B = *(Uint32*)(srcpix + (my_max(0,looph-1)*srcpitch) + (4*loopw));
				C = *(Uint32*)(srcpix + (my_max(0,looph-1)*srcpitch) + (4*my_min(width-1,loopw+1)));
				D = *(Uint32*)(srcpix + (looph*srcpitch) + (4*my_max(0,loopw-1)));
				E = *(Uint32*)(srcpix + (looph*srcpitch) + (4*loopw));
				F = *(Uint32*)(srcpix + (looph*srcpitch) + (4*my_min(width-1,loopw+1)));
				G = *(Uint32*)(srcpix + (my_min(height-1,looph+1)*srcpitch) + (4*my_max(0,loopw-1)));
				H = *(Uint32*)(srcpix + (my_min(height-1,looph+1)*srcpitch) + (4*loopw));
				I = *(Uint32*)(srcpix + (my_min(height-1,looph+1)*srcpitch) + (4*my_min(width-1,loopw+1)));

			if (B != H && D != F) {
				*E0 = D == B ? D : E;
				*E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
				*E2 = B == F ? F : E;
				*E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
				*E4 = E;
				*E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
				*E6 = D == H ? D : E;
				*E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
				*E8 = H == F ? F : E;
			} else {
				*E0 = E;
				*E1 = E;
				*E2 = E;
				*E3 = E;
				*E4 = E;
				*E5 = E;
				*E6 = E;
				*E7 = E;
				*E8 = E;
			}


			E0 += 3;
			E1 += 3;
			E2 += 3;
			E3 += 3;
			E4 += 3;
			E5 += 3;
			E6 += 3;
			E7 += 3;
			E8 += 3;
		}
	}
}


void gfx_blit_2x_resample(SDL_Surface *dest, SDL_Surface *src)
{
	scale2x(src,dest);
}

void gfx_blit_3x_resample(SDL_Surface *dest, SDL_Surface *src)
{
	scale3x(src,dest);
}


void gfx_circle(SDL_Surface *dest, const int xc, const int yc, const int r, const Uint32 color)
{
	const int h = my_min(yc+r+1, dest->h);
	for (int y=my_max(0,yc-r); y<h; ++y)
	{
		const int w = (int)(sqrt(SQR(r) - SQR(y - yc)));
		SDL_Rect r = {xc - w, y, w*2, 1};
		SDL_FillRect(dest, &r, color);
	}
}

void gfx_circle_inverted(SDL_Surface *dest, const int xc, const int yc, const int r, const Uint32 color)
{
	const int h = my_min(yc+r+1, dest->h);
	for (int y=my_max(0,yc-r); y<h; ++y)
	{
		const int w = (int)(sqrt(SQR(r) - SQR(y - yc)));

		{
			SDL_Rect rect = {xc - r, y, r - w, 1};
			SDL_FillRect(dest, &rect, color);
		}

		{
			SDL_Rect rect = {xc + w, y, r - w, 1};
			SDL_FillRect(dest, &rect, color);
		}
	}
}


static void gfx_domain_set_framerate(GfxDomain *d)
{
#ifdef WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&d->clock_resolution);
	QueryPerformanceCounter((LARGE_INTEGER*)&d->start_time);
#else
	d->clock_resolution = 1000;
	d->start_time = SDL_GetTicks();
#endif

	d->dt = d->clock_resolution / d->fps;
	d->accumulator = 0;
}


#ifndef USESDL_GPU
static void create_scanlines_texture(GfxDomain *domain)
{
	if (domain->scanlines_texture)
		SDL_DestroyTexture(domain->scanlines_texture);

	SDL_Surface *temp = SDL_CreateRGBSurface(0, 8, domain->screen_h * domain->scale, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);

	// alpha = 1.0
	SDL_FillRect(temp, NULL, 0x00ffffff);

	debug("%d", domain->scale);

	const int depth = 32 * domain->scale;

	for (int y = 0 ; y < domain->screen_h * domain->scale ; ++y)
	{
		SDL_Rect r = {0, y, 8, 1};
		Uint8 c = fabs(sin((float)y * M_PI / domain->scale)) * depth + (255 - depth);

		SDL_FillRect(temp, &r, (c << 8) | (c << 16) | c);
	}

	domain->scanlines_texture = SDL_CreateTextureFromSurface(domain->renderer, temp);

	SDL_SetTextureBlendMode(domain->scanlines_texture, SDL_BLENDMODE_MOD);

	SDL_FreeSurface(temp);
}
#endif


void gfx_domain_update(GfxDomain *domain, bool resize_window)
{
	debug("Setting screen mode (scale = %d%s)", domain->scale, domain->fullscreen ? ", fullscreen" : "");
#ifdef USESDL_GPU
	GPU_SetWindowResolution(domain->screen_w * domain->scale, domain->screen_h * domain->scale);
	GPU_SetVirtualResolution(domain->screen, domain->screen_w, domain->screen_h);

	domain->window_w = domain->screen_w * domain->scale;
	domain->window_h = domain->screen_h * domain->scale;

#else

	if (resize_window)
		SDL_SetWindowSize(domain->window, domain->screen_w * domain->scale, domain->screen_h * domain->scale);

	if (domain->fullscreen)
	{
		debug("Setting fullscreen");
		if (SDL_SetWindowFullscreen(domain->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
		{
			warning("Fullscreen failed: %s", SDL_GetError());

			if (SDL_SetWindowFullscreen(domain->window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
				warning("Fake fullscreen failed: %s", SDL_GetError());
		}
	}
	else
		SDL_SetWindowFullscreen(domain->window, 0);

	if (domain->render_to_texture && !(domain->flags & GFX_DOMAIN_DISABLE_RENDER_TO_TEXTURE) /* && domain->scale > 1*/)
	{
		debug("Rendering to texture enabled");

		SDL_SetRenderTarget(domain->renderer, NULL);

		if (domain->scale_texture)
			SDL_DestroyTexture(domain->scale_texture);

		domain->scale_texture = SDL_CreateTexture(domain->renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, domain->screen_w, domain->screen_h);

		if (!domain->scale_texture)
		{
			warning("Could not create texture: %s", SDL_GetError());
		}
		else
		{
			SDL_SetRenderTarget(domain->renderer, domain->scale_texture);
		}
	}
	else
	{
		debug("Rendering to texture disabled");

		SDL_SetRenderTarget(domain->renderer, NULL);
		SDL_RenderSetScale(domain->renderer, domain->scale, domain->scale);
	}

	if (domain->scale_type == GFX_SCALE_SCANLINES && domain->scale > 1)
		create_scanlines_texture(domain);

	SDL_RenderSetViewport(domain->renderer, NULL);

	SDL_SetWindowMinimumSize(domain->window, domain->window_min_w * domain->scale, domain->window_min_h * domain->scale);
	SDL_GetWindowSize(domain->window, &domain->window_w, &domain->window_h);
#endif

	debug("Screen size is %dx%d", domain->screen_w, domain->screen_h);
	debug("Window size is %dx%d", domain->window_w, domain->window_h);

	gfx_domain_set_framerate(domain);
}


void gfx_domain_flip(GfxDomain *domain)
{
#ifdef USESDL_GPU
    GPU_Flip(domain->screen);
#else
	if (domain->render_to_texture && !(domain->flags & GFX_DOMAIN_DISABLE_RENDER_TO_TEXTURE) /*&& domain->scale > 1*/)
	{
		SDL_RenderPresent(domain->renderer);

		SDL_SetRenderTarget(domain->renderer, NULL);
		SDL_RenderSetViewport(domain->renderer, NULL);
		SDL_RenderCopy(domain->renderer, domain->scale_texture, NULL, NULL);

		if (domain->scale_type == GFX_SCALE_SCANLINES && domain->scale > 1)
		{
			SDL_RenderCopy(domain->renderer, domain->scanlines_texture, NULL, NULL);
		}

		SDL_RenderPresent(domain->renderer);

		SDL_SetRenderTarget(domain->renderer, domain->scale_texture);
		SDL_RenderSetViewport(domain->renderer, NULL);
	}
	else
	{
		if (domain->scale_type == GFX_SCALE_SCANLINES && domain->scale > 1)
		{
			SDL_RenderCopy(domain->renderer, domain->scanlines_texture, NULL, NULL);
		}

		SDL_RenderPresent(domain->renderer);
	}

#endif

#ifdef DEBUG
	domain->calls_per_frame = 0;
#endif
}


void gfx_domain_free(GfxDomain *domain)
{
#ifdef USESDL_GPU
    GPU_Quit();
#else
	if (domain->scale_texture)
		SDL_DestroyTexture(domain->scale_texture);

	if (domain->scanlines_texture)
		SDL_DestroyTexture(domain->scanlines_texture);

	SDL_DestroyRenderer(domain->renderer);
	SDL_DestroyWindow(domain->window);
#endif

	free(domain);
}


GfxDomain * gfx_create_domain(const char *title, Uint32 window_flags, int window_w, int window_h, int scale)
{
	GfxDomain *d = malloc(sizeof(GfxDomain));
	d->screen_w = window_w / scale;
	d->screen_h = window_h / scale;
	d->scale = scale;
	d->scale_type = GFX_SCALE_NEAREST;
	d->fullscreen = 0;
	d->fps = 50;
	d->flags = 0;
	d->window_min_w = d->screen_w;
	d->window_min_h = d->screen_h;

#ifdef USESDL_GPU
	d->screen = GPU_Init(window_w, window_h, GPU_DEFAULT_INIT_FLAGS);
#else
	d->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, window_flags);
	d->renderer = SDL_CreateRenderer(d->window, -1, SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(d->renderer, &info);

	if (!(info.flags & SDL_RENDERER_TARGETTEXTURE))
	{
		warning("Renderer doesn't support rendering to texture");
		d->render_to_texture = false;
	}
	else
	{
		d->render_to_texture = true;
	}

	d->scale_texture = NULL;
	d->scanlines_texture = NULL;
#endif

#ifdef DEBUG

	d->calls_per_frame = 0;

#ifndef USESDL_GPU

	{
		SDL_RendererInfo info;
		SDL_GetRendererInfo(d->renderer, &info);

		debug("Renderer: %s (%s)", info.name, (info.flags & SDL_RENDERER_ACCELERATED) ? "Accelerated" : "Not accelerated");
	}

#endif

#endif

	gfx_domain_set_framerate(d);

	return d;
}


int gfx_domain_is_next_frame(GfxDomain *domain)
{
#ifdef WIN32
	Uint64 ticks;
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
#else
	Uint32 ticks = SDL_GetTicks();
#endif

	FramerateTimer frameTime = ticks - domain->start_time;
    domain->start_time = ticks;
	domain->accumulator += frameTime;
	int frames = 0;

    while ( domain->accumulator > domain->dt )
    {
        domain->accumulator -= domain->dt;
		++frames;
    }

	return frames;
}


GfxSurface * gfx_create_surface(GfxDomain *domain, int w, int h)
{
	GfxSurface *gs = calloc(1, sizeof(GfxSurface));
	gs->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	return gs;
}


void gfx_update_texture(GfxDomain *domain, GfxSurface *surface)
{
#ifdef USESDL_GPU
	if (!surface->texture)
		surface->texture = GPU_CreateImage(surface->surface->w, surface->surface->h, GPU_FORMAT_RGBA);

	GPU_UpdateImage(surface->texture, surface->surface, NULL);
	GPU_SetImageFilter(surface->texture, GPU_FILTER_NEAREST);
	GPU_SetSnapMode(surface->texture, GPU_SNAP_POSITION_AND_DIMENSIONS);
	GPU_SetBlending(surface->texture, 1);
	GPU_SetBlendMode(surface->texture, GPU_BLEND_NORMAL);
#else
	if (surface->texture)
		SDL_DestroyTexture(surface->texture);

	surface->texture = SDL_CreateTextureFromSurface(domain->renderer, surface->surface);
#endif
}


void gfx_free_surface(GfxSurface *surface)
{
#ifdef USESDL_GPU
	if (surface->texture) GPU_FreeImage(surface->texture);
#else
	if (surface->texture) SDL_DestroyTexture(surface->texture);
#endif
	if (surface->surface) SDL_FreeSurface(surface->surface);
	if (surface->mask) free(surface->mask);

	free(surface);
}


void gfx_clear(GfxDomain *domain, Uint32 color)
{
#ifdef USESDL_GPU
	GPU_ClearRGBA(domain->screen, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
#else
	SDL_SetRenderDrawColor(domain->renderer, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
	SDL_RenderClear(domain->renderer);
#endif
}



#define SCALEHELPER(test) do {\
		for (int dy = dest_rect->y, sy = 256 * (int)src_rect->y ; dy < dest_rect->h + dest_rect->y ; ++dy, sy += yspd)\
		{\
			Uint32 * dptr = (Uint32*)((Uint8*)dest->pixels + dy * dest->pitch) + dest_rect->x;\
			Uint32 * sptr = (Uint32*)((Uint8*)src->pixels + sy/256 * src->pitch);\
			\
			for (int dx = dest_rect->w, sx = 256 * (int)src_rect->x ; dx > 0 ; --dx, sx += xspd)\
			{\
				if (test) *dptr = sptr[sx / 256];\
				++dptr;\
			}\
		}\
	} while(0)


void gfx_blit(GfxSurface *_src, SDL_Rect *_src_rect, GfxDomain *domain, SDL_Rect *_dest_rect)
{
#ifdef USESDL_GPU
	GPU_Rect rect;

	if (_src_rect)
		rect = GPU_MakeRect(_src_rect->x, _src_rect->y, _src_rect->w, _src_rect->h);
	else
		rect = GPU_MakeRect(0, 0, _src->surface->w, _src->surface->h);

	GPU_BlitScale(_src->texture, &rect, domain->screen, _dest_rect->x + (float)_dest_rect->w / 2, _dest_rect->y + (float)_dest_rect->h / 2, (float)_dest_rect->w / rect.w, (float)_dest_rect->h / rect.h);
#else
	SDL_RenderCopy(domain->renderer, _src->texture,  _src_rect, _dest_rect);
#endif

#ifdef DEBUG
	domain->calls_per_frame++;
#endif
}


void gfx_rect(GfxDomain *domain, SDL_Rect *dest, Uint32 color)
{
#ifdef USESDL_GPU
	SDL_Color rgb = { (color >> 16) & 255, (color >> 8) & 255, color & 255, 255 };

	if (dest)
		GPU_RectangleFilled(domain->screen, dest->x, dest->y, dest->w + dest->x - 1, dest->y + dest->h - 1, rgb);
	else
		GPU_RectangleFilled(domain->screen, 0, 0, domain->screen_w, domain->screen_h, rgb);
#else
	SDL_SetRenderDrawColor(domain->renderer, (color >> 16) & 255, (color >> 8) & 255, color & 255, 255);
	SDL_RenderFillRect(domain->renderer, dest);
#endif
}


void my_BlitSurface(GfxSurface *src, SDL_Rect *src_rect, GfxDomain *dest, SDL_Rect *dest_rect)
{
	gfx_blit(src, src_rect, dest, dest_rect);
}


void gfx_domain_set_clip(GfxDomain *domain, const SDL_Rect *rect)
{
#ifdef USESDL_GPU
	if (rect)
		memcpy(&domain->clip, rect, sizeof(*rect));
	else
	{
		domain->clip.x = 0;
		domain->clip.y = 0;
		domain->clip.w = domain->screen_w;
		domain->clip.h = domain->screen_h;
	}

	GPU_SetClip(domain->screen, domain->clip.x, domain->clip.y, domain->clip.w, domain->clip.h);
#else
	SDL_RenderSetClipRect(domain->renderer, rect);
#endif
}


void gfx_domain_get_clip(GfxDomain *domain, SDL_Rect *rect)
{
#ifdef USESDL_GPU
	memcpy(rect, &domain->clip, sizeof(*rect));
#else
	SDL_RenderGetClipRect(domain->renderer, rect);
#endif
}


void gfx_surface_set_color(GfxSurface *surf, Uint32 color)
{
#ifdef USESDL_GPU
	GPU_SetRGB(surf->texture, (color >> 16) & 255, (color >> 8) & 255, color & 255);
#else
	SDL_SetTextureColorMod(surf->texture, (color >> 16) & 255, (color >> 8) & 255, color & 255);
#endif
}


void gfx_convert_mouse_coordinates(GfxDomain *domain, int *x, int *y)
{
	if (domain->window_w)
		*x = *x * domain->screen_w / domain->window_w;

	if (domain->window_h)
		*y = *y * domain->screen_h / domain->window_h;
}
