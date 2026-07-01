#include <cstring>
#include "kssplay.h"
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
    PLUGIN_libkss_NAME, // name.
    0x00012300, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
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

class pluginLibkss {
    FMOD_CODEC_STATE *_codec;

public:
    pluginLibkss(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibkss() {
        // delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;

    Info *info;
    KSS *kss = nullptr;
    KSSPLAY *kssplay = nullptr;
    int loopNum;
};

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
    auto *plugin = new pluginLibkss(codec);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 48000;
    plugin->waveformat.pcmblocksize = 2048 * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info = static_cast<Info *>(userexinfo->userdata);

    const auto typeInfo = KSS_check_type(plugin->info->fileBuffer, static_cast<uint32_t>(plugin->info->filesize),
                                         plugin->info->filePath.c_str());

    if (typeInfo.type == MBMDATA) {
        /* TODO:
         *  seems the KSS_autoload_mbk invocation introduces a beginning audio delay
         *  somewhere in the code not in this function
         */
        KSS_autoload_mbk(plugin->info->filePath.c_str(), "", nullptr);
    }

    plugin->kss = KSS_bin2kss(plugin->info->fileBuffer, static_cast<uint32_t>(plugin->info->filesize),
                              plugin->info->filePath.c_str());

    if (plugin->kss == nullptr) {
        return FMOD_ERR_FORMAT;
    }

    plugin->kssplay = KSSPLAY_new(plugin->waveformat.frequency, plugin->waveformat.channels, 16);
    KSSPLAY_set_data(plugin->kssplay, plugin->kss);

    uint8_t subsongOffset = 0;

    if (typeInfo.type == KSSDATA) {
        plugin->info->numSubsongs = plugin->kss->trk_max - plugin->kss->trk_min + 1;
        subsongOffset = plugin->kss->trk_min;
    }

    KSSPLAY_reset(plugin->kssplay, plugin->info->currentSubsong + subsongOffset, 0);

    // TODO setting to 0 for continuous playback, however it results silent playback for some tracks (eg REQUIEM2.MGS)
    plugin->loopNum = 1;

    constexpr uint32_t quality = 1;

    KSSPLAY_set_device_quality(plugin->kssplay, KSS_DEVICE_PSG, quality);
    KSSPLAY_set_device_quality(plugin->kssplay, KSS_DEVICE_SCC, quality);
    KSSPLAY_set_device_quality(plugin->kssplay, KSS_DEVICE_OPLL, quality);
    KSSPLAY_set_device_quality(plugin->kssplay, KSS_DEVICE_OPL, quality);

    KSSPLAY_set_master_volume(plugin->kssplay, 32);

    if (plugin->waveformat.channels > 1) {
        KSSPLAY_set_device_pan(plugin->kssplay, KSS_DEVICE_PSG, -32);
        KSSPLAY_set_device_pan(plugin->kssplay, KSS_DEVICE_SCC, 32);

        plugin->kssplay->opll_stereo = 1;
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 0, 1);
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 1, 2);
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 2, 1);
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 3, 2);
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 4, 1);
        KSSPLAY_set_channel_pan(plugin->kssplay, KSS_DEVICE_OPLL, 5, 2);
    }

    plugin->kssplay->silent_limit = 5000;

    plugin->info->title = KSS_get_title(plugin->kss);
    plugin->info->fileFormat = reinterpret_cast<char *>(plugin->kss->idstr);

    if (plugin->kss->extra) {
        plugin->info->comments = reinterpret_cast<char *>(plugin->kss->extra);
    }

    plugin->info->useShiftJis = true;
    plugin->info->plugin = PLUGIN_libkss;
    plugin->info->pluginName = PLUGIN_libkss_NAME;

    plugin->info->setSeekable(false);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    const auto *plugin = static_cast<pluginLibkss *>(codec->plugindata);

    if (plugin != nullptr) {
        KSSPLAY_delete(plugin->kssplay);
        KSS_delete(plugin->kss);
    }

    delete plugin;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginLibkss *>(codec->plugindata);

    // TODO
    // plugin->kss->loop_detectable;

    // TODO need to check stop flag only if loopcount has been reached?
    // TODO should be checked after KSSPLAY_calc?
    if (KSSPLAY_get_stop_flag(plugin->kssplay)) {
        return FMOD_ERR_FILE_EOF;
    }

    // TODO
    if (KSSPLAY_get_loop_count(plugin->kssplay) >= plugin->loopNum) {
        return FMOD_ERR_FILE_EOF;
    }

    KSSPLAY_calc(plugin->kssplay, static_cast<int16_t *>(buffer), plugin->waveformat.pcmblocksize);

    // TODO *read = plugin->waveformat.pcmblocksize * sizeof(int16_t) * plugin->waveformat.channels;
    *read = plugin->waveformat.pcmblocksize;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
