#include "FC.h"
#include "fc14audiodecoder.h"
#include <string.h>
#include <iostream>
#include <stdio.h>
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
    0 // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    fcplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
        decoder = NULL;
    }

    ~fcplugin()
    {
        //delete some stuff
        fc14dec_delete(decoder);
        decoder = NULL;
    }

    void* decoder;
    int queueSize;
    Info* info;
    queue<unsigned char*> vumeterBuffer;

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

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    fcplugin* fc = new fcplugin(codec);

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

    fc->info = (Info*)userexinfo->userdata;

    signed short* myBuffer;
    myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    fc->decoder = fc14dec_new();
    int ok = fc14dec_init(fc->decoder, myBuffer, filesize);
    delete[] myBuffer;
    if (!ok)
    {
        return FMOD_ERR_FORMAT;
    }

    fc->fcwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels = 2;
    fc->fcwaveformat.frequency = 44100;
    fc->fcwaveformat.pcmblocksize = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm = fc14dec_duration(fc->decoder) / 1000.0 * fc->fcwaveformat.frequency;

    codec->waveformat = &(fc->fcwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = fc; /* user data value */

    fc14dec_mixer_init(fc->decoder, fc->fcwaveformat.frequency, 16, fc->fcwaveformat.channels, 0);
    if (isSMOD)
    {
        fc->info->fileformat = "Future Composer 1.0-1.3";
    }
    else if (isFC14)
    {
        fc->info->fileformat = "Future Composer 1.4";
    }

    fc->info->plugin = PLUGIN_libfc14audiodecoder;
    fc->info->pluginName = PLUGIN_libfc14audiodecoder_NAME;
    fc->info->numUsedPatterns = fc14dec_get_used_patterns(fc->decoder);
    fc->info->numSndModSeqs = fc14dec_get_used_snd_mod_seqs(fc->decoder);
    fc->info->numVolModSeqs = fc14dec_get_used_vol_mod_seqs(fc->decoder);

    fc->info->numSamples = 10;


    fc->info->samplesSize = new unsigned int[10];
    fc->info->samplesLoopStart = new unsigned int[10];
    fc->info->samplesLoopLength = new unsigned int[10];

    for (int j = 1; j <= 10; j++)
    {
        fc->info->samplesSize[j - 1] = fc14dec_get_sample_length(fc->decoder, j - 1);
        fc->info->samplesLoopStart[j - 1] = fc14dec_get_sample_rep_offset(fc->decoder, j - 1);
        fc->info->samplesLoopLength[j - 1] = fc14dec_get_sample_rep_length(fc->decoder, j - 1);
    }

    fc->info->numChannels = 4;
    fc->info->setSeekable(true);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    unsigned char* vumeters = new unsigned char[16];

    fc14dec_buffer_fill(fc->decoder, buffer, size << 2);
    *read = size;

    for (int i = 0; i < 4; i++)
    {
        unsigned char newValue = fc14dec_get_channel_volume(fc->decoder, i);

        vumeters[i] = newValue;
    }
    if (fc->vumeterBuffer.size() >= 68)
    {
        fc->vumeterBuffer.pop();
    }

    fc->vumeterBuffer.push(vumeters);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MS)
    {
        fc14dec_seek(fc->decoder, position);
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < 4; i++)
        {
            int m = position >> i & 1;
            bool mute = m == 1 ? true : false;
            fc14dec_mute_channel(fc->decoder, mute, i);
        }

        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        fc->info->modVUMeters = fc->vumeterBuffer.front();
        //        cout << "get vumeter " <<  ": " << (int)fc->info->modVUMeters[0] << endl;
        //        cout << "get vumeter " <<  ": " << (int)fc->info->modVUMeters[1] << endl;
        //        cout << "get vumeter " <<  ": " << (int)fc->info->modVUMeters[2] << endl;
        //        cout << "get vumeter " <<  ": " << (int)fc->info->modVUMeters[3] << endl;
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
