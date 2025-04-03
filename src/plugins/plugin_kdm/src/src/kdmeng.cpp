#include "kdmeng.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cstring>
#include <cmath>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#else
#include <cstdint>
#include <iostream>
#include <dirent.h>

typedef int64_t __int64;

using LARGE_INTEGER = union _LARGE_INTEGER {
	struct {
		uint32_t LowPart;
		int32_t HighPart;
	};

	struct {
		uint32_t LowPart;
		int32_t HighPart;
	} u;

	int64_t QuadPart;
};

int MulDiv(int number, int numerator, int denominator) {
	long long ret = number;
	ret *= numerator;
	ret /= denominator;
	return (int) ret;
}

bool caseInsensitiveCompare(const std::string &a, const std::string &b) {
	if (a.size() != b.size()) return false;
	for (size_t i = 0; i < a.size(); ++i) {
		if (tolower(a[i]) != tolower(b[i])) {
			return false;
		}
	}
	return true;
}

std::string findFileCaseInsensitive(const std::string &filename, const std::string &directory) {
	DIR *dir = opendir(directory.c_str());
	if (!dir) {
		return "";
	}

	dirent const *entry;
	while ((entry = readdir(dir)) != nullptr) {
		if (caseInsensitiveCompare(entry->d_name, filename)) {
			closedir(dir);
			return std::string(entry->d_name);
		}
	}

	closedir(dir);
	return "";
}
#endif

// Convert high bytes and low bytes of MSW and LSW to 32-bit word.
// Used to read 32-bit words in little-endian order.
inline uint32_t readEndian(uint8_t hihi, uint8_t hilo, uint8_t hi, uint8_t lo) {
	return (((uint32_t) hihi << 24) + ((uint32_t) hilo << 16) +
	        ((uint32_t) hi << 8) + (uint32_t) lo);
}

using namespace std;
#define _inline inline
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

#define scale(a, b, c) MulDiv( a, b, c )

static _inline void fsin ( long * a )
{
	const float oneshr10 = 1.f / ( float ) ( 1 << 10 );
	const float oneshl14 = 1.f * ( float ) ( 1 << 14 );

	*a = ( long ) ( sin( ( float ) *a * M_PI * oneshr10 ) * oneshl14 );
}

static _inline void calcvolookupmono ( long * t, long a, long b )
{
	for ( unsigned i = 0; i < 256; i++ )
	{
		*t++ = a;
		a += b;
	}
}

static _inline void calcvolookupstereo ( long * t, long a, long b, long c, long d )
{
	for ( unsigned i = 0; i < 256; i++ )
	{
		*t++ = a;
		*t++ = c;
		a += b;
		c += d;
	}
}

static _inline long mulscale16 ( long a, long d )
{
	return ( long ) ( ( ( __int64 ) a * ( __int64 ) d ) >> 16 );
}

static _inline long mulscale24 ( long a, long d )
{
	return ( long ) ( ( ( __int64 ) a * ( __int64 ) d ) >> 24 );
}

static _inline long mulscale30 ( long a, long d )
{
	return ( long ) ( ( ( __int64 ) a * ( __int64 ) d ) >> 30 );
}

static _inline void clearbuf (void *d, long c, long a)
{
	unsigned long * dl = ( unsigned long * ) d;
	for ( unsigned i = 0; i < c; i++ ) *dl++ = a;
}

static _inline void copybuf (void *s, void *d, long c)
{
	unsigned long * sl = ( unsigned long * ) s;
	unsigned long * dl = ( unsigned long * ) d;
	for ( unsigned i = 0; i < c; i++ ) *dl++ = *sl++;
}

static void bound2char( unsigned count, long * in, unsigned char * out )
{
	for ( unsigned i = 0, j = count * 2; i < j; i++ )
	{
		long sample = *in >> 8;
		*in++ = kdmeng::MAXSAMPLESTOPROCESS;
		if ( sample < 0 ) sample = 0;
		else if ( sample > 255 ) sample = 255;
		*out++ = sample;
	}
}

