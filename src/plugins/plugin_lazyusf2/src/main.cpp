#include <cmath>
#include <cstring>
#include "psflib.h"
#include "usf.h"
#include "main.h"
#include "fmod_errors.h"
#include "info.h"
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
    PLUGIN_lazyusf2_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
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

class pluginLazyusf2 {
    FMOD_CODEC_STATE *_codec;
    FILE *file;

public:
    pluginLazyusf2(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLazyusf2() {
        if (m_lazyState) {
            usf_shutdown(m_lazyState);
            delete[] m_lazyState;
        }
    }

    static int InfoMetaPSF(void *context, const char *name, const char *value) {
        auto *plugin = static_cast<pluginLazyusf2 *>(context);
        if (!strcasecmp(name, "_enablecompare")) {
            plugin->m_loaderState.enablecompare = 1;
        } else if (!strcasecmp(name, "_enablefifofull")) {
            plugin->m_loaderState.enablefifofull = 1;
        } else if (!strncasecmp(name, "replaygain_", sizeof("replaygain_") - 1)) {
        }

        /* "length" & "fade" tags (https://web.archive.org/web/20110902100659/http://wiki.neillcorlett.com/PSFTagFormat):
         * Length of the song, and the length of the ending fadeout. These may be in one of three formats:
         *
         * seconds.decimal
         * minutes:seconds.decimal
         * hours:minutes:seconds.decimal
         *
         * The decimal portion may be omitted. Commas are also recognized as decimal separators.
         */

        else if (!strcasecmp(name, "length")) {
            auto getDigit = [](const char *&value) {
                bool isDigit = false;
                int32_t digit = 0;
                while (*value && (*value < '0' || *value > '9'))
                    ++value;
                while (*value && *value >= '0' && *value <= '9') {
                    isDigit = true;
                    digit = digit * 10 + *value - '0';
                    ++value;
                }
                if (isDigit)
                    return digit;
                return -1;
            };

            if (auto length = getDigit(value); length >= 0) {
                while (*value && *value != ':' && *value != '.' && *value != ',')
                    ++value;
                if (*value) {
                    while (*value == ':') {
                        if (auto d = getDigit(++value); d >= 0)
                            length = length * 60 + Clamp(d, 0, 59);
                    }
                    length *= 1000;
                    while (*value && *value != '.' && *value != ',')
                        ++value;
                    if (*value == '.' || *value == ',') {
                        // up to 3 decimal digits are supported
                        // 0.0nn & 0.00n values are not handled
                        const auto d = getDigit(++value);
                        const int digits = d == 0 ? 1 : static_cast<int>(log10(d)) + 1;
                        length += d * static_cast<int>(pow(10, 3 - digits));
                    }
                } else
                    length *= 1000;

                if (length > 0)
                    plugin->m_length = length;
            }
        } else if (!strcasecmp(name, "fade")) {
        } else if (!strcasecmp(name, "utf8")) {
        } else if (!strcasecmp(name, "_lib")) {
            //plugin->m_hasLib = true;
        } else if (name[0] == '_') {
        } else {
            plugin->m_tags[name] = value;
        }
        return 0;
    }

    static void *OpenPSF(void *context, const char *uri) {
        const auto plugin = static_cast<pluginLazyusf2 *>(context);
        unsigned int filesize;
        FMOD_CODEC_FILE_SIZE(plugin->_codec, &filesize);
        plugin->file = fopen(uri, "rb");
        return plugin->file;
    }

    static size_t ReadPSF(void *buffer, size_t size, size_t count, void *handle) {
        return fread(buffer, size, count, static_cast<FILE *>(handle));
    }

    static int SeekPSF(void *handle, int64_t offset, int whence) {
        return fseek(static_cast<FILE *>(handle), offset, whence);
    }

    static int ClosePSF(void *handle) {
        return fclose(static_cast<FILE *>(handle));
    }

    static long TellPSF(void *handle) {
        return ftell(static_cast<FILE *>(handle));
    }

    static int UsfLoad(void *context, const uint8_t *exe, size_t exe_size, const uint8_t *reserved,
                       size_t reserved_size) {
        const auto *plugin = static_cast<pluginLazyusf2 *>(context);
        if (exe && exe_size > 0)
            return -1;

        return usf_upload_section(plugin->m_lazyState, reserved, reserved_size);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    unordered_map<string, string> m_tags;
    unsigned int m_length = -1;
    uint8_t *m_lazyState = nullptr;

    struct LoaderState {
        uint32_t enablecompare = 0;
        uint32_t enablefifofull = 0;
    } m_loaderState;

    const psf_file_callbacks m_psfFileSystem = {
        "\\/|:",
        this,
        OpenPSF,
        ReadPSF,
        SeekPSF,
        ClosePSF,
        TellPSF
    };
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
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
    unsigned int bytesread;

    auto *buffer = new uint8_t[4];
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, 4, &bytesread);

    // skip midi and riff
    if (memcmp(buffer, "MThd", 4) == 0 || memcmp(buffer, "RIFF", 4) == 0) {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] buffer;

    auto *plugin = new pluginLazyusf2(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    const auto psfType = psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x21, nullptr, nullptr,
                                  pluginLazyusf2::InfoMetaPSF,
                                  plugin, 0, nullptr, nullptr);
    if (psfType != 0x21) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (const auto extPos = plugin->info->filename.find_last_of('.'); extPos != string::npos && strcasecmp(
                                                                          plugin->info->filename.c_str() + extPos + 1,
                                                                          "usflib") == 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (plugin->m_lazyState == nullptr) {
        plugin->m_lazyState = new uint8_t[usf_get_state_size()];
    }

    usf_clear(plugin->m_lazyState);
    if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, static_cast<uint8_t>(psfType),
                 pluginLazyusf2::UsfLoad, plugin,
                 nullptr, nullptr, 0, nullptr, nullptr) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->plugin = PLUGIN_lazyusf2;
    plugin->info->pluginName = PLUGIN_lazyusf2_NAME;
    plugin->info->fileformat = "Nintendo 64 (USF)";

    if (keyExists(plugin->m_tags, "title")) {
        plugin->info->title = plugin->m_tags["title"];
    }
    if (keyExists(plugin->m_tags, "artist")) {
        plugin->info->artist = plugin->m_tags["artist"];
    }
    if (keyExists(plugin->m_tags, "game")) {
        plugin->info->game = plugin->m_tags["game"];
    }
    if (keyExists(plugin->m_tags, "copyright")) {
        plugin->info->copyright = plugin->m_tags["copyright"];
    }
    if (keyExists(plugin->m_tags, "usfby")) {
        plugin->info->ripper = plugin->m_tags["usfby"];
    }
    if (keyExists(plugin->m_tags, "year")) {
        plugin->info->date = plugin->m_tags["year"];
    }
    if (keyExists(plugin->m_tags, "volume")) {
        plugin->info->volumeAmplificationStr = plugin->m_tags["volume"];
    }
    if (keyExists(plugin->m_tags, "genre")) {
        plugin->info->genre = plugin->m_tags["genre"];
    }
    if (keyExists(plugin->m_tags, "comment")) {
        plugin->info->comments = plugin->m_tags["comment"];
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLazyusf2 *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);
    usf_render_resampled(plugin->m_lazyState, static_cast<short *>(buffer), size, plugin->waveformat.frequency);
    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);
    usf_shutdown(plugin->m_lazyState);
    usf_clear(plugin->m_lazyState);
    psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x21, pluginLazyusf2::UsfLoad, plugin, nullptr,
             nullptr, 0, nullptr, nullptr);
    usf_set_hle_audio(plugin->m_lazyState, 1);
    usf_set_compare(plugin->m_lazyState, plugin->m_loaderState.enablecompare);
    usf_set_fifo_full(plugin->m_lazyState, plugin->m_loaderState.enablefifofull);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS) {
        if (plugin->m_length > 0) {
            *length = plugin->m_length;
        } else {
            *length = -1;
        }

        plugin->info->numSubsongs = 1;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
