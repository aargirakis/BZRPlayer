/*  Copyright (C) 2006 yopyop
    yopyop156@ifrance.com
    yopyop156.ifrance.com
    Copyright (C) 2006 Theo Berkau
    Copyright (C) 2008-2009 DeSmuME team

    Ideas borrowed from Stephane Dallongeville's SCSP core

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "spu_exports.h"

#define _SPU_CPP_

#include "debug.h"
#include "MMU.h"
#include "SPU.h"
#include "mem.h"
#include "armcpu.h"
#include "NDSSystem.h"
#include "matrix.h"

#include "state.h"

#include <algorithm>
#include <map>
#include <vector>

//===================CONFIGURATION========================
bool isChannelMuted(NDS_state *state, int num) { return state->dwChannelMute&(1<<num) ? true : false; }
SPUInterpolationMode spuInterpolationMode(NDS_state *state) { return (SPUInterpolationMode)state->dwInterpolation; }
//=========================================================

static const int format_shift[] = { 2, 1, 3, 0 };

static const s8 indextbl[8] =
{
	-1, -1, -1, -1, 2, 4, 6, 8
};

static const u16 adpcmtbl[89] =
{
	0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010,
	0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F, 0x0022, 0x0025,
	0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042, 0x0049, 0x0050, 0x0058,
	0x0061, 0x006B, 0x0076, 0x0082, 0x008F, 0x009D, 0x00AD, 0x00BE, 0x00D1,
	0x00E6, 0x00FD, 0x0117, 0x0133, 0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE,
	0x0220, 0x0256, 0x0292, 0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E,
	0x0502, 0x0583, 0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD,
	0x0BD0, 0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
	0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B, 0x3BB9,
	0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462, 0x7FFF
};

static const s16 wavedutytbl[8][8] = {
	{ -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, -0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF },
	{ -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF, -0x7FFF }
};

static s32 precalcdifftbl[89][16];
static u8 precalcindextbl[89][8];

static const float ARM7_CLOCK = 33513982;

//////////////////////////////////////////////////////////////////////////////

template<typename T>
static FORCEINLINE T MinMax(T val, T min, T max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;

	return val;
}

//////////////////////////////////////////////////////////////////////////////

extern "C" int SPU_ChangeSoundCore(NDS_state *state, SoundInterface_struct *core)
{
	// Make sure the old core is freed
	if (state->SNDCore)
		state->SNDCore->DeInit(state);

  state->SNDCore = core;

	if (state->SNDCore == NULL)
		return -1;

	// Since it failed, instead of it being fatal, disable the user spu
	if (state->SNDCore->Init(state, state->SPU_core->bufsize * 2) == -1)
	{
		state->SNDCore = 0;
		return -1;
	}

	return 0;
}

extern "C" void SPU_Reset(NDS_state *state)
{
	int i;

	state->SPU_core->reset();

	// Reset Registers
	for (i = 0x400; i < 0x51D; i++)
		T1WriteByte(state->MMU->ARM7Mem->ARM7_REG, i, 0);
}


//////////////////////////////////////////////////////////////////////////////

extern "C" int SPU_Init(NDS_state *state, int coreid, int buffersize)
{
	int i, j;

	state->SPU_core = new SPU_struct(state, 44100); //pick a really big number just to make sure the plugin doesnt request more
	SPU_Reset(state);

	for(i = 0; i < 16; i++)
	{
		for(j = 0; j < 89; j++)
		{
			precalcdifftbl[j][i] = (((i & 0x7) * 2 + 1) * adpcmtbl[j] / 8);
			if(i & 0x8) precalcdifftbl[j][i] = -precalcdifftbl[j][i];
		}
	}

	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 89; j++)
		{
			precalcindextbl[j][i] = MinMax((j + indextbl[i]), 0, 88);
		}
	}

	//return SPU_ChangeSoundCore(coreid, buffersize);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
struct SamplesCache
{
  std::map<u32, std::vector<s16>> Samples;
  
  void Lookup(channel_struct& chan)
  {
    auto& buf = Samples[chan.addr];
    if (buf.size() < chan.samplimit)
    {
      buf.resize(chan.samplimit);
      Decrunch(chan.buf8, buf);
    }
    chan.buf16 = buf.data();
    if (chan.samploop < 8)
    {
      chan.samploop = 8;
    }
  }
  
private:
  static void Decrunch(const s8* adpcm, std::vector<s16>& result)
  {
    s16 pcm16b = (s16)((adpcm[1] << 8) | adpcm[0]);
    int index = adpcm[2] & 0x7F;
    auto it = std::fill_n(result.begin(), 8, 0);
    const auto lim = result.end();
    adpcm += 4;
    while (it < lim)
    {
      auto data = (u32)(*adpcm);
      ++adpcm;
      
      const auto diff_lo = precalcdifftbl[index][data & 0xF];
      index = precalcindextbl[index][data & 0x7];
      *it = pcm16b = (s16)(MinMax(pcm16b + diff_lo, -0x8000, 0x7FFF));
      ++it;

      data >>= 4;
      const auto diff_hi = precalcdifftbl[index][data & 0xF];
      index = precalcindextbl[index][data & 0x7];
      *it = pcm16b = (s16)(MinMax(pcm16b + diff_hi, -0x8000, 0x7FFF));
      ++it;
    }
  }
};


void SPU_struct::reset()
{
	memset(sndbuf,0,bufsize*2*4);
	memset(outbuf,0,bufsize*2*2);

	memset((void *)channels, 0, sizeof(channel_struct) * 16);

	for(int i = 0; i < 16; i++)
	{
		channels[i].num = i;
	}
}

SPU_struct::SPU_struct(struct NDS_state *pstate, int buffersize)
	: state(pstate)
    , bufpos(0)
	, buflength(0)
	, sndbuf(0)
	, outbuf(0)
  , cache(0)
	, bufsize(buffersize)
{
	sndbuf = new s32[buffersize*2];
	outbuf = new s16[buffersize*2];
  cache = new SamplesCache();
	reset();
}

SPU_struct::~SPU_struct()
{
  delete cache;
	if(sndbuf) delete[] sndbuf;
	if(outbuf) delete[] outbuf;
}

extern "C" void SPU_DeInit(NDS_state *state)
{
	if(state->SNDCore)
		state->SNDCore->DeInit(state);
	state->SNDCore = 0;

	delete state->SPU_core; state->SPU_core=0;
}

//////////////////////////////////////////////////////////////////////////////

void SPU_struct::ShutUp()
{
	for(int i=0;i<16;i++)
		 channels[i].status = CHANSTAT_STOPPED;
}

static FORCEINLINE void adjust_channel_timer(channel_struct *chan)
{
	chan->sampinc = float(ARM7_CLOCK) / (44100 * 2) / (0x10000 - chan->timer);
}

static FORCEINLINE void adjust_channel_sample(channel_struct *chan)
{
   const int shift = format_shift[chan->format];
   chan->samploop = chan->loopstart << shift;
   chan->samplimit = chan->samploop + (chan->length << shift);
}

void SPU_struct::KeyOn(int channel)
{
	channel_struct &thischan = channels[channel];

    thischan.init_resampler();
    resampler_clear(thischan.resampler);
    resampler_set_quality(thischan.resampler, thischan.format == 3 ? RESAMPLER_QUALITY_BLEP : spuInterpolationMode(state));

	adjust_channel_timer(&thischan);
  adjust_channel_sample(&thischan);

  MMU_Core_struct* const core = state->MMU->Cores + 1;
	switch(thischan.format)
	{
	case 0: // 8-bit
		thischan.buf8 = (const s8*)&core->MemMap[(thischan.addr>>20)&0xFF][(thischan.addr & core->MemMask[(thischan.addr >> 20) & 0xFF])];
		thischan.samppos = 0;
		break;
	case 1: // 16-bit
		thischan.buf16 = (const s16 *)&core->MemMap[(thischan.addr>>20)&0xFF][(thischan.addr & core->MemMask[(thischan.addr >> 20) & 0xFF])];
		thischan.samppos = 0;
		break;
	case 2: // ADPCM
 		thischan.buf8 = (const s8*)&core->MemMap[(thischan.addr>>20)&0xFF][(thischan.addr & core->MemMask[(thischan.addr >> 20) & 0xFF])];
 		thischan.samppos = 8;
    cache->Lookup(thischan);
 		break;
	case 3: // PSG
    /*
      Via: http://problemkaputt.de/gbatek.htm#dssoundnotes

      Each duty cycle consists of eight HIGH or LOW samples, so the sound frequency is 1/8th of the selected sample rate.
      The duty cycle always starts at the begin of the LOW period when the sound gets (re-)started.
      
      Noise randomly switches between HIGH and LOW samples <...> The initial value when (re-)starting the sound is X=7FFFh
    */
 		thischan.x = 0x7FFF;
 		thischan.lastsamppos = 0;
  	thischan.samppos = 0;
 		break;
	default:
    break;
	}

	if(thischan.samplimit == 0 && thischan.format != 3)
	{
 		thischan.status = CHANSTAT_STOPPED;
	}
}

