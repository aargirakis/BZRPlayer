#ifndef _KDMENG_H_
#define _KDMENG_H_

//#include <foobar2000.h>
#include <cstdint>
#include <vector>

class kdmeng
{
	enum
	{
		NUMCHANNELS = 16,
		MAXWAVES = 256,
		MAXTRACKS = 256,
		MAXNOTES = 8192,
		MAXEFFECTS = 16,
	};

public:
	enum
	{
		MAXSAMPLESTOPROCESS = 32768
	};

	unsigned kdmsamplerate, kdmnumspeakers, kdmbytespersample;

	unsigned numwaves;
	char instname[MAXWAVES][16];
	unsigned wavleng[MAXWAVES];
	unsigned repstart[MAXWAVES], repleng[MAXWAVES];
	int finetune[MAXWAVES];

	unsigned totsndbytes;
	unsigned char * wavoffs[MAXWAVES];

	unsigned eff[MAXEFFECTS][256];

	uint32_t kdmversionum, numnotes, numtracks;
	char trinst[MAXTRACKS], trquant[MAXTRACKS];
	char trvol1[MAXTRACKS], trvol2[MAXTRACKS];
	uint32_t nttime[MAXNOTES];
	char nttrack[MAXNOTES], ntfreq[MAXNOTES];
	char ntvol1[MAXNOTES], ntvol2[MAXNOTES];
	char ntfrqeff[MAXNOTES], ntvoleff[MAXNOTES], ntpaneff[MAXNOTES];

	long timecount, notecnt, musicstatus, musicrepeat, loopcnt;

	long kdmasm1, kdmasm3;
	unsigned char * kdmasm2, * kdmasm4;

    char *snd;

	long stemp[MAXSAMPLESTOPROCESS];

	long splc[NUMCHANNELS], sinc[NUMCHANNELS];
	unsigned char * soff[NUMCHANNELS];
	long svol1[NUMCHANNELS], svol2[NUMCHANNELS];
	long volookup[NUMCHANNELS<<9];
	long swavenum[NUMCHANNELS];
	long frqeff[NUMCHANNELS], frqoff[NUMCHANNELS];
	long voleff[NUMCHANNELS], voloff[NUMCHANNELS];
	long paneff[NUMCHANNELS], panoff[NUMCHANNELS];

	long frqtable[256];
	long ramplookup[64];

	long monohicomb( long * b, long c, long d, long si, long * di );
	long stereohicomb( long * b, long c, long d, long si, long * di );

    long loadwaves( const char * refdir);

	void startwave ( long wavnum, long dafreq, long davolume1, long davolume2, long dafrqeff, long davoleff, long dapaneff );

public:
	kdmeng( unsigned samplerate, unsigned numspeakers, unsigned bytespersample );
    ~kdmeng();

	void musicon();
	void musicoff();

    //long load (const char * filename);
    long load(void* data, unsigned long int length, const char* filepath);

	long rendersound ( void * dasnd, long numbytes );

	void seek ( long seektoms );

    unsigned int getNumtracks(){ return numtracks;}
    unsigned getNumwaves(){ return numwaves;}
    void getInstname(unsigned int idx,char* buff);
    char getTrackInstrument(unsigned int idx);
    char getTrackQuantize(unsigned int idx);
    unsigned char getTrackVolume1(unsigned int idx);
    unsigned char getTrackVolume2(unsigned int idx);
    unsigned getInstsize(unsigned int idx);
    unsigned getInstrepstart(unsigned int idx);
    unsigned getInstreplength(unsigned int idx);
    int getInstfinetune(unsigned int idx);

};

#endif
