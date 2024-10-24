#include "FC.h"
#include "fc14audiodecoder.h"
#include <cstring>
#include <iostream>
#include <cstdio>
#include "fmod_errors.h"
#include <queue>
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libfc14audiodecoder_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODVUMETER,
    // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    &fcgetlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    &fcgetposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLibfc14
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibfc14(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
        decoder = NULL;
    }

    ~pluginLibfc14()
    {
        //delete some stuff
        fc14dec_delete(decoder);
        decoder = NULL;
    }

    void* decoder;
    int queueSize;
    Info* info;
    queue<unsigned char*> vumeterBuffer;

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
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    auto* plugin = new pluginLibfc14(codec);

    unsigned int bytesread;
    char* smallBuffer;
    smallBuffer = new char[5];
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 5, &bytesread);

    bool isSMOD = false;
    bool isFC14 = false;
    // Check for "SMOD" ID (Future Composer 1.0 to 1.3).
    isSMOD = (smallBuffer[0] == 0x53 && smallBuffer[1] == 0x4D &&
        smallBuffer[2] == 0x4F && smallBuffer[3] == 0x44 &&
        smallBuffer[4] == 0x00);

    // Check for "FC14" ID (Future Composer 1.4).
    isFC14 = (smallBuffer[0] == 0x46 && smallBuffer[1] == 0x43 &&
        smallBuffer[2] == 0x31 && smallBuffer[3] == 0x34 &&
        smallBuffer[4] == 0x00);

    delete[] smallBuffer;
    if (!isSMOD && !isFC14)
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->info = static_cast<Info*>(userexinfo->userdata);

    signed short* myBuffer;
    myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    plugin->decoder = fc14dec_new();
    int ok = fc14dec_init(plugin->decoder, myBuffer, filesize);
    delete[] myBuffer;
    if (!ok)
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = fc14dec_duration(plugin->decoder) / 1000.0 * plugin->waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    fc14dec_mixer_init(plugin->decoder, plugin->waveformat.frequency, 16, plugin->waveformat.channels, 0);
    if (isSMOD)
    {
        plugin->info->fileformat = "Future Composer 1.0-1.3";
    }
    else if (isFC14)
    {
        plugin->info->fileformat = "Future Composer 1.4";
    }

    plugin->info->plugin = PLUGIN_libfc14audiodecoder;
    plugin->info->pluginName = PLUGIN_libfc14audiodecoder_NAME;
    plugin->info->numUsedPatterns = fc14dec_get_used_patterns(plugin->decoder);
    plugin->info->numSndModSeqs = fc14dec_get_used_snd_mod_seqs(plugin->decoder);
    plugin->info->numVolModSeqs = fc14dec_get_used_vol_mod_seqs(plugin->decoder);

    plugin->info->numSamples = 10;


    plugin->info->samplesSize = new unsigned int[10];
    plugin->info->samplesLoopStart = new unsigned int[10];
    plugin->info->samplesLoopLength = new unsigned int[10];

    for (int j = 1; j <= 10; j++)
    {
        plugin->info->samplesSize[j - 1] = fc14dec_get_sample_length(plugin->decoder, j - 1);
        plugin->info->samplesLoopStart[j - 1] = fc14dec_get_sample_rep_offset(plugin->decoder, j - 1);
        plugin->info->samplesLoopLength[j - 1] = fc14dec_get_sample_rep_length(plugin->decoder, j - 1);
    }

    plugin->info->numChannels = 4;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginLibfc14*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibfc14*>(codec->plugindata);
    unsigned char* vumeters = new unsigned char[16];

    fc14dec_buffer_fill(plugin->decoder, buffer, size << 2);
    *read = size;

    for (int i = 0; i < 4; i++)
    {
        unsigned char newValue = fc14dec_get_channel_volume(plugin->decoder, i);

        vumeters[i] = newValue;
    }
    if (plugin->vumeterBuffer.size() >= 68)
    {
        plugin->vumeterBuffer.pop();
    }

    plugin->vumeterBuffer.push(vumeters);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibfc14*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        fc14dec_seek(plugin->decoder, position);
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < 4; i++)
        {
            int m = position >> i & 1;
            bool mute = m == 1 ? true : false;
            fc14dec_mute_channel(plugin->decoder, mute, i);
        }

        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibfc14*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        plugin->info->modVUMeters = plugin->vumeterBuffer.front();
        //        cout << "get vumeter " <<  ": " << (int)plugin->info->modVUMeters[0] << endl;
        //        cout << "get vumeter " <<  ": " << (int)plugin->info->modVUMeters[1] << endl;
        //        cout << "get vumeter " <<  ": " << (int)plugin->info->modVUMeters[2] << endl;
        //        cout << "get vumeter " <<  ": " << (int)plugin->info->modVUMeters[3] << endl;
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
