#include <cstring>
#include <fstream>
#include "SndhRenderer.h"
#include "fmod_errors.h"
#include "info.h"
#include "logger.h"
#include "plugins.h"

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
    PLUGIN_atari_audio_library_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL | FMOD_TIMEUNIT_MUTE_VOICE,
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

static constexpr unsigned int sampleRate = 44100;

class pluginAtariAudioLibrary {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAtariAudioLibrary(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAtariAudioLibrary() {
        SndhRenderer::Destroy(sndh);
    }

    static uint32_t samplesToMs(const uint32_t samples) {
        const uint64_t ms = static_cast<uint64_t>(samples) * 1000 / sampleRate;
        return static_cast<uint32_t>(ms);
    }

    static uint32_t msToSamples(const uint32_t ms) {
        const uint64_t samples = static_cast<uint64_t>(ms) * sampleRate / 1000;
        return static_cast<uint32_t>(samples);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    SndhRenderer *sndh;
    uint32_t renderingPosition = 0;
    uint32_t seekPosition;
    bool isSeeking = false;
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
    logDebug("Try", PLUGIN_atari_audio_library_NAME);

    auto *plugin = new pluginAtariAudioLibrary(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->sndh = SndhRenderer::Create(plugin->info->fileBuffer, static_cast<uint32_t>(plugin->info->filesize),
                                        sampleRate);

    if (!plugin->sndh) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (!plugin->sndh->InitSubSong(plugin->info->currentSubsong + 1)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGIN_CONFIGS_DIR "/" CONFIG_FILENAME;
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

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 1;
    plugin->waveformat.frequency = sampleRate;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    auto songInfo = plugin->sndh->GetSongInfo();

    plugin->info->artist = songInfo.musicAuthor;
    plugin->info->title = songInfo.musicName;
    plugin->info->ripper = songInfo.ripper;
    plugin->info->converter = songInfo.converter;
    plugin->info->date = songInfo.year;
    plugin->info->clockSpeed = songInfo.playerTickRate;
    plugin->info->numSubsongs = songInfo.subsongCount;
    plugin->info->numChannels = 4;
    plugin->info->plugin = PLUGIN_atari_audio_library;
    plugin->info->pluginName = PLUGIN_atari_audio_library_NAME;
    plugin->info->fileFormat = "SNDH";

    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginAtariAudioLibrary *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    if (auto *plugin = static_cast<pluginAtariAudioLibrary *>(codec->plugindata);
        plugin->isSeeking) {
        if (plugin->renderingPosition < plugin->seekPosition) {
            uint32_t toSkip = plugin->seekPosition - plugin->renderingPosition;

            if (toSkip > 32768) {
                toSkip = 32768;
                memset(buffer, 0, size * plugin->waveformat.pcmblocksize);
                *read = size;
            }

            plugin->sndh->AudioRender(nullptr, toSkip);
            plugin->renderingPosition += toSkip;
        } else {
            plugin->isSeeking = false;
            *read = 0;
        }
    } else {
        plugin->sndh->AudioRender(static_cast<int16_t *>(buffer), size);
        plugin->renderingPosition += size;
        *read = size;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginAtariAudioLibrary *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->sndh->GetSubsongDurationMs(plugin->info->currentSubsong + 1);
        return FMOD_OK;
    }

    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginAtariAudioLibrary *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        bool shouldReinit = false;

        if (position == 0) {
            shouldReinit = plugin->renderingPosition != 0;
        } else {
            plugin->seekPosition = pluginAtariAudioLibrary::msToSamples(position);
            shouldReinit = plugin->seekPosition < plugin->renderingPosition;
            plugin->isSeeking = true;
        }

        if (shouldReinit) {
            plugin->sndh->InitSubSong(plugin->info->currentSubsong + 1);
            plugin->renderingPosition = 0;
        }

        return FMOD_OK;
    }

    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        plugin->sndh->MuteVoices(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginAtariAudioLibrary *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS_REAL) {
        *position = pluginAtariAudioLibrary::samplesToMs(plugin->renderingPosition);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
