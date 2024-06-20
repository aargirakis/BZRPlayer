//
//  state.c
//  vio2sf
//
//  Created by Christopher Snowhill on 10/13/13.
//  Copyright (c) 2013 Christopher Snowhill. All rights reserved.
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "NDSSystem.h"
#include "MMU.h"
#include "armcpu.h"
#include "cp15.h"
#include "spu_exports.h"

#include "state.h"
#include "registers.h"

static int SNDStateInit(NDS_state *state, int buffersize)
{
    state->sample_buffer = (s16 *) malloc(buffersize * sizeof(s16) * 2);
    state->sample_pointer = 0;
    state->sample_size = buffersize;
    
	return state->sample_buffer == NULL ? -1 : 0;
}

static void SNDStateDeInit(NDS_state *state)
{
    if ( state->sample_buffer ) free( state->sample_buffer );
    state->sample_buffer = NULL;
}

static void SNDStateUpdateAudio(NDS_state *state, s16 *buffer, u32 num_samples)
{
    memcpy( state->sample_buffer + state->sample_pointer * 2, buffer, num_samples * sizeof(s16) * 2);
    state->sample_pointer += num_samples;
}

SoundInterface_struct SNDState = {
	SNDStateInit,
	SNDStateDeInit,
	SNDStateUpdateAudio,
};

int state_init(struct NDS_state *state)
{
    int i;
    
    memset(state, 0, sizeof(NDS_state));
    
    state->nds = (NDSSystem *) calloc(1, sizeof(NDSSystem));
    if (!state->nds)
        return -1;
    
    state->NDS_ARM7 = (armcpu_t *) calloc(1, sizeof(armcpu_t));
    if (!state->NDS_ARM7)
        return -1;
    
    state->NDS_ARM9 = (armcpu_t *) calloc(1, sizeof(armcpu_t));
    if (!state->NDS_ARM9)
        return -1;
    
    state->MMU = (MMU_struct *) calloc(1, sizeof(MMU_struct));
    if (!state->MMU)
        return -1;
    
    MMU_Init(state);
    
    state->partie = 1;
    
    if (NDS_Init(state))
        return -1;
    
    SPU_ChangeSoundCore(state, &SNDState);

	state->execute = FALSE;
    
	MMU_unsetRom(state);
    
    state->cycles = 0;
    
    return 0;
}

void state_deinit(struct NDS_state *state)
{
    if (state->MMU)
        MMU_unsetRom(state);
    if (state->nds && state->NDS_ARM7 && state->NDS_ARM9)
        NDS_DeInit(state);
    if (state->MMU)
        MMU_DeInit(state);
    if (state->nds) free(state->nds);
    state->nds = NULL;
    if (state->NDS_ARM7) free(state->NDS_ARM7);
    state->NDS_ARM7 = NULL;
    if (state->NDS_ARM9) free(state->NDS_ARM9);
    state->NDS_ARM9 = NULL;
    if (state->MMU) free(state->MMU);
    state->MMU = NULL;
}

void state_setrom(struct NDS_state *state, u8 * rom, u32 rom_size)
{
    assert(!(rom_size & (rom_size - 1)));
    NDS_SetROM(state, rom, rom_size - 1);
    NDS_Reset(state);
    state->execute = TRUE;
}

static void load_setstate(struct NDS_state *state, const u8 *ss, u32 ss_size);

