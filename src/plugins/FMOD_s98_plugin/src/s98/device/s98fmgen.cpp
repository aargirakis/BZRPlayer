#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "s98device.h"
#include "fmgen/types.h"
#include "fmgen/opna.h"
#include "fmgen/opm.h"

class S98DEVICE_PSG : public S98DEVICEIF
{
public:
	S98DEVICE_PSG() {}
	~S98DEVICE_PSG() { enable = false; }
	void Init(Uint32 clock, Uint32 rate)
	{
		device.SetClock(clock, rate);
		Reset();
	}
	void Reset(void)
	{
		device.Reset();
		enable = false;
	}
	void SetReg(Uint32 addr, Uint32 data)
	{
		if (addr & 0x100)
		{
			if (addr == 0x100)
				reg = data;
			else
			{
				device.SetReg(reg, data);
				enable = true;
			}
		}
		else
		{
			device.SetReg(addr, data);
			enable = true;
		}
	}
	void Mix(Sample* buffer, int nsamples)
	{
		if (enable) device.Mix(buffer, nsamples);
	}
	void Disable(void)
	{
		enable = false;
	}
private:
	PSG device;
	bool enable;
	Uint32 reg;
};

class S98DEVICE_OPN : public S98DEVICEIF
{
public:
	S98DEVICE_OPN() {}
	~S98DEVICE_OPN() { enable = false; }
	void Init(Uint32 clock, Uint32 rate)
	{
		device.Init(clock, rate, false);
		Reset();
	}
	void Reset(void)
	{
		device.Reset();
		enable = false;
	}
	void SetReg(Uint32 addr, Uint32 data)
	{
		device.SetReg(addr, data);
		enable = true;
	}
	void Mix(Sample* buffer, int nsamples)
	{
		if (enable) device.Mix(buffer, nsamples);
	}
	void Disable(void)
	{
		enable = false;
	}
private:
	FM::OPN device;
	bool enable;
};

class S98DEVICE_OPN2 : public S98DEVICEIF
{
public:
	S98DEVICE_OPN2() {}
	~S98DEVICE_OPN2() { enable = false; }
	void Init(Uint32 clock, Uint32 rate)
	{
		device.Init(clock, rate, true);
		Reset();
	}
	void Reset(void)
	{
		device.Reset();
		device.SetReg(0x29, 0x9f);
		device.SetReg(0x2a, 0x80);
		device.SetReg(0x2b, 0x00);
		enable = false;
	}
	void SetReg(Uint32 addr, Uint32 data)
	{
		device.SetReg(addr, data);
		enable = true;
	}
	void Mix(Sample* buffer, int nsamples)
	{
		if (enable) device.Mix(buffer, nsamples);
	}
	void Disable(void)
	{
		enable = false;
	}
private:
	FM::OPN2 device;
	bool enable;
};

char *GetDLLArgv0(void);

class S98DEVICE_OPNA : public S98DEVICEIF
{
public:
	S98DEVICE_OPNA() {}
	~S98DEVICE_OPNA() { enable = false; }
	void Init(Uint32 clock, Uint32 rate)
	{
#if 0
		CHAR rhythmPath[MAX_PATH];
		LPSTR fnoff;
		DWORD ret;
		ret = GetFullPathName(GetDLLArgv0(), MAX_PATH, rhythmPath, &fnoff);
		if (ret == 0 || ret > MAX_PATH)
		{
#endif
			device.Init(clock, rate, true, 0);
#if 0
		}
		else
		{
			*fnoff = '\0';
			//lstrcpy(fnoff, "\\");
			device.Init(clock, rate, true, rhythmPath);
		}
#endif
		Reset();
	}
	void Reset(void)
	{
		device.Reset();
		device.SetReg(0x29, 0x9f);
		enable = false;
	}
	void SetReg(Uint32 addr, Uint32 data)
	{
		device.SetReg(addr, data);
		enable = true;
	}
	void Mix(Sample* buffer, int nsamples)
	{
		if (enable) device.Mix(buffer, nsamples);
	}
	void Disable(void)
	{
		enable = false;
	}
private:
	FM::OPNA device;
	bool enable;
};

class S98DEVICE_OPM : public S98DEVICEIF
{
public:
	S98DEVICE_OPM() {}
	~S98DEVICE_OPM() { enable = false; }
	void Init(Uint32 clock, Uint32 rate)
	{
		device.Init(clock, rate, false);
		Reset();
	}
	void Reset(void)
	{
		device.Reset();
		enable = false;
	}
	void SetReg(Uint32 addr, Uint32 data)
	{
		device.SetReg(addr, data);
		enable = true;
	}
	void Mix(Sample* buffer, int nsamples)
	{
		if (enable) device.Mix(buffer, nsamples);
	}
	void Disable(void)
	{
		enable = false;
	}
private:
	FM::OPM device;
	bool enable;
};

S98DEVICEIF *CreateS98DevicePSG(void) { return new S98DEVICE_PSG; }
S98DEVICEIF *CreateS98DeviceOPN(void) { return new S98DEVICE_OPN; }
S98DEVICEIF *CreateS98DeviceOPN2(void) { return new S98DEVICE_OPN2; }
S98DEVICEIF *CreateS98DeviceOPNA(void) { return new S98DEVICE_OPNA; }
S98DEVICEIF *CreateS98DeviceOPM(void) { return new S98DEVICE_OPM; }

