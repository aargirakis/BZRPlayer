#include <queue>
#include "engine.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

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
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    &getPosition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

static constexpr uint32_t maxSamples = 2048;

class pluginFurnace
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginFurnace(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginFurnace()
    {
        //delete some stuff
        if (m_engine != nullptr) {
            m_engine->quit(false);
        }
        delete m_engine;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info* info;
    DivEngine* m_engine = nullptr;
    float m_samples[2][maxSamples];
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

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    auto *buffer = new uint8_t[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, 4, &bytesread);

    // only furnace, deflemask and zlib (with no/fast/default/best compression) are allowed here
    if (memcmp(buffer, "-F'", 2) != 0 && memcmp(buffer, ".D", 2) != 0 &&
    memcmp(buffer, "\x78\x01", 2) != 0 && memcmp(buffer, "\x78\x5e", 2) != 0 &&
    memcmp(buffer, "\x78\x9c", 2) != 0 && memcmp(buffer, "\x78\xda", 2) != 0)
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, filesize, &bytesread);

    initLog(stdout);

    const auto plugin = new pluginFurnace(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->m_engine = new DivEngine;
    plugin->m_engine->preInit();
    if (!plugin->m_engine->load(buffer, filesize) || !plugin->m_engine->init())
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = static_cast<int>(plugin->m_engine->getAudioDescGot().rate);
    plugin->waveformat.pcmblocksize = maxSamples;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    const DivSong song = plugin->m_engine->song;
    char ver[16];

    string format;
    if (song.version == DIV_VERSION_FTM)
    {
        format = "Famitracker";
        plugin->info->fileformat = format;
    }
    else
    {
        format = song.isDMF ? "DefleMask v" : "Furnace v";
        sprintf(ver, "%d", song.version);
        plugin->info->fileformat = format + string(ver);
    }

    plugin->info->artist = song.author;
    plugin->info->title = song.name;
    plugin->info->system = song.systemName;
    plugin->info->numChannels = plugin->m_engine->getTotalChannelCount();
    plugin->info->plugin = PLUGIN_furnace;
    plugin->info->pluginName = PLUGIN_furnace_NAME;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginFurnace*>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    const auto plugin = static_cast<pluginFurnace*>(codec->plugindata);

    auto bufferPtr = static_cast<float*>(buffer); // Pointer to the buffer
    unsigned int numSamples = size;

    float* samples[] = {plugin->m_samples[0], plugin->m_samples[1]};
    uint32_t numSamplesRendered = 0;
    uint32_t numRemainingSamples = plugin->m_numRemainingSamples;
    while (numSamples)
    {
        if (numRemainingSamples == 0)
        {
            plugin->m_engine->nextBuf(nullptr, samples, 0, 2, maxSamples);
            numRemainingSamples = maxSamples;
            plugin->m_lastLoopPos = plugin->m_engine->lastLoopPos;
        }
        auto numSamplesAvailable = numRemainingSamples;
        if (plugin->m_lastLoopPos > -1)
        {
            numSamplesAvailable = plugin->m_lastLoopPos - (maxSamples - numRemainingSamples);
            if (numSamplesAvailable == 0)
            {
                if (numSamplesRendered == 0) {
                    plugin->m_lastLoopPos = -1;
                }
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

        auto* vumeters = new unsigned char[plugin->m_engine->getTotalChannelCount()];
        for (int i = 0; i < plugin->m_engine->getTotalChannelCount(); i++)
        {
            const auto* divDispatchOscBuffer = plugin->m_engine->getOscBuffer(i);
            int displaySize = 0;
            if (divDispatchOscBuffer != nullptr)
            {
                displaySize = static_cast<float>(divDispatchOscBuffer->rate) * 0.03f;
            }

            short minLevel = 32767;
            short maxLevel = -32768;
            unsigned short needlePos = 0;
            if (divDispatchOscBuffer != nullptr)
            {
                needlePos = divDispatchOscBuffer->needle;
            }

            needlePos -= displaySize;

            int bufsize = 64;

            for (unsigned short j = 0; j < bufsize; j++)
            {
                short y = 0;
                if (divDispatchOscBuffer != nullptr)
                {
                    y = plugin->m_engine->getOscBuffer(i)->data[static_cast<unsigned short>(needlePos + (j * displaySize / bufsize))];
                }
                if (minLevel > y) minLevel = y;
                if (maxLevel < y) maxLevel = y;
            }

            float estimate = pow(static_cast<float>(maxLevel - minLevel) / 32768.0f, 0.5f);
            if (estimate > 1.0f) estimate = 1.0f;
            estimate = estimate * 100;

            if (!plugin->vumeterBuffer.empty())
            {
                vumeters[i] = (MAX(plugin->vumeterBuffer.back()[i]*0.87f, estimate));
            }
            else
            {
                vumeters[i] = estimate;
            }
        }

        plugin->vumeterBuffer.push(vumeters);
        if (plugin->vumeterBuffer.size() >= 10)
        {
            plugin->vumeterBuffer.pop();
        }
    }

    plugin->m_numRemainingSamples = numRemainingSamples;
    *read = numSamplesRendered;
    if (plugin->m_engine->endOfSong)
    {
        return FMOD_ERR_FILE_ENDOFDATA;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    const auto plugin = static_cast<pluginFurnace*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        plugin->m_engine->initDispatch();
        plugin->m_engine->renderSamplesP();
        plugin->m_engine->play();
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->m_engine->changeSongP(position);
        plugin->m_engine->play();
        plugin->m_position = 0;
        plugin->m_lastLoopPos = -1;
        plugin->m_numRemainingSamples = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < plugin->m_engine->getTotalChannelCount(); i++)
        {
            int m = position >> i & 1;
            bool mute = m == 1 ? true : false;
            plugin->m_engine->muteChannel(i, mute);
        }
    }
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    *length = -1;
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    const auto plugin = static_cast<pluginFurnace*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        plugin->info->modVUMeters = plugin->vumeterBuffer.front();
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