void state_loadstate(struct NDS_state *state, const u8 *ss, u32 ss_size)
{
	if (ss && ss_size)
	{
		armcp15_t *c9 = (armcp15_t *)state->NDS_ARM9->coproc[15];
		int proc;
		if (state->initial_frames == -1)
		{
            
			/* set initial ARM9 coprocessor state */
            
			armcp15_moveARM2CP(c9, 0x00000000, 0x01, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x05, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x06, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x0a, 0, 4);
			armcp15_moveARM2CP(c9, 0x04000033, 0x06, 0x00, 0, 4);
			armcp15_moveARM2CP(c9, 0x0200002d, 0x06, 0x01, 0, 0);
			armcp15_moveARM2CP(c9, 0x027e0021, 0x06, 0x02, 0, 0);
			armcp15_moveARM2CP(c9, 0x08000035, 0x06, 0x03, 0, 0);
			armcp15_moveARM2CP(c9, 0x027e001b, 0x06, 0x04, 0, 0);
			armcp15_moveARM2CP(c9, 0x0100002f, 0x06, 0x05, 0, 0);
			armcp15_moveARM2CP(c9, 0xffff001d, 0x06, 0x06, 0, 0);
			armcp15_moveARM2CP(c9, 0x027ff017, 0x06, 0x07, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000020, 0x09, 0x01, 0, 1);
            
			armcp15_moveARM2CP(c9, 0x027e000a, 0x09, 0x01, 0, 0);
            
			armcp15_moveARM2CP(c9, 0x00000042, 0x02, 0x00, 0, 1);
			armcp15_moveARM2CP(c9, 0x00000042, 0x02, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x00000002, 0x03, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x05100011, 0x05, 0x00, 0, 3);
			armcp15_moveARM2CP(c9, 0x15111011, 0x05, 0x00, 0, 2);
			armcp15_moveARM2CP(c9, 0x07dd1e10, 0x01, 0x00, 0, 0);
			armcp15_moveARM2CP(c9, 0x0005707d, 0x01, 0x00, 0, 0);
            
			armcp15_moveARM2CP(c9, 0x00000000, 0x07, 0x0a, 0, 4);
			armcp15_moveARM2CP(c9, 0x02004000, 0x07, 0x05, 0, 1);
			armcp15_moveARM2CP(c9, 0x02004000, 0x07, 0x0e, 0, 1);
            
			/* set initial timer state */
            
			MMU_write16(state, 0, REG_TM0CNTL, 0x0000);
			MMU_write16(state, 0, REG_TM0CNTH, 0x00C1);
			MMU_write16(state, 1, REG_TM0CNTL, 0x0000);
			MMU_write16(state, 1, REG_TM0CNTH, 0x00C1);
			MMU_write16(state, 1, REG_TM1CNTL, 0xf7e7);
			MMU_write16(state, 1, REG_TM1CNTH, 0x00C1);
            
			/* set initial interrupt state */
            
			state->MMU->Cores[0].reg_IME = 0x00000001;
			state->MMU->Cores[0].reg_IE  = 0x00042001;
			state->MMU->Cores[1].reg_IME = 0x00000001;
			state->MMU->Cores[1].reg_IE  = 0x0104009d;
		}
		else if (state->initial_frames > 0)
		{
			/* execute boot code */
			int i;
			for (i=0; i<state->initial_frames; i++)
				NDS_exec_frame(state, 0, 0);
			i = 0;
		}
        
		/* load state */
        
		load_setstate(state, ss, ss_size);
        
		if (state->initial_frames == -1)
		{
			armcp15_moveARM2CP(c9, (state->NDS_ARM9->R13_irq & 0x0fff0000) | 0x0a, 0x09, 0x01, 0, 0);
		}
        
		/* restore timer state */
        
		for (proc = 0; proc < 2; proc++)
		{
      MMU_Core_struct* const core = state->MMU->Cores + proc;
      u8* const mem = core->MemMap[0x40];
			MMU_write16(state, proc, REG_TM0CNTH, T1ReadWord(mem, REG_TM0CNTH & 0xFFF));
			MMU_write16(state, proc, REG_TM1CNTH, T1ReadWord(mem, REG_TM1CNTH & 0xFFF));
			MMU_write16(state, proc, REG_TM2CNTH, T1ReadWord(mem, REG_TM2CNTH & 0xFFF));
			MMU_write16(state, proc, REG_TM3CNTH, T1ReadWord(mem, REG_TM3CNTH & 0xFFF));
		}
	}
	else if (state->initial_frames > 0)
	{
		/* skip 1 sec */
		int i;
		for (i=0; i<state->initial_frames; i++)
			NDS_exec_frame(state, 0, 0);
	}

    state->execute = TRUE;
}

