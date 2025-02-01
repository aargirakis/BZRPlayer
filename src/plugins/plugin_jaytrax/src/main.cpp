#include <string.h>
#include <string>
#include "fmod_errors.h"

extern "C" {
#include "jxs.h"
#include "jaytrax.h"
}

#include "info.h"
#include <iostream>
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION jaytraxcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "Jaytrax player plugin", // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class jaytraxplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    jaytraxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&jaytraxwaveformat, 0, sizeof(jaytraxwaveformat));
    }

    ~jaytraxplugin()
    {
    }

    FMOD_CODEC_WAVEFORMAT jaytraxwaveformat;
    JT1Player* jay;
    JT1Song* song;
    uint8_t* myBuffer;
    unsigned int length;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &jaytraxcodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    Info* info = (Info*)userexinfo->userdata;


    jaytraxplugin* jaytrax = new jaytraxplugin(codec);


    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);


    jaytrax->myBuffer = new uint8_t[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, jaytrax->myBuffer, filesize, &bytesread);

    bool isErr = jxsfile_readSongMem(jaytrax->myBuffer, filesize, &jaytrax->song) != 0;
    delete[] jaytrax->myBuffer;
    if (isErr)
    {
        return FMOD_ERR_FORMAT;
    }

	jaytrax->jay = jaytrax_init();
    jaytrax->jay->song = jaytrax->song;

    jaytrax_changeSubsong(jaytrax->jay, 0);

    jaytrax->jaytraxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    jaytrax->jaytraxwaveformat.channels = 2;
    jaytrax->jaytraxwaveformat.frequency = 44100;
    jaytrax->jaytraxwaveformat.pcmblocksize = (16 >> 3) * jaytrax->jaytraxwaveformat.channels;
    jaytrax->jaytraxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &jaytrax->jaytraxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = jaytrax; /* user data value */

    info->plugin = PLUGIN_jaytrax;
    info->pluginName = PLUGIN_jaytrax_NAME;
    info->fileformat = "Jaytrax";
    info->setSeekable(false);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    jaytraxplugin* jaytrax = (jaytraxplugin*)codec->plugindata;
	if(jaytrax!=nullptr)
	{
		jaytrax_free(jaytrax->jay);
	}
    delete jaytrax;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    jaytraxplugin* jaytrax = (jaytraxplugin*)codec->plugindata;

    jaytrax_renderChunk(jaytrax->jay, (int16_t*)buffer, size, jaytrax->jaytraxwaveformat.frequency);

    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    jaytraxplugin* jaytrax = (jaytraxplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MS)
    {
        //        jaytrax_stopSong(jaytrax->jay);
        //        jaytrax_loadSong(jaytrax->jay,jaytrax->jay->song);
        //        jaytrax_continueSong(jaytrax->jay);
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        jaytrax->jay->subsongNr = position;
        jaytrax->length = (jaytrax_getLength(jaytrax->jay, position, 1,
                                             jaytrax->jaytraxwaveformat.frequency) /
            jaytrax->jaytraxwaveformat.frequency) * 1000.0;

        return FMOD_OK;
    }
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    jaytraxplugin* jaytrax = (jaytraxplugin*)codec->plugindata;
    if (lengthtype == FMOD_TIMEUNIT_MS)
    {
        *length = 0xffffffff;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = jaytrax->jay->song->nrofsongs;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = jaytrax->length;
    }
    return FMOD_OK;
}
