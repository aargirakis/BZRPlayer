#include "levelbase.h"
#include <stdlib.h>

#define ptr_read(dest, ptr) SDL_RWread(data, &dest, sizeof(dest), 1);

int lev_load(Background *bg, int *n_layers, SDL_RWops* data, int (*interpret_event)(void *, const LevEvent *), void* pdata)
{
	int current_layer = -1;
	BgCell * cell = NULL;
	
	while (1)
	{
		Uint8 x = 0;
		SDL_RWread(data, &x, 1, 1);
		
		if (x == LOP_END) break;
		
		LevOpCode opcode;
		opcode.opcode = x;
		ptr_read(opcode.repeat, data);
		
		FIX_ENDIAN(opcode.repeat);
		
		for (int di = 0 ; di < opcode.repeat ; ++di)
		{
			switch (opcode.opcode)
			{
				default:
				fatal("Unknown opcode %d\n", opcode.opcode);
				return 0;
				break;
				
				case LOP_EVENT:
				{
					LevEvent event = {0};
					ptr_read(event.x, data);
					ptr_read(event.y, data);
					ptr_read(event.w, data);
					ptr_read(event.h, data);
					SDL_RWread(data, &event.param[0], sizeof(event.param[0]), EV_PARAMS);
					
					if (!interpret_event) break;
					
					FIX_ENDIAN(event.x);
					FIX_ENDIAN(event.y);
					FIX_ENDIAN(event.w);
					FIX_ENDIAN(event.h);
					
					for (int i = 0 ; i < EV_PARAMS ; ++i)
					{
						FIX_ENDIAN(event.param[i]);
					}
					
					if (!interpret_event(pdata, &event)) 
					{
						warning("Unknown event %d\n", event.param[0]);
					}
				}
				break;
				
				case LOP_LAYER:
				{
					++current_layer;
					
					if (current_layer >= *n_layers)
					{
						fatal("Too many layers");
						return 0;
					}
					
					LevLayer header;
	
					ptr_read(header.flags, data);
					ptr_read(header.w, data);
					ptr_read(header.h, data);
					ptr_read(header.prx_mlt_x, data);
					ptr_read(header.prx_mlt_y, data);
					ptr_read(header.off_x, data);
					ptr_read(header.off_y, data);
					
					FIX_ENDIAN(header.flags);
					FIX_ENDIAN(header.w);
					FIX_ENDIAN(header.h);
					FIX_ENDIAN(header.prx_mlt_x);
					FIX_ENDIAN(header.prx_mlt_y);
					FIX_ENDIAN(header.off_x);
					FIX_ENDIAN(header.off_y);
					
					bg_create(&bg[current_layer], header.w, header.h);
					bg[current_layer].flags = header.flags;
					bg[current_layer].prx_mlt_x = header.prx_mlt_x;
					bg[current_layer].prx_mlt_y = header.prx_mlt_y;
					bg[current_layer].off_x = header.off_x;
					bg[current_layer].off_y = header.off_y;
					cell = bg[current_layer].data;
				}
				break;
				
				case LOP_TILE:
				{
					LevTile tile = {0};
					
					ptr_read(tile.type, data);
					
					FIX_ENDIAN(tile.type);
					
					cell->tile = tile.type;
					
					++cell;
				}
				break;
				
				case LOP_REP_TILE:
				{
					LevRepTile tile = {0};
					
					ptr_read(tile.repeat, data);
					ptr_read(tile.type, data);
					
					FIX_ENDIAN(tile.type);
					FIX_ENDIAN(tile.repeat);
					
					for (int i = 0 ; i < tile.repeat ; ++i)
					{
						cell->tile = tile.type;
						++cell;
					}
				}
				break;
			}
		}
	}
	
	*n_layers = current_layer+1;
	
	return 1;
}

void lev_unload(Background *bg, int n_layers)
{
	for (int i = 0 ; i < n_layers ; ++i)
	{
		free(bg[i].data);
		bg[i].data = NULL;
	}
}