static unsigned state_exec_frame(struct NDS_state *state)
{
#define HBASE_CYCLES (33509300.322234)
#define HLINE_CYCLES (6 * (99 + 256))
#define HSAMPLES ((u32)((44100.0 * HLINE_CYCLES) / HBASE_CYCLES))
#define VDIVISION 100
#define VLINES 263
#define VBASE_CYCLES (((double)HBASE_CYCLES) / VDIVISION)
#define VSAMPLES ((u32)((44100.0 * HLINE_CYCLES * VLINES) / HBASE_CYCLES))
            
 	unsigned numsamples;
 	if (state->sync_type == 1)
 	{
 		/* vsync */
 		state->cycles += ((44100 / VDIVISION) * HLINE_CYCLES * VLINES);
 		if (state->cycles >= (u32)(VBASE_CYCLES * (VSAMPLES + 1)))
 		{
 			numsamples = (VSAMPLES + 1);
 			state->cycles -= (u32)(VBASE_CYCLES * (VSAMPLES + 1));
 		}
 		else
 		{
 			numsamples = (VSAMPLES + 0);
 			state->cycles -= (u32)(VBASE_CYCLES * (VSAMPLES + 0));
 		}
 		NDS_exec_frame(state, state->arm9_clockdown_level, state->arm7_clockdown_level);
 	}
 	else
 	{
 		/* hsync */
 		state->cycles += (44100 * HLINE_CYCLES);
 		if (state->cycles >= (u32)(HBASE_CYCLES * (HSAMPLES + 1)))
 		{
 			numsamples = (HSAMPLES + 1);
 			state->cycles -= (u32)(HBASE_CYCLES * (HSAMPLES + 1));
 		}
 		else
 		{
 			numsamples = (HSAMPLES + 0);
 			state->cycles -= (u32)(HBASE_CYCLES * (HSAMPLES + 0));
 		}
 		NDS_exec_hframe(state, state->arm9_clockdown_level, state->arm7_clockdown_level);
 	}
  return numsamples;
}

void state_render(struct NDS_state *state, s16 * buffer, unsigned int sample_count)
{
    s16 * ptr = buffer;
    
	while (sample_count)
	{
		unsigned long remain_samples = state->sample_pointer;
		if (remain_samples > 0)
		{
			if (remain_samples >= sample_count)
			{
        if (ptr)
        {
				  memcpy(ptr, state->sample_buffer, sample_count * sizeof(s16) * 2);
  			  ptr += sample_count * 2;
        }
        memmove(state->sample_buffer, state->sample_buffer + sample_count * 2, ( remain_samples - sample_count ) * sizeof(s16) * 2 );
				state->sample_pointer -= sample_count;
				remain_samples -= sample_count; /**/
				sample_count = 0;  /**/
				break;
			}
			else
			{
        if (ptr)
        {
				  memcpy(ptr, state->sample_buffer, remain_samples * sizeof(s16) * 2);
  			  ptr += remain_samples * 2;
        }
				state->sample_pointer = 0;
				sample_count -= remain_samples;
				remain_samples = 0;
			}
		}
		while (remain_samples == 0 && state->sample_pointer < 1024)
		{
      unsigned numsamples = state_exec_frame(state);
      if (!ptr)
      {
        const unsigned to_skip = numsamples <= sample_count ? numsamples : sample_count;
        SPU_SkipSamples(state, to_skip);
        numsamples -= to_skip;
        sample_count -= to_skip;
      }
      if (numsamples)
      {
			  SPU_EmulateSamples(state, numsamples);
      }
		}
	}
}

static u16 getwordle(const unsigned char *pData)
{
	return pData[0] | (((u16)pData[1]) << 8);
}

static u32 getdwordle(const unsigned char *pData)
{
    return pData[0] | (((u32)pData[1]) << 8) | (((u32)pData[2]) << 16) | (((u32)pData[3]) << 24);
}

