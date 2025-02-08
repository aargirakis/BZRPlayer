#include <cmath>
#include <queue>
#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "engine.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_furnace_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_MODVUMETER,
    // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    &getposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

static constexpr uint32_t kMaxSamples = 2048;

class ahxplugin
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    ahxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
    {
        //delete some stuff
        m_engine->quit(false);
        delete m_engine;
    }


    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    Info* info;
    DivEngine* m_engine;
    float m_samples[2][kMaxSamples];
    uint64_t m_position = 0;
    int32_t m_lastLoopPos = -1;
    uint32_t m_numRemainingSamples = 0;
    queue<unsigned char*> vumeterBuffer;
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


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    ahxplugin* ahx = new ahxplugin(codec);

    ahx->info = (Info*)userexinfo->userdata;

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    uint8_t* buffer;
    buffer = new uint8_t[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, 4, &bytesread);


    if ((buffer[0] != '-' || buffer[1] != 'F') //Doesn't start with "-F"
        && (buffer[0] != '.' || buffer[1] != 'D') //Doesn't start with ".D"
        && (buffer[0] != 0x78 || buffer[1] != 0x01) //Not zlib no compression
        && (buffer[0] != 0x78 || buffer[1] != 0x5e) //Not zlib fast compression
        && (buffer[0] != 0x78 || buffer[1] != 0x9c) //Not zlib default compression
        && (buffer[0] != 0x78 || buffer[1] != 0xda) //Not zlib best compression

    ) //it's not a Furnace file
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, filesize, &bytesread);

    initLog(stdout);

    ahx->m_engine = new DivEngine;
    ahx->m_engine->preInit();
    if (ahx->m_engine->load(buffer, filesize) && ahx->m_engine->init())
    {
    }
    else
    {
        ahx->m_engine->quit(false);

        delete ahx->m_engine;
        return FMOD_ERR_FORMAT;
    }


    int freq = ahx->m_engine->getAudioDescGot().rate;
    int channels = 2;


    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    ahx->ahxwaveformat.channels = channels;
    ahx->ahxwaveformat.frequency = freq;
    ahx->ahxwaveformat.pcmblocksize = kMaxSamples;
    ahx->ahxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */

    DivSong song = ahx->m_engine->song;
    char ver[16];

    string format;
    if (song.version == DIV_VERSION_FTM)
    {
        format = "Famitracker";
        ahx->info->fileformat = format;
    }
    else
    {
        format = song.isDMF ? "DefleMask v" : "Furnace v";
        sprintf(ver, "%d", song.version);
        ahx->info->fileformat = format + string(ver);
    }
    ahx->info->artist = song.author;
    ahx->info->title = song.name;
    ahx->info->system = song.systemName;
    ahx->info->numChannels = ahx->m_engine->getTotalChannelCount();
    ahx->info->plugin = PLUGIN_furnace;
    ahx->info->pluginName = PLUGIN_furnace_NAME;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    delete ahx;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    float* bufferPtr = (float*)buffer; // Pointer to the buffer
    unsigned int numSamples = size;

    float* samples[] = {ahx->m_samples[0], ahx->m_samples[1]};
    uint32_t numSamplesRendered = 0;
    uint32_t numRemainingSamples = ahx->m_numRemainingSamples;
    while (numSamples)
    {
        if (numRemainingSamples == 0)
        {
            ahx->m_engine->nextBuf(nullptr, samples, 0, 2, kMaxSamples);
            numRemainingSamples = kMaxSamples;
            ahx->m_lastLoopPos = ahx->m_engine->lastLoopPos;
        }
        auto numSamplesAvailable = numRemainingSamples;
        if (ahx->m_lastLoopPos > -1)
        {
            numSamplesAvailable = ahx->m_lastLoopPos - (kMaxSamples - numRemainingSamples);
            if (numSamplesAvailable == 0)
            {
                if (numSamplesRendered == 0)
                    ahx->m_lastLoopPos = -1;
                break;
            }
        }
        auto numSamplesToCopy = min(numSamples, numSamplesAvailable);
        for (uint32_t i = 0; i < numSamplesToCopy; i++)
        {
            // Copy the samples to buffer
            *bufferPtr++ = samples[0][i];
            *bufferPtr++ = samples[1][i];
        }
        numSamples -= numSamplesToCopy;
        numRemainingSamples -= numSamplesToCopy;
        numSamplesRendered += numSamplesToCopy;

        unsigned char* vumeters = new unsigned char[ahx->m_engine->getTotalChannelCount()];
        for (int i = 0; i < ahx->m_engine->getTotalChannelCount(); i++)
        {
            DivDispatchOscBuffer* divDispatchOscBuffer = ahx->m_engine->getOscBuffer(i);
            int rate = 0;
            int displaySize = 0;
            if (divDispatchOscBuffer != NULL)
            {
                displaySize = (float)(divDispatchOscBuffer->rate) * 0.03f;
            }

            short minLevel = 32767;
            short maxLevel = -32768;
            unsigned short needlePos = 0;
            if (divDispatchOscBuffer != NULL)
            {
                needlePos = divDispatchOscBuffer->needle;
            }

            needlePos -= displaySize;

            int bufsize = 64;

            for (unsigned short j = 0; j < bufsize; j++)
            {
                short y = 0;
                if (divDispatchOscBuffer != NULL)
                {
                    y = ahx->m_engine->getOscBuffer(i)->data[(unsigned short)(needlePos + (j * displaySize / bufsize))];
                }
                if (minLevel > y) minLevel = y;
                if (maxLevel < y) maxLevel = y;
            }

            float estimate = pow((float)(maxLevel - minLevel) / 32768.0f, 0.5f);
            if (estimate > 1.0f) estimate = 1.0f;
            estimate = estimate * 100;

            if (ahx->vumeterBuffer.size() > 0)
            {
                vumeters[i] = (MAX(ahx->vumeterBuffer.back()[i]*0.87f, estimate));
            }
            else
            {
                vumeters[i] = estimate;
            }
        }

        ahx->vumeterBuffer.push(vumeters);
        if (ahx->vumeterBuffer.size() >= 10)
        {
            ahx->vumeterBuffer.pop();
        }
    }


    ahx->m_numRemainingSamples = numRemainingSamples;
    *read = numSamplesRendered;
    if (ahx->m_engine->endOfSong)
    {
        return FMOD_ERR_FILE_ENDOFDATA;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MS)
    {
        ahx->m_engine->initDispatch();
        ahx->m_engine->renderSamplesP();
        ahx->m_engine->play();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        ahx->m_engine->changeSongP(position);
        ahx->m_engine->play();
        ahx->m_position = 0;
        ahx->m_lastLoopPos = -1;
        ahx->m_numRemainingSamples = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < ahx->m_engine->getTotalChannelCount(); i++)
        {
            int m = position >> i & 1;
            bool mute = m == 1 ? true : false;
            ahx->m_engine->muteChannel(i, mute);
        }
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    *length = 0xffffffff;
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        ahx->info->modVUMeters = ahx->vumeterBuffer.front();
        return FMOD_OK;
    }
    return FMOD_OK;
}
