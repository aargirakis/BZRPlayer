/*-----------------------------------------------------------------------------

	ST-Sound ( YM files player library )

	Copyright (C) 1995-1999 Arnaud Carre ( http://leonard.oxg.free.fr )

	Extended YM-2149 Emulator, with ATARI music demos effects.
	(SID-Like, Digidrum, Sync Buzzer, Sinus SID and Pattern SID)

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

	This file is part of ST-Sound

	ST-Sound is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	ST-Sound is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ST-Sound; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-----------------------------------------------------------------------------*/

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdio>

#ifdef _WIN32
#define BUILDING_DLL
#endif

#include "Ym2149Ex.h"
#include "YmProfiles.h"

//-------------------------------------------------------------------
// env shapes.
//-------------------------------------------------------------------
static	const ymint		Env00xx[8]={ 1,0,0,0,0,0,0,0 };
static	const ymint		Env01xx[8]={ 0,1,0,0,0,0,0,0 };
static	const ymint		Env1000[8]={ 1,0,1,0,1,0,1,0 };
static	const ymint		Env1001[8]={ 1,0,0,0,0,0,0,0 };
static	const ymint		Env1010[8]={ 1,0,0,1,1,0,0,1 };
static	const ymint		Env1011[8]={ 1,0,1,1,1,1,1,1 };
static	const ymint		Env1100[8]={ 0,1,0,1,0,1,0,1 };
static	const ymint		Env1101[8]={ 0,1,1,1,1,1,1,1 };
static	const ymint		Env1110[8]={ 0,1,1,0,0,1,1,0 };
static	const ymint		Env1111[8]={ 0,1,0,0,0,0,0,0 };
static	const ymint *	EnvWave[16] = {	Env00xx,Env00xx,Env00xx,Env00xx,
										Env01xx,Env01xx,Env01xx,Env01xx,
										Env1000,Env1001,Env1010,Env1011,
										Env1100,Env1101,Env1110,Env1111};

static	ymint ymVolumeTable[5][16] =
// Original STSound values
//{	62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767};

// Log scale hand-calculated by CloudStrife
//{0,256,362,512,724,1024,1448,2048,2896,4096,5793,8192,11585,16384,23170,32767};

// Measurements done on a real CPC by Targhan/Arkos
// Note : when there is noise, the table is altered. We have 5 different ones
{
{0,116,348,579,1042,1390,2084,3358,4053,6600,9147,12157,16094,20378,26400,32767},
{0,0,  348,348,1042,1042,2084,2084,4053,4053,9147,12157,16094,16094,26400,32767},
{0,0,  0,  0,  3358,3358,3358,4053,4053,4053,6600,6600,  6600,32767,32767,32767},
{0,0,  0,  0,  0,   4053,4053,4053,4053,4053,4053,32767,32767,32767,32767,32767},
{0,0,  0,  0,  0,   0,  4053,4053,4053,4053,32767,32767,32767,32767,32767,32767}
};

/*
 * 15 = 65535
 * 14 = 52799
 * 13 = 40757
 * 12 = 32189
 * 11 = 24315
 * 10 = 18294
 * 9 = 13200
 * 8 = 8105
 * 7 = 6716
 * 6 = 4168
 * 5 = 2779
  4 = 2084
 * 3 = 1158
 * 2 = 695
 * 1 = 116
 * 0 = 0
 */



static ymu8 *ym2149EnvInit(ymu8 *pEnv,ymint a,ymint b)
{
    ymint i;
    ymint d;

    d = b - a;
    a *= 15;

    for(i = 0; i < 16; i++)
    {
        *pEnv++ = (ymu8)a;
        a += d;
    }

    return pEnv;
}


