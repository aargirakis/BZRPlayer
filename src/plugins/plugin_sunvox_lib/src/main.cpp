#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstring>
#include "sunvox.h"
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
    PLUGIN_sunvox_lib_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN |
    FMOD_TIMEUNIT_MODPATTERN_INFO | FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS | FMOD_TIMEUNIT_MODVUMETER |
    FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_SPEED |
    FMOD_TIMEUNIT_BPM, // the time format we would like to accept into setposition/getposition
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    nullptr, // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &setPosition, // setposition callback
    nullptr, // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginSunvoxLib {
    FMOD_CODEC_STATE *_codec;

public:
    pluginSunvoxLib(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSunvoxLib() {
        if (isDllLoaded) {
            sv_stop(0);
            sv_close_slot(0);
            sv_deinit();
        }
    }

    uint8_t *myBuffer;
    bool isDllLoaded = false;
    FMOD_CODEC_WAVEFORMAT waveformat;
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
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize == 4294967295) // stream
    {
        return FMOD_ERR_FORMAT;
    }

    uint8_t id[4] = "";
    unsigned int bytesread;

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, id, 4, &bytesread);

    if (memcmp(id, "SVOX", 4) != 0) {
        return FMOD_ERR_FORMAT;
    }

    auto *plugin = new pluginSunvoxLib(codec);
    const auto info = static_cast<Info *>(userexinfo->userdata);

    int res = sv_load_dll2((info->libPath + PLUGINS_DIR + "/" + SUNVOX_LIB).c_str());
    plugin->isDllLoaded = true;

    if (res == -1) {
        delete plugin;
        return FMOD_ERR_INTERNAL;
    }

    info->fileFormat = "SunVox";

    constexpr int sampleRate = 44100;
    constexpr int channels = 2;

    sv_init(nullptr, sampleRate, channels, SV_INIT_FLAG_USER_AUDIO_CALLBACK);

    sv_open_slot(0);

    plugin->myBuffer = new uint8_t[filesize];

    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    res = sv_load_from_memory(0, plugin->myBuffer, filesize);

    delete[] plugin->myBuffer;

    if (res != 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = sampleRate;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = sv_get_song_length_frames(0);

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    info->plugin = PLUGIN_sunvox_lib;
    info->pluginName = PLUGIN_sunvox_lib_NAME;
    info->title = sv_get_song_name(0);
    info->setSeekable(false);
    sv_play_from_beginning(0);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginSunvoxLib *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    sv_audio_callback(buffer, static_cast<int>(size), 0, sv_get_ticks());
    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