static void bound2short( unsigned count, long * in, unsigned char * out )
{
	signed short * outs = ( signed short * ) out;
	for ( unsigned i = 0, j = count * 2; i < j; i++ )
	{
		long sample = *in;
		*in++ = kdmeng::MAXSAMPLESTOPROCESS;
		if ( sample < 0 ) sample = 0;
		else if ( sample > 65535 ) sample = 65535;
		*outs++ = sample ^ 0x8000;
	}
}

long kdmeng::monohicomb( long * b, long c, long d, long si, long * di )
{
	LARGE_INTEGER dasinc, sample_offset, interp;

	if ( si >= 0 ) return 0;

	dasinc.QuadPart = ( ( __int64 ) d ) << ( 32 - 12 );

	sample_offset.QuadPart = ( ( __int64 ) si ) << ( 32 - 12 );

	while ( c )
	{
		unsigned short sample = * ( unsigned short * ) ( kdmasm4 + sample_offset.HighPart );
		interp.QuadPart = sample_offset.LowPart;
		interp.QuadPart *= ( sample >> 8 ) - ( sample & 0xFF );
		d = interp.HighPart + sample & 0xFF;
		*di++ += b[ d ];
		sample_offset.QuadPart += dasinc.QuadPart;
		if ( sample_offset.QuadPart >= 0 )
		{
			if ( !kdmasm1 ) break;
			kdmasm4 = kdmasm2;
			sample_offset.QuadPart -= ( ( __int64 ) kdmasm3 ) << ( 32 - 12 );
		}
		c--;
	}

	return sample_offset.QuadPart >> ( 32 - 12 );
}

long kdmeng::stereohicomb( long * b, long c, long d, long si, long * di )
{
	LARGE_INTEGER dasinc, sample_offset, interp;

	if ( si >= 0 ) return 0;

	dasinc.QuadPart = ( ( __int64 ) d ) << ( 32 - 12 );

	sample_offset.QuadPart = ( ( __int64 ) si ) << ( 32 - 12 );

	while ( c )
	{
		unsigned short sample = * ( unsigned short * ) ( kdmasm4 + sample_offset.HighPart );
		interp.QuadPart = sample_offset.LowPart;
		interp.QuadPart *= ( sample >> 8 ) - ( sample & 0xFF );
		d = interp.HighPart + ( sample & 0xFF );
		*di++ += b[ d * 2 ];
		*di++ += b[ d * 2 + 1 ];
		sample_offset.QuadPart += dasinc.QuadPart;
		if ( sample_offset.QuadPart >= 0 )
		{
			if ( !kdmasm1 ) break;
			kdmasm4 = kdmasm2;
			sample_offset.QuadPart -= ( ( __int64 ) kdmasm3 ) << ( 32 - 12 );
		}
		c--;
	}

	return sample_offset.QuadPart >> ( 32 - 12 );
}

void kdmeng::musicon()
{
	notecnt = 0;
	timecount = nttime[notecnt];
	musicrepeat = 1;
	musicstatus = 1;
}

void kdmeng::musicoff()
{
	musicstatus = 0;
	for(unsigned i = 0; i < NUMCHANNELS; i++ ) splc [i] = 0;
	musicrepeat = 0;
	timecount = 0;
	notecnt = 0;
}

