#include <fstream>
#include <queue>
#include "engine.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_furnace_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODVUMETER,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    &getPosition,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

static constexpr uint32_t maxSamples = 2048;

class pluginFurnace {
    FMOD_CODEC_STATE *_codec;

public:
    pluginFurnace(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginFurnace() {
        // delete some stuff
        if (engine != nullptr) {
            engine->quit(false);
        }

        delete engine;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    DivEngine *engine = nullptr;
    float samples[2][maxSamples];
    int32_t lastLoopPos = -1;
    uint32_t numRemainingSamples = 0;
    queue<unsigned char *> vuMeterBuffer;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin. This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C! C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    initLog(stdout);

    const auto plugin = new pluginFurnace(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->engine = new DivEngine;
    plugin->engine->preInit();

    // this is deleted by furnace when needed
    const auto fileBufferCopy = new uint8_t[plugin->info->filesize];

    memcpy(fileBufferCopy, plugin->info->fileBuffer, sizeof(uint8_t) * plugin->info->filesize);

    if (!plugin->engine->load(fileBufferCopy, plugin->info->filesize) || !plugin->engine->init()) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/furnace.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "continuousPlayback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    plugin->engine->initDispatch(false);
    plugin->engine->changeSongP(plugin->info->currentSubsong);
    plugin->engine->setLoops(plugin->info->isContinuousPlaybackActive ? -1 : 1);
    plugin->engine->play();

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = static_cast<int>(plugin->engine->getAudioDescGot().rate);
    plugin->waveformat.pcmblocksize = maxSamples;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    const auto song = plugin->engine->song;
    const auto subsong = *song.subsong[plugin->info->currentSubsong];

    plugin->info->fileFormat = fmt::format("{} v{}", song.isDMF ? "DefleMask" : "Furnace", song.version);
    plugin->info->numSubsongs = static_cast<int>(song.subsong.size());
    plugin->info->title = song.name;

    if (any_of(subsong.name.begin(), subsong.name.end(), [](const unsigned char c) { return !isspace(c); })) {
        if (!song.name.empty()) {
            plugin->info->title += " / ";
        }

        plugin->info->title += subsong.name;
    }

    plugin->info->artist = song.author;
    plugin->info->album = song.category;
    plugin->info->system = song.systemName;
    plugin->info->comments = song.notes;

    if (!subsong.notes.empty()) {
        if (!song.notes.empty()) {
            plugin->info->comments += "\n\n";
        }

        plugin->info->comments += subsong.notes;
    }

    plugin->info->numChannels = plugin->engine->getTotalChannelCount();
    plugin->info->plugin = PLUGIN_furnace;
    plugin->info->pluginName = PLUGIN_furnace_NAME;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginFurnace *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto plugin = static_cast<pluginFurnace *>(codec->plugindata);

    auto bufferPtr = static_cast<float *>(buffer);
    unsigned int numSamples = size;

    float *samples[] = {plugin->samples[0], plugin->samples[1]};
    uint32_t numSamplesRendered = 0;
    uint32_t numRemainingSamples = plugin->numRemainingSamples;

    while (numSamples) {
        if (numRemainingSamples == 0) {
            plugin->engine->nextBuf(nullptr, samples, 0, 2, maxSamples);
            numRemainingSamples = maxSamples;
            plugin->lastLoopPos = plugin->engine->lastLoopPos;
        }

        auto numSamplesAvailable = numRemainingSamples;

        if (plugin->lastLoopPos > -1) {
            numSamplesAvailable = plugin->lastLoopPos - (maxSamples - numRemainingSamples);

            if (numSamplesAvailable == 0) {
                if (numSamplesRendered == 0) {
                    plugin->lastLoopPos = -1;
                }

                break;
            }
        }

        const auto numSamplesToCopy = min(numSamples, numSamplesAvailable);

        for (uint32_t i = 0; i < numSamplesToCopy; i++) {
            *bufferPtr++ = samples[0][maxSamples - numRemainingSamples + i];
            *bufferPtr++ = samples[1][maxSamples - numRemainingSamples + i];
        }

        numSamples -= numSamplesToCopy;
        numRemainingSamples -= numSamplesToCopy;
        numSamplesRendered += numSamplesToCopy;
    }

    plugin->numRemainingSamples = numRemainingSamples;
    *read = numSamplesRendered;

    if (!plugin->engine->isPlaying() && !plugin->info->isContinuousPlaybackActive) {
        return FMOD_ERR_FILE_EOF;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto plugin = static_cast<pluginFurnace *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        // position is a mask
        for (int i = 0; i < plugin->engine->getTotalChannelCount(); i++) {
            const int m = position >> i & 1;
            const bool mute = m == 1;
            plugin->engine->muteChannel(i, mute);
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = -1;
        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto plugin = static_cast<pluginFurnace *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODVUMETER) {
        auto *vuMeters = new unsigned char[plugin->engine->getTotalChannelCount()];

        for (int i = 0; i < plugin->engine->getTotalChannelCount(); i++) {
            const auto *divDispatchOscBuffer = plugin->engine->getOscBuffer(i);
            int displaySize = 0;

            if (divDispatchOscBuffer != nullptr) {
                displaySize = static_cast<float>(divDispatchOscBuffer->rate) * 0.03f;
            }

            short minLevel = 32767;
            short maxLevel = -32768;
            unsigned short needlePos = 0;

            if (divDispatchOscBuffer != nullptr) {
                needlePos = divDispatchOscBuffer->needle;
            }

            needlePos -= displaySize;

            constexpr int bufferSize = 64;

            for (unsigned short j = 0; j < bufferSize; j++) {
                short y = 0;

                if (divDispatchOscBuffer != nullptr) {
                    y = plugin->engine->getOscBuffer(i)->data[static_cast<unsigned short>(
                        needlePos + j * displaySize / bufferSize)];
                }

                if (minLevel > y) minLevel = y;
                if (maxLevel < y) maxLevel = y;
            }

            float estimate = pow(static_cast<float>(maxLevel - minLevel) / 32768.0f, 0.5f);

            if (estimate > 1.0f) estimate = 1.0f;

            estimate = estimate * 100;

            if (!plugin->vuMeterBuffer.empty()) {
                vuMeters[i] = max(plugin->vuMeterBuffer.back()[i] * 0.87f, estimate);
            } else {
                vuMeters[i] = estimate;
            }
        }

        plugin->vuMeterBuffer.push(vuMeters);

        if (plugin->vuMeterBuffer.size() >= 10) {
            plugin->vuMeterBuffer.pop();
        }

        plugin->info->modVuMeters = plugin->vuMeterBuffer.front();
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
