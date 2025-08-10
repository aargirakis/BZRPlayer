#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include "fmod_errors.h"
#include "types.h"
#include "info.h"
#include <iostream>
#include "plugins.h"

extern "C" {
#include "wsr_player.h"
#include "ws_audio.h"
}

short* sample_buffer = nullptr;

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_audiodecoder_wsr_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginWsr
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginWsr(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginWsr()
    {
        //delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto info = static_cast<Info *>(userexinfo->userdata);


    int freq = 44100;
    int channels = 2;
    SampleRate = freq;

    auto *plugin = new pluginWsr(codec);
    string filename_lowercase = info->filename;
    std::transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(), ::tolower);
    if (filename_lowercase.substr(filename_lowercase.find_last_of(".") + 1) != "wsr")
    {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    ROMSize = filesize;
    ROMBank = (ROMSize + 0xFFFF) >> 16;
    ROM = static_cast<BYTE *>(malloc(ROMBank * 0x10000));
    if (!ROM)
    {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }
    FMOD_RESULT result;

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, ROM, filesize, &bytesread);


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = 128 * 2 * 2;
    plugin->waveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->plugin = PLUGIN_audiodecoder_wsr;
    info->pluginName = PLUGIN_audiodecoder_wsr_NAME;
    info->fileformat = "Wonderswan";
    info->setSeekable(false);
    info->numSubsongs = 255;
    Init_WSR();

    Reset_WSR(Get_FirstSong());

    return FMOD_OK;
}

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    Close_WSR();
    auto* plugin = static_cast<pluginWsr*>(codec->plugindata);
    delete plugin;

    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginWsr*>(codec->plugindata);

    if (size == plugin->waveformat.pcmblocksize)
    {
        Update_WSR(40157, 0);
    }
    ws_audio_update(static_cast<short int *>(buffer), size);
    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    if (postype == FMOD_TIMEUNIT_MS)
    {
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        Reset_WSR(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = 0xffffffff;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = 255;
    }
    return FMOD_OK;
}
