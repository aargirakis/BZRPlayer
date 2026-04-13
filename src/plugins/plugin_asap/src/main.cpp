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
    PLUGIN_asap_NAME, // name.
    0x00012300, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE,
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

class pluginAsap {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAsap(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAsap() {
        // delete some stuff
        ASAP_Delete(asap);
    }

    Info *info;
    FMOD_CODEC_WAVEFORMAT waveformat;
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
    auto *plugin = new pluginAsap(codec);
    plugin->asap = ASAP_New();

    plugin->info = static_cast<Info *>(userexinfo->userdata);

    if (static constexpr ASAPFileLoader loader;
        !ASAP_LoadWithExtraFiles(plugin->asap, plugin->info->filePath.c_str(), plugin->info->fileBuffer,
                                 static_cast<int>(plugin->info->filesize), &loader)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->asap_info = ASAP_GetInfo(plugin->asap);

    plugin->songLength = ASAPInfo_GetDuration(plugin->asap_info, plugin->info->currentSubsong);

    if (!ASAP_PlaySong(plugin->asap, plugin->info->currentSubsong, plugin->songLength)) {
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
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->numSubsongs = ASAPInfo_GetSongs(plugin->asap_info);
    plugin->info->defaultSubsong = plugin->info->numSubsongs > 1
                                       ? ASAPInfo_GetDefaultSong(plugin->asap_info) + 1
                                       : -1;
    plugin->info->author = ASAPInfo_GetAuthor(plugin->asap_info);
    plugin->info->title = ASAPInfo_GetTitle(plugin->asap_info);
    plugin->info->chips = format("x{} ({})", plugin->waveformat.channels,
                                 plugin->waveformat.channels > 1 ? "Stereo" : "Mono");
    plugin->info->numChannels = plugin->waveformat.channels * 4;
    plugin->info->date = ASAPInfo_GetDate(plugin->asap_info);
    plugin->info->clockSpeedStr = format("{} Hz / {} scanlines ({})", ASAPInfo_GetPlayerRateHz(plugin->asap_info),
                                         ASAPInfo_GetPlayerRateScanlines(plugin->asap_info),
                                         ASAPInfo_IsNtsc(plugin->asap_info) ? "NTSC" : "PAL");
    plugin->info->plugin = PLUGIN_asap;
    plugin->info->pluginName = PLUGIN_asap_NAME;
    plugin->info->setSeekable(true);

    if (const char *originalModuleExt = ASAPInfo_GetOriginalModuleExt(plugin->asap_info);
        originalModuleExt == nullptr) {
        plugin->info->fileFormat = format("Slight Atari Player (Type {:c})", ASAPInfo_GetTypeLetter(plugin->asap_info));
    } else {
        plugin->info->fileFormat = ASAPInfo_GetExtDescription(originalModuleExt);
    }

    if (int offset = ASAPInfo_GetInstrumentNamesOffset(plugin->asap_info, plugin->info->fileBuffer,
                                                       static_cast<int>(plugin->info->filesize));
        offset > 0) {
        plugin->info->numInstruments = 0;
        vector<string> instruments;

        do {
            const char *instrument = reinterpret_cast<const char *>(plugin->info->fileBuffer) + offset;
            instruments.emplace_back(instrument);
            offset += static_cast<int>(strnlen(instrument, plugin->info->filesize - offset)) + 1;
            plugin->info->numInstruments++;
        } while (offset < plugin->info->filesize);

        plugin->info->instruments = new string[plugin->info->numInstruments];

        for (int i = 0; i < plugin->info->numInstruments; i++) {
            plugin->info->instruments[i] = instruments[i];
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
        /*
         * asap issue workaround
         * in version 8.0.0 has been fixed for forward seeking only,
         * however still present for backward seeking
         */
        if (ASAP_Seek(plugin->asap, static_cast<int>(position))) {
            ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(plugin->mask));
        }
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        // mutes voices
        // position is a mask
        ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(position));
        plugin->mask = position;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
