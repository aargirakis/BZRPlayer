#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "fmod_errors.h"

#include "info.h"
#include "kmp_pi.h"


FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD FLOD plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    0,                                   // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,	 // The time format we would like to accept into setposition/getposition.
    &fcopen,                             // Open callback.
    &fcclose,                            // Close callback.
    &fcread,                             // Read callback.
    0,                        // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE *_codec;

public:
    fcplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
        //memset(&ay, 0, sizeof(ay));
        pos = 0;
    }

    ~fcplugin()
    {
        //delete some stuff
        delete[] myBuffer;
        myBuffer=0;
        S98_Close(s98);
    }
    BYTE* myBuffer;
    Info* info;
    void* s98;
    SOUNDINFO s98info;
    size_t pos;

    FMOD_CODEC_WAVEFORMAT fcwaveformat;
};
/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &fccodec;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    FMOD_RESULT       result;

    fcplugin *fc = new fcplugin(codec);
    fc->info = (Info*)userexinfo->userdata;

    char* smallBuffer;
    smallBuffer = new char[4];
    unsigned int bytesread;

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,smallBuffer,4,&bytesread);

    if(bytesread!=4)
    {
        delete smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    if(smallBuffer[0]=='S' && smallBuffer[1]=='9' && smallBuffer[2]=='8' && smallBuffer[3]=='3')
    {
        fc->info->fileformat = "S98 V.3";
    }
    else if(smallBuffer[0]=='S' && smallBuffer[1]=='9' && smallBuffer[2]=='8' && smallBuffer[3]=='2')
    {
        fc->info->fileformat = "S98 V.2";
    }
    else if(smallBuffer[0]=='S' && smallBuffer[1]=='9' && smallBuffer[2]=='8' && smallBuffer[3]=='1')
    {
        fc->info->fileformat = "S98 V.1";
    }
    else if(smallBuffer[0]=='S' && smallBuffer[1]=='9' && smallBuffer[2]=='8' && smallBuffer[3]=='0')
    {
        fc->info->fileformat = "S98 V.0";
    }
    else
    {
        delete smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete smallBuffer;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    fc->myBuffer = new BYTE[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,smallBuffer,filesize,&bytesread);

    fc->s98info.dwSamplesPerSec=44100;
    fc->s98 = S98_OpenFromBuffer(fc->myBuffer, filesize,&fc->s98info);

    if(!fc->s98 )
    {
        return FMOD_ERR_FORMAT;
    }

    fc->info = (Info*)userexinfo->userdata;

    fc->fcwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels     = 2;
    fc->fcwaveformat.frequency    = 44100;
    fc->fcwaveformat.pcmblocksize   = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm = fc->s98info.dwLength/1000*fc->fcwaveformat.frequency;

    codec->waveformat   = &(fc->fcwaveformat);
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = fc;                    /* user data value */


    fc->info->plugin = "KbMedia Player";
    fc->info->setSeekable(true);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec)
{
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{

    fcplugin* fc = (fcplugin*)codec->plugindata;
    S98_Render(fc->s98, (BYTE*)buffer, size);
    *read=size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if(postype==FMOD_TIMEUNIT_MS)
    {
        fcplugin* fc = (fcplugin*)codec->plugindata;
        //S98_SetPosition(fc->s98, position);
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}


