#include "v2mplayer.h"
#include "v2mconv.h"
#include "sounddef.h"
#include "fmod_errors.h"
#include "info.h"
#include "logger.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_v2m_player_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

static constexpr unsigned int channels = 2;
static constexpr unsigned int pcmFloatSize = sizeof(uint32_t);
static constexpr unsigned int maxSamples = 512;
static constexpr unsigned int audioChunkSize = channels * pcmFloatSize * maxSamples;

class pluginV2mPlayer {
    FMOD_CODEC_STATE *_codec;

public:
    pluginV2mPlayer(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginV2mPlayer() {
        // delete some stuff

        player->Close();
        delete player;
        delete[] convertedSong;
    }

    V2MPlayer *player;
    uint8_t *convertedSong;
    FMOD_CODEC_WAVEFORMAT waveformat;
    unsigned int songLength;
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
    logDebug("Try", PLUGIN_v2m_player_NAME);

    auto *info = static_cast<Info *>(userexinfo->userdata);

    sdInit();

    if (ssbase base{}; CheckV2MVersion(info->fileBuffer, static_cast<int>(info->filesize), base) < 0) {
        return FMOD_ERR_FORMAT;
    }

    auto *plugin = new pluginV2mPlayer(codec);

    int newSize;
    ConvertV2M(info->fileBuffer, static_cast<int>(info->filesize), &plugin->convertedSong, &newSize);

    plugin->player = new V2MPlayer();
    plugin->player->Init();

    constexpr uint32_t sampleRate = 44100;

    if (!plugin->player->Open(plugin->convertedSong, sampleRate, true)) {
        return FMOD_ERR_FORMAT;
    }

    sS32 *positions;
    const uint32_t numPositions = plugin->player->CalcPositions(&positions);

    plugin->songLength = static_cast<unsigned int>(positions[2 * (numPositions - 1)]);

    delete[] positions;

    // add two extra seconds for reverb
    plugin->songLength += 2000;

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = sampleRate;
    plugin->waveformat.pcmblocksize = audioChunkSize;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    info->fileFormat = "Farbrausch V2M";
    info->setSeekable(true);
    info->plugin = PLUGIN_v2m_player;
    info->pluginName = PLUGIN_v2m_player_NAME;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    if (plugin) {
        plugin->player->Stop();
        plugin->player->Close();
    }

    delete plugin;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    plugin->player->Render(static_cast<float *>(buffer), maxSamples);

    if (size < maxSamples) {
        *read = size;
    } else {
        *read = maxSamples;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->songLength;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->player->Play(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
