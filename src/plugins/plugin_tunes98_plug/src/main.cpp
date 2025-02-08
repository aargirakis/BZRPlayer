#include <cstring>
#include <fstream>
#include <cstdio>
#include "fmod_errors.h"
#include "info.h"
#include "kmp_pi.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_tunes98_plug_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginTunes98
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginTunes98(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
        //memset(&ay, 0, sizeof(ay));
        pos = 0;
    }

    ~pluginTunes98()
    {
        //delete some stuff
        delete[] myBuffer;
        myBuffer = 0;
        S98_Close(s98);
    }

    FILE* fp;
    BYTE* myBuffer;
    Info* info;
    void* s98;
    SOUNDINFO s98info;
    size_t pos;

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

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto* plugin = new pluginTunes98(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->fp = fopen(plugin->info->filename.c_str(), "rb");

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    plugin->myBuffer = new BYTE[filesize];

    // buffer will store the DWORD read from the file
    fread(plugin->myBuffer, 1, filesize, plugin->fp);

    if (plugin->myBuffer[0] == 'S' && plugin->myBuffer[1] == '9' && plugin->myBuffer[2] == '8' && plugin->myBuffer[3] == '3')
    {
        plugin->info->fileformat = "S98 V.3";
    }
    else if (plugin->myBuffer[0] == 'S' && plugin->myBuffer[1] == '9' && plugin->myBuffer[2] == '8' && plugin->myBuffer[3] == '2')
    {
        plugin->info->fileformat = "S98 V.2";
    }
    else if (plugin->myBuffer[0] == 'S' && plugin->myBuffer[1] == '9' && plugin->myBuffer[2] == '8' && plugin->myBuffer[3] == '1')
    {
        plugin->info->fileformat = "S98 V.1";
    }
    else if (plugin->myBuffer[0] == 'S' && plugin->myBuffer[1] == '9' && plugin->myBuffer[2] == '8' && plugin->myBuffer[3] == '0')
    {
        plugin->info->fileformat = "S98 V.0";
    }
    else
    {
        delete plugin->myBuffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->s98info.dwSamplesPerSec = 44100;
    plugin->s98 = S98_OpenFromBuffer(plugin->myBuffer, filesize, &plugin->s98info);

    if (!plugin->s98)
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = plugin->s98info.dwLength / 1000 * plugin->waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->plugin = PLUGIN_tunes98_plug;
    plugin->info->pluginName = PLUGIN_tunes98_plug_NAME;
    plugin->info->setSeekable(true);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginTunes98*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginTunes98*>(codec->plugindata);
    S98_Render(plugin->s98, static_cast<BYTE*>(buffer), size);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    if (postype == FMOD_TIMEUNIT_MS)
    {
        auto* plugin = static_cast<pluginTunes98*>(codec->plugindata);
        S98_SetPosition(plugin->s98, position);
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