//////////////////////////////////////////////////////////////////////////////

void SPU_struct::WriteByte(u32 addr, u8 val)
{
	channel_struct &thischan=channels[(addr >> 4) & 0xF];
	switch(addr & 0xF) {
		case 0x0:
			thischan.vol = val & 0x7F;
			break;
		case 0x1: {
			thischan.datashift = val & 0x3;
			if (thischan.datashift == 3)
				thischan.datashift = 4;
			thischan.hold = (val >> 7) & 0x1;
			break;
		}
		case 0x2:
			thischan.pan = val & 0x7F;
			break;
		case 0x3: {
			thischan.waveduty = val & 0x7;
			thischan.repeat = (val >> 3) & 0x3;
			thischan.format = (val >> 5) & 0x3;
			thischan.status = (val >> 7) & 0x1;
			if(thischan.status)
				KeyOn((addr >> 4) & 0xF);
			break;
		}
	}

}

extern "C" void SPU_WriteByte(struct NDS_state *state, u32 addr, u8 val)
{
	addr &= 0xFFF;

	if (addr < 0x500)
	{
		state->SPU_core->WriteByte(addr,val);
	}

	T1WriteByte(state->MMU->ARM7Mem->ARM7_REG, addr, val);
}

//////////////////////////////////////////////////////////////////////////////

