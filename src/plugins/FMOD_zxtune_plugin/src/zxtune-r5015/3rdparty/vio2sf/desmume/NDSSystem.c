/*  Copyright (C) 2006 yopyop
    yopyop156@ifrance.com
    yopyop156.ifrance.com

    This file is part of DeSmuME

    DeSmuME is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DeSmuME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DeSmuME; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <stdlib.h>

#include "state.h"

#include "NDSSystem.h"
#include "MMU.h"
//#include "cflash.h"
#include "spu_exports.h"

/* the count of bytes copied from the firmware into memory */
#define NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT 0x70

static u32
calc_CRC16( u32 start, const u8 *data, int count) {
  int i,j;
  u32 crc = start & 0xffff;
  static u16 val[] = { 0xC0C1,0xC181,0xC301,0xC601,0xCC01,0xD801,0xF001,0xA001 };
  for(i = 0; i < count; i++)
  {
    crc = crc ^ data[i];

    for(j = 0; j < 8; j++) {
      int do_bit = 0;

      if ( crc & 0x1)
        do_bit = 1;

      crc = crc >> 1;

      if ( do_bit) {
        crc = crc ^ (val[j] << (7-j));
      }
    }
  }
  return crc;
}

static int
copy_firmware_user_data( u8 *dest_buffer, const u8 *fw_data) {
  /*
   * Determine which of the two user settings in the firmware is the current
   * and valid one and then copy this into the destination buffer.
   *
   * The current setting will have a greater count.
   * Settings are only valid if its CRC16 is correct.
   */
  int user1_valid = 0;
  int user2_valid = 0;
  u32 user_settings_offset;
  u32 fw_crc;
  u32 crc;
  int copy_good = 0;

  user_settings_offset = fw_data[0x20];
  user_settings_offset |= fw_data[0x21] << 8;
  user_settings_offset <<= 3;

  if ( user_settings_offset <= 0x3FE00) {
    s32 copy_settings_offset = -1;

    crc = calc_CRC16( 0xffff, &fw_data[user_settings_offset],
                      NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
    fw_crc = fw_data[user_settings_offset + 0x72];
    fw_crc |= fw_data[user_settings_offset + 0x73] << 8;
    if ( crc == fw_crc) {
      user1_valid = 1;
    }

    crc = calc_CRC16( 0xffff, &fw_data[user_settings_offset + 0x100],
                      NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
    fw_crc = fw_data[user_settings_offset + 0x100 + 0x72];
    fw_crc |= fw_data[user_settings_offset + 0x100 + 0x73] << 8;
    if ( crc == fw_crc) {
      user2_valid = 1;
    }

    if ( user1_valid) {
      if ( user2_valid) {
        u16 count1, count2;

        count1 = fw_data[user_settings_offset + 0x70];
        count1 |= fw_data[user_settings_offset + 0x71] << 8;

        count2 = fw_data[user_settings_offset + 0x100 + 0x70];
        count2 |= fw_data[user_settings_offset + 0x100 + 0x71] << 8;

        if ( count2 > count1) {
          copy_settings_offset = user_settings_offset + 0x100;
        }
        else {
          copy_settings_offset = user_settings_offset;
        }
      }
      else {
        copy_settings_offset = user_settings_offset;
      }
    }
    else if ( user2_valid) {
      /* copy the second user settings */
      copy_settings_offset = user_settings_offset + 0x100;
    }

    if ( copy_settings_offset > 0) {
      memcpy( dest_buffer, &fw_data[copy_settings_offset],
              NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT);
      copy_good = 1;
    }
  }

  return copy_good;
}


int NDS_Init( NDS_state *state) {
     state->nds->ARM9Cycle = 0;
     state->nds->ARM7Cycle = 0;
     state->nds->cycles = 0;

     state->nds->nextHBlank = 3168;
     state->nds->VCount = 0;
     state->nds->lignerendu = FALSE;

	 armcpu_new(state,state->NDS_ARM7,1);
     armcpu_new(state,state->NDS_ARM9,0);

     if (SPU_Init(state, 0, 0) != 0)
        return -1;

     return 0;
}

static void armcpu_deinit(armcpu_t *armcpu)
{
	if(armcpu->coproc[15])
	{
		free(armcpu->coproc[15]);
		armcpu->coproc[15] = 0;
	}
}

void NDS_DeInit(NDS_state *state) {
     if(state->MMU->CART_ROM != state->MMU->UNUSED_RAM)
        NDS_FreeROM(state);

	armcpu_deinit(state->NDS_ARM7);
	armcpu_deinit(state->NDS_ARM9);

     state->nds->nextHBlank = 3168;
     SPU_DeInit(state);
}

BOOL NDS_SetROM(NDS_state *state, u8 * rom, u32 mask)
{
     MMU_setRom(state, rom, mask);

     return TRUE;
} 

NDS_header * NDS_getROMHeader(NDS_state *state)
{
	NDS_header * header = malloc(sizeof(NDS_header));

	memcpy(header->gameTile, state->MMU->CART_ROM, 12);
	memcpy(header->gameCode, state->MMU->CART_ROM + 12, 4);
	header->makerCode = T1ReadWord(state->MMU->CART_ROM, 16);
	header->unitCode = state->MMU->CART_ROM[18];
	header->deviceCode = state->MMU->CART_ROM[19];
	header->cardSize = state->MMU->CART_ROM[20];
	memcpy(header->cardInfo, state->MMU->CART_ROM + 21, 8);
	header->flags = state->MMU->CART_ROM[29];
	header->ARM9src = T1ReadLong(state->MMU->CART_ROM, 32);
	header->ARM9exe = T1ReadLong(state->MMU->CART_ROM, 36);
	header->ARM9cpy = T1ReadLong(state->MMU->CART_ROM, 40);
	header->ARM9binSize = T1ReadLong(state->MMU->CART_ROM, 44);
	header->ARM7src = T1ReadLong(state->MMU->CART_ROM, 48);
	header->ARM7exe = T1ReadLong(state->MMU->CART_ROM, 52);
	header->ARM7cpy = T1ReadLong(state->MMU->CART_ROM, 56);
	header->ARM7binSize = T1ReadLong(state->MMU->CART_ROM, 60);
	header->FNameTblOff = T1ReadLong(state->MMU->CART_ROM, 64);
	header->FNameTblSize = T1ReadLong(state->MMU->CART_ROM, 68);
	header->FATOff = T1ReadLong(state->MMU->CART_ROM, 72);
	header->FATSize = T1ReadLong(state->MMU->CART_ROM, 76);
	header->ARM9OverlayOff = T1ReadLong(state->MMU->CART_ROM, 80);
	header->ARM9OverlaySize = T1ReadLong(state->MMU->CART_ROM, 84);
	header->ARM7OverlayOff = T1ReadLong(state->MMU->CART_ROM, 88);
	header->ARM7OverlaySize = T1ReadLong(state->MMU->CART_ROM, 92);
	header->unknown2a = T1ReadLong(state->MMU->CART_ROM, 96);
	header->unknown2b = T1ReadLong(state->MMU->CART_ROM, 100);
	header->IconOff = T1ReadLong(state->MMU->CART_ROM, 104);
	header->CRC16 = T1ReadWord(state->MMU->CART_ROM, 108);
	header->ROMtimeout = T1ReadWord(state->MMU->CART_ROM, 110);
	header->ARM9unk = T1ReadLong(state->MMU->CART_ROM, 112);
	header->ARM7unk = T1ReadLong(state->MMU->CART_ROM, 116);
	memcpy(header->unknown3c, state->MMU->CART_ROM + 120, 8);
	header->ROMSize = T1ReadLong(state->MMU->CART_ROM, 128);
	header->HeaderSize = T1ReadLong(state->MMU->CART_ROM, 132);
	memcpy(header->unknown5, state->MMU->CART_ROM + 136, 56);
	memcpy(header->logo, state->MMU->CART_ROM + 192, 156);
	header->logoCRC16 = T1ReadWord(state->MMU->CART_ROM, 348);
	header->headerCRC16 = T1ReadWord(state->MMU->CART_ROM, 350);
	memcpy(header->reserved, state->MMU->CART_ROM + 352, 160);
    
	return header;
} 



void NDS_FreeROM(NDS_state *state)
{
   if (state->MMU->CART_ROM != state->MMU->UNUSED_RAM)
      free(state->MMU->CART_ROM);
   MMU_unsetRom(state);
//   if (MMU->bupmem.fp)
//      fclose(MMU->bupmem.fp);
//   cpu->state->MMUbupmem.fp = NULL;
}

                         

void NDS_Reset( NDS_state *state)
{
   BOOL oldexecute=state->execute;
   unsigned int i;
   u32 src;
   u32 dst;
   NDS_header * header = NDS_getROMHeader(state);

	if (!header) return ;

   state->execute = FALSE;

   MMU_clearMem(state);

   src = header->ARM9src;
   dst = header->ARM9cpy;

   for(i = 0; i < (header->ARM9binSize>>2); ++i)
   {
      MMU_write32(state, 0, dst, T1ReadLong(state->MMU->CART_ROM, src));
      dst += 4;
      src += 4;
   }

   src = header->ARM7src;
   dst = header->ARM7cpy;
     
   for(i = 0; i < (header->ARM7binSize>>2); ++i)
   {
      MMU_write32(state, 1, dst, T1ReadLong(state->MMU->CART_ROM, src));
      dst += 4;
      src += 4;
   }

   armcpu_init(state->NDS_ARM7, header->ARM7exe);
   armcpu_init(state->NDS_ARM9, header->ARM9exe);
     
   state->nds->ARM9Cycle = 0;
   state->nds->ARM7Cycle = 0;
   state->nds->cycles = 0;
   memset(state->nds->timerCycle, 0, sizeof(s32) * 2 * 4);
   memset(state->nds->timerOver, 0, sizeof(BOOL) * 2 * 4);
   state->nds->nextHBlank = 3168;
   state->nds->VCount = 0;
   state->nds->old = 0;
   state->nds->diff = 0;
   state->nds->lignerendu = FALSE;
   state->nds->touchX = state->nds->touchY = 0;

   MMU_write16(state, 0, 0x04000130, 0x3FF);
   MMU_write16(state, 1, 0x04000130, 0x3FF);
   MMU_write8(state, 1, 0x04000136, 0x43);

   /*
    * Setup a copy of the firmware user settings in memory.
    * (this is what the DS firmware would do).
    */
   {
     u8 temp_buffer[NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT];
     int fw_index;

     if ( copy_firmware_user_data( temp_buffer, state->MMU->fw.data)) {
       for ( fw_index = 0; fw_index < NDS_FW_USER_SETTINGS_MEM_BYTE_COUNT; fw_index++) {
         MMU_write8( state, 0, 0x027FFC80 + fw_index, temp_buffer[fw_index]);
       }
     }
   }

	// Copy the whole header to Main RAM 0x27FFE00 on startup.
	//  Reference: http://nocash.emubase.de/gbatek.htm#dscartridgeheader
	for (i = 0; i < ((0x170+0x90)/4); i++)
	{
		MMU_write32 (state, 0, 0x027FFE00+i*4, LE_TO_LOCAL_32(((u32*)state->MMU->CART_ROM)[i]));
	}
     
     //ARM7 BIOS IRQ HANDLER
     MMU_write32(state, 1, 0x00, 0xE25EF002);
     MMU_write32(state, 1, 0x04, 0xEAFFFFFE);
     MMU_write32(state, 1, 0x18, 0xEA000000);
     MMU_write32(state, 1, 0x20, 0xE92D500F);
     MMU_write32(state, 1, 0x24, 0xE3A00301);
     MMU_write32(state, 1, 0x28, 0xE28FE000);
     MMU_write32(state, 1, 0x2C, 0xE510F004);
     MMU_write32(state, 1, 0x30, 0xE8BD500F);
     MMU_write32(state, 1, 0x34, 0xE25EF004);
    
     //ARM9 BIOS IRQ HANDLER
     MMU_write32(state, 0, 0xFFFF0018, 0xEA000000);
     MMU_write32(state, 0, 0xFFFF0020, 0xE92D500F);
     MMU_write32(state, 0, 0xFFFF0024, 0xEE190F11);
     MMU_write32(state, 0, 0xFFFF0028, 0xE1A00620);
     MMU_write32(state, 0, 0xFFFF002C, 0xE1A00600);
     MMU_write32(state, 0, 0xFFFF0030, 0xE2800C40);
     MMU_write32(state, 0, 0xFFFF0034, 0xE28FE000);
     MMU_write32(state, 0, 0xFFFF0038, 0xE510F004);
     MMU_write32(state, 0, 0xFFFF003C, 0xE8BD500F);
     MMU_write32(state, 0, 0xFFFF0040, 0xE25EF004);
        
     MMU_write32(state, 0, 0x0000004, 0xE3A0010E);
     MMU_write32(state, 0, 0x0000008, 0xE3A01020);
//     MMU_write32(state, 0, 0x000000C, 0xE1B02110);
     MMU_write32(state, 0, 0x000000C, 0xE1B02040);
     MMU_write32(state, 0, 0x0000010, 0xE3B02020);
//     MMU_write32(state, 0, 0x0000010, 0xE2100202);

   free(header);

   SPU_Reset(state);

   state->execute = oldexecute;
}

static void dma_check(NDS_state *state)
{
  MMU_Core_struct* const core0 = state->MMU->Cores + 0;
  for (int chanId = 0; chanId < 4; ++chanId)
  {
    if((core0->DMA[chanId].Active)&&(core0->DMA[chanId].Cycle<=state->nds->cycles))
    {
    	T1WriteLong(state->MMU->ARM9Mem->ARM9_REG, 0xB8 + (0xC*chanId), T1ReadLong(state->MMU->ARM9Mem->ARM9_REG, 0xB8 + (0xC*chanId)) & 0x7FFFFFFF);
  		if((core0->DMA[chanId].Crt)&(1<<30)) NDS_makeARM9Int(state, 8 + chanId);
  		core0->DMA[chanId].Active = FALSE;
    }
  }
  
  MMU_Core_struct* const core1 = state->MMU->Cores + 1;
  for (int chanId = 0; chanId < 4; ++chanId)
  {
    if((core1->DMA[chanId].Active)&&(core1->DMA[chanId].Cycle<=state->nds->cycles))
    {
    	T1WriteLong(state->MMU->ARM9Mem->ARM9_REG, 0xB8 + (0xC*chanId), T1ReadLong(state->MMU->ARM9Mem->ARM9_REG, 0xB8 + (0xC*chanId)) & 0x7FFFFFFF);
  		if((core1->DMA[chanId].Crt)&(1<<30)) NDS_makeARM7Int(state, 8 + chanId);
  		core1->DMA[chanId].Active = FALSE;
    }
  }

	if((core0->reg_IF&core0->reg_IE) && (core0->reg_IME))
	{
		if ( armcpu_irqExeption(state->NDS_ARM9))
		{
			state->nds->ARM9Cycle = state->nds->cycles;
		}
	}

	if((core1->reg_IF&core1->reg_IE) && (core1->reg_IME))
	{
		if ( armcpu_irqExeption(state->NDS_ARM7))
		{
			state->nds->ARM7Cycle = state->nds->cycles;
		}
	}

}

static void timer_check(NDS_state *state)
{
	int p, t;
	for (p = 0; p < 2; p++)
	{
    MMU_Core_struct* const core = state->MMU->Cores + p;
		for (t = 0; t < 4; t++)
		{
			state->nds->timerOver[p][t] = 0;
			if(core->Timers[t].On)
			{
				if(core->Timers[t].Run)
				{
					switch(core->Timers[t].Mode)
					{
					case 0xFFFF :
						if(t > 0 && state->nds->timerOver[p][t - 1])
						{
							++(core->Timers[t].Counter);
							state->nds->timerOver[p][t] = !core->Timers[t].Counter;
							if (state->nds->timerOver[p][t])
							{
								if (p == 0)
								{
									if(T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 0x102 + (t << 2)) & 0x40)
										NDS_makeARM9Int(state, 3 + t);
								}
								else
								{
									if(T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 0x102 + (t << 2)) & 0x40)
										NDS_makeARM7Int(state, 3 + t);
								}
								core->Timers[t].Counter = core->Timers[t].Reload;
							}
						}
						break;
					default :
						{
							state->nds->diff = (state->nds->cycles >> core->Timers[t].Mode) - (state->nds->timerCycle[p][t] >> core->Timers[t].Mode);
							state->nds->old = core->Timers[t].Counter;
							core->Timers[t].Counter += state->nds->diff;
							state->nds->timerCycle[p][t] += state->nds->diff << core->Timers[t].Mode;
							state->nds->timerOver[p][t] = state->nds->old >= core->Timers[t].Counter;
							if(state->nds->timerOver[p][t])
							{
								if (p == 0)
								{
									if(T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 0x102 + (t << 2)) & 0x40)
										NDS_makeARM9Int(state, 3 + t);
								}
								else
								{
									if(T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 0x102 + (t << 2)) & 0x40)
										NDS_makeARM7Int(state, 3 + t);
								}
								core->Timers[t].Counter = core->Timers[t].Reload + core->Timers[t].Counter - state->nds->old;
							}
						}
						break;
					}
				}
				else
				{
					core->Timers[t].Run = TRUE;
					state->nds->timerCycle[p][t] = state->nds->cycles;
				}
			}
		}
	}
}

