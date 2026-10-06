#include <cstring>
#include <fstream>
#include "mdxmini.h"
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

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_mdxmini_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginMdxmini {
    FMOD_CODEC_STATE *_codec;

public:
    pluginMdxmini(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginMdxmini() {
        // delete some stuff
    }

    static uint32_t msToSamples(const uint32_t ms) {
        const uint64_t samples = static_cast<uint64_t>(ms) * sampleRate * channels / 1000;
        return static_cast<uint32_t>(samples);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    t_mdxmini data;
    Info *info;
    unsigned int length;
    static constexpr unsigned int sampleRate = 44100;
    static constexpr unsigned int channels = 2;
    uint32_t renderingPosition = 0;
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
    logDebug("Try", PLUGIN_mdxmini_NAME);

    auto *plugin = new pluginMdxmini(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string filename = plugin->info->userPath + PLUGIN_CONFIGS_DIR "/" CONFIG_FILENAME;
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    bool isCustomPdxDirEnabled;
    string customPdxDirPath;
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            auto i = line.find_first_of('=');
            if (i == string::npos) continue;

            string word = line.substr(0, i);
            string value = line.substr(i + 1);

            if (word == "mdxminiCustomPdxDirEnabled") {
                if (value == "true") {
                    isCustomPdxDirEnabled = true;
                } else {
                    isCustomPdxDirEnabled = false;
                }
            } else if (word == "mdxminiCustomPdxDirPath") {
                customPdxDirPath = value;
            } else if (word == "continuousPlayback") {
                plugin->info->isContinuousPlaybackActive = plugin->info->isPlayModeRepeatSongEnabled && value == "true";
            }
        }

        ifs.close();
    }

    if (const int success = mdx_open(&plugin->data, &plugin->info->filePath[0], &plugin->info->fileBuffer[0],
                                     static_cast<int>(plugin->info->filesize),
                                     isCustomPdxDirEnabled ? customPdxDirPath.data() : nullptr);
        success < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    mdx_set_max_loop(&plugin->data, 1);
    plugin->length = mdx_get_length(&plugin->data) * 1000;

    if (plugin->info->isContinuousPlaybackActive) {
        plugin->data.mdx->fade_out_speed = 0;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = static_cast<int>(pluginMdxmini::channels);
    plugin->waveformat.frequency = static_cast<int>(pluginMdxmini::sampleRate);
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->numChannels = mdx_get_tracks(&plugin->data);
    mdx_set_rate(pluginMdxmini::sampleRate);
    char title[MDX_MAX_TITLE_LENGTH];

    mdx_get_title(&plugin->data, title);
    plugin->info->title = title;

    if (plugin->data.mdx->pdx_name[0] == NULL) {
        plugin->info->sampleFile = "-";
    } else {
        plugin->info->sampleFile = plugin->data.mdx->pdx_name;

        if (plugin->data.mdx->haspdx == FLAG_FALSE) {
            plugin->info->sampleFile += " (MISSING)";
        }
    }

    plugin->info->fileFormat = "MDX";
    plugin->info->plugin = PLUGIN_mdxmini;
    plugin->info->pluginName = PLUGIN_mdxmini_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    const auto plugin = static_cast<pluginMdxmini *>(codec->plugindata);

    if (plugin != nullptr) {
        mdx_close(&plugin->data);
    }

    delete plugin;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    static constexpr unsigned int maxSamples = 256;

    const auto plugin = static_cast<pluginMdxmini *>(codec->plugindata);

    if (!mdx_calc_sample(&plugin->data, static_cast<short *>(buffer), maxSamples) &&
        !plugin->info->isContinuousPlaybackActive) {
        return FMOD_ERR_FILE_EOF;
    }

    plugin->renderingPosition += maxSamples;
    *read = maxSamples;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginMdxmini *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->length;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginMdxmini *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        bool shouldReinit = false;
        bool shouldSeek = false;
        uint32_t seekPosition = 0;

        if (position == 0) {
            shouldReinit = plugin->renderingPosition != 0;
        } else {
            seekPosition = pluginMdxmini::msToSamples(position) & ~1;
            shouldReinit = seekPosition < plugin->renderingPosition;
            shouldSeek = true;
        }

        if (shouldReinit) {
            // quick way to reinit
            mdx_parse_mml_ym2151_async_get_length(plugin->data.songdata);

            plugin->renderingPosition = 0;
        }

        if (shouldSeek) {
            const uint32_t samplesToSkip = seekPosition - plugin->renderingPosition;

            mdx_calc_log(&plugin->data, nullptr, static_cast<int>(samplesToSkip / pluginMdxmini::channels));
            plugin->renderingPosition += samplesToSkip;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