void SPU_struct::WriteWord(u32 addr, u16 val)
{
	channel_struct &thischan=channels[(addr >> 4) & 0xF];
	switch(addr & 0xF)
	{
	case 0x0:
		thischan.vol = val & 0x7F;
		thischan.datashift = (val >> 8) & 0x3;
		if (thischan.datashift == 3)
			thischan.datashift = 4;
		thischan.hold = (val >> 15) & 0x1;
		break;
	case 0x2:
		thischan.pan = val & 0x7F;
		thischan.waveduty = (val >> 8) & 0x7;
		thischan.repeat = (val >> 11) & 0x3;
		thischan.format = (val >> 13) & 0x3;
		thischan.status = (val >> 15) & 0x1;
		if (thischan.status)
			KeyOn((addr >> 4) & 0xF);
		break;
	case 0x8:
		thischan.timer = val & 0xFFFF;
		adjust_channel_timer(&thischan);
		break;
	case 0xA:
		thischan.loopstart = val;
    adjust_channel_sample(&thischan);
		break;
	case 0xC:
		WriteLong(addr,((u32)T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, addr+2) << 16) | val);
		break;
	case 0xE:
		WriteLong(addr,((u32)T1ReadWord(state->MMU->ARM7Mem->ARM7_REG, addr-2)) | ((u32)val<<16));
		break;
	}
}

extern "C" void SPU_WriteWord(NDS_state *state, u32 addr, u16 val)
{
	addr &= 0xFFF;

	if (addr < 0x500)
	{
		state->SPU_core->WriteWord(addr,val);
	}

	T1WriteWord(state->MMU->ARM7Mem->ARM7_REG, addr, val);
}

//////////////////////////////////////////////////////////////////////////////

void SPU_struct::WriteLong(u32 addr, u32 val)
{
	channel_struct &thischan=channels[(addr >> 4) & 0xF];
	switch(addr & 0xF)
	{
	case 0x0:
		thischan.vol = val & 0x7F;
		thischan.datashift = (val >> 8) & 0x3;
		if (thischan.datashift == 3)
			thischan.datashift = 4;
		thischan.hold = (val >> 15) & 0x1;
		thischan.pan = (val >> 16) & 0x7F;
		thischan.waveduty = (val >> 24) & 0x7;
		thischan.repeat = (val >> 27) & 0x3;
		thischan.format = (val >> 29) & 0x3;
		thischan.status = (val >> 31) & 0x1;
		if (thischan.status)
			KeyOn((addr >> 4) & 0xF);
		break;
	case 0x4:
		thischan.addr = val & 0x7FFFFFF;
		break;
	case 0x8:
		thischan.timer = val & 0xFFFF;
		thischan.loopstart = val >> 16;
  	adjust_channel_timer(&thischan);
    adjust_channel_sample(&thischan);
		break;
	case 0xC:
		thischan.length = val & 0x3FFFFF;
    adjust_channel_sample(&thischan);
		break;
	}
}

