
#include <stdio.h>
#include <string.h>
#include <string>
#include <algorithm>
#include "fmod_errors.h"
#include "types.h"
#include <stdlib.h>

extern "C" {
#include "wsr_player.h"
#include "ws_audio.h"
}
short* sample_buffer=NULL;

#include "info.h"
#include <iostream>
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION wsrcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "Wonderswan player plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    1,                                   // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS|FMOD_TIMEUNIT_SUBSONG,	              // The time format we would like to accept into setposition/getposition.
    &open,                             // Open callback.
    &close,                            // Close callback.
    &read,                             // Read callback.
    &getlength,                                   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};

class wsrplugin
{
    FMOD_CODEC_STATE *_codec;

public:
    wsrplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&wsrwaveformat, 0, sizeof(wsrwaveformat));
    }

    ~wsrplugin()
    {
        //delete some stuff

    }
    FMOD_CODEC_WAVEFORMAT wsrwaveformat;

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
    return &wsrcodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

    Info* info = (Info*)userexinfo->userdata;


    int freq = 44100;
    int channels = 2;
    SampleRate = freq;

    wsrplugin *wsr = new wsrplugin(codec);
    string filename_lowercase = info->filename;
    std::transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(), ::tolower);
    if(filename_lowercase.substr(filename_lowercase.find_last_of(".") + 1) != "wsr")
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    ROMSize = filesize;
    ROMBank = (ROMSize+0xFFFF)>>16;
    ROM = (BYTE*)malloc(ROMBank*0x10000);
    if (!ROM)
    {
        return FMOD_ERR_FORMAT;
    }
    FMOD_RESULT       result;

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,ROM,filesize,&bytesread);


    wsr->wsrwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    wsr->wsrwaveformat.channels     = channels;
    wsr->wsrwaveformat.frequency    = freq;
    wsr->wsrwaveformat.pcmblocksize   = 128*2*2;
    wsr->wsrwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat   = &wsr->wsrwaveformat;
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = wsr;                    /* user data value */

    info->plugin = PLUGIN_audiodecoder_wsr;
    info->pluginName = PLUGIN_audiodecoder_wsr_NAME;
    info->fileformat ="Wonderswan";
    info->setSeekable(false);
    info->numSubsongs = 255;
    Init_WSR();

    Reset_WSR(Get_FirstSong());

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
    Close_WSR();
    wsrplugin* wsr = (wsrplugin*)codec->plugindata;
    delete wsr;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    wsrplugin* wsr = (wsrplugin*)codec->plugindata;

    if(size== wsr->wsrwaveformat.pcmblocksize)
    {
        Update_WSR(40157, 0);
    }
    ws_audio_update((short int*)buffer,size);
    *read=size;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if(postype==FMOD_TIMEUNIT_MS)
    {
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_SUBSONG)
    {
        if(position<0) position = 0;
        Reset_WSR(position);
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{

    wsrplugin* wsr = (wsrplugin*)codec->plugindata;
    if(lengthtype==FMOD_TIMEUNIT_SUBSONG_MS)
    {
         *length = 0xffffffff;
    }
    else if(lengthtype==FMOD_TIMEUNIT_SUBSONG)
    {
        *length = 255;
    }
    return FMOD_OK;


}
