/*===============================================================================================
 codec_libmod.dll
 Ver. 0.9
 by Andreas Argirakis 2008.
 FMOD input plugin
 Visit http://andreas.blazer.nu for latest version
 Built with libmodplug 0.8, http://modplug-xmms.sourceforge.net/
 
===============================================================================================*/

#include "modplug.h"
#include <libmodplug/stdafx.h>
#include <libmodplug/sndfile.h>
#include "amifilemagic.c"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "info.h"

using namespace std;
#include "fmod_errors.h"

//#include "Logfile.h"

//CLogFile *LogFile;

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        //LogFile->Print("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}
FMOD_RESULT handle_error( const char* str )
{
	if ( str )
	{
		//LogFile->Print( "Error: %s\n", str );
		return FMOD_ERR_INTERNAL;
	}
	else
		return FMOD_OK;
}
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION libmodcodec =
{
    "FMOD Libmod Plugin",			// Name.
    0x00009000,                         // Version 0xAAAABBBB   A = major, B = minor.
    1,                                  // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS|FMOD_TIMEUNIT_MODROW|FMOD_TIMEUNIT_MODPATTERN|FMOD_TIMEUNIT_MODPATTERN_INFO|FMOD_TIMEUNIT_MODVUMETER,					// The time format we would like to accept into setposition/getposition.
    &open,                           // Open callback.
    &close,                          // Close callback.
    &read,                           // Read callback.
    0,                                  // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                    // Setposition callback.
    &getposition,                      // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                   // Sound create callback (don't need it)
};
class libmodplugin
{
	FMOD_CODEC_STATE *_codec;

	public:
	libmodplugin(FMOD_CODEC_STATE *codec)
	{
		_codec = codec;
		//LogFile = new CLogFile("libmod.log");
		memset(&lpwaveformat, 0, sizeof(lpwaveformat));
	}

	~libmodplugin()
	{
		//delete some stuff
		if(!moddata)
		{
			ModPlug_Unload(moddata);
		}
	}
	ModPlugFile *moddata;
	Info* info;
	ModPlug_Settings settings;
	FMOD_CODEC_WAVEFORMAT lpwaveformat;
	
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_DECLSPEC F_DLLEXPORT FMOD_CODEC_DESCRIPTION * F_API FMODGetCodecDescription()
{
    return &libmodcodec;
}

#ifdef __cplusplus
}
#endif



FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

	FMOD_RESULT       result;
	libmodplugin *lp = new libmodplugin(codec);
	
	unsigned int bytesread;

	/* Allocate space for buffer. */
	signed short* myBuffer = new signed short[codec->filesize];

	//rewind file pointer
	result = codec->fileseek(codec->filehandle,0,(signed short*)myBuffer);
	ERRCHECK(result);

	//read whole file to memory
	result = codec->fileread(codec->filehandle,(signed short*)myBuffer,codec->filesize,&bytesread,0);
	ERRCHECK(result);


	//load music file, if it's reported as a mod use UADE code to do more refined check
	lp->moddata = ModPlug_Load(myBuffer,codec->filesize);
			
	if (!lp->moddata)
	{
		delete [] myBuffer;
		return FMOD_ERR_FORMAT;
	}

	lp->info = (Info*)userexinfo->userdata;
	int type = ModPlug_GetModuleType(lp->moddata);

	switch(type)
	{
		case MOD_TYPE_NONE:
			lp->info->fileformat="LibMod None";
			break;
		case MOD_TYPE_S3M:
			lp->info->fileformat="ScreamTracker 3";
			break;
		case MOD_TYPE_XM:
			lp->info->fileformat="FastTracker 2";
			break;
		case MOD_TYPE_MED:
			lp->info->fileformat="OctaMed";
			break;
		case MOD_TYPE_MTM:
			lp->info->fileformat="MultiTracker";
			break;
		case MOD_TYPE_IT:
			lp->info->fileformat="Impulse Tracker";
			break;
		case MOD_TYPE_669:
			lp->info->fileformat="UNIS 669";
			break;
		case MOD_TYPE_FAR:
			lp->info->fileformat="Farandole Composer";
			break;
		case MOD_TYPE_WAV:
			lp->info->fileformat="Wav";
			break;
		case MOD_TYPE_AMF:
			lp->info->fileformat="Asylum / DSMI";
			break;
		case MOD_TYPE_AMS:
			lp->info->fileformat="Velvet Studio AMS";
			break;
		case MOD_TYPE_DSM:
			lp->info->fileformat="Digisound Interface Kit (DSIK)";
			break;
		case MOD_TYPE_MDL:
			lp->info->fileformat="DigiTracker 1.x";
			break;
		case MOD_TYPE_OKT:
			lp->info->fileformat="Oktalyzer";
			break;
		case MOD_TYPE_MID:
			lp->info->fileformat="MIDI";
			break;
		case MOD_TYPE_DMF:
			lp->info->fileformat="Delusion/XTracker";
			break;
		case MOD_TYPE_PTM:
			lp->info->fileformat="PolyTracker";
			break;
		case MOD_TYPE_DBM:
			lp->info->fileformat="DigiBooster Pro";
			break;
		case MOD_TYPE_MT2:
			lp->info->fileformat="MadTracker 2.0";
			break;
		case MOD_TYPE_AMF0:
			lp->info->fileformat="Asylum / DSMI";
			break;
		case MOD_TYPE_PSM:
			lp->info->fileformat="Protracker Studio";
			break;
		case MOD_TYPE_J2B:
			lp->info->fileformat="J2B";
			break;
		case MOD_TYPE_UMX:
			lp->info->fileformat="Unreal Music";
			break;
		case MOD_TYPE_ULT:
			lp->info->fileformat="UltraTracker";
			break;
		case MOD_TYPE_STM:
			lp->info->fileformat="ScreamTracker II";
			break;
	}
	


	if(type == MOD_TYPE_MOD)
	{
		//use UADE to check if it's a valid mod file

		int t = mod15check((unsigned char *)myBuffer, codec->filesize, codec->filesize);
		if(t!=0)
		{
			switch (t)
			{
				case 1:
					lp->info->fileformat="DOC Soundtracker";
					break;
				case 2:
					lp->info->fileformat="Ultimate Soundtracker";
					break;
				case 3:
					lp->info->fileformat="Mastersoundtracker";
					break;
				case 4:
					lp->info->fileformat="SoundtrackerV2.0 -V4.0";
					break;
				default:
					lp->info->fileformat="Error in libmod";
					break;
			}
		}
		else //try to see if its a 32 instruments mod
		{
			t = mod32check((unsigned char *)myBuffer, codec->filesize, codec->filesize,0);
			switch (t)
			{
				case 0:
					delete [] myBuffer;
					ModPlug_Unload(lp->moddata);
					return FMOD_ERR_FORMAT;
					break;
				case 1:
					lp->info->fileformat="Soundtracker2.5/Noisetracker 1.0";
					break;
				case 2:
					lp->info->fileformat="Noisetracker 1.2";
					break;
				case 3:
					lp->info->fileformat="Noisetracker 2.0";
					break;
				case 4:
					lp->info->fileformat="Startrekker 4ch";
					break;
				case 5:
					lp->info->fileformat="Startrekker 8ch";
					break;
				case 6:
					lp->info->fileformat="Audiosculpture 4 ch/fm";
					break;
				case 7:
					lp->info->fileformat="Audiosculpture 8 ch/fm";
					break;
				case 8:
					lp->info->fileformat="Protracker";
					break;
				case 9:
					lp->info->fileformat="Fasttracker";
					break;
				case 10:
					lp->info->fileformat="Noisetracker (M&K!)";
					break;
				case 11:
					lp->info->fileformat="Protracker Compatible";
					break;
				case 12:
					lp->info->fileformat="Soundtracker 31instr";
					break;
				default:
					lp->info->fileformat="Error in libmod";
					break;
			}
		}
	}

	//read config from disk

	
	string filename = lp->info->applicationPath + "/plugin/config/libmodplug.cfg";
	ifstream ifs( filename.c_str() );
	string line;

	bool useDefaults = false;
	if(ifs.fail())
	{
		//The file could not be opened
		useDefaults = true;
	}

	int emulator = 0;
	int freq = 44100;
	int channels = 2;
	lp->lpwaveformat.channels = 2;
	int resampling = MODPLUG_RESAMPLE_FIR;
	int oversampling = MODPLUG_ENABLE_OVERSAMPLING;
	int noise_reduction = MODPLUG_ENABLE_NOISE_REDUCTION;
	int reverb = 0;
	int megabass = 0;
	int surround = 0;
	int reverb_depth = 50;
	int reverb_delay = 100;
	int surround_depth = 50;
	int surround_delay = 20;
	int megabass_amount = 50;
	int megabass_range = 20;
	int maxMixChannels = 32;
	int stereoSeparation = 128;
	if(!useDefaults)
	{
		while( getline( ifs , line) )
		{
			int i = line.find_first_of("=");
			
			if(i!=-1)
			{
				string word = line.substr(0,i);
				string value = line.substr(i+1);
				if(word.compare("frequency")==0)
				{
					freq = atoi(value.c_str());
				}
				else if(word.compare("playback")==0)
				{
					if(value.compare("stereo")==0)
					{
						channels = 2;
					}
					else
					{
						channels = 1;
					}
				}
				else if(word.compare("resampling")==0)
				{
					if(value.compare("nearest")==0)
					{
						resampling = MODPLUG_RESAMPLE_NEAREST;
					}
					else if(value.compare("linear")==0)
					{
						resampling = MODPLUG_RESAMPLE_LINEAR;
					}
					else if(value.compare("spline")==0)
					{
						resampling = MODPLUG_RESAMPLE_SPLINE;
					}
					else if(value.compare("fir")==0)
					{
						resampling = MODPLUG_RESAMPLE_FIR;
					}
				}
				else if(word.compare("oversampling")==0)
				{
					if(value.compare("yes")==0)
					{
						oversampling = MODPLUG_ENABLE_OVERSAMPLING;
					}
					else if(value.compare("no")==0)
					{
						oversampling = 0;
					}
				}
				else if(word.compare("noise_reduction")==0)
				{
					if(value.compare("yes")==0)
					{
						noise_reduction = MODPLUG_ENABLE_NOISE_REDUCTION;
					}
					else if(value.compare("no")==0)
					{
						noise_reduction = 0;
					}
				}
				else if(word.compare("reverb")==0)
				{
					if(value.compare("yes")==0)
					{
						reverb = MODPLUG_ENABLE_REVERB;
					}
					else if(value.compare("no")==0)
					{
						reverb = 0;
					}
				}
				else if(word.compare("megabass")==0)
				{
					if(value.compare("yes")==0)
					{
						megabass = MODPLUG_ENABLE_MEGABASS;
					}
					else if(value.compare("no")==0)
					{
						megabass = 0;
					}
				}
				else if(word.compare("surround")==0)
				{
					if(value.compare("yes")==0)
					{
						surround = MODPLUG_ENABLE_SURROUND;
					}
					else if(value.compare("no")==0)
					{
						surround = 0;
					}
				}
				else if(word.compare("surround_delay")==0)
				{
					surround_delay = atoi(value.c_str());
				}
				else if(word.compare("surround_depth")==0)
				{
					surround_depth = atoi(value.c_str());
				}
				else if(word.compare("reverb_delay")==0)
				{
					reverb_delay = atoi(value.c_str());
				}
				else if(word.compare("reverb_depth")==0)
				{
					reverb_depth = atoi(value.c_str());
				}
				else if(word.compare("megabass_amount")==0)
				{
					megabass_amount = atoi(value.c_str());
				}
				else if(word.compare("megabass_range")==0)
				{
					megabass_range = atoi(value.c_str());
				}
				else if(word.compare("max_mix_channels")==0)
				{
					maxMixChannels = atoi(value.c_str());
				}
				else if(word.compare("stereo_separation")==0)
				{
					stereoSeparation = atoi(value.c_str());
				}
			}
		}
		ifs.close();
	}




	lp->lpwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
	lp->lpwaveformat.channels     = channels;
	lp->lpwaveformat.frequency    = freq;
	lp->lpwaveformat.blockalign   = (16 >> 3) * lp->lpwaveformat.channels;
	

	codec->waveformat   = &(lp->lpwaveformat);
	codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
	codec->plugindata   = lp;                    /* user data value */

	lp->lpwaveformat.lengthpcm = (ModPlug_GetLength(lp->moddata)/1000*lp->lpwaveformat.frequency);
	//lp->lpwaveformat.lengthpcm = 0xffffffff;


    ModPlug_GetSettings(&lp->settings);

	lp->settings.mChannels = channels;
	lp->settings.mLoopCount = 0;
    lp->settings.mResamplingMode = resampling;
	lp->settings.mFlags=oversampling | noise_reduction | reverb | megabass | surround;
    lp->settings.mBits = 16;
    lp->settings.mFrequency = freq;
	lp->settings.mReverbDelay = reverb_delay;
	lp->settings.mReverbDepth = reverb_depth;
	lp->settings.mSurroundDelay = surround_delay;
	lp->settings.mSurroundDepth = surround_depth;
	lp->settings.mBassAmount = megabass_amount;
	lp->settings.mBassRange = megabass_range;
	lp->settings.mMaxMixChannels = maxMixChannels;
	lp->settings.mStereoSeparation = stereoSeparation;
	
    ModPlug_SetSettings(&lp->settings);

	//load music file AGAIN, since basic settings isn't applied until next load
	lp->moddata = ModPlug_Load(myBuffer,codec->filesize);
	delete [] myBuffer;

	lp->info->numPatterns = ModPlug_NumPatterns(lp->moddata);
	lp->info->numChannels = ModPlug_NumChannels(lp->moddata);

	lp->info->modVUMeters = new unsigned char(lp->info->numChannels);

	if(ModPlug_GetName(lp->moddata))
		lp->info->title = ModPlug_GetName(lp->moddata);
	
	if(ModPlug_GetMessage(lp->moddata))
		lp->info->comments = ModPlug_GetMessage(lp->moddata);

	char* c = new char[40];
	
	int numSamples = ModPlug_NumSamples(lp->moddata);
	lp->info->numSamples = numSamples;

	if(numSamples>0)
	{
		lp->info->samples = new string[numSamples];
		lp->info->samplesSize = new unsigned int[numSamples];
		lp->info->samplesLoopStart = new unsigned int[numSamples];
		lp->info->samplesLoopEnd = new unsigned int[numSamples];
		lp->info->samplesVolume = new unsigned short[numSamples];
		lp->info->samplesFineTune = new signed char[numSamples];
		lp->info->samples16Bit = new bool[numSamples];
		lp->info->samplesStereo = new bool[numSamples];
		//lp->info->samplesData = new signed char*[numSamples];
		for(int j = 1; j<=numSamples; j++)
		{
			int k = ModPlug_SampleName(lp->moddata, j, c);
			lp->info->samplesSize[j-1] = ModPlug_SampleSize(lp->moddata,j);
			lp->info->samplesLoopStart[j-1] = ModPlug_SampleLoopStart(lp->moddata,j);
			lp->info->samplesLoopEnd[j-1] = ModPlug_SampleLoopEnd(lp->moddata,j);
			lp->info->samplesVolume[j-1] = ModPlug_SampleVolume(lp->moddata,j);
			lp->info->samplesFineTune[j-1] = ModPlug_SampleFineTune(lp->moddata,j);
			lp->info->samples16Bit[j-1] = ModPlug_Sample16Bit(lp->moddata,j);
			lp->info->samplesStereo[j-1] = ModPlug_SampleStereo(lp->moddata,j);
			//ModPlug_SampleData(lp->moddata,j,lp->info->samplesData[j-1]);
			lp->info->samples[j-1] = c;
		}
	}
	



	int numInstruments = ModPlug_NumInstruments(lp->moddata);
	lp->info->numInstruments = numInstruments;

	if(numInstruments>0)
	{
		lp->info->instruments = new string[numInstruments];
		
		for(int j = 1; j<=numInstruments; j++)
		{
			int k = ModPlug_InstrumentName(lp->moddata, j, c);
			lp->info->instruments[j-1] = c;
		}
	}

	lp->info->plugin = "libmodplug";

	delete c;
	return FMOD_OK;

}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
	libmodplugin* lp = (libmodplugin*)codec->plugindata;
	//delete (libmodplugin*)codec->plugindata;
	//delete LogFile;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
	libmodplugin* lp = (libmodplugin*)codec->plugindata;
	*read=ModPlug_Read(lp->moddata,buffer,size);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
	libmodplugin* lp = (libmodplugin*)codec->plugindata;
	ModPlug_Seek(lp->moddata,position);
	return FMOD_OK;
}
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype)
{
	libmodplugin* lp = (libmodplugin*)codec->plugindata;

	if(postype==FMOD_TIMEUNIT_MS)
	{
		*position = 0;
		return FMOD_OK;
	}
	else if(postype==FMOD_TIMEUNIT_MODPATTERN)
	{
		//current mod pattern position
		*position = ModPlug_GetCurrentPattern(lp->moddata);
	}
	else if(postype==FMOD_TIMEUNIT_MODPATTERN_INFO)
	{
		//set the mod pattern (notes etc.) in the info struct and just return 0
		lp->info->modPattern = ModPlug_GetPattern(lp->moddata, ModPlug_GetCurrentPattern(lp->moddata), &lp->info->modPatternRows);
		*position = 0;
		return FMOD_OK;
	}
	else if(postype==FMOD_TIMEUNIT_MODROW)
	{
		*position = ModPlug_GetCurrentRow(lp->moddata);
		return FMOD_OK;
	}
	else if(postype==FMOD_TIMEUNIT_MODVUMETER)
	{
		*position = ModPlug_GetChannelVUMeter(lp->moddata,lp->info->modVUMeters);
		return FMOD_OK;
	}
	
	else
	{
		return FMOD_ERR_UNSUPPORTED;
	}
}