extern "C" void SPU_WriteLong(NDS_state *state, u32 addr, u32 val)
{
	addr &= 0xFFF;

	if (addr < 0x500)
	{
		state->SPU_core->WriteLong(addr,val);
	}

	T1WriteLong(state->MMU->ARM7Mem->ARM7_REG, addr, val);
}

//////////////////////////////////////////////////////////////////////////////
static FORCEINLINE s32 Fetch8BitDataInternal(channel_struct * const chan)
{
	return (s32)chan->buf8[chan->samppos] << 8;
}

static FORCEINLINE s32 Fetch16BitDataInternal(channel_struct * const chan)
{
	return (s32)chan->buf16[chan->samppos];
}

static FORCEINLINE s32 FetchPSGDataInternal(channel_struct *chan)
{
	if(chan->num < 8)
	{
		return 0;
	}
	else if(chan->num < 14)
	{
		return (s32)wavedutytbl[chan->waveduty][chan->samppos & 0x7];
	}
	else
	{
    for (; chan->lastsamppos < chan->samppos; ++chan->lastsamppos)
		{
			if(chan->x & 0x1)
			{
				chan->x = (chan->x >> 1);
				chan->psgnoise_last = -0x7FFF;
			}
			else
			{
				chan->x = ((chan->x >> 1) ^ 0x6000);
				chan->psgnoise_last = 0x7FFF;
			}
		}

		return (s32)chan->psgnoise_last;
	}
}

//////////////////////////////////////////////////////////////////////////////

static FORCEINLINE void MixL(SPU_struct* SPU, channel_struct *chan, s32 data)
{
	data = spumuldiv7(data, chan->vol) >> chan->datashift;
	SPU->sndbuf[SPU->bufpos<<1] += data;
}

static FORCEINLINE void MixR(SPU_struct* SPU, channel_struct *chan, s32 data)
{
	data = spumuldiv7(data, chan->vol) >> chan->datashift;
	SPU->sndbuf[(SPU->bufpos<<1)+1] += data;
}

static FORCEINLINE void MixLR(SPU_struct* SPU, channel_struct *chan, s32 data)
{
	data = spumuldiv7(data, chan->vol) >> chan->datashift;
	SPU->sndbuf[SPU->bufpos<<1] += spumuldiv7fast(data, 127 - chan->pan);
	SPU->sndbuf[(SPU->bufpos<<1)+1] += spumuldiv7fast(data, chan->pan);
}

typedef s32 (*FetchFunc)(channel_struct *chan);
typedef bool (*FinishFunc)(channel_struct *chan);
typedef void (*MixFunc)(SPU_struct* SPU, channel_struct *chan, s32 data);

//////////////////////////////////////////////////////////////////////////////
static FORCEINLINE void StopChannel(SPU_struct *SPU, channel_struct *chan)
{
  NDS_state* const state = SPU->state;
  chan->status = CHANSTAT_STOPPED;
  if(SPU == state->SPU_core)
  {
   		state->MMU->ARM7Mem->ARM7_REG[0x403 + (((chan-SPU->channels) ) * 0x10)] &= 0x7F;
  }
}

static FORCEINLINE bool FinishedSample(channel_struct *chan)
{
	if (chan->samppos >= chan->samplimit)
	{
		// Do we loop? Or are we done?
		if (chan->repeat == 1)
		{
		  chan->samppos -= chan->samplimit - chan->samploop;
		}
		else
		{
      return true;
    }
  }
  return false;
}

static FORCEINLINE bool FinishedPSGSample(channel_struct */*chan*/)
{
  return false;
}

