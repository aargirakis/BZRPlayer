#include "s_logtbl.h"
#include "s_sng.h"
#include "divfix.h"

#define CPS_SHIFT 18
#define LOG_KEYOFF (31 << LOG_BITS)

#define FB_WNOISE   0x14002
#define FB_PNOISE   0x08000
#define NG_PRESET   0x0F35

typedef struct {
	Uint32 cycles;
	Uint32 spd;
	Uint32 vol;
	Uint8 adr;
	Uint8 mute;
	Uint8 pad4[2];
} SNG_SQUARE;

typedef struct {
	Uint32 cycles;
	Uint32 spd;
	Uint32 vol;
	Uint32 rng;
	Uint32 fb;
	Uint8 mode;
	Uint8 mute;
	Uint8 pad4[2];
} SNG_NOISE;

typedef struct {
	KMIF_SOUND_DEVICE kmif;
	KMIF_LOGTABLE *logtbl;
	SNG_SQUARE square[3];
	SNG_NOISE noise;
	struct {
		Uint32 cps;
		Int32 mastervolume;
		Uint8 first;
		Uint8 ggs;
	} common;
	Uint8 type;
} SNGSOUND;

#define V(a) (((a * (1 << LOG_BITS)) / 3) << 1)
static Uint32 voltbl[16] = {
	V(0x0), V(0x1), V(0x2),V(0x3),V(0x4), V(0x5), V(0x6),V(0x7),
	V(0x8), V(0x9), V(0xA),V(0xB),V(0xC), V(0xD), V(0xE),LOG_KEYOFF
};
#undef V

__inline static Int32 SNGSoundSquareSynth(SNGSOUND *sndp, SNG_SQUARE *ch)
{
	if (ch->spd < (0x10 << CPS_SHIFT))
	{
		return LogToLin(sndp->logtbl, ch->vol + sndp->common.mastervolume, LOG_LIN_BITS - 21);
	}
	ch->cycles += sndp->common.cps;
	while (ch->cycles >= ch->spd)
	{
		ch->cycles -= ch->spd;
		ch->adr++;
	}
	if (ch->mute || (ch->adr & 1)) return 0;
	return LogToLin(sndp->logtbl, ch->vol + sndp->common.mastervolume, LOG_LIN_BITS - 21);
}

__inline static Int32 SNGSoundNoiseSynth(SNGSOUND *sndp, SNG_NOISE *ch)
{
	if (ch->spd < (0x10 << (CPS_SHIFT - 1))) return 0;
	ch->cycles += sndp->common.cps >> 1;
	while (ch->cycles >= ch->spd)
	{
		ch->cycles -= ch->spd;
		if (ch->rng & 1) ch->rng ^= ch->fb;
		ch->rng >>= 1;
	}
	if (ch->mute || (ch->rng & 1)) return 0;
	return LogToLin(sndp->logtbl, ch->vol + sndp->common.mastervolume, LOG_LIN_BITS - 21);
}

static void SNGSoundSquareReset(SNG_SQUARE *ch)
{
	XMEMSET(ch, 0, sizeof(SNG_SQUARE));
	ch->vol = LOG_KEYOFF;
}

static void SNGSoundNoiseReset(SNG_NOISE *ch)
{
	XMEMSET(ch, 0, sizeof(SNG_NOISE));
	ch->vol = LOG_KEYOFF;
	ch->rng = NG_PRESET;
}


static void sndsynth(SNGSOUND *sndp, Int32 *p)
{
	Uint32 ch;
	Int32 accum = 0;
	for (ch = 0; ch < 3; ch++)
	{
		accum = SNGSoundSquareSynth(sndp, &sndp->square[ch]);
		if ((sndp->common.ggs >> ch) & 0x01) p[0] += accum;
		if ((sndp->common.ggs >> ch) & 0x10) p[1] += accum;
	}
	accum = SNGSoundNoiseSynth(sndp, &sndp->noise);
	if (sndp->common.ggs & 0x08) p[0] += accum;
	if (sndp->common.ggs & 0x80) p[1] += accum;
}