CYm2149Ex::CYm2149Ex(ymu32 masterClock,ymint prediv,ymu32 playRate)
{
    ymint env;


    frameCycle = 0;
    //--------------------------------------------------------
    // build env shapes.
    //--------------------------------------------------------
    ymu8 *pEnv = &envData[0][0][0];
    for (env=0;env<16;env++)
    {
        const ymint *pse = EnvWave[env];
        for (ymint phase=0;phase<4;phase++)
        {
            pEnv = ym2149EnvInit(pEnv,pse[phase*2+0],pse[phase*2+1]);
        }
    }

    internalClock = masterClock/prediv;		// YM at 2Mhz on ATARI ST
    replayFrequency = playRate;				// DAC at 44.1Khz on PC
    cycleSample = 0;

    // Set volume voice pointers.
    pVolA = &volA;
    pVolB = &volB;
    pVolC = &volC;

//    setProfile(profileAtari);
    setProfile(profileCPC);

    for(ymint i = 0; i < 3; i++)
    {
        filters[i].push_back(new SimpleLowPassFilter());
        filters[i].push_back(new DCRemover());
    }

    // Reset YM2149
    reset();
}


CYm2149Ex::CYm2149Ex(ymProfile profile, ymu32 playRate)
{
    ymint env;

    frameCycle = 0;
    //--------------------------------------------------------
    // build env shapes.
    //--------------------------------------------------------
    ymu8 *pEnv = &envData[0][0][0];
    for (env=0;env<16;env++)
    {
        const ymint *pse = EnvWave[env];
        for (ymint phase=0;phase<4;phase++)
        {
            pEnv = ym2149EnvInit(pEnv,pse[phase*2+0],pse[phase*2+1]);
        }
    }

    setProfile(profile);

    internalClock = profile.masterClock;
    replayFrequency = playRate;
    cycleSample = 0;

    // Set volume voice pointers.
    pVolA = &volA;
    pVolB = &volB;
    pVolC = &volC;

    for(ymint i = 0; i < 3; i++)
    {
        filters[i].push_back(new SimpleLowPassFilter());
        filters[i].push_back(new DCRemover());
    }

    // Reset YM2149
    reset();
}

CYm2149Ex::~CYm2149Ex()
{
}

void	CYm2149Ex::setClock(ymu32 _clock)
{
		internalClock = _clock;
}


ymu32 CYm2149Ex::toneStepCompute(ymu8 rHigh,ymu8 rLow)
{
	ymint per = rHigh&15;
	per = (per<<8)+rLow;
	if (per<=5)
	{
		return 0;
	}

#ifdef YM_INTEGER_ONLY
	yms64 step = internalClock;
	step <<= (15+16-3);
	step /= (per * replayFrequency);
#else
	ymfloat step = internalClock / (per * 8.0f * replayFrequency);
	step *= 32768.0f*65536.0f;
#endif
	ymu32 istep = (ymu32)step;
	return istep;
}

ymu32 CYm2149Ex::noiseStepCompute(ymu8 rNoise)
{
	ymint per = (rNoise&0x1f);
	if (per == 0)
		return 0;

#ifdef YM_INTEGER_ONLY
	yms64 step = internalClock;
	step <<= (16-1-3);
	step /= (per * replayFrequency);
#else
	ymfloat step = internalClock * 4096.0f;
	step /= (ymfloat)per * (ymfloat)replayFrequency;
	/*
	step /= ((ymfloat)per*8.0*(ymfloat)replayFrequency);
	step *= 65536.0/2.0;
	*/
#endif

	return (ymu32)step;
}

/**
 * Emulation of noise channel
 */
ymu32	CYm2149Ex::rndCompute(void)
{
	/*
		ymint	rBit = (rndRack&1) ^ ((rndRack>>2)&1);
		rndRack = (rndRack>>1) | (rBit<<16);
		return (rBit ? 0 : 0xffff);
*/
		rndRack = ( ( ((rndRack & 1)>0) ^ ((rndRack & 8)>0)) ? 0x10000 : 0) | (rndRack >> 1);
		return rndRack & 1 ? 0xFFFF:0;
}

ymu32 CYm2149Ex::envStepCompute(ymu8 rHigh,ymu8 rLow)
{


	ymint per = rHigh;
	per = (per<<8)+rLow;
	if (per<3)
		return 0;

#ifdef YM_INTEGER_ONLY
	yms64 step = internalClock;
	step <<= (16+16-9);
	step /= (per * replayFrequency);
#else
	ymfloat step = internalClock / (per*512.0f*replayFrequency);
	step *= 65536.0f*65536.0f;
#endif

	return (ymu32)step;
}

