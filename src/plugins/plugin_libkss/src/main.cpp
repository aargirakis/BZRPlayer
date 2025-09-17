#include <cstring>
#include <format>
#include "kss.h"
#include "kssplay.h"
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
    PLUGIN_libkss_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLibkss
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibkss(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibkss()
    {
        //delete some stuff
        delete[] myBuffer;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;

    Info* info;
    KSS *kss = nullptr;
    KSSPLAY *kssplay = nullptr;
    int currentSubsong;
    uint32_t filesize;
    uint8_t* myBuffer;
    int loopNum;
    bool setPositionWithTimeunitSubSongHasBeenInvoked = false;
    unsigned int totalSkippedBytes = 0;
};

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
    auto *plugin = new pluginLibkss(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    FMOD_CODEC_FILE_SIZE(codec, &plugin->filesize);
    if (plugin->filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 48000;
    plugin->waveformat.pcmblocksize = 2048 * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->myBuffer = new uint8_t[plugin->filesize];

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, plugin->filesize, nullptr);

    if (KSS_check_type(plugin->myBuffer, plugin->filesize, plugin->info->filename.c_str()) == KSSDATA) {
        /* TODO:
         *  KSS format support in libkss is better than game-music-emu (which doesn't support FM sound),
         *  however currently subsongs are not handled here, so better to stick with game-music-emu
         *  until proper subsongs support will be implemented
         */
        return FMOD_ERR_FORMAT;
    }

    if (KSS_check_type(plugin->myBuffer, plugin->filesize, plugin->info->filename.c_str()) == MBMDATA) {
        /* TODO:
         *  seems the KSS_autoload_mbk invocation introduces a beginning audio delay (somewhere in the code not in this function)
         *  sufficient for having a smooth plauback without need to skip the fmod pre-buffering...
         *  ... but still not a solution
         */
        KSS_autoload_mbk(plugin->info->filename.c_str(), "", nullptr);
    }

    plugin->kss = KSS_bin2kss(plugin->myBuffer, plugin->filesize, plugin->info->filename.c_str());

    if (plugin->kss == nullptr) {
        return FMOD_ERR_FORMAT;
    }

    plugin->kssplay = KSSPLAY_new(plugin->waveformat.frequency, plugin->waveformat.channels, 16);
    KSSPLAY_set_data(plugin->kssplay, plugin->kss);

    plugin->currentSubsong = 0;

    KSSPLAY_reset(plugin->kssplay, plugin->currentSubsong, 0);

    // TODO setting to 0 for continuous playback, however it results silent playback for some tracks (eg REQUIEM2.MGS)
    plugin->loopNum = 1;

    uint32_t quality = 1;

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
    plugin->info->fileformat = reinterpret_cast<char *>(plugin->kss->idstr);

    if (plugin->kss->extra) {
        plugin->info->comments = reinterpret_cast<char *>(plugin->kss->extra);
    }

    plugin->info->numSamples = 0;
    plugin->info->plugin = PLUGIN_libkss;
    plugin->info->pluginName = PLUGIN_libkss_NAME;

    plugin->info->setSeekable(false);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginLibkss*>(codec->plugindata);

    if (plugin != nullptr) {
        KSSPLAY_delete(plugin->kssplay);
        KSS_delete(plugin->kss);
    }

    delete plugin;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto *plugin = static_cast<pluginLibkss *>(codec->plugindata);

    // TODO workaround: skipping fmod pre-buffering in order to avoid initial playback glitch
    if (!plugin->setPositionWithTimeunitSubSongHasBeenInvoked) {
        *read = 8;
        plugin->totalSkippedBytes += *read;
        return FMOD_OK;
    }

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

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG) {
        *length = 1;
        return FMOD_OK;
    }

    *length = -1;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibkss*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->setPositionWithTimeunitSubSongHasBeenInvoked = true;
    }

    return FMOD_OK;
}
