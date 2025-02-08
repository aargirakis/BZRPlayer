#include <iostream>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <vector>
#include <windows.h>
#include <fmod_errors.h>
#include "info.h"
#include "sunvox.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK sunvoxopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK sunvoxclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK sunvoxread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK sunvoxgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK sunvoxsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                         FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK sunvoxgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);


FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_sunvox_lib_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS |
    FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN | FMOD_TIMEUNIT_MODPATTERN_INFO |
    FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS | FMOD_TIMEUNIT_MODVUMETER | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_SPEED |
    FMOD_TIMEUNIT_BPM, // The time format we would like to accept into setposition/getposition.
    &sunvoxopen, // Open callback.
    &sunvoxclose, // Close callback.
    &sunvoxread, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &sunvoxsetposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginSunvoxLib
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginSunvoxLib(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSunvoxLib()
    {
    }

    unsigned char* myBuffer;

    Info* info;
    FMOD_CODEC_WAVEFORMAT waveformat;
};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

FMOD_RESULT F_CALLBACK sunvoxopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    if (sv_load_dll2((static_cast<string>(&DATA_PLUGINS_DIR[1]) + "/sunvox.dll").c_str()))
        return FMOD_ERR_FORMAT;
    FMOD_RESULT result;

    unsigned int bytesread;

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto* plugin = new pluginSunvoxLib(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    char id[4] = "";
    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, id, 4, &bytesread);

    Info* info = static_cast<Info*>(userexinfo->userdata);
    // HivelyTracker file
    if ((id[0] == 'S') && (id[1] == 'V') && (id[2] == 'O') && (id[3] == 'X'))
    {
        info->fileformat = "SunVox";
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }


    cout << "SunVox sv_init\n";
    flush(cout);
    int ver = sv_init(0, 44100, 2, SV_INIT_FLAG_USER_AUDIO_CALLBACK);
    if (ver >= 0)
    {
        int major = (ver >> 16) & 255;
        int minor1 = (ver >> 8) & 255;
        int minor2 = (ver) & 255;
    }
    sv_open_slot(0);

    plugin->myBuffer = new unsigned char[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    int res = -1;
    res = sv_load_from_memory(0, plugin->myBuffer, filesize);

    delete[] plugin->myBuffer;
    if (res == 0)
    {
        plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
        plugin->waveformat.channels = 2;
        plugin->waveformat.frequency = 44100;
        plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
        plugin->waveformat.lengthpcm = sv_get_song_length_frames(0);

        codec->waveformat = &(plugin->waveformat);
        codec->numsubsounds = 0;
        /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata = plugin; /* user data value */

        info->plugin = PLUGIN_sunvox_lib;
        info->pluginName = PLUGIN_sunvox_lib_NAME;
        info->title = sv_get_song_name(0);
        info->setSeekable(false);
        sv_play_from_beginning(0);

        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }
}

FMOD_RESULT F_CALLBACK sunvoxclose(FMOD_CODEC_STATE* codec)
{
    sv_stop(0);
    sv_close_slot(0);
    sv_deinit();
    delete static_cast<pluginSunvoxLib*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK sunvoxread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginSunvoxLib*>(codec->plugindata);
    sv_audio_callback(buffer, size, 0, sv_get_ticks());
    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK sunvoxsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                         FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginSunvoxLib*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        sv_stop(0);
        sv_play_from_beginning(0);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALLBACK sunvoxgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginSunvoxLib*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODROW)
    {
        int line = sv_get_current_line2(0);
        *position = line;
        return FMOD_OK;
    }
}