/**
 * Reset of all internal registers of the PSG
 */
void	CYm2149Ex::reset(void)
{

	for (int i=0;i<14;i++)
	{
		registers[i] = 0; // set to 0 of all register
	}

	for (int i=0;i<14;i++)
	{
		writeRegister(i,0); // write and initialisation of register
	}

	writeRegister(7,0xff);

	currentNoise = 0xffff;
	rndRack = 1;
	/*sidStop(0);
	sidStop(1);
	sidStop(2);
*/
	envShape = 0;
	envPhase = 0;
	envPos = 0;

/*	memset(specialEffect,0,sizeof(specialEffect));

	syncBuzzerStop();
*/

    // Reset of all filter
    std::list<Filter*>::iterator f_it;

    for(ymint i = 0; i < 3; i++)
    {
        for(f_it = filters[i].begin(); f_it != filters[i].end(); f_it++)
        {
            (*f_it)->Reset();
        }
    }
}

/*
void	CYm2149Ex::sidVolumeCompute(ymint voice,ymint *pVol)
{

		struct	YmSpecialEffect	*pVoice = specialEffect+voice;

		if (pVoice->bSid)
		{
			if (pVoice->sidPos & (1<<31))
				writeRegister(8+voice,pVoice->sidVol);
			else
				writeRegister(8+voice,0);
		}
		else if (pVoice->bDrum)
		{
//			writeRegister(8+voice,pVoice->drumData[pVoice->drumPos>>DRUM_PREC]>>4);

			*pVol = (pVoice->drumData[pVoice->drumPos>>DRUM_PREC] * 255) / 6;

			switch (voice)
			{
				case 0:
					pVolA = &volA;
					mixerTA = 0xffff;
					mixerNA = 0xffff;
					break;
				case 1:
					pVolB = &volB;
					mixerTB = 0xffff;
					mixerNB = 0xffff;
					break;
				case 2:
					pVolC = &volC;
					mixerTC = 0xffff;
					mixerNC = 0xffff;
					break;
			}

			pVoice->drumPos += pVoice->drumStep;
			if ((pVoice->drumPos>>DRUM_PREC) >= pVoice->drumSize)
			{
				pVoice->bDrum = YMFALSE;
			}

		}
}
*/

