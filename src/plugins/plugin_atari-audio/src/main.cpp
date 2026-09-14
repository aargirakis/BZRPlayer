#include <cstring>
#include <fstream>
#include "AtariAudioRenderer.h"
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
    PLUGIN_atari_audio_NAME, // name.
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

class pluginAtariAudio {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAtariAudio(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAtariAudio() {
        AtariAudioRenderer::Destroy(atariAudio);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    AtariAudioRenderer *atariAudio;
    static constexpr unsigned int sampleRate = 48000;
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
    logDebug("Try", PLUGIN_atari_audio_NAME);

    auto *plugin = new pluginAtariAudio(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->atariAudio = AtariAudioRenderer::Create(plugin->info->fileBuffer,
                                                    static_cast<uint32_t>(plugin->info->filesize),
                                                    pluginAtariAudio::sampleRate);

    if (!plugin->atariAudio) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (!plugin->atariAudio->InitSubSong(plugin->info->currentSubsong + 1)) {
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
    plugin->waveformat.frequency = pluginAtariAudio::sampleRate;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    auto songInfo = plugin->atariAudio->GetSongInfo();

    plugin->info->title = songInfo.musicName;
    plugin->info->date = songInfo.year;
    plugin->info->clockSpeed = songInfo.playerTickRate;

    if (songInfo.fileType == AtariAudioRenderer::eFileType::eSndh) {
        plugin->info->defaultSubsong = songInfo.subsongCount > 1 ? songInfo.defaultSubsong : -1;
        plugin->info->composer = songInfo.musicAuthor;
        plugin->info->ripper = songInfo.ripper;
        plugin->info->converter = songInfo.converter;
        plugin->info->fileFormatSpecific = "SNDH";
    } else {
        plugin->info->artist = songInfo.musicAuthor;
        plugin->info->comments = songInfo.converter;
        plugin->info->fileFormatSpecific = "YM";
    }

    switch (songInfo.ym2149Clock) {
        case 1000000:
            plugin->info->system = "Amstrad CPC / Oric";
            break;
        case 1228800:
            plugin->info->system = "FM-7";
            break;
        case 1500000:
            plugin->info->system = "Vectrex";
            break;
        case 1750000:
            plugin->info->system = "Pentagon";
            break;
        case 1764000:
            plugin->info->system = "T/S 2068";
            break;
        case 1773400:
        case 1773447:
        case 1773450:
            plugin->info->system = "ZX Spectrum";
            break;
        case 1789772:
        case 1789773:
            plugin->info->system = "MSX";
            break;
        case 2000000:
            plugin->info->system = "Atari ST / Sharp X1";
            break;
        case 3500000:
            plugin->info->system = "Taganrog";
            break;
        default:
            plugin->info->system = "Unknown";
    }

    plugin->info->fileFormat = songInfo.fileFormat;
    plugin->info->numSubsongs = songInfo.subsongCount;
    plugin->info->numChannels = 4;
    plugin->info->plugin = PLUGIN_atari_audio;
    plugin->info->pluginName = PLUGIN_atari_audio_NAME;

    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginAtariAudio *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    if (auto *plugin = static_cast<pluginAtariAudio *>(codec->plugindata);
        plugin->isSeeking) {
        if (plugin->renderingPosition < plugin->seekPosition) {
            uint32_t toSkip = plugin->seekPosition - plugin->renderingPosition;

            if (toSkip > 32768) {
                toSkip = 32768;
                memset(buffer, 0, size * plugin->waveformat.pcmblocksize);
                *read = size;
            }

            plugin->atariAudio->FastForward(toSkip);
            plugin->renderingPosition += toSkip;
        } else {
            plugin->isSeeking = false;
            *read = 0;
        }
    } else {
        plugin->atariAudio->AudioRender(static_cast<int16_t *>(buffer), size);
        plugin->renderingPosition += size;
        *read = size;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginAtariAudio *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->atariAudio->SampleToMs(
            plugin->atariAudio->GetSubsongDurationSample(plugin->info->currentSubsong + 1));
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
    auto *plugin = static_cast<pluginAtariAudio *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        bool shouldReinit = false;

        if (position == 0) {
            shouldReinit = plugin->renderingPosition != 0;
        } else {
            plugin->seekPosition = plugin->atariAudio->MsToSample(position);
            shouldReinit = plugin->seekPosition < plugin->renderingPosition;
            plugin->isSeeking = true;
        }

        if (shouldReinit) {
            plugin->atariAudio->InitSubSong(plugin->info->currentSubsong + 1);
            plugin->renderingPosition = 0;
        }

        return FMOD_OK;
    }

    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        plugin->atariAudio->MuteVoices(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginAtariAudio *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS_REAL) {
        *position = plugin->atariAudio->SampleToMs(plugin->renderingPosition);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