static void load_getsta(Status_Reg *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l << 2;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
    {
        u32 st = getdwordle(*ss + (i << 2));
        ptr[i].bits.N = (st >> 31) & 1;
        ptr[i].bits.Z = (st >> 30) & 1;
        ptr[i].bits.C = (st >> 29) & 1;
        ptr[i].bits.V = (st >> 28) & 1;
        ptr[i].bits.Q = (st >> 27) & 1;
        ptr[i].bits.RAZ = (st >> 8) & ((1 << 19) - 1);
        ptr[i].bits.I = (st >> 7) & 1;
        ptr[i].bits.F = (st >> 6) & 1;
        ptr[i].bits.T = (st >> 5) & 1;
        ptr[i].bits.mode = (st >> 0) & 0x1f;
    }
	*ss += s;
}

static void load_getbool(BOOL *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l << 2;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
		ptr[i] = (BOOL)getdwordle(*ss + (i << 2));
	*ss += s;
}

#if defined(SIGNED_IS_NOT_2S_COMPLEMENT)
/* 2's complement */
#define u32tos32(v) ((s32)((((s64)(v)) ^ 0x80000000) - 0x80000000))
#else
/* 2's complement */
#define u32tos32(v) ((s32)v)
#endif

static void load_gets32(s32 *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l << 2;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
		ptr[i] = u32tos32(getdwordle(*ss + (i << 2)));
	*ss += s;
}

static void load_getu32(u32 *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l << 2;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
		ptr[i] = getdwordle(*ss + (i << 2));
	*ss += s;
}

static void load_getu16(u16 *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l << 1;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
		ptr[i] = getwordle(*ss + (i << 1));
	*ss += s;
}

static void load_getu8(u8 *ptr, unsigned l, const u8 **ss, const u8 *sse)
{
	unsigned s = l;
	unsigned i;
	if ((*ss >= sse) || ((*ss + s) > sse))
		return;
	for (i = 0; i < l; i++)
		ptr[i] = (*ss)[i];
	*ss += s;
}

void gdb_stub_fix(armcpu_t *armcpu)
{
	/* armcpu->R[15] = armcpu->instruct_adr; */
	armcpu->next_instruction = armcpu->instruct_adr;
	if(armcpu->CPSR.bits.T == 0)
	{
		armcpu->instruction = MMU_read32_acl(armcpu->state, armcpu->proc_ID, armcpu->next_instruction,CP15_ACCESS_EXECUTE);
		armcpu->instruct_adr = armcpu->next_instruction;
		armcpu->next_instruction += 4;
		armcpu->R[15] = armcpu->next_instruction + 4;
	}
	else
	{
		armcpu->instruction = MMU_read16_acl(armcpu->state, armcpu->proc_ID, armcpu->next_instruction,CP15_ACCESS_EXECUTE);
		armcpu->instruct_adr = armcpu->next_instruction;
		armcpu->next_instruction += 2;
		armcpu->R[15] = armcpu->next_instruction + 2;
	}
}

