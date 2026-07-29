#include <cstring>
#include "mdxmini.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

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
    nullptr,
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

    t_mdxmini data;
    Info *info;

    FMOD_CODEC_WAVEFORMAT waveformat;
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
    auto *plugin = new pluginMdxmini(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    if (const int success = mdx_open(&plugin->data, &plugin->info->filePath[0], &plugin->info->fileDir[0]);
        success < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = mdx_get_length(&plugin->data) * plugin->waveformat.frequency;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->numChannels = mdx_get_tracks(&plugin->data);
    mdx_set_rate(plugin->waveformat.frequency);
    char title[MDX_MAX_TITLE_LENGTH];

    mdx_get_title(&plugin->data, title);
    plugin->info->title = title;
    plugin->info->fileFormat = "MDX";
    plugin->info->plugin = PLUGIN_mdxmini;
    plugin->info->pluginName = PLUGIN_mdxmini_NAME;
    plugin->info->setSeekable(false);

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

    if (const auto plugin = static_cast<pluginMdxmini *>(codec->plugindata);
        !mdx_calc_sample(&plugin->data, static_cast<short *>(buffer), maxSamples)) {
        return FMOD_ERR_FILE_EOF;
    }

    *read = maxSamples;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
