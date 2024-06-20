#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "device/s98device.h"
#include "kmp_pi.h"
//#include "reg.h"

#define USE_ZLIB 0

#if USE_ZLIB
#include "zlib.h"
#endif

#define SUPPORT_VGM 0
#define SUPPORT_MYM 1

#define KMPMODULE_PLUGIN_VERSION    10
#define KMPMODULE_PLUGIN_NAME       "m_s98.kpi v1.0r8 by Mamiya"
#define KMPMODULE_PLUGIN_COPYRIGHT  "FM Sound Generator rel.008 (C) cisc 1998-2003. / emu2413 by Mitsutaka Okazaki 2001-2002 / zlib 1.1.4 (C) 1995-1998 Jean-loup Gailly and Mark Adler"
#define KMPMODULE_REENTRANT 0

#define MASTER_CLOCK (7987200)
#define LOOPNUM 2
#define SAMPLE_RATE 44100
#define SYNC_RATE 60 /* (Hz) */
#define UNIT_RENDER (SAMPLE_RATE/SYNC_RATE)
#define FADEOUT_TIME 0/*(10 * SYNC_RATE)*/

#define S98DEVICE_MAX 16

/* S98 file header */
#define S98_MAGIC_V0	(0x53393830)	/* 'S980' */
#define S98_MAGIC_V1	(0x53393831)	/* 'S981' */
#define S98_MAGIC_V2	(0x53393832)	/* 'S982' */
#define S98_MAGIC_V3	(0x53393833)	/* 'S983' */
#define S98_MAGIC_VZ	(0x5339385A)	/* 'S98Z' */
#define S98_OFS_MAGIC		(0x00)
#define S98_OFS_TIMER_INFO1	(0x04)
#define S98_OFS_TIMER_INFO2	(0x08)
#define S98_OFS_COMPRESSING	(0x0C)
#define S98_OFS_OFSTITLE	(0x10)
#define S98_OFS_OFSDATA		(0x14)
#define S98_OFS_OFSLOOP		(0x18)
#define S98_OFS_OFSCOMP		(0x1C)
#define S98_OFS_DEVICEINFO	(0x20)

#define VGM_MAGIC		(0x56676D20)	/* 'Vgm ' */

S98DEVICEIF *S98DeviceCreate(int type, int clock, int rate)
{
	S98DEVICEIF *ret = 0;
	switch (type)
	{
		case S98DEVICETYPE_PSG:
			ret = CreateS98DevicePSG();
			break;
		case S98DEVICETYPE_OPN:
			ret = CreateS98DeviceOPN();
			break;
		case S98DEVICETYPE_OPN2:
			ret = CreateS98DeviceOPN2();
			break;
		case S98DEVICETYPE_OPNA:
			ret = CreateS98DeviceOPNA();
			break;
		case S98DEVICETYPE_OPM:
			ret = CreateS98DeviceOPM();
			break;
		case S98DEVICETYPE_OPLL:
			ret = CreateS98DeviceOPLL();
			break;
		case S98DEVICETYPE_SNG:
			ret = CreateS98DeviceSNG();
			break;
	}
	if (ret) ret->Init(clock, rate);
	return ret;
}

class s98File {
public:
	bool OpenFromBuffer(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo);
	/* bool Open(const char *cszFileName, SOUNDINFO *pInfo);*/
	void Close(void);
	DWORD SetPosition(DWORD dwpos);
	DWORD Write(Int16 *Buffer, DWORD numSample);
	s98File();
	~s98File();
protected:
	int number_of_devices;
	S98DEVICEIF *devices[S98DEVICE_MAX];
	BYTE devicemap[0x40];

	BYTE *s98data;
	BYTE *s98head;
	BYTE *s98top;
	BYTE *s98loop;
	int length;
	DWORD playtime; /* syncs */
	DWORD looptime; /* syncs */

	BYTE *s98cur;
	DWORD curtime;

	int loopnum;
#if FADEOUT_TIME
	int loopcur;
	DWORD fader;
#endif

#define SPS_SHIFT 28
#define SPS_LIMIT (1 << SPS_SHIFT)
	enum { SAMPLE_PER_SYNC, SYNC_PER_SAMPLE } spsmode;
	DWORD sps;		/* sync/sample or sample/syjnc */
	DWORD timerinfo1;
	DWORD timerinfo2;

