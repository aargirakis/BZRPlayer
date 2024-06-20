#include "StSoundLibrary.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);
FMOD_CODEC_DESCRIPTION gamecodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD YM Plugin",					// Name.
    0x00012300,                         // Version 0xAAAABBBB   A = major, B = minor.
    1,                                  // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,					// The time format we would like to accept into setposition/getposition.
    &open,                           // Open callback.
    &close,                          // Close callback.
    &read,                           // Read callback.
    0,								// Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                    // Setposition callback.
    nullptr,								  // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr ,                               // Sound create callback (don't need it)
    nullptr
};
class gameplugin
{
	FMOD_CODEC_STATE *_codec;

	public:
	gameplugin(FMOD_CODEC_STATE *codec)
	{
		_codec = codec;
		memset(&gpwaveformat, 0, sizeof(gpwaveformat));
	}

	~gameplugin()
	{
		//delete some stuff
        ymMusicDestroy(pMusic);
        delete[] myBuffer;
	}
	YMMUSIC *pMusic;
	FMOD_CODEC_WAVEFORMAT gpwaveformat;
    signed char* myBuffer;
	
};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &gamecodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

	FMOD_RESULT       result;
	gameplugin *gp = new gameplugin(codec);
    Info* info = static_cast<Info*>(userexinfo->userdata);
	gp->pMusic = ymMusicCreate();
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    unsigned int bytesread;
    if(filesize==4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }
    gp->myBuffer = new signed char[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,gp->myBuffer,filesize,&bytesread);

    if(!ymMusicLoadMemory(gp->pMusic,static_cast<signed char*>(gp->myBuffer),filesize))
    {
        ymMusicDestroy(gp->pMusic);
        delete gp->myBuffer;
		return FMOD_ERR_FORMAT;
    }
    cout << "we got ym!\n";
    flush(cout);

	ymMusicInfo_t yminfo;
	ymMusicGetInfo(gp->pMusic,&yminfo);


	gp->gpwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    gp->gpwaveformat.channels     = 2;
	gp->gpwaveformat.frequency    = 44100;
    gp->gpwaveformat.pcmblocksize   = (16 >> 3) * gp->gpwaveformat.channels;
    gp->gpwaveformat.lengthpcm = 0xffffffff;
	if(yminfo.musicTimeInMs>0)
	{
		gp->gpwaveformat.lengthpcm = yminfo.musicTimeInMs/1000*gp->gpwaveformat.frequency;
	}
	
	
	codec->waveformat   = &(gp->gpwaveformat);
	codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
	codec->plugindata   = gp;                    /* user data value */

    ymMusicSetLoopMode(gp->pMusic,YMTRUE);
	

    info->artist = yminfo.pSongAuthor;
    info->title = yminfo.pSongName;
    info->comments = yminfo.pSongComment;
    info->songPlayer = yminfo.pSongPlayer;
    info->songType = yminfo.pSongType;
    info->comments = yminfo.pSongComment;
    info->numSamples = 0;
	info->plugin = PLUGIN_libstsound;
	info->pluginName = PLUGIN_libstsound_NAME;


    gp->gpwaveformat.channels     = 2;
    if(info->songType=="MIX1")
    {
        gp->gpwaveformat.channels = 1;
    }

	info->fileformat = yminfo.pSongType;
    info->setSeekable(false);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
    gameplugin* gp = static_cast<gameplugin*>(codec->plugindata);
    delete gp;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    gameplugin* gp = static_cast<gameplugin*>(codec->plugindata);

    int nbSample = size;
    ymMusicCompute(gp->pMusic,static_cast<ymsample*>(buffer),nbSample);
    *read=size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    gameplugin* gp = static_cast<gameplugin*>(codec->plugindata);
    if(postype==FMOD_TIMEUNIT_MS)
    {
        ymMusicStop(gp->pMusic);
        ymMusicPlay(gp->pMusic);
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
	return FMOD_OK;

}

