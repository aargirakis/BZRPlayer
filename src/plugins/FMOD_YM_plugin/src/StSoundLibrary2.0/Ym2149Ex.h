/*-----------------------------------------------------------------------------

	ST-Sound ( YM files player library )

	Extended YM-2149 Emulator, with ATARI music demos effects.
	(SID-Like, Digidrum, Sync Buzzer, Sinus SID and Pattern SID)

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
* ST-Sound, ATARI-ST Music Emulator
* Copyright (c) 1995-1999 Arnaud Carre ( http://leonard.oxg.free.fr )
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
-----------------------------------------------------------------------------*/


#ifndef __YM2149EX__
#define __YM2149EX__

#ifdef BUILDING_DLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// Keep for compatibility reason, remove in next API (v3)
#define	AMSTRAD_CLOCK	1000000L
#define	ATARI_CLOCK		2000000L
#define	SPECTRUM_CLOCK	1773400L
#define	MFP_CLOCK		2457600L
// End

#define	NOISESIZE		16384
#define	DRUM_PREC		15

#define	PI				3.1415926
#define	SIDSINPOWER		0.7

#include "YmTypes.h"
#include "YmProfiles.h"
#include "YmFilters.h"
#include <list>

enum
{
	VOICE_A=0,
	VOICE_B=1,
	VOICE_C=2,
};

enum
{
    F_MONO=0,
    F_LEFT=1,
    F_RIGHT=2,
};

enum
{
	R_A_TONE_PERIOD_LOW=0,
	R_A_TONE_PERIOD_HIGH=1,
	R_B_TONE_PERIOD_LOW=2,
	R_B_TONE_PERIOD_HIGH=3,
	R_C_TONE_PERIOD_LOW=4,
	R_C_TONE_PERIOD_HIGH=5,
	R_NOISE_PERIOD=6,
	R_ENABLE=7,
	R_A_AMPLITUDE=8,
	R_B_AMPLITUDE=9,
	R_C_AMPLITUDE=10,
	R_ENVELOPE_PERIOD_LOW=11,
	R_ENVELOPE_PERIOD_HIGH=12,
	R_ENVELOPE_SHAPE=13,
	R_A_IO_PORT=14,
	R_B_IO_PORT=15,
};

#define ENABLE_NOT_TONE_A  (1 << 0)
#define ENABLE_NOT_TONE_B  (1 << 1)
#define ENABLE_NOT_TONE_C  (1 << 2)
#define ENABLE_NOT_NOISE_A (1 << 3)
#define ENABLE_NOT_NOISE_B (1 << 4)
#define ENABLE_NOT_NOISE_C (1 << 5)
#define ENABLE_OUT_IO_A    (1 << 6)
#define ENABLE_OUT_IO_B    (1 << 7)

struct	YmSpecialEffect
{

		ymbool		bDrum;
		ymu32		drumSize;
		ymu8	*	drumData;
		ymu32		drumPos;
		ymu32		drumStep;

		ymbool		bSid;
		ymu32		sidPos;
		ymu32		sidStep;
		ymint		sidVol;

};

class CYm2149Ex
{
    public:
//        CYm2149Ex(ymu32 masterClock=ATARI_CLOCK,ymint prediv=1,ymu32 playRate=44100);
        EXPORT CYm2149Ex(ymu32 masterClock=profileAtari.masterClock,ymint prediv=1,ymu32 playRate=44100);
        EXPORT CYm2149Ex(ymProfile profile, ymu32 playRate = 44100);
        EXPORT ~CYm2149Ex();

        void EXPORT	reset(void);
//      void	update(ymsample *pSampleBuffer,ymint nbSample); // TODO: Don't need for Mono mode and compatibility ?
        void EXPORT	updateStereo(ymsample *pSampleBuffer,ymint nbSample);

        void EXPORT    setProfile(ymProfile profile);
        void EXPORT    outputMixerMono(ymfloat out[3]);
        void EXPORT    outputMixerStereo(ymfloat leftOut[3], ymfloat rightOut[3]);

	void EXPORT	setClock(ymu32 _clock);
	void EXPORT	writeRegister(ymint reg,ymint value);
	ymint EXPORT	readRegister(ymint reg);
/*
	void	drumStart(ymint voice,ymu8 *drumBuffer,ymu32 drumSize,ymint drumFreq);
	void	drumStop(ymint voice);
	void	sidStart(ymint voice,ymint freq,ymint vol);
	void	sidSinStart(ymint voice,ymint timerFreq,ymint sinPattern);
	void	sidStop(ymint voice);
	void	syncBuzzerStart(ymint freq,ymint envShape);
	void	syncBuzzerStop(void);
*/

        bool EXPORT    isIntegerVersion()
        {
#if YM_INTEGER_ONLY
            return true;
#else
            return false;
#endif
        }

    private:
        std::list<Filter*> filters[3];
//        std::list<Filter*> filtersStereo[2];

		ymu32	frameCycle;
		ymu32	cyclePerSample;
		inline	ymsample nextSample(void);
		inline	void nextSampleStereo(ymsample& left, ymsample& right);
		ymu32 toneStepCompute(ymu8 rHigh,ymu8 rLow);
		ymu32 noiseStepCompute(ymu8 rNoise);
		ymu32 envStepCompute(ymu8 rHigh,ymu8 rLow);
		void	updateEnvGen(ymint nbSample);
		void	updateNoiseGen(ymint nbSample);
		void	updateToneGen(ymint voice,ymint nbSample);
		ymu32	rndCompute(void);

//		void	sidVolumeCompute(ymint voice,ymint *pVol);

		ymint	replayFrequency;
		ymu32	internalClock;
		ymu8	registers[14];

		ymu32	cycleSample;
		ymu32	stepA,stepB,stepC;
		ymu32	posA,posB,posC;
		ymint	volA,volB,volC,volEA, volEB, volEC;
		ymu32	mixerTA,mixerTB,mixerTC;
		ymu32	mixerNA,mixerNB,mixerNC;
		ymint	*pVolA,*pVolB,*pVolC;

		ymu32	noiseStep;
		ymu32 noisePos;
		ymu32	rndRack;
		ymu32	currentNoise;
		ymu32	bWrite13;

		ymu32	envStep;
		ymu32 envPos;
		ymint		envPhase;
		ymint		envShape;
		ymu8	envData[16][2][16*2];
		ymint	globalVolume;
/*
		struct	YmSpecialEffect	specialEffect[3];
		ymbool	bSyncBuzzer;
		ymu32	syncBuzzerStep;
		ymu32	syncBuzzerPhase;
		ymint	syncBuzzerShape;
*/

        // Output Mixer
#if YM_INTEGER_ONLY
        ymu8 vOut[3]; // Mono Output
        ymu8 vLeftOut[3]; // Left Output
        ymu8 vRightOut[3]; // Right Output
#else
        ymfloat vOut[3]; // Mono Output
        ymfloat vLeftOut[3]; // Left Output
        ymfloat vRightOut[3]; // Right Output
#endif
};

#endif
