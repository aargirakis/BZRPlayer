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

#include <stdlib.h>
#include "editor.h"

void layer_load(Background *layer, FILE *f)
{
	int size = 0;
	fread(&size, 1, sizeof(int), f);
	fread(layer, 1, size, f);
	
	if (layer->h > 0 && layer->w > 0) 
	{
		layer->data = malloc(layer->w*layer->h*sizeof(layer->data[0]));
		fread(layer->data, layer->w*layer->h, sizeof(layer->data[0]), f);
	}
}


void layer_save(Background *layer, FILE *f)
{
	int size = sizeof(*layer);
	fwrite(&size, 1, sizeof(int), f);
	fwrite(layer, 1, size, f);
	fwrite(layer->data, layer->w*layer->h, sizeof(layer->data[0]), f);
}

void level_load(Level *level, FILE *f)
{
	fread(&level->n_layers, 1, sizeof(int), f);
	for (int i = 0 ; i < level->n_layers ; ++i)
		layer_load(&level->layer[i], f);
	fread(&level->n_events, 1, sizeof(int), f);
	level->event = NULL;
	if (level->n_events > 0)
	{
		level->event = malloc(sizeof(*level->event)*level->n_events);
		for (int i = 0 ; i < level->n_events ; ++i)
			fread(&level->event[i], 1, sizeof(level->event[0]), f);
	}
}


void level_save(Level *level, FILE *f)
{
	fwrite(&level->n_layers, 1, sizeof(int), f);
	for (int i = 0 ; i < level->n_layers ; ++i)
		layer_save(&level->layer[i], f);
	fwrite(&level->n_events, 1, sizeof(int), f);
	for (int i = 0 ; i < level->n_events ; ++i)
		fwrite(&level->event[i], 1, sizeof(level->event[0]), f);
}