long kdmeng::loadwaves ( const char * refdir)
{
    unsigned i, j, dawaversionum;
    unsigned int p = 0;
    if (snd) return(1);

    string filename(refdir);

#ifdef WIN32
	filename+= "waves.kwv";
#else
	std::string fileFound = findFileCaseInsensitive("waves.kwv", refdir);
	if (fileFound.empty()) {
		return 0;
	}
	filename += fileFound;
#endif

    ifstream file (filename.c_str(), ios::in | ios::binary | ios::ate);
    ifstream::pos_type fileSize;
    char* d;
    if(file.is_open())
    {
        fileSize = file.tellg();
        d = new char[fileSize];
        file.seekg(0, ios::beg);
        if(!file.read(d, fileSize))
        {
            delete[] d;
            return 0;
        }
        file.close();

        dawaversionum = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
        if ( dawaversionum != 0 ) return 0;
        totsndbytes = 0;
        numwaves = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;

        for ( i = 0; i < numwaves; i++ )
        {
            for(int j = 0;j<16;j++)
            {
                instname[i][j] = d[p];p++;
            }
            wavleng[i]=readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
            repstart[i]=readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
            repleng[i]=readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
            finetune[i]=readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
            wavoffs[i] = ( unsigned char * ) totsndbytes;
            totsndbytes += wavleng[i];
        }

        for( i = numwaves; i < MAXWAVES; i++ )
        {
            memset( instname[i], 0, sizeof( instname[i] ) );
            wavoffs[i] = ( unsigned char * ) totsndbytes;
            wavleng[i] = 0L;
            repstart[i] = 0L;
            repleng[i] = 0L;
            finetune[i] = 0L;
        }

        if (!(snd = (char *)malloc(totsndbytes+2))) return(0);
        for(int i=0;i<MAXWAVES;i++)
        {
            wavoffs[i] += ((int64_t)snd);
        }
        for(int i=0;i<totsndbytes;i++)
        {
            snd[i]=d[p];p++;
        }
        snd[ totsndbytes ] = snd[ totsndbytes + 1 ] = 128;
    }
    else
    {
        return 0;
    }
    delete[] d;
	return 1;
}

