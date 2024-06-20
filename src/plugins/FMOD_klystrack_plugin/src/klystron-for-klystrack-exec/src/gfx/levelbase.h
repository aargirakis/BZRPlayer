#ifndef LEVELBASE_H
#define LEVELBASE_H

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


#include "background.h"
#include "SDL_rwops.h"

enum
{
	LOP_LAYER,
	LOP_END,
	LOP_TILE,
	LOP_REP_TILE,
	LOP_EVENT
};

#define PACKED __attribute__ ((packed))
#define EV_PARAMS 16
#define EV_NEXT 15
#define EV_TRGPARENT 14

typedef struct
{
	Uint8 opcode;
	Uint32 repeat;
} PACKED LevOpCode;

typedef struct
{
	Uint32 flags;
	Uint16 w, h;
	Sint16 prx_mlt_x, prx_mlt_y;
	Sint16 off_x, off_y;
} PACKED LevLayer;

typedef struct
{
	Sint16 x, y;
	Uint16 w, h;
	Sint32 param[EV_PARAMS];
} PACKED LevEvent;

typedef struct
{
	Uint16 type;
} PACKED LevTile;

typedef struct
{
	Uint16 repeat;
	Uint16 type;
} PACKED LevRepTile;

int lev_load(Background *bg, int *n_layers, SDL_RWops* data, int (*interpret_event)(void *, const LevEvent *), void* pdata);
void lev_unload(Background *bg, int n_layers);

#endif
