
#include "types.h"
#include "ws_io.h"
#include "ws_audio.h"
#include "wsr_player.h"
#include <stdlib.h>
#include <memory.h>

uint8	*ws_rom;
uint8	*ws_internalRam = NULL;
uint8	*ws_staticRam = NULL;
uint32	romAddressMask;
uint32	romSize;
uint32	baseBank;

void ws_memory_init(uint8 *rom, uint32 wsRomSize)
{
	ws_rom = rom;
	romSize = wsRomSize;
	romAddressMask = romSize-1;
	baseBank = 0x100 - (romSize>>16);
	if (ws_internalRam == NULL)
		ws_internalRam = (uint8*)malloc(0x10000);
	if (ws_staticRam == NULL)
		ws_staticRam = (uint8*)malloc(0x10000);
}

void ws_memory_reset(void)
{
	memset(ws_internalRam, 0, 0x10000);
	memset(ws_staticRam, 0, 0x10000);
}

void ws_memory_done(void)
{
	if (ws_internalRam)
		free(ws_internalRam);
	ws_internalRam = NULL;
	if (ws_staticRam)
		free(ws_staticRam);
	ws_staticRam = NULL;
}

BYTE cpu_readmem20(DWORD addr)
{
	uint32	offset = addr&0xffff;
	uint32	bank = (addr>>16)&0xf;
	int romBank;

	switch (bank)
	{
	case 0:		// 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM
				return ws_internalRam[offset];
	case 1:		// 1 - SRAM (cart)
				return ws_staticRam[offset];
	case 2:
	case 3:
				romBank = ws_ioRam[0xC0+bank];
				if (romBank < baseBank)
					return 0xff;
				else
					return ws_rom[(unsigned)(offset + ((romBank-baseBank)<<16))];
	default:
				romBank = ((ws_ioRam[0xC0]&0xf)<<4)|(bank&0xf);
				if (romBank < baseBank)
					return 0xff;
				else
					return ws_rom[(unsigned)(offset + ((romBank-baseBank)<<16))];
	}
	return (0xff);
}

void cpu_writemem20(DWORD addr,BYTE value)
{
	uint32	offset = addr&0xffff;
	uint32	bank = (addr>>16)&0xf;

	switch (bank)
	{
	case 0:		// 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM

				//ギルティギアプチ2のサンプリングドラムは、PCMVoiceを使わず
				//波形メモリをHBlank間隔で更新させていってる
				//その対策として、波形メモリが更新される場合は、出力波形を更新させる
				if (WaveAdrs <= offset && offset < (WaveAdrs+0x40))
					Update_SampleData();

				ws_internalRam[offset] = value;
				break;
	case 1:		// 1 - SRAM (cart)
				ws_staticRam[offset] = value;
				break;
				// other banks are read-only
	}
}