template<FetchFunc Fetch, FinishFunc Finished, MixFunc Mix>
static void RenderSample(SPU_struct* const SPU, channel_struct *chan)
{
	resampler_set_rate( chan->resampler, chan->sampinc );

  for (;;)
	{
    if (chan->status != CHANSTAT_EMPTYBUFFER)
    {
      //push original
      if (const int inSamples = resampler_get_free_count(chan->resampler))
      {
        for (int cnt = 0; cnt < inSamples; ++cnt)
        {
       		const s32 data = Fetch(chan);
       		resampler_write_sample(chan->resampler, data);
          ++chan->samppos;
          if (Finished(chan))
          {
            chan->status = CHANSTAT_EMPTYBUFFER;
            break;
          }
        }
      }
    }
    if (const int outSamples = resampler_get_sample_count(chan->resampler))
    {
      //pull resampled
      const u32 limit = std::min<u32>(SPU->buflength, SPU->bufpos + outSamples);
      for (; SPU->bufpos < limit; ++SPU->bufpos)
      {
       	const s32 data = resampler_get_sample(chan->resampler);
       	resampler_remove_sample(chan->resampler, 1);
        Mix(SPU, chan, data);
      }
      if (limit == SPU->buflength)
      {
        break;
      }
    }
    else
    {
      StopChannel(SPU, chan);
      break;
    }
  }
}

template<MixFunc Mix>
static void __SPU_ChanUpdate(SPU_struct* const SPU, channel_struct* const chan)
{
	switch(chan->format)
	{
		case 0: RenderSample<Fetch8BitDataInternal, FinishedSample, Mix>(SPU, chan); break;
		case 1: 
		case 2: RenderSample<Fetch16BitDataInternal, FinishedSample, Mix>(SPU, chan); break;
		case 3: RenderSample<FetchPSGDataInternal, FinishedPSGSample, Mix>(SPU, chan); break;
	}
}

FORCEINLINE static void _SPU_ChanUpdate(SPU_struct* const SPU, channel_struct* const chan)
{
  if (chan->pan == 0)
    __SPU_ChanUpdate<MixL>(SPU, chan);
	else if (chan->pan == 127)
    __SPU_ChanUpdate<MixR>(SPU, chan);
	else
    __SPU_ChanUpdate<MixLR>(SPU, chan);
}

static void SPU_MixAudio(NDS_state *state, SPU_struct *SPU, int length)
{
	memset(SPU->sndbuf, 0, length*4*2);
	
	const u8 vol = T1ReadByte(state->MMU->ARM7Mem->ARM7_REG, 0x500) & 0x7F;

	for(int i=0;i<16;i++)
	{
		channel_struct *chan = &SPU->channels[i];
	
		if (isChannelMuted(state, i) || 
        !chan->resampler || 
        chan->status == CHANSTAT_STOPPED)
    {
			continue;
    }

		SPU->bufpos = 0;
		SPU->buflength = length;

		// Mix audio
		_SPU_ChanUpdate(SPU, chan);
	}

  // convert from 32-bit->16-bit
  if (vol == 0x7f)
  {
  	for (int i = 0; i < length*2; i++)
		{
			// Apply Master Volume
      SPU->outbuf[i] = MinMax(SPU->sndbuf[i], -0x8000, 0x7FFF);
		}
  }
  else
  {
		for (int i = 0; i < length*2; i++)
		{
			// Apply Master Volume
      SPU->outbuf[i] = MinMax(spumuldiv7fast(SPU->sndbuf[i], vol), -0x8000, 0x7FFF);
		}
  }
}

static void _SPU_ChanSkip(SPU_struct* const SPU, channel_struct* const chan, int numsamples)
{
  chan->samppos += numsamples;
  if (chan->format != 3 && chan->samppos >= chan->samplimit)
  {
    if (chan->repeat == 1)
    {
      chan->samppos = chan->samploop + (chan->samppos - chan->samplimit) % (chan->samplimit - chan->samploop);
    }
    else
    {
      StopChannel(SPU, chan);
      chan->samppos = chan->samplimit;
    }
  }
}

static void SPU_SkipAudio(NDS_state *state, SPU_struct *SPU, int length)
{
  for(int i=0;i<16;i++)
	{
		channel_struct *chan = &SPU->channels[i];
	
		if (isChannelMuted(state, i) || 
        !chan->resampler || 
        chan->status == CHANSTAT_STOPPED)
    {
			continue;
    }

		_SPU_ChanSkip(SPU, chan, chan->sampinc * length);
	}
}

//////////////////////////////////////////////////////////////////////////////

extern "C" void SPU_EmulateSamples(NDS_state *state, int numsamples)
{
	SPU_MixAudio(state, state->SPU_core,numsamples);
	state->SNDCore->UpdateAudio(state, state->SPU_core->outbuf, numsamples);
}

extern "C" void SPU_SkipSamples(NDS_state *state, int numsamples)
{
  SPU_SkipAudio(state, state->SPU_core, numsamples);
}
