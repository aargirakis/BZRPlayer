#include "lib/ksnd.h"
#include "snd/music.h"
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
    PLUGIN_klystron_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL,
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

class pluginKlystron {
    FMOD_CODEC_STATE *_codec;

public:
    pluginKlystron(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    static uint32_t samplesToMs(const uint32_t samples) {
        const uint64_t ms = static_cast<uint64_t>(samples) * 1000 / (sampleRate * channels);
        return static_cast<uint32_t>(ms);
    }

    static uint32_t msToSamples(const uint32_t ms) {
        const uint64_t samples = static_cast<uint64_t>(ms) * sampleRate * channels / 1000;
        return static_cast<uint32_t>(samples);
    }

    ~pluginKlystron() {
        if (song) KSND_FreeSong(song);
        if (player) KSND_FreePlayer(player);
        delete songinfo;
    }

    static constexpr unsigned int sampleRate = 44100;
    static constexpr unsigned int channels = 2;
    KPlayer *player;
    KSongInfo *songinfo = nullptr;
    KSong *song = nullptr;
    FMOD_CODEC_WAVEFORMAT waveformat;
    unsigned int songLength;
    uint32_t renderingPosition = 0;
    uint32_t seekPosition;
    bool isSeeking = false;
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
    logDebug("Try", PLUGIN_klystron_NAME);

    auto *plugin = new pluginKlystron(codec);

    plugin->player = KSND_CreatePlayerUnregistered(pluginKlystron::sampleRate);
    if (!plugin->player) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    const auto info = static_cast<Info *>(userexinfo->userdata);

    plugin->song = KSND_LoadSongFromMemory(plugin->player, info->fileBuffer, static_cast<int>(info->filesize));

    if (!plugin->song) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = pluginKlystron::channels;
    plugin->waveformat.frequency = pluginKlystron::sampleRate;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->songinfo = new KSongInfo();
    KSND_GetSongInfo(plugin->song, plugin->songinfo);

    const int numPatternRows = KSND_GetSongLength(plugin->song);

    plugin->songLength = KSND_GetPlayTime(plugin->song, numPatternRows);

    info->title = plugin->songinfo->song_title;
    info->numInstruments = plugin->songinfo->n_instruments;
    info->numChannels = plugin->songinfo->n_channels;
    info->numPatterns = numPatternRows;

    if (info->numInstruments > 0) {
        info->instruments = new string[info->numInstruments];
        for (int j = 0; j < info->numInstruments; j++) {
            info->instruments[j] = plugin->songinfo->instrument_name[j];
        }
    }

    KSND_PlaySong(plugin->player, plugin->song, 0);

    info->fileFormat = "Klystrack";
    info->plugin = PLUGIN_klystron;
    info->pluginName = PLUGIN_klystron_NAME;
    info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginKlystron *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    if (auto *plugin = static_cast<pluginKlystron *>(codec->plugindata);
        plugin->isSeeking) {
        if (plugin->renderingPosition < plugin->seekPosition) {
            uint32_t toSkip = plugin->seekPosition - plugin->renderingPosition;

            if (toSkip > 32768) {
                toSkip = 32768;
            }

            vector<short int> dummyBuffer(toSkip);

            KSND_FillBuffer(plugin->player, dummyBuffer.data(), static_cast<int>(toSkip * plugin->waveformat.format));
            plugin->renderingPosition += toSkip;

            memset(buffer, 0, size * plugin->waveformat.pcmblocksize);
            *read = size;
        } else {
            plugin->isSeeking = false;
            *read = 0;
        }
    } else {
        KSND_FillBuffer(plugin->player, static_cast<short int *>(buffer),
                        static_cast<int>(size * plugin->waveformat.pcmblocksize));
        plugin->renderingPosition += size * pluginKlystron::channels;
        *read = size;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginKlystron *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->songLength;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginKlystron *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        bool shouldReinit = false;

        if (position == 0) {
            shouldReinit = plugin->renderingPosition != 0;
        } else {
            plugin->seekPosition = pluginKlystron::msToSamples(position) & ~1;
            shouldReinit = plugin->seekPosition < plugin->renderingPosition;
            plugin->isSeeking = true;
        }

        if (shouldReinit) {
            KSND_PlaySong(plugin->player, plugin->song, 0);
            plugin->renderingPosition = 0;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginKlystron *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS_REAL) {
        *position = pluginKlystron::samplesToMs(plugin->renderingPosition);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