void NDS_exec_hframe(NDS_state *state, int cpu_clockdown_level_arm9, int cpu_clockdown_level_arm7)
{
	int h;
	for (h = 0; h < 2; h++)
	{
		s32 nb = state->nds->cycles + (h ? (99 * 12) : (256 * 12));

		while (nb > state->nds->ARM9Cycle && !state->NDS_ARM9->waitIRQ)
			state->nds->ARM9Cycle += armcpu_exec(state->NDS_ARM9) << (cpu_clockdown_level_arm9);
		if (state->NDS_ARM9->waitIRQ) state->nds->ARM9Cycle = nb;
		while (nb > state->nds->ARM7Cycle && !state->NDS_ARM7->waitIRQ)
			state->nds->ARM7Cycle += armcpu_exec(state->NDS_ARM7) << (1 + (cpu_clockdown_level_arm7));
		if (state->NDS_ARM7->waitIRQ) state->nds->ARM7Cycle = nb;
		state->nds->cycles = (state->nds->ARM9Cycle<state->nds->ARM7Cycle)?state->nds->ARM9Cycle : state->nds->ARM7Cycle;

		/* HBLANK */
		if (h)
		{
			T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) | 2);
			T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) | 2);
			NDS_ARM9HBlankInt(state);
			NDS_ARM7HBlankInt(state);

			if(state->nds->VCount<192)
			{
        MMU_Core_struct* const core0 = state->MMU->Cores + 0;
        for (int chanId = 0; chanId < 4; ++chanId)
        {
          if (core0->DMA[chanId].StartTime == 2)
            MMU_doDMA(state, 0, chanId);
        }
			}
		}
		else
		{
			/* HDISP */
			u32 vmatch;

			state->nds->nextHBlank += 4260;
			++state->nds->VCount;
			T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) & 0xFFFD);
			T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) & 0xFFFD);

      MMU_Core_struct* const core0 = state->MMU->Cores + 0;
      for (int chanId = 0; chanId < 4; ++chanId)
      {
        if (core0->DMA[chanId].StartTime == 3)
          MMU_doDMA(state, 0, chanId);
      }
  		// Main memory display
      for (int coreId = 0; coreId < 2; ++coreId)
      {
        MMU_Core_struct* const core = state->MMU->Cores + coreId;
        for (int chanId = 0; chanId < 4; ++chanId)
        {
          if (core->DMA[chanId].StartTime == 4)
            MMU_doDMA(state, coreId, chanId);
          core->DMA[chanId].StartTime = 0;
        }
      }
                            
			if(state->nds->VCount == 192)
			{
				/* VBLANK */
				T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) | 1);
				T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) | 1);
				NDS_ARM9VBlankInt(state);
				NDS_ARM7VBlankInt(state);

        for (int coreId = 0; coreId < 2; ++coreId)
        {
          MMU_Core_struct* const core = state->MMU->Cores + coreId;
          for (int chanId = 0; chanId < 4; ++chanId)
          {
            if (core->DMA[chanId].StartTime == 1)
              MMU_doDMA(state, coreId, chanId);
          }
        }
			}
			else if(state->nds->VCount == 263)
			{
				const int cycles_per_frame = (263 * (99 * 12 + 256 * 12));
				/* VDISP */
				state->nds->nextHBlank = 3168;
				state->nds->VCount = 0;
				T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) & 0xFFFE);
				T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) & 0xFFFE);

				state->nds->cycles -= cycles_per_frame;
				state->nds->ARM9Cycle -= cycles_per_frame;
				state->nds->ARM7Cycle -= cycles_per_frame;
				nb -= cycles_per_frame;
        
        for (int coreId = 0; coreId < 2; ++coreId)
        {
          MMU_Core_struct* const core = state->MMU->Cores + coreId;
          for (int chanId = 0; chanId < 4; ++chanId)
          {
            if (core->Timers[chanId].On)
              state->nds->timerCycle[coreId][chanId] -= cycles_per_frame;
          }
          for (int chanId = 0; chanId < 4; ++chanId)
          {
            if (core->DMA[chanId].Active)
              core->DMA[chanId].Cycle -= cycles_per_frame;
          }
        }
			}

			T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 6, state->nds->VCount);
			T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 6, state->nds->VCount);

			vmatch = T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4);
			if(state->nds->VCount==((vmatch>>8)|((vmatch<<1)&(1<<8))))
			{
				T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) | 4);
				if(T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) & 32)
					NDS_makeARM9Int(state, 2);
			}
			else
				T1WriteWord(state->MMU->ARM9Mem->ARM9_REG, 4, T1ReadWord(state->MMU->ARM9Mem->ARM9_REG, 4) & 0xFFFB);

			vmatch = T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4);
			if(state->nds->VCount==((vmatch>>8)|((vmatch<<1)&(1<<8))))
			{
				T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) | 4);
				if(T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) & 32)
					NDS_makeARM7Int(state, 2);
			}
			else
				T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, 4, T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, 4) & 0xFFFB);

			timer_check(state);
			dma_check(state);
		}
	}
}

void NDS_exec_frame(NDS_state *state, int cpu_clockdown_level_arm9, int cpu_clockdown_level_arm7)
{
	int v;
	for (v = 0; v < 263; v++)
	{
		NDS_exec_hframe(state, cpu_clockdown_level_arm9, cpu_clockdown_level_arm7);
	}
}