ymsample CYm2149Ex::nextSample(void)
{
    ymint vol;
    ymint bt,bn;

		if (noisePos&0xffff0000)
		{
			currentNoise = rndCompute();
			noisePos &= 0xffff;
		}
		bn = currentNoise;

		volEA = std::min(ymVolumeTable[std::min((int)(~mixerNA&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volEB = std::min(ymVolumeTable[std::min((int)(~mixerNB&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volEC = std::min(ymVolumeTable[std::min((int)(~mixerNC&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volA = std::min(ymVolumeTable[std::min((int)(~mixerNA&currentNoise)&registers[6],5)][registers[8]&15],ymVolumeTable[0][registers[8]&15]);
		volB = std::min(ymVolumeTable[std::min((int)(~mixerNB&currentNoise)&registers[6],5)][registers[9]&15],ymVolumeTable[0][registers[9]&15]);
		volC = std::min(ymVolumeTable[std::min((int)(~mixerNC&currentNoise)&registers[6],5)][registers[10]&15],ymVolumeTable[0][registers[10]&15]);
/*
		sidVolumeCompute(0,&volA);
		sidVolumeCompute(1,&volB);
		sidVolumeCompute(2,&volC);
*/
	//---------------------------------------------------
	// Tone+noise+env+DAC for three voices !
	//---------------------------------------------------
    // bn = 0xFFFF or 0
    // mixerNx = 0xFFFF or 0
    // mixerTx = 0xFFFF or 0
    // bt = 0xFFFF or 0
#ifdef YM_INTEGER_ONLY
    bt = ((((yms32)posA)>>31) | mixerTA) & (bn | mixerNA);
    vol  = (*pVolA)&bt * vOut[0]; // 16*8 = 24 bits
    bt = ((((yms32)posB)>>31) | mixerTB) & (bn | mixerNB);
    vol += (*pVolB)&bt * vOut[1];
    bt = ((((yms32)posC)>>31) | mixerTC) & (bn | mixerNC);
    vol += (*pVolC)&bt * vOut[2];
    vol >>= 7;
#else
    bt = ((((yms32)posA)>>31) | mixerTA) & (bn | mixerNA);
    vol  = (ymint)(((*pVolA)&bt) * vOut[0]);
    bt = ((((yms32)posB)>>31) | mixerTB) & (bn | mixerNB);
    vol += (ymint)(((*pVolB)&bt) * vOut[1]);
    bt = ((((yms32)posC)>>31) | mixerTC) & (bn | mixerNC);
    vol += (ymint)(((*pVolC)&bt) * vOut[2]);
#endif

	//---------------------------------------------------
	// Inc
	//---------------------------------------------------
		posA += stepA;
		posB += stepB;
		posC += stepC;
		noisePos += noiseStep;
		envPos += envStep;
		if (0 == envPhase)
		{
			if (envPos<envStep)
			{
				envPhase = 1;
			}
		}
/*
		syncBuzzerPhase += syncBuzzerStep;
		if (syncBuzzerPhase&(1<<31))
		{
			envPos = 0;
			envPhase = 0;
			syncBuzzerPhase &= 0x7fffffff;
		}

		specialEffect[0].sidPos += specialEffect[0].sidStep;
		specialEffect[1].sidPos += specialEffect[1].sidStep;
		specialEffect[2].sidPos += specialEffect[2].sidStep;
*/

    // Apply all mono filters
    std::list<Filter*>::iterator f_it;

    ymint in = vol;
    for(f_it = filters[F_MONO].begin(); f_it != filters[F_MONO].end(); f_it++)
    {
        (*f_it)->AddSample(in);
        in = (*f_it)->GetResult();
    }
    return in;
}


void CYm2149Ex::nextSampleStereo(ymsample& left, ymsample& right)
{
		if (noisePos&0xffff0000)
		{
			currentNoise = rndCompute();
			noisePos &= 0xffff;
		}
		ymint bn = currentNoise;

		// TODO - Generate only the one we're going to use (pVolA, pVolB, pVolC points on one of these only)
		volEA = std::min(ymVolumeTable[std::min((int)(~mixerNA&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volEB = std::min(ymVolumeTable[std::min((int)(~mixerNB&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volEC = std::min(ymVolumeTable[std::min((int)(~mixerNC&currentNoise)&registers[6],5)][envData[envShape][envPhase][envPos>>(32-5)]],ymVolumeTable[0][envData[envShape][envPhase][envPos>>(32-5)]]);
		volA = std::min(ymVolumeTable[std::min((int)(~mixerNA&currentNoise)&registers[6],5)][registers[8]&15],ymVolumeTable[0][registers[8]&15]);
		volB = std::min(ymVolumeTable[std::min((int)(~mixerNB&currentNoise)&registers[6],5)][registers[9]&15],ymVolumeTable[0][registers[9]&15]);
		volC = std::min(ymVolumeTable[std::min((int)(~mixerNC&currentNoise)&registers[6],5)][registers[10]&15],ymVolumeTable[0][registers[10]&15]);

/*		sidVolumeCompute(0,&volA);
		sidVolumeCompute(1,&volB);
		sidVolumeCompute(2,&volC);
*/
	//---------------------------------------------------
	// Tone+noise+env+DAC for three voices !
	//---------------------------------------------------
#ifdef YM_INTEGER_ONLY
    ymint volLeft, volRight;
    ymint bt; // TODO: How many bits is bt ?!?

    // bn = 0xFFFF ou 0
    // mixerNx = 0xFFFF ou 0
    // mixerTx = 0xFFFF ou 0
    // bt = 0xFFFF ou 0

    bt = ((((yms32)posA)>>31) | mixerTA) & (bn | mixerNA);
    volLeft = ((*pVolA)&bt) * vLeftOut[0];
    volRight = ((*pVolA)&bt) * vRightOut[0];
    bt = ((((yms32)posB)>>31) | mixerTB) & (bn | mixerNB);
    volLeft += ((*pVolB)&bt) * vLeftOut[1];
    volRight =+ ((*pVolB)&bt) * vRightOut[1];
    bt = ((((yms32)posC)>>31) | mixerTC) & (bn | mixerNC);
    volLeft += ((*pVolC)&bt) * vLeftOut[2];
    volRight =+ ((*pVolC)&bt) * vRightOut[2];
    volLeft >>= 7;
    volRight >>= 7;
#else
    ymfloat volLeft, volRight;
    ymint bt; // bt = enable tone

    bt = ((((yms32)posA)>>31) | mixerTA) & (bn | mixerNA);
//    volLeft = ((*pVolA)&bt)*.687*0.66;
    volLeft = ((*pVolA)&bt)*vLeftOut[0]*0.66f; // TODO: Why *0.66 ?!? Don't remeber :/
    volRight = ((*pVolA)&bt)*vRightOut[0]*0.66f;
    bt = ((((yms32)posB)>>31) | mixerTB) & (bn | mixerNB);
//    volRight = ((*pVolB)&bt)*.687*0.66;
    volLeft += ((*pVolB)&bt)*vLeftOut[1]*0.66f;
    volRight += ((*pVolB)&bt)*vRightOut[1]*0.66f;
    bt = ((((yms32)posC)>>31) | mixerTC) & (bn | mixerNC);
//    volLeft += ((*pVolC)&bt)*.312*0.67;
//    volRight += ((*pVolC)&bt)*.312*0.67;
    volLeft += ((*pVolC)&bt)*vLeftOut[2]*0.66f;
    volRight += ((*pVolC)&bt)*vRightOut[2]*0.66f;
#endif

	//---------------------------------------------------
	// Inc
	//---------------------------------------------------
		posA += stepA;
		posB += stepB;
		posC += stepC;
		noisePos += noiseStep;
		envPos += envStep;
		if (0 == envPhase)
		{
			if (envPos<envStep)
			{
				envPhase = 1;
			}
		}

/*		syncBuzzerPhase += syncBuzzerStep;
		if (syncBuzzerPhase&(1<<31))
		{
			envPos = 0;
			envPhase = 0;
			syncBuzzerPhase &= 0x7fffffff;
		}

		specialEffect[0].sidPos += specialEffect[0].sidStep;
		specialEffect[1].sidPos += specialEffect[1].sidStep;
		specialEffect[2].sidPos += specialEffect[2].sidStep;
*/
	// Apply all filter from filters list.
    std::list<Filter*>::iterator f_it;

    ymint in[2] = {(ymint)volLeft, (ymint)volRight};
    for(ymint i = F_LEFT; i < F_RIGHT; i++)
    {
        for(f_it = filters[i].begin(); f_it != filters[i].end(); f_it++)
        {
            (*f_it)->AddSample(in[i]);
            in[i] = (*f_it)->GetResult();
        }
    }
    left = in[0];
    right = in[1];
}

ymint		CYm2149Ex::readRegister(ymint reg)
{
		if ((reg>=0) && (reg<=13)) return registers[reg];
		else return -1;
}

void	CYm2149Ex::writeRegister(ymint reg,ymint data)
{

		switch (reg)
		{
		case R_A_TONE_PERIOD_LOW:
			registers[R_A_TONE_PERIOD_LOW] = data & 0xFF;
			stepA = toneStepCompute(registers[R_A_TONE_PERIOD_HIGH], registers[R_A_TONE_PERIOD_LOW]);
			if (!stepA) posA = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_B_TONE_PERIOD_LOW:
			registers[R_B_TONE_PERIOD_LOW] = data & 0xFF;
			stepB = toneStepCompute(registers[R_B_TONE_PERIOD_HIGH], registers[R_B_TONE_PERIOD_LOW]);
			if (!stepB) posB = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_C_TONE_PERIOD_LOW:
			registers[R_C_TONE_PERIOD_LOW] = data & 0xFF;
			stepC = toneStepCompute(registers[R_C_TONE_PERIOD_HIGH], registers[R_C_TONE_PERIOD_LOW]);
			if (!stepC) posC = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_A_TONE_PERIOD_HIGH:
			registers[R_A_TONE_PERIOD_HIGH] = data & 0x0F;
			stepA = toneStepCompute(registers[R_A_TONE_PERIOD_HIGH], registers[R_A_TONE_PERIOD_LOW]);
			if (!stepA) posA = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_B_TONE_PERIOD_HIGH:
			registers[R_B_TONE_PERIOD_HIGH] = data & 0x0F;
			stepB = toneStepCompute(registers[R_B_TONE_PERIOD_HIGH], registers[R_B_TONE_PERIOD_LOW]);
			if (!stepB) posB = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_C_TONE_PERIOD_HIGH:
			registers[R_C_TONE_PERIOD_HIGH] = data & 0x0F;
			stepC = toneStepCompute(registers[R_C_TONE_PERIOD_HIGH], registers[R_C_TONE_PERIOD_LOW]);
			if (!stepC) posC = (1<<31);		// Assume output always 1 if 0 period (for Digi-sample !)
			break;

		case R_NOISE_PERIOD:
			registers[R_NOISE_PERIOD] = data&0x1f;
			noiseStep = noiseStepCompute(registers[R_NOISE_PERIOD]);
			if (!noiseStep)
			{
				noisePos = 0;
				currentNoise = 0xffff;
			}
			break;

		case R_ENABLE:
			registers[7] = data & 0xFF;
			mixerTA = (data&(1<<0)) ? 0xffff : 0;
			mixerTB = (data&(1<<1)) ? 0xffff : 0;
			mixerTC = (data&(1<<2)) ? 0xffff : 0;
			mixerNA = (data&(1<<3)) ? 0xffff : 0;
			mixerNB = (data&(1<<4)) ? 0xffff : 0;
			mixerNC = (data&(1<<5)) ? 0xffff : 0;
			break;

		case R_A_AMPLITUDE:
			registers[R_A_AMPLITUDE] = data & 0x1F;
			if (data & 0x10)
				pVolA = &volEA;
			else
				pVolA = &volA;
			break;

		case R_B_AMPLITUDE:
			registers[R_B_AMPLITUDE] = data & 0x1F;
			if (data & 0x10)
				pVolB = &volEB;
			else
				pVolB = &volB;
			break;

		case R_C_AMPLITUDE:
			registers[R_C_AMPLITUDE] = data & 0x1F;
			if (data & 0x10)
				pVolC = &volEC;
			else
				pVolC = &volC;
			break;

		case R_ENVELOPE_PERIOD_LOW:
			registers[R_ENVELOPE_PERIOD_LOW] = data & 0xFF;
			envStep = envStepCompute(registers[R_ENVELOPE_PERIOD_HIGH],registers[R_ENVELOPE_PERIOD_LOW]);
			break;

		case R_ENVELOPE_PERIOD_HIGH:
			registers[R_ENVELOPE_PERIOD_HIGH] = data & 0xFF;
			envStep = envStepCompute(registers[R_ENVELOPE_PERIOD_HIGH],registers[R_ENVELOPE_PERIOD_LOW]);
			break;

		case R_ENVELOPE_SHAPE:
			registers[R_ENVELOPE_SHAPE] = data & 0x0F;
			envPos = 0;
			envPhase = 0;
			envShape = data & 0x0F;
			break;
		}
}
/*
void	CYm2149Ex::update(ymsample *pSampleBuffer,ymint nbSample)
{
		ymsample *pBuffer = pSampleBuffer;
		if (nbSample>0)
		{
			do
			{
				*pBuffer++ = nextSampleStereo();
			}
			while (--nbSample);
		}
}
*/

/**
 * Generate nbSample sample and put in them in the buffer pSampleBuffer
 */
void CYm2149Ex::updateStereo(ymsample *pSampleBuffer,ymint nbSample)
{
    ymsample *pBuffer = pSampleBuffer;
    if (nbSample>0)
    {
        do
        {
            ymsample left, right;
            nextSampleStereo(left, right);
            *pBuffer++ = left;
            *pBuffer++ = right;
        }
        while (--nbSample);
    }
}
#if 0
void	CYm2149Ex::drumStart(ymint voice,ymu8 *pDrumBuffer,ymu32 drumSize,ymint drumFreq)
{
	specialEffect[voice].drumData = pDrumBuffer;
	specialEffect[voice].drumPos = 0;
	specialEffect[voice].drumSize = drumSize;
	specialEffect[voice].drumStep = (drumFreq<<DRUM_PREC)/replayFrequency;
	specialEffect[voice].bDrum = YMTRUE;
}

void	CYm2149Ex::drumStop(ymint voice)
{
		specialEffect[voice].bDrum = YMFALSE;
}

void	CYm2149Ex::sidStart(ymint voice,ymint timerFreq,ymint vol)
{
#ifdef YM_INTEGER_ONLY
		ymu32 tmp = timerFreq * ((1<<31) / replayFrequency);
#else
		ymfloat tmp = (ymfloat)timerFreq*((ymfloat)(1<<31))/(ymfloat)replayFrequency;
#endif
		specialEffect[voice].sidStep = (ymu32)tmp;
		specialEffect[voice].sidVol = vol&15;
		specialEffect[voice].bSid = YMTRUE;
}

void	CYm2149Ex::sidSinStart(ymint /*voice*/,ymint /*timerFreq*/,ymint /*vol*/)
{
	// TODO
}

void	CYm2149Ex::sidStop(ymint voice)
{
		specialEffect[voice].bSid = YMFALSE;
}

void	CYm2149Ex::syncBuzzerStart(ymint timerFreq,ymint /*_envShape*/)
{
#ifdef YM_INTEGER_ONLY
		ymu32 tmp = timerFreq * ((1<<31) / replayFrequency);
#else
		ymfloat tmp = (ymfloat)timerFreq*((ymfloat)(1<<31))/(ymfloat)replayFrequency;
#endif
		envShape = envShape&15;
		syncBuzzerStep = (ymu32)tmp;
		syncBuzzerPhase = 0;
		bSyncBuzzer = YMTRUE;
}

void	CYm2149Ex::syncBuzzerStop(void)
{
		bSyncBuzzer = YMFALSE;
		syncBuzzerPhase = 0;
		syncBuzzerStep = 0;
}
#endif

/**
 * Set mono output mixer ratio, after calculate if fixed point mode
 */
void CYm2149Ex::outputMixerMono(ymfloat out[3])
{
#if YM_INTEGER_ONLY
    // Fixed Point Mode: u1.7
    vOut[0] = (ymu8)(out[0] * 128);
    vOut[1] = (ymu8)(out[1] * 128);
    vOut[2] = (ymu8)(out[2] * 128);
#else
    vOut[0] = out[0];
    vOut[1] = out[1];
    vOut[2] = out[2];
#endif
}

/**
 * Set stereo output mixer ratio, after calculate if fixed point mode
 */
void CYm2149Ex::outputMixerStereo(ymfloat leftOut[3], ymfloat rightOut[3])
{
    // Set default output mixer.
#ifdef YM_INTEGER_ONLY
    // Fixed Point Mode: u1.7
    // Left Output
    vLeftOut[0] = (ymu8)(leftOut[0] * 128);
    vLeftOut[1] = (ymu8)(leftOut[1] * 128);
    vLeftOut[2] = (ymu8)(leftOut[2] * 128);
    // Right Output
    vRightOut[0] = (ymu8)(rightOut[0] * 128);
    vRightOut[1] = (ymu8)(rightOut[1] * 128);
    vRightOut[2] = (ymu8)(rightOut[2] * 128);
#else
    // Left Output
    vLeftOut[0] = leftOut[0];
    vLeftOut[1] = leftOut[1];
    vLeftOut[2] = leftOut[2];
    // Right Output
    vRightOut[0] = rightOut[0];
    vRightOut[1] = rightOut[1];
    vRightOut[2] = rightOut[2];
#endif
}

/**
 * Import emulation parameters from computer profiles (Clock, volume tables, mixer...)
 */
void CYm2149Ex::setProfile(ymProfile p)
{
    outputMixerMono(p.volOut);
    outputMixerStereo(p.volLeftOut, p.volRightOut);

    for(ymu8 i = 0; i < 5; i++)
    {
        for(ymu8 j = 0; j < 16; j++)
        {
            ymVolumeTable[i][j] = p.volumeTable[i][j];
        }
    }
}