	double sync_per_sec;

	DWORD lefthi;
	DWORD leftlo;

	Sample bufdev[UNIT_RENDER * 2];

	void CalcTime(void);
	void Step(void);
	void Reset(void);

	void WriteSub(Int16 *Buffer, DWORD numSample);
	DWORD SyncToMsec(DWORD sync);
	DWORD MsecToSync(DWORD ms);
};

static void SetDwordLE(Uint8 *p, Uint32 v)
{
	p[0] = (v >> (8 * 0)) & 0xFF;
	p[1] = (v >> (8 * 1)) & 0xFF;
	p[2] = (v >> (8 * 2)) & 0xFF;
	p[3] = (v >> (8 * 3)) & 0xFF;
}
static void SetDwordBE(Uint8 *p, Uint32 v)
{
	p[0] = (v >> (8 * 3)) & 0xFF;
	p[1] = (v >> (8 * 2)) & 0xFF;
	p[2] = (v >> (8 * 1)) & 0xFF;
	p[3] = (v >> (8 * 0)) & 0xFF;
}
static Uint32 GetWordLE(Uint8 *p)
{
	int ret;
	ret  = ((Uint32)(Uint8)p[0]) << 0x00;
	ret |= ((Uint32)(Uint8)p[1]) << 0x08;
	return ret;
}
static Uint32 GetDwordLE(Uint8 *p)
{
	int ret;
	ret  = ((Uint32)(Uint8)p[0]) << 0x00;
	ret |= ((Uint32)(Uint8)p[1]) << 0x08;
	ret |= ((Uint32)(Uint8)p[2]) << 0x10;
	ret |= ((Uint32)(Uint8)p[3]) << 0x18;
	return ret;
}
static Uint32 GetDwordBE(Uint8 *p)
{
	Uint32 ret;
	ret  = ((Uint32)(Uint8)p[0]) << 0x18;
	ret |= ((Uint32)(Uint8)p[1]) << 0x10;
	ret |= ((Uint32)(Uint8)p[2]) << 0x08;
	ret |= ((Uint32)(Uint8)p[3]) << 0x00;
	return ret;
}


s98File::s98File()
{
	s98data = 0;
	number_of_devices = 0;
}
s98File::~s98File()
{
	Close();
}

void s98File::CalcTime(void)
{
	BYTE *p = s98top;
	looptime = 0;
	playtime = 0;
	if (!s98data) return;
	while (1)
	{
		if (p == s98loop) looptime = playtime;
		if (*p < 0x80)
		{
			p += 3;
		}
		else if (*p == 0xff)
		{
			playtime += 1;
			p += 1;
		}
		else if (*p == 0xfe)
		{
			int s = 0, n = 0;
			do
			{
				n |= (*(++p) & 0x7f) << s;
				s += 7;
			}
			while (*p & 0x80);
			playtime += n + 2;
			p += 1;
		}
		else
		{
			return;
		}
	}
}

