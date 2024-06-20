#include "s_sng.h"
#include "s98device.h"

class S98DEVICE_SNG : public S98DEVICEIF {

public:
	S98DEVICE_SNG(void);
	~S98DEVICE_SNG();

	void Init(Uint32 clock, Uint32 rate);
	void Reset(void);
	void SetReg(Uint32 addr, Uint32 data);
	void Mix(Sample* buffer, int nsamples);
	void Disable(void);

private:
	Uint32 uClock;
	Uint32 uFreq;
	bool bEnable;
	KMIF_SOUND_DEVICE *ifp;
};

void S98DEVICE_SNG::Mix(Sample *pBuffer, int numSamples)
{
	Int32 i, d[2];
	for (i = 0; i < numSamples; i++)
	{
		d[0] = d[1] = 0;
		if (bEnable && ifp) ifp->synth(ifp->ctx, d);
		*pBuffer++ += d[0] >> 8;
		*pBuffer++ += d[1] >> 8;
	}
}


void S98DEVICE_SNG::Reset(void)
{
	if (ifp) ifp->reset(ifp->ctx, uClock, uFreq);
}

void S98DEVICE_SNG::SetReg(Uint32 addr, Uint32 data)
{
	if (!ifp) return;
	if (addr & 1)
		ifp->write(ifp->ctx, 1, data);
	else
		ifp->write(ifp->ctx, 0, data);
	bEnable = true;
}

void S98DEVICE_SNG::Init(Uint32 clock, Uint32 rate)
{
	uClock = clock;
	uFreq = rate;
	Reset();
}

void S98DEVICE_SNG::Disable(void)
{
	bEnable = false;
}

S98DEVICE_SNG::S98DEVICE_SNG(void)
{
	Disable();
	ifp = SNGSoundAlloc(SNG_TYPE_GAMEGEAR);
}

S98DEVICE_SNG::~S98DEVICE_SNG()
{
	if (ifp) ifp->release(ifp->ctx);
}

S98DEVICEIF *CreateS98DeviceSNG(void) { return new S98DEVICE_SNG; }