static void load_setstate(struct NDS_state *state, const u8 *ss, u32 ss_size)
{
    const u8 *sse = ss + ss_size;
    
	if (!ss || !ss_size)
		return;
    
    /* Skip over "Desmume Save File" crap */
    ss += 0x17;
    
	/* Read ARM7 cpu registers */
	load_getu32(&state->NDS_ARM7->proc_ID, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->instruction, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->instruct_adr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->next_instruction, 1, &ss, sse);
	load_getu32(state->NDS_ARM7->R, 16, &ss, sse);
	load_getsta(&state->NDS_ARM7->CPSR, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_usr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_usr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_svc, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_svc, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_abt, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_abt, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_und, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_und, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_irq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_irq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R8_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R9_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R10_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R11_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R12_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R13_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->R14_fiq, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR_svc, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR_abt, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR_und, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR_irq, 1, &ss, sse);
	load_getsta(&state->NDS_ARM7->SPSR_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM7->intVector, 1, &ss, sse);
	load_getu8(&state->NDS_ARM7->LDTBit, 1, &ss, sse);
	load_getbool(&state->NDS_ARM7->waitIRQ, 1, &ss, sse);
	load_getbool(&state->NDS_ARM7->wIRQ, 1, &ss, sse);
	load_getbool(&state->NDS_ARM7->wirq, 1, &ss, sse);
    
	/* Read ARM9 cpu registers */
	load_getu32(&state->NDS_ARM9->proc_ID, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->instruction, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->instruct_adr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->next_instruction, 1, &ss, sse);
	load_getu32(state->NDS_ARM9->R, 16, &ss, sse);
	load_getsta(&state->NDS_ARM9->CPSR, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_usr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_usr, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_svc, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_svc, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_abt, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_abt, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_und, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_und, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_irq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_irq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R8_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R9_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R10_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R11_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R12_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R13_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->R14_fiq, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR_svc, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR_abt, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR_und, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR_irq, 1, &ss, sse);
	load_getsta(&state->NDS_ARM9->SPSR_fiq, 1, &ss, sse);
	load_getu32(&state->NDS_ARM9->intVector, 1, &ss, sse);
	load_getu8(&state->NDS_ARM9->LDTBit, 1, &ss, sse);
	load_getbool(&state->NDS_ARM9->waitIRQ, 1, &ss, sse);
	load_getbool(&state->NDS_ARM9->wIRQ, 1, &ss, sse);
	load_getbool(&state->NDS_ARM9->wirq, 1, &ss, sse);
    
	/* Read in other internal variables that are important */
	load_gets32(&state->nds->ARM9Cycle, 1, &ss, sse);
	load_gets32(&state->nds->ARM7Cycle, 1, &ss, sse);
	load_gets32(&state->nds->cycles, 1, &ss, sse);
	load_gets32(state->nds->timerCycle[0], 4, &ss, sse);
	load_gets32(state->nds->timerCycle[1], 4, &ss, sse);
	load_getbool(state->nds->timerOver[0], 4, &ss, sse);
	load_getbool(state->nds->timerOver[1], 4, &ss, sse);
	load_gets32(&state->nds->nextHBlank, 1, &ss, sse);
	load_getu32(&state->nds->VCount, 1, &ss, sse);
	load_getu32(&state->nds->old, 1, &ss, sse);
	load_gets32(&state->nds->diff, 1, &ss, sse);
	load_getbool(&state->nds->lignerendu, 1, &ss, sse);
	load_getu16(&state->nds->touchX, 1, &ss, sse);
	load_getu16(&state->nds->touchY, 1, &ss, sse);
    
	/* Read in memory/registers specific to the ARM9 */
	load_getu8 (state->MMU->ARM9Mem->ARM9_ITCM, 0x8000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_DTCM, 0x4000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_WRAM, 0x1000000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->MAIN_MEM, 0x400000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_REG, 0x10000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_VMEM, 0x800, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_OAM, 0x800, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_ABG, 0x80000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_BBG, 0x20000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_AOBJ, 0x40000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_BOBJ, 0x20000, &ss, sse);
	load_getu8 (state->MMU->ARM9Mem->ARM9_LCD, 0xA4000, &ss, sse);
    
	/* Read in memory/registers specific to the ARM7 */
	load_getu8 (state->MMU->ARM7Mem->ARM7_ERAM, 0x10000, &ss, sse);
	load_getu8 (state->MMU->ARM7Mem->ARM7_REG, 0x10000, &ss, sse);
	load_getu8 (state->MMU->ARM7Mem->ARM7_WIRAM, 0x10000, &ss, sse);
    
	/* Read in shared memory */
	load_getu8 (state->MMU->SWIRAM, 0x8000, &ss, sse);
    
	gdb_stub_fix(state->NDS_ARM9);
	gdb_stub_fix(state->NDS_ARM7);
}
