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


#include "font.h"
#include "gfx.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static int tile_width(TileDescriptor *desc)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	Uint32 key;
	SDL_GetColorKey(desc->surface->surface, &key);
#else
	const Uint32 key = desc->surface->surface->format->colorkey;
#endif

	my_lock(desc->surface->surface);

	int result = 0;

	for (int y = 0 ; y < desc->rect.h ; ++y)
	{
		Uint8 *pa = (Uint8 *)desc->surface->surface->pixels + ((int)desc->rect.y + y) * desc->surface->surface->pitch + (int)desc->rect.x * desc->surface->surface->format->BytesPerPixel;

		for (int x = 0 ; x < desc->rect.w ; ++x)
		{
			int p = 0;

			switch (desc->surface->surface->format->BytesPerPixel)
			{
				default:
				case 1: p = ((*((Uint8*)pa))) != key; break;
				case 2: p = ((*((Uint16*)pa))&0xffff) != key; break;
				case 3: p = ((*((Uint32*)pa))&0xffffff) != key; break;
				case 4: p = ((*((Uint32*)pa))&0xffffff) != key; break;
			}

			if (p)
			{
				result = my_max(result, x);
			}

			pa += desc->surface->surface->format->BytesPerPixel;
		}
	}

	my_unlock(desc->surface->surface);

	return result + 1;
}


static const TileDescriptor * findchar(const Font *font, char c)
{
	return font->ordered_tiles[(unsigned char)c];
}


static const TileDescriptor * findchar_slow(const Font *font, char c)
{
	if (c == ' ') return NULL;

	char *tc;
	const TileDescriptor *tile;
	for (tile = font->tiledescriptor, tc = font->charmap; *tc ; ++tile, ++tc)
		if (*tc == c) return tile;

	c = tolower(c);

	for (tile = font->tiledescriptor, tc = font->charmap; *tc ; ++tile, ++tc)
		if (tolower(*tc) == c) return tile;
	//debug("Could not find character '%c'", c);
	return NULL;
}


void font_create(Font *font, GfxSurface *tiles, const int w, const int h, const int char_spacing, const int space_width, char *charmap)
{
	font->tiledescriptor = gfx_build_tiledescriptor(tiles, w, h, NULL);
	font->charmap = strdup(charmap);
	font->w = w;
	font->h = h;
	font->char_spacing = char_spacing;
	font->space_width = space_width ? space_width : w;

	if (space_width)
	{
		for (int i = 0 ; i < (tiles->surface->w/w)*(tiles->surface->h/h) ; ++i)
			font->tiledescriptor[i].rect.w = tile_width(&font->tiledescriptor[i]);
	}

	for (int i = 0 ; i < 256 ; ++i)
	{
		font->ordered_tiles[i] = findchar_slow(font, i);
	}
}


void font_destroy(Font *font)
{
	if (font->tiledescriptor) free(font->tiledescriptor);
	if (font->charmap) free(font->charmap);
	if (font->surface) gfx_free_surface(font->surface);

	font->tiledescriptor = NULL;
	font->charmap = NULL;
	font->surface = NULL;
}


static void inner_write(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 * cursor, SDL_Rect *bounds, const char * text)
{
	const char *c = text;
	int x = (*cursor & 0xff) * font->w, y = ((*cursor >> 8) & 0xff) * font->h, cr = 0, right = dest->screen_w;

	if (r)
	{
		x = x + (cr = r->x);
		y = r->y + y;
		right = r->w + r->x;
	}

	for (;*c;++c)
	{
		if (*c == '\n' || x >= right)
		{
			y += font->h;
			x = cr;
			*cursor &= (Uint16)0xff00;
			*cursor += 0x0100;

			if (*c == '\n')
				continue;
		}

		const TileDescriptor *tile = findchar(font, *c);

		if (tile)
		{
			SDL_Rect rect = { x, y, tile->rect.w, tile->rect.h };
			my_BlitSurface(tile->surface, (SDL_Rect*)&tile->rect, dest, &rect);

			if (bounds)
			{
				if (bounds->w == 0)
					memcpy(bounds, &rect, sizeof(rect));
				else
				{
					bounds->x = my_min(rect.x, bounds->x);
					bounds->y = my_min(rect.y, bounds->y);
					bounds->w = my_max(rect.x + rect.w - bounds->x, bounds->w);
					bounds->h = my_max(rect.y + rect.h - bounds->y, bounds->h);
				}
			}

			x += tile->rect.w + font->char_spacing;
		}
		else
		{
			if (bounds)
			{
				if (bounds->w == 0)
				{
					bounds->x = x;
					bounds->y = y;
					bounds->w = font->w;
					bounds->h = font->h;
				}
				else
				{
					bounds->x = my_min(x, bounds->x);
					bounds->y = my_min(y, bounds->y);
					bounds->w = my_max(x + font->w - bounds->x, bounds->w);
					bounds->h = my_max(y + font->h - bounds->y, bounds->h);
				}
			}

			x += font->space_width;
		}

		++*cursor;
	}
}


