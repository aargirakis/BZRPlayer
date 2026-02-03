#include <cstring>
#include <format>
#include "asap.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

struct ASAPFileLoaderVtbl {
    int (*load)(const ASAPFileLoader *self, const char *filename, uint8_t *buffer, int length);
};

static constexpr ASAPFileLoaderVtbl loadSamplesFile = {
    [](const ASAPFileLoader *, const char *filename, uint8_t *buffer, int length) {
        FILE *fp = fopen(filename, "rb");
        if (!fp) return -1;
        length = static_cast<int>(fread(buffer, 1, length, fp));
        fclose(fp);
        return length;
    }
};

struct ASAPFileLoader {
    const ASAPFileLoaderVtbl *vtbl = &loadSamplesFile;
};

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_asap_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE,
    // The time format we would like to accept into setposition/getposition.
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

class pluginAsap {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAsap(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAsap() {
        //delete some stuff
        delete[] myBuffer;
        ASAP_Delete(asap);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    uint8_t *myBuffer = nullptr;
    ASAP *asap;
    const ASAPInfo *asap_info;
    int songLength;
    unsigned int mask = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codec;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto *plugin = new pluginAsap(codec);

    plugin->myBuffer = new uint8_t[filesize];

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    plugin->asap = ASAP_New();

    auto *info = static_cast<Info *>(userexinfo->userdata);

    if (static constexpr ASAPFileLoader loader;
        !ASAP_LoadWithExtraFiles(plugin->asap, info->filename.c_str(), plugin->myBuffer, static_cast<int>(filesize),
                                 &loader)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->asap_info = ASAP_GetInfo(plugin->asap);

    plugin->songLength = ASAPInfo_GetDuration(plugin->asap_info, info->currentSubsong);

    if (!ASAP_PlaySong(plugin->asap, info->currentSubsong, plugin->songLength)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = ASAPInfo_GetChannels(plugin->asap_info);
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->numSubsongs = ASAPInfo_GetSongs(plugin->asap_info);
    info->defaultSubSong = info->numSubsongs > 1
                               ? ASAPInfo_GetDefaultSong(plugin->asap_info) + 1
                               : -1;
    info->author = ASAPInfo_GetAuthor(plugin->asap_info);
    info->title = ASAPInfo_GetTitle(plugin->asap_info);
    info->chips = format("x{} ({})", plugin->waveformat.channels,
                         plugin->waveformat.channels > 1 ? "Stereo" : "Mono");
    info->numChannels = plugin->waveformat.channels * 4;
    info->date = ASAPInfo_GetDate(plugin->asap_info);
    info->clockSpeedStr = format("{} Hz / {} scanlines ({})", ASAPInfo_GetPlayerRateHz(plugin->asap_info),
                                 ASAPInfo_GetPlayerRateScanlines(plugin->asap_info),
                                 ASAPInfo_IsNtsc(plugin->asap_info) ? "NTSC" : "PAL");
    info->plugin = PLUGIN_asap;
    info->pluginName = PLUGIN_asap_NAME;
    info->setSeekable(true);

    if (const char *originalModuleExt = ASAPInfo_GetOriginalModuleExt(plugin->asap_info);
        originalModuleExt == nullptr) {
        info->fileformat = format("Slight Atari Player (Type {:c})", ASAPInfo_GetTypeLetter(plugin->asap_info));
    } else {
        info->fileformat = ASAPInfo_GetExtDescription(originalModuleExt);
    }

    if (int offset = ASAPInfo_GetInstrumentNamesOffset(plugin->asap_info, plugin->myBuffer, static_cast<int>(filesize));
        offset > 0) {
        info->numInstruments = 0;
        vector<string> instruments;

        do {
            const char *instrument = reinterpret_cast<const char *>(plugin->myBuffer) + offset;
            instruments.emplace_back(instrument);
            offset += static_cast<int>(strnlen(instrument, filesize - offset)) + 1;
            info->numInstruments++;
        } while (offset < filesize);

        info->instruments = new string[info->numInstruments];

        for (int i = 0; i < info->numInstruments; i++) {
            info->instruments[i] = instruments[i];
        }
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginAsap *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginAsap *>(codec->plugindata);

    const int bufferedBytes = ASAP_Generate(plugin->asap, static_cast<unsigned char *>(buffer),
                                            static_cast<int>(size) << plugin->waveformat.channels,
                                            ASAPSampleFormat_S16_L_E);

    if (bufferedBytes <= 0) {
        return FMOD_ERR_FILE_EOF;
    }

    *read = bufferedBytes;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto const *plugin = static_cast<pluginAsap *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->songLength;
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
    auto *plugin = static_cast<pluginAsap *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        if (ASAP_Seek(plugin->asap, static_cast<int>(position))) {
            ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(plugin->mask));
        }
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        //mutes voices
        //position is a mask
        ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(position));
        plugin->mask = position;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