static void sndvolume(SNGSOUND *sndp, Int32 volume)
{
	volume = (volume << (LOG_BITS - 8)) << 1;
	sndp->common.mastervolume = volume;
}

static Uint32 sndread(SNGSOUND *sndp, Uint32 a)
{
	return 0;
}

static void sndwrite(SNGSOUND *sndp, Uint32 a, Uint32 v)
{
	if (a & 1)
	{
		if (sndp->type == SNG_TYPE_GAMEGEAR) sndp->common.ggs = v;
	}
	else if (sndp->common.first)
	{
		Uint32 ch = (sndp->common.first >> 5) & 3;
		sndp->square[ch].spd = (((v & 0x3F) << 4) + (sndp->common.first & 0xF)) << CPS_SHIFT;
		if (ch == 2 && sndp->noise.mode == 3)
		{
			sndp->noise.spd = ((sndp->square[2].spd >> 4) & 0x3f) + 1;
		}
		sndp->common.first = 0;
	}
	else
	{
		Uint32 ch;
		switch (v & 0xF0)
		{
			case 0x80:	case 0xA0:	case 0xC0:
				sndp->common.first = v;
				break;
			case 0x90:	case 0xB0:	case 0xD0:
				ch = (v & 0x60) >> 5;
				sndp->square[ch].vol = voltbl[v & 0xF];
				break;
			case 0xE0:
				sndp->noise.mode = v & 0x3;
				sndp->noise.fb = (v & 4) ? FB_WNOISE : FB_PNOISE;
				if (sndp->noise.mode == 3)
					sndp->noise.spd = ((sndp->square[2].spd >> 4) & 0x3f) + 1;
				else
					sndp->noise.spd = 1 << (4 + sndp->noise.mode + CPS_SHIFT);
				break;
			case 0xF0:
				sndp->noise.vol = voltbl[v & 0xF];
				break;
		}
	}

}

static void sndreset(SNGSOUND *sndp, Uint32 clock, Uint32 freq)
{
	XMEMSET(&sndp->common, 0, sizeof(sndp->common));
	sndp->common.cps = DivFix(clock, 16 * freq, CPS_SHIFT);
	sndp->common.ggs = 0xff;
	SNGSoundSquareReset(&sndp->square[0]);
	SNGSoundSquareReset(&sndp->square[1]);
	SNGSoundSquareReset(&sndp->square[2]);
	SNGSoundNoiseReset(&sndp->noise);
	sndwrite(sndp, 0, 0xE0);
	sndwrite(sndp, 0, 0x9F);
	sndwrite(sndp, 0, 0xBF);
	sndwrite(sndp, 0, 0xDF);
	sndwrite(sndp, 0, 0xFF);
}

static void sndrelease(SNGSOUND *sndp)
{
	if (sndp->logtbl) sndp->logtbl->release(sndp->logtbl->ctx);
	XFREE(sndp);
}

static void setinst(void *ctx, Uint32 n, void *p, Uint32 l){}

KMIF_SOUND_DEVICE *SNGSoundAlloc(Uint32 sng_type)
{
	SNGSOUND *sndp;
	sndp = XMALLOC(sizeof(SNGSOUND));
	if (!sndp) return 0;
	sndp->type = sng_type;
	sndp->kmif.ctx = sndp;
	sndp->kmif.release = sndrelease;
	sndp->kmif.reset = sndreset;
	sndp->kmif.synth = sndsynth;
	sndp->kmif.volume = sndvolume;
	sndp->kmif.write = sndwrite;
	sndp->kmif.read = sndread;
	sndp->kmif.setinst = setinst;
	sndp->logtbl = LogTableAddRef();
	if (!sndp->logtbl)
	{
		sndrelease(sndp);
		return 0;
	}
	return &sndp->kmif;
}