void font_write_va(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 * cursor, SDL_Rect *bounds, const char * text, va_list va)
{
#ifdef USESTATICTEMPSTRINGS
   const int len = 2048;
   char formatted[len];
#else
   va_list va_cpy;
   va_copy( va_cpy, va );
   const int len = vsnprintf(NULL, 0, text, va_cpy) + 1;
   char * formatted = malloc(len * sizeof(*formatted));

   va_end( va_cpy );
#endif

	vsnprintf(formatted, len, text, va);

	inner_write(font, dest, r, cursor, bounds, formatted);

#ifndef USESTATICTEMPSTRINGS
	free(formatted);
#endif
}


static int font_load_inner(GfxDomain *domain, Font *font, Bundle *fb)
{
	SDL_RWops *rw = SDL_RWFromBundle(fb, "font.bmp");

#ifdef USESDL_IMAGE
	if (!rw)
		rw = SDL_RWFromBundle(fb, "font.png");
#endif

	if (rw)
	{
		GfxSurface * s= gfx_load_surface_RW(domain, rw, GFX_KEYED);

		char map[1000];
		memset(map, 0, sizeof(map));

		{
			SDL_RWops *rw = SDL_RWFromBundle(fb, "charmap.txt");
			if (rw)
			{
				char temp[1000];
				memset(temp, 0, sizeof(temp));
				rw->read(rw, temp, 1, sizeof(temp)-1);
				SDL_RWclose(rw);

				size_t len = my_min(strlen(temp), 256);
				const char *c = temp;
				char *m = map;

				debug("Read charmap (%lu chars)", (unsigned long)len);

				while (*c)
				{
					if (*c == '\\' && len > 1)
					{
						char hex = 0;
						int digits = 0;

						while (len > 1 && digits < 2)
						{
							char digit = tolower(*(c + 1));

							if (!((digit >= '0' && digit <= '9') || (digit >= 'a' && digit <= 'f')))
							{
								if (digits < 1)
									goto not_hex;

								break;
							}

							hex <<= 4;

							hex |= (digit >= 'a' ? digit - 'a' + 10 : digit - '0') & 0x0f;

							--len;
							++digits;
							++c;
						}

						if (digits < 1) goto not_hex;

						++c;

						*(m++) = hex;
					}
					else
					{
					not_hex:
						*(m++) = *(c++);

						--len;
					}
				}

				if (*c && len == 0)
					warning("Excess font charmap chars (max. 256)");
			}
			else
			{
				strcpy(map, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
				debug("Charmap not found in font file");
			}
		}

		int w, h, spacing = 0, spacewidth = 0;

		{
			SDL_RWops *rw = SDL_RWFromBundle(fb, "res.txt");
			char res[10] = { 0 };

			if (rw)
			{
				rw->read(rw, res, 1, sizeof(res)-1);
				SDL_RWclose(rw);

				sscanf(res, "%d %d %d %d", &w, &h, &spacing, &spacewidth);
			}
			else
			{
				w = 8;
				h = 9;
			}
		}

		font_create(font, s, w, h, spacing, spacewidth, map);

		font->surface = s;

		bnd_free(fb);

		return 1;
	}
	else
	{
		warning("Bitmap not found in font file");

		bnd_free(fb);

		return 0;
	}
}


int font_load_file(GfxDomain *domain, Font *font, char *filename)
{
	debug("Loading font '%s'", filename);

	Bundle fb;
	if (bnd_open(&fb, filename))
	{
		return font_load_inner(domain, font, &fb);
	}

	return 0;
}


int font_load(GfxDomain *domain, Font *font, Bundle *bundle, char *name)
{
	debug("Loading font '%s'", name);

	SDL_RWops *rw = SDL_RWFromBundle(bundle, name);

	if (rw)
	{
		int r = 0;

		Bundle fb;

		if (bnd_open_RW(&fb, rw))
		{
			r = font_load_inner(domain, font, &fb);
		}

		SDL_RWclose(rw);

		return r;
	}
	else
		return 0;
}


void font_write_cursor_args(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 *cursor, SDL_Rect *bounds, const char * text, ...)
{
	va_list va;
	va_start(va, text);

	font_write_va(font, dest, r, cursor, bounds, text, va);

	va_end(va);
}


void font_write_cursor(const Font *font, GfxDomain *dest, const SDL_Rect *r, Uint16 *cursor, SDL_Rect *bounds, const char * text)
{
	inner_write(font, dest, r, cursor, bounds, text);
}


void font_write(const Font *font, GfxDomain *dest, const SDL_Rect *r, const char * text)
{
	Uint16 cursor = 0;
	inner_write(font, dest, r, &cursor, NULL, text);
}


void font_write_args(const Font *font, GfxDomain *dest, const SDL_Rect *r, const char * text, ...)
{
	Uint16 cursor = 0;

	va_list va;
	va_start(va, text);

	font_write_va(font, dest, r, &cursor, NULL, text, va);

	va_end(va);
}


int font_load_RW(GfxDomain *domain, Font *font, SDL_RWops *rw)
{
	int r = 0;

	Bundle fb;

	if (bnd_open_RW(&fb, rw))
	{
		r = font_load_inner(domain, font, &fb);
	}

	return r;
}


int font_text_width(const Font *font, const char *text)
{
	const char *c = text;
	int w = 0;

	for (;*c && *c != '\n';++c)
	{
		const TileDescriptor *tile = findchar(font, *c);
		w += tile ? tile->rect.w + font->char_spacing : font->space_width;
	}

	return w;

}


void font_set_color(Font *font, Uint32 rgb)
{
	gfx_surface_set_color(font->surface, rgb);
}
