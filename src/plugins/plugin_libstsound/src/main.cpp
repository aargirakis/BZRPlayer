#include <cstring>
#include "StSoundLibrary.h"
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
    PLUGIN_libstsound_NAME, // name.
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

class pluginLibstsound {
    FMOD_CODEC_STATE *_codec;

public:
    pluginLibstsound(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibstsound() {
        // delete some stuff
        ymMusicDestroy(pMusic);
        delete[] myBuffer;
    }

    YMMUSIC *pMusic;
    FMOD_CODEC_WAVEFORMAT waveformat;
    uint8_t *myBuffer;
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

    auto *plugin = new pluginLibstsound(codec);

    plugin->myBuffer = new uint8_t[filesize];

    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    unsigned int bytesread;
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    plugin->pMusic = ymMusicCreate();

    if (!ymMusicLoadMemory(plugin->pMusic, plugin->myBuffer, filesize)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    ymMusicSetLoopMode(plugin->pMusic, YMTRUE);

    ymMusicPlay(plugin->pMusic);

    ymMusicInfo_t yminfo;
    ymMusicGetInfo(plugin->pMusic, &yminfo);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = string(yminfo.pSongType) == "MIX1" ? 1 : 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = yminfo.musicTimeInMs > 0
                                       ? static_cast<unsigned int>(
                                           yminfo.musicTimeInMs / 1000.0 * plugin->waveformat.frequency)
                                       : -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    const auto info = static_cast<Info *>(userexinfo->userdata);

    info->artist = yminfo.pSongAuthor;
    info->title = yminfo.pSongName;
    info->comments = yminfo.pSongComment;
    info->songPlayer = yminfo.pSongPlayer;
    info->comments = yminfo.pSongComment;
    info->numSamples = 0;
    info->plugin = PLUGIN_libstsound;
    info->pluginName = PLUGIN_libstsound_NAME;
    info->fileFormat = yminfo.pSongType;
    info->setSeekable(false);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLibstsound *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginLibstsound *>(codec->plugindata);

    ymMusicCompute(plugin->pMusic, static_cast<ymsample *>(buffer), size);
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