static Uint32 DivFix(Uint32 p1, Uint32 p2, Uint32 fix)
{
	Uint32 ret;
	ret = p1 / p2;
	p1  = p1 % p2;/* p1 = p1 - p2 * ret; */
	while (fix--)
	{
		p1 += p1;
		ret += ret;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

void s98File::Reset(void)
{
	for (int d = 0; d < number_of_devices; d++) devices[d]->Reset();

#if FADEOUT_TIME
	loopcur = 0;
	fader = 0;
#endif

	s98cur = s98top;
	curtime = 0;

	lefthi = 0;
	leftlo = 0;
	Step();
}

void s98File::Step(void)
{
#if FADEOUT_TIME
	if (fader && ++fader >= FADEOUT_TIME)
	{
		lefthi = 0;
		for (int d = 0; d < number_of_devices; d++) devices[d]->Disable();
		return;
	}
#endif
	while (1)
	{
		if (*s98cur < 0x80)
		{
			int d = devicemap[*s98cur >> 1];
			if (d != S98DEVICE_MAX)
			{
				if (*s98cur & 1)
					devices[d]->SetReg(0x100 | s98cur[1], s98cur[2]);
				else
					devices[d]->SetReg(s98cur[1], s98cur[2]);
			}
			s98cur += 3;
			continue;
		}
		if (*s98cur == 0xfe || *s98cur == 0xff) break;
		if (*s98cur == 0xfd && s98loop)
		{
			s98cur = s98loop;
#if FADEOUT_TIME
			if (loopnum && !fader &&  ++loopcur == loopnum) fader = 1;
#endif
			continue;
		}
		lefthi = 0;
		for (int d = 0; d < number_of_devices; d++) devices[d]->Disable();
		return;
	}
	while (1)
	{
		if (*s98cur == 0xff)
		{
			lefthi += 1;
			s98cur++;
		}
		else if (*s98cur == 0xfe)
		{
			int s = 0, n = 0;
			do
			{
				n |= (*(++s98cur) & 0x7f) << s;
				s += 7;
			} while (*s98cur & 0x80);
			lefthi += n + 2;
			s98cur++;
		}
		else
		{
			break;
		}
	}
	return;
}

DWORD s98File::SyncToMsec(DWORD sync)
{
	return (DWORD)(((double)sync) * ((double)1000) / sync_per_sec);
}

DWORD s98File::MsecToSync(DWORD ms)
{
	return (DWORD)(((double)ms) * sync_per_sec / ((double)1000));
}

DWORD s98File::SetPosition(DWORD dwpos_)
{
	if (!s98data) return 0;
	dwpos_ = MsecToSync(dwpos_);
/*
	char buf[1024];
	wsprintf(buf, "s98debug:%d:%d\n", dwpos_, curtime);
	OutputDebugString(buf);
*/
	if (dwpos_ < curtime)
	{
		Reset();
	}
	while (dwpos_ > curtime)
	{
		curtime++;
		if (lefthi && --lefthi == 0) Step();
	}
	return SyncToMsec(curtime);
}

#if SUPPORT_VGM
#include "vgm.h"
#endif
#if SUPPORT_MYM
#include "x1f.h"
#include "mym.h"
#endif
#include "tar.h"

static int default_sample_rate = SAMPLE_RATE;
static int default_loopnum = LOOPNUM;

bool s98File::OpenFromBuffer(const BYTE *Buffer, DWORD dwSize, SOUNDINFO *pInfo)
{
	int sample_rate = (pInfo->dwSamplesPerSec == 0) ? default_sample_rate : pInfo->dwSamplesPerSec;
	Close();

	DWORD dataofs, loopofs;
	BYTE *buf = 0;
	DWORD length = dwSize;

	do
	{
		DWORD magic;
		if (length < 0x40) break;
		buf = (BYTE *)malloc(length);
		if (!buf) break;
		XMEMCPY(buf, Buffer, length);
#if USE_ZLIB
		/* Uncompress GZIP */
		if (buf[0] == 0x1f && buf[1] == 0x8b)
		{
			BYTE *des = 0;
			BYTE *gzp;
			unsigned deslen = 4096;
			int z_err;
			z_stream zs;

			des = (BYTE *)malloc(deslen);
			if (!des) break;

			XMEMSET(&zs, 0, sizeof(z_stream));

			gzp = buf + 10;
			if (buf[3] & 4)
			{
				DWORD extra = *gzp++;
				extra += *gzp++ << 8;
				gzp += extra;
			}
			if (buf[3] & 8) while (*gzp++);
			if (buf[3] & 16) while (*gzp++);
			if (buf[3] & 2) gzp += 2;

			zs.next_in = gzp;
			zs.avail_in = length - (gzp - buf);
			zs.next_out = des;
			zs.avail_out = deslen;
			zs.zalloc = (alloc_func)0;
			zs.zfree = (free_func)0;
			zs.opaque = (voidpf)0;

			z_err = inflateInit2(&zs, -MAX_WBITS);
			if (z_err != Z_OK)
			{
				inflateEnd(&zs);
				break;
			}
			inflateReset(&zs);
			while (1)
			{
				z_err = inflate(&zs, Z_SYNC_FLUSH);
				if (z_err == Z_STREAM_END) break;
				if (z_err != Z_OK || zs.avail_in == 0)
				{
					free(des);
					des = 0;
					break;
				}
				if (zs.avail_out == 0)
				{
					BYTE *p;
					p = (BYTE *)realloc(des, deslen + 4096);
					if (!p)
					{
						free(des);
						des = 0;
						break;
					}
					des = p;
					zs.next_out = des + deslen;
					zs.avail_out += 4096;
					deslen += 4096;
				}
			};
			if (des)
			{
				free(buf);
				buf = des;
				length = zs.total_out;
			}
			inflateEnd(&zs);
			if (!des) break;
		}
#endif
		/* skip TAR header */
		s98head = buf;
		if (length >= 512)
		{
			DWORD lentarheader = IsTarHeader(s98head);
			s98head += lentarheader;
			length -= lentarheader;
		}
		if (length < 0x40) break;
		magic = GetDwordBE(s98head + S98_OFS_MAGIC);
		if (S98_MAGIC_V0 <= magic && magic <= S98_MAGIC_VZ)
		{
			/* version check */
			if (S98_MAGIC_V3 < magic) break; // was V2
		}
#if SUPPORT_VGM
		else if (VGM_MAGIC == magic)
		{
			DWORD cnvs98length;
			BYTE *cnvs98;
			cnvs98 = vgm2s98(s98head, length, &cnvs98length);
			if (!cnvs98) break;
			free(buf);
			s98head = buf = cnvs98;
			length = cnvs98length;
		}
#endif
		else
		{
#if SUPPORT_MYM
			DWORD cnvs98length;
			BYTE *cnvs98;
			cnvs98 = mym2s98(s98head, length, &cnvs98length);
			if (!cnvs98) break;
			free(buf);
			s98head = buf = cnvs98;
			length = cnvs98length;
#else
			break;
#endif
		}
		loopofs = GetDwordLE(s98head + S98_OFS_OFSLOOP);
		dataofs = GetDwordLE(s98head + S98_OFS_OFSDATA);
		/* Uncompress internal deflate(old gimmick) */
		if (GetDwordLE(s98head + S98_OFS_COMPRESSING))
		{
#if !USE_ZLIB
			break;	/* NOT SUPPORT */
#else
			uLongf dessize;
			BYTE *des;
			DWORD compofs;
			compofs = GetDwordLE(s98head + S98_OFS_OFSCOMP);
			if (!compofs) compofs = dataofs;
			des = (BYTE *)malloc(compofs + GetDwordLE(s98head + S98_OFS_COMPRESSING));
			if (!des) break;
			XMEMCPY(des, s98head, compofs);
			if (Z_OK != uncompress(&des[compofs], &dessize, s98head + compofs, length - compofs))
			{
				free(des);
				break;
			}
			length = dessize;
			s98head = des;
			s98data = des;
			s98top  = s98head + dataofs;
			s98loop = loopofs ? (s98head + loopofs) : 0;
			des = 0;
#endif
		}
		else
		{
			s98data = buf;
			s98top  = s98head + dataofs;
			s98loop = loopofs ? (s98head + loopofs) : 0;
			buf = 0;
		}
		/* if (length <= loopofs) s98loop = 0; */
	} while(0);
	if (buf) free(buf);

	if (!s98data) return false;

	timerinfo1 = GetDwordLE(s98head + S98_OFS_TIMER_INFO1);
	timerinfo2 = GetDwordLE(s98head + S98_OFS_TIMER_INFO2);
	if (timerinfo1 == 0) timerinfo1 = 10;
	if (timerinfo2 == 0) timerinfo2 = 1000;

	int d;
	for (d = 0; d < 0x40; d++) devicemap[d] = S98DEVICE_MAX;
	number_of_devices = 0;
	if (!GetDwordLE(s98head + S98_OFS_DEVICEINFO))
	{
		devices[0] = S98DeviceCreate(S98DEVICETYPE_OPNA, MASTER_CLOCK, sample_rate);
		if (devices[0])
		{
			devicemap[0] = 0;
			number_of_devices = 1;
		}
	}
	else
	{
		BYTE *devinfo = s98head + S98_OFS_DEVICEINFO;
		for (d = 0; d < S98DEVICE_MAX && GetDwordLE(devinfo); d++)
		{
			devices[number_of_devices] = S98DeviceCreate(GetDwordLE(devinfo), GetDwordLE(devinfo + 4), sample_rate);
			if (devices[number_of_devices]) devicemap[d] = number_of_devices++;
			devinfo += 16;
		}
	}
	double dsps;
	double sample_per_sec = (double)sample_rate;
	sync_per_sec = ((double)timerinfo2) / ((double)timerinfo1);
	if (sync_per_sec > sample_per_sec)
	{
		dsps = sample_per_sec / sync_per_sec;
		spsmode = SAMPLE_PER_SYNC;
	}
	else
	{
		dsps = sync_per_sec / sample_per_sec;
		spsmode = SYNC_PER_SAMPLE;
	}
	sps = (DWORD)(dsps * (double)SPS_LIMIT);
	if (sps >= SPS_LIMIT)
	{
		sps = SPS_LIMIT;
		spsmode = SYNC_PER_SAMPLE;
	}
	/* 現時点では高解像度S98は存在しない */
	if (spsmode == SAMPLE_PER_SYNC) return false;
	CalcTime();
	pInfo->dwSamplesPerSec = sample_rate;
	pInfo->dwChannels = 2;
	pInfo->dwBitsPerSample = 16;
	pInfo->dwSeekable = 1;
	pInfo->dwUnitRender = UNIT_RENDER * 4;
	pInfo->dwReserved1 = 1;
	pInfo->dwReserved2 = 0;

	if (s98loop)
	{
		loopnum = default_loopnum;
		pInfo->dwLength = playtime + (playtime - looptime) * (loopnum - 1) + FADEOUT_TIME;
#if !FADEOUT_TIME
		/* 単曲リピート処理は此処 */
		loopnum = 0;
#endif
	}
	else
	{
		pInfo->dwLength = playtime;
		loopnum = 0;
	}
	/* syncs to msec */
	pInfo->dwLength = SyncToMsec(pInfo->dwLength);

	Reset();

	return true;
}

void s98File::Close(void)
{
	if (s98data) { free(s98data); s98data = 0; }
	for (int d = 0; d < number_of_devices; d++) delete devices[d];
	number_of_devices = 0;
}

void s98File::WriteSub(Int16 *Buffer, DWORD numSample)
{
	DWORD i, len;
	while (numSample)
	{
		len = (numSample > UNIT_RENDER) ? UNIT_RENDER : numSample;
		XMEMSET(bufdev, 0, len * 2 * sizeof(Sample));
		for (int d = 0; d < number_of_devices; d++)
			devices[d]->Mix(bufdev, len);
		for (i = 0; i < len * 2; i++)
		{
			Sample s;
			s = bufdev[i];
			if (((Uint32)(s + 0x8000)) > 0xffff)
			{
				s = (s > 0x7fff) ? 0x7fff : -0x8000;
			}
			*Buffer++ = (Int16)s; 
		}
		numSample -= len;
	}
}

DWORD s98File::Write(Int16 *Buffer, DWORD numSample)
{
	DWORD pos, numWrite = 0;
	if (!s98data) return numWrite;
	if (numSample == 0) return numWrite;

	for (pos = 0; pos < numSample; pos++)
	{
		if (lefthi)
		{
			leftlo += sps;
			if (leftlo >= SPS_LIMIT)
			{
				leftlo -= SPS_LIMIT;
				lefthi -= 1;
				curtime += 1;
			}
			if (lefthi == 0)
			{
				if (Buffer) WriteSub(&Buffer[numWrite * 2], pos + 1 - numWrite);
				numWrite = pos + 1;
				Step();
				if (lefthi == 0) break;
			}
		}
	}
	if (lefthi && numWrite != numSample) // was: if (/*lefthi && */numWrite != numSample)
	{
		if (Buffer) WriteSub(&Buffer[numWrite * 2], numSample - numWrite);
		numWrite = numSample;
	}
	return numWrite;
}

extern "C" {

void *S98_OpenFromBuffer (const BYTE *buffer, DWORD dwSize, SOUNDINFO *pInfo) {
	s98File *pFile;
	pFile = new s98File;
	if (pFile) {
		if (pFile->OpenFromBuffer(buffer, dwSize, pInfo)) {
			return pFile;
		}
		delete pFile;
	}
	return 0;
}

void S98_Close (void *s98) {
	if (s98) {
		delete((s98File *)s98);
	}
}

DWORD S98_Render (void *s98, BYTE *buffer, DWORD frames) {
	if (!s98) return 0;
	return ((s98File *)s98)->Write((Int16 *)buffer, frames);
}

DWORD S98_SetPosition (void *s98, DWORD dwPos) {
	if (!s98) return 0;
	return ((s98File *)s98)->SetPosition(dwPos);
}

}