kdmeng::kdmeng( unsigned samplerate, unsigned numspeakers, unsigned bytespersample )
	: kdmsamplerate( samplerate ), kdmnumspeakers( numspeakers ), kdmbytespersample( bytespersample )
{
    snd = 0;
	long i, j, k;

	j = ( ( ( 11025L * 2093 ) / kdmsamplerate ) << 13 );
	for( i = 1; i < 76; i++ )
	{
		frqtable[i] = j;
		j = mulscale30( j, 1137589835 );  //(pow(2,1/12)<<30) = 1137589835
	}
	for( i = 0; i >= -14; i-- ) frqtable[ i & 255 ] = ( frqtable[ ( i + 12 ) & 255 ] >> 1 );

	timecount = notecnt = musicstatus = musicrepeat = 0;

	clearbuf(stemp, MAXSAMPLESTOPROCESS, MAXSAMPLESTOPROCESS);
	for( i = 0; i < ( kdmsamplerate >> 11 ); i++ )
	{
		j = 1536 - ( i << 10 ) / ( kdmsamplerate >> 11 );
		fsin( &j );
		ramplookup[ i ] = ( ( 16384 - j ) << 1 );
	}

	for( i = 0; i < 256; i++ )
	{
		j = i * 90; fsin( &j );
		eff[ 0 ][ i ] = 65536 + j / 9;
		eff[ 1 ][ i ] = min( 58386 + ( ( i * ( 65536 - 58386 ) ) / 30 ), 65536 );
		eff[ 2 ][ i ] = max( 69433 + ( ( i * ( 65536 - 69433 ) ) / 30 ), 65536 );
		j = ( i * 2048 ) / 120; fsin( &j );
		eff[ 3 ][ i ] = 65536 + ( j << 2 );
		j = ( i * 2048 ) / 30; fsin( &j );
		eff[ 4 ][ i ] = 65536 + j;
		switch( ( i >> 1 ) % 3 )
		{
			case 0: eff[ 5 ][ i ] = 65536; break;
			case 1: eff[ 5 ][ i ] = 65536 * 330 / 262; break;
			case 2: eff[ 5 ][ i ] = 65536 * 392 / 262; break;
		}
		eff[ 6 ][ i ] = min( ( i << 16 ) / 120, 65536 );
		eff[ 7 ][ i ] = max( 65536 - ( i << 16 ) / 120, 0 );
	}

	musicoff();
}
kdmeng::~kdmeng()
{
    if(snd)
    {
        free(snd);
    }
}
void kdmeng::startwave( long wavnum, long dafreq, long davolume1, long davolume2, long dafrqeff, long davoleff, long dapaneff )
{
	long i, j, chanum;

	if ( ( davolume1 | davolume2 ) == 0 ) return;

	chanum = 0;
	for( i = NUMCHANNELS - 1; i > 0; i-- )
		if ( splc[ i ] > splc[ chanum ] )
			chanum = i;

	splc[ chanum ] = 0;     //Disable channel temporarily for clean switch

	if ( kdmnumspeakers == 1 )
		calcvolookupmono( &volookup[ chanum << 9 ], -( davolume1 + davolume2 ) << 6, ( davolume1 + davolume2 ) >> 1 );
	else
		calcvolookupstereo( &volookup[ chanum << 9], -( davolume1 << 7 ), davolume1, -( davolume2 << 7 ), davolume2 );

	sinc[ chanum ] = dafreq;
	svol1[ chanum ] = davolume1;
	svol2[ chanum ] = davolume2;
	soff[ chanum ] = wavoffs[ wavnum ] + wavleng[ wavnum ];
	splc[ chanum ] = -(static_cast<long>(wavleng[ wavnum ]) << 12 );              //splc's modified last
	swavenum[ chanum ] = wavnum;
	frqeff[ chanum ] = dafrqeff; frqoff[ chanum ] = 0;
	voleff[ chanum ] = davoleff; voloff[ chanum ] = 0;
	paneff[ chanum ] = dapaneff; panoff[ chanum ] = 0;
}
long kdmeng::load(void* data, unsigned long int length, const char* filepath)
{
    if(length<27) return 0;
    if ( !loadwaves( filepath ) ) return 0;
    if ( !snd ) return 0;

    unsigned int p = 0;
    unsigned char *d = static_cast<unsigned char*>(data);

    musicoff();

    kdmversionum = 0;
    numnotes = 0;
    numtracks = 0;
    memset( trinst, 0, sizeof( trinst ) );
    memset( trquant, 0, sizeof( trquant ) );
    memset( trvol1, 0, sizeof( trvol1 ) );
    memset( trvol2, 0, sizeof( trvol2 ) );
    memset( nttime, 0, sizeof( nttime ) );
    memset( nttrack, 0, sizeof( nttrack ) );
    memset( ntfreq, 0, sizeof( ntfreq ) );
    memset( ntvol1, 0, sizeof( ntvol1 ) );
    memset( ntvol2, 0, sizeof( ntvol2 ) );
    memset( ntfrqeff, 0, sizeof( ntfrqeff ) );
    memset( ntvoleff, 0, sizeof( ntvoleff ) );
    memset( ntpaneff, 0, sizeof( ntpaneff ) );

    kdmversionum = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
    if ( kdmversionum != 0 ) return 0;
    numnotes = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
    numtracks = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
    if(numtracks > MAXTRACKS || numtracks==0) return 0;
    if(numnotes > MAXNOTES || numnotes==0) return 0;

    for(int i = 0;i<numtracks;i++)
    {
        trinst[i] = d[p];p++;
    }
    for(int i = 0;i<numtracks;i++)
    {
        trquant[i] = d[p];p++;
    }
    for(int i = 0;i<numtracks;i++)
    {
        trvol1[i] = d[p];p++;
    }
    for(int i = 0;i<numtracks;i++)
    {
        trvol2[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        nttime[i] = readEndian(d[p+3],d[p+2],d[p+1],d[p]);p+=4;
    }
    for(int i = 0;i<numnotes;i++)
    {
        nttrack[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntfreq[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntvol1[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntvol2[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntfrqeff[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntvoleff[i] = d[p];p++;
    }
    for(int i = 0;i<numnotes;i++)
    {
        ntpaneff[i] = d[p];p++;
    }
    loopcnt = 0;

    return ( scale( nttime[ numnotes - 1 ] - nttime[ 0 ], 1000, 120 ) );
}

long kdmeng::rendersound( void * dasnd, long numbytes)
{
	long i, j, k, voloffs1, voloffs2, *voloffs3, *stempptr;
	long daswave, dasinc, dacnt, bytespertic, numsamplestoprocess;
	unsigned char *sndptr;

	sndptr = ( unsigned char * ) dasnd;

	numsamplestoprocess = ( numbytes >> ( kdmnumspeakers + kdmbytespersample - 2 ) );
	bytespertic = ( kdmsamplerate / 120 );
	for( dacnt = 0; dacnt < numsamplestoprocess; dacnt += bytespertic )
	{
		if ( musicstatus > 0 )    //Gets here 120 times/second
		{
			while ( ( notecnt < numnotes ) && ( timecount >= nttime[ notecnt ] ) )
			{
				j = trinst[ nttrack[ notecnt ] ];
				k = mulscale24( frqtable[ ntfreq[ notecnt ] ], finetune[ j ] + 748 );

				if ( ntvol1[ notecnt ] == 0 )   //Note off
				{
					for( i = NUMCHANNELS - 1 ; i >= 0; i-- )
						if ( splc[ i ] < 0 )
							if ( swavenum[ i ] == j )
								if ( sinc[ i ] == k )
									splc[ i ] = 0;
				}
				else                        //Note on
					startwave( j, k, ntvol1[ notecnt ], ntvol2[ notecnt ], ntfrqeff[ notecnt ], ntvoleff[ notecnt ], ntpaneff[ notecnt ] );

				notecnt++;
				if ( notecnt >= numnotes )
				{
					loopcnt++;
					if ( musicrepeat > 0 )
						notecnt = 0, timecount = nttime[ 0 ];
				}
			}
			timecount++;
		}

		for( i = NUMCHANNELS - 1; i >= 0; i-- )
		{
			if ( splc[ i ] >= 0 ) continue;

			dasinc = sinc[ i ];

			if ( frqeff[ i ] != 0 )
			{
				dasinc = mulscale16( dasinc, eff[ frqeff[ i ] - 1 ][ frqoff[ i ] ] );
				frqoff[ i ]++; if ( frqoff[ i ] >= 256 ) frqeff[ i ] = 0;
			}
			if ( ( voleff[ i ] ) || ( paneff[ i ] ) )
			{
				voloffs1 = svol1[ i ];
				voloffs2 = svol2[ i ];
				if ( voleff[ i ] )
				{
					voloffs1 = mulscale16( voloffs1, eff[ voleff[ i ] - 1 ][ voloff[ i ] ] );
					voloffs2 = mulscale16( voloffs2, eff[ voleff[ i ] - 1 ][ voloff[ i ] ] );
					voloff[ i ]++; if ( voloff[ i ] >= 256 ) voleff[ i ] = 0;
				}

				if ( kdmnumspeakers == 1 )
					calcvolookupmono( &volookup[ i << 9 ], -( voloffs1 + voloffs2 ) << 6, ( voloffs1 + voloffs2 ) >> 1 );
				else
				{
					if ( paneff[ i ] )
					{
						voloffs1 = mulscale16( voloffs1, 131072 - eff[ paneff[ i ] - 1 ][ panoff[ i ] ] );
						voloffs2 = mulscale16( voloffs2, eff[ paneff[ i ] - 1 ][ panoff[ i ] ] );
						panoff[ i ]++; if ( panoff[ i ] >= 256 ) paneff[ i ] = 0;
					}
					calcvolookupstereo( &volookup[ i << 9 ], -( voloffs1 << 7 ), voloffs1, -( voloffs2 << 7 ), voloffs2 );
				}
			}

			daswave = swavenum[ i ];
			voloffs3 = &volookup[ i << 9 ];

			kdmasm1 = repleng[ daswave ];
			kdmasm2 = wavoffs[ daswave ] + repstart[ daswave ] + repleng[ daswave ];
			kdmasm3 = ( repleng[ daswave ] << 12 ); //repsplcoff
			kdmasm4 = soff[ i ];
			if ( kdmnumspeakers == 1 )
				{ splc[ i ] = monohicomb( voloffs3, bytespertic, dasinc, splc[ i ], stemp ); }
			else
				{ splc[ i ] = stereohicomb( voloffs3, bytespertic, dasinc, splc[ i ], stemp ); }
			soff[ i ] = kdmasm4;

			if ( splc[ i ] >= 0 ) continue;
			if ( kdmnumspeakers == 1 )
				{ monohicomb( voloffs3, kdmsamplerate >> 11, dasinc, splc[ i ], &stemp[ bytespertic ] ); }
			else
				{ stereohicomb( voloffs3, kdmsamplerate >> 11, dasinc, splc[ i ], &stemp[ bytespertic << 1 ] ); }
		}

		if ( kdmnumspeakers == 1 )
		{
			for( i = ( kdmsamplerate >> 11 ) - 1; i >= 0; i-- )
				stemp[ i ] += mulscale16( stemp[ i + 1024 ] - stemp[ i ], ramplookup[ i ] );
			j = bytespertic; k = ( kdmsamplerate >> 11 );
			copybuf( ( void * ) &stemp[ j ], ( void * ) &stemp[ 1024 ], k );
			clearbuf( ( void * ) &stemp[ j ], k, MAXSAMPLESTOPROCESS );
		}
		else
		{
			for( i = ( kdmsamplerate >> 11 ) - 1; i >= 0; i-- )
			{
				j = ( i << 1 );
				stemp[ j + 0 ] += mulscale16( stemp[ j + 1024 ] - stemp[ j + 0 ], ramplookup[ i ] );
				stemp[ j + 1 ] += mulscale16( stemp[ j + 1025 ] - stemp[ j + 1 ], ramplookup[ i ] );
			}
			j = ( bytespertic << 1 ); k = ( ( kdmsamplerate >> 11 ) << 1 );
			copybuf( ( void * ) &stemp[ j ], ( void * ) &stemp[ 1024 ], k );
			clearbuf( ( void * ) &stemp[ j ], k, MAXSAMPLESTOPROCESS );
		}

		if ( kdmnumspeakers == 1 )
		{
			if ( kdmbytespersample == 1 ) bound2char( bytespertic >> 1, stemp, sndptr + dacnt );
			else bound2short( bytespertic >> 1, stemp, sndptr + ( dacnt << 1 ) );
		}
		else
		{
			if (kdmbytespersample == 1) bound2char( bytespertic, stemp, sndptr + ( dacnt << 1 ) );
			else bound2short( bytespertic, stemp, sndptr + ( dacnt << 2 ) );
		}
	}
	return(loopcnt);
}

void kdmeng::seek( long seektoms )
{
	long i;

	for( i = 0; i < NUMCHANNELS; i++ ) splc[ i ] = 0;

	i = scale( seektoms, 120, 1000 ) + nttime[0];

	notecnt = 0;
	while ( ( nttime[ notecnt ] < i ) && ( notecnt < numnotes ) ) notecnt++;
	if ( notecnt >= numnotes ) notecnt = 0;

	timecount = nttime[ notecnt ]; loopcnt = 0;
}
void kdmeng::getInstname(unsigned int idx,char* buff)
{
    char sztmp[17] = "";
    memcpy(sztmp, instname[idx],16);
    sztmp[16]=0;
    if (buff) strcpy(buff, sztmp);
}
unsigned kdmeng::getInstsize(unsigned int idx)
{
    return wavleng[idx];
}
unsigned kdmeng::getInstrepstart(unsigned int idx)
{
    return repstart[idx];
}

unsigned kdmeng::getInstreplength(unsigned int idx)
{
    return repleng[idx];
}
int kdmeng::getInstfinetune(unsigned int idx)
{
    return finetune[idx];
}
char kdmeng::getTrackInstrument(unsigned int idx)
{
    return trinst[idx];
}
char kdmeng::getTrackQuantize(unsigned int idx)
{
    return trquant[idx];
}
unsigned char kdmeng::getTrackVolume1(unsigned int idx)
{
    return trvol1[idx];
}
unsigned char kdmeng::getTrackVolume2(unsigned int idx)
{
    return trvol2[idx];
}
