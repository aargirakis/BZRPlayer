#include <algorithm>
#include <cmath>
#include <cstring>
#ifndef WIN32
#include <filesystem>
#endif
#include "psflib.h"
#include "usf.h"
#include "main.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

#ifndef WIN32
static bool case_insensitive_compare(const std::string &a, const std::string &b) {
    if (a.size() != b.size()) return false;

    for (size_t i = 0; i < a.size(); ++i) {
        const auto ca = static_cast<unsigned char>(a[i]);

        if (const auto cb = static_cast<unsigned char>(b[i]); std::tolower(ca) != std::tolower(cb)) return false;
    }

    return true;
}

std::FILE *fopen_case_insensitive(const std::string &name, const char *mode) {
    std::FILE *fp = std::fopen(name.c_str(), mode);

    if (fp) return fp;

    const filesystem::path p(name);
    const filesystem::path dir = p.parent_path();

    if (dir.empty() || dir == ".") {
        return nullptr;
    }

    const std::string target_basename = p.filename().string();

    for (std::error_code ec; auto const &entry: filesystem::directory_iterator(dir, ec)) {
        if (ec) break;

        if (!entry.is_regular_file(ec) && !entry.is_symlink(ec)) continue;

        if (std::string cand = entry.path().filename().string(); case_insensitive_compare(cand, target_basename)) {
            std::string candidate_path = (dir / cand).string();
            fp = std::fopen(candidate_path.c_str(), mode);

            if (fp) return fp;
        }
    }

    return nullptr;
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_lazyusf2_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
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

class pluginLazyusf2 {
    FMOD_CODEC_STATE *_codec;
    FILE *file;

public:
    pluginLazyusf2(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLazyusf2() {
        if (lazyState) {
            usf_shutdown(lazyState);
            delete[] lazyState;
        }
    }

    static int InfoMetaPSF(void *context, const char *name, const char *value) {
        auto *plugin = static_cast<pluginLazyusf2 *>(context);

        if (!strcasecmp(name, "_enablecompare")) {
            plugin->loaderState.enablecompare = 1;
        } else if (!strcasecmp(name, "_enablefifofull")) {
            plugin->loaderState.enablefifofull = 1;
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
                    plugin->length = length;
            }
        } else if (!strcasecmp(name, "fade")) {
        } else if (!strcasecmp(name, "utf8")) {
        } else if (!strcasecmp(name, "_lib")) {
            //plugin->hasLib = true;
        } else if (name[0] == '_') {
        } else {
            plugin->tags[name] = value;
        }

        return 0;
    }

    static void *OpenPSF(void *context, const char *uri) {
        const auto plugin = static_cast<pluginLazyusf2 *>(context);

#ifdef WIN32
        plugin->file = fopen(uri, "rb");
#else
        string ext = filesystem::path(uri).filename().string();
        std::ranges::transform(ext, ext.begin(), ::tolower);

        if (ext.ends_with(".usflib")) {
            plugin->file = fopen_case_insensitive(uri, "rb");
        } else {
            plugin->file = fopen(uri, "rb");
        }
#endif

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

        return usf_upload_section(plugin->lazyState, reserved, reserved_size);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    unordered_map<string, string> tags;
    unsigned int length = -1;
    uint8_t *lazyState = nullptr;

    struct LoaderState {
        uint32_t enablecompare = 0;
        uint32_t enablefifofull = 0;
    } loaderState;

    const psf_file_callbacks psfFileSystem = {
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
    auto *plugin = new pluginLazyusf2(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    const auto psfType = psf_load(plugin->info->filePath.c_str(), &plugin->psfFileSystem, 0x21, nullptr, nullptr,
                                  pluginLazyusf2::InfoMetaPSF,
                                  plugin, 0, nullptr, nullptr);

    if (psfType != 0x21) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (plugin->lazyState == nullptr) {
        plugin->lazyState = new uint8_t[usf_get_state_size()];
    }

    usf_clear(plugin->lazyState);

    if (psf_load(plugin->info->filePath.c_str(), &plugin->psfFileSystem, static_cast<uint8_t>(psfType),
                 pluginLazyusf2::UsfLoad, plugin, nullptr, nullptr, 0, nullptr, nullptr) < 0) {
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
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->plugin = PLUGIN_lazyusf2;
    plugin->info->pluginName = PLUGIN_lazyusf2_NAME;
    plugin->info->fileFormat = "Nintendo 64 (USF)";

    if (keyExists(plugin->tags, "title")) {
        plugin->info->title = plugin->tags["title"];
    }
    if (keyExists(plugin->tags, "artist")) {
        plugin->info->artist = plugin->tags["artist"];
    }
    if (keyExists(plugin->tags, "game")) {
        plugin->info->game = plugin->tags["game"];
    }
    if (keyExists(plugin->tags, "copyright")) {
        plugin->info->copyright = plugin->tags["copyright"];
    }
    if (keyExists(plugin->tags, "usfby")) {
        plugin->info->ripper = plugin->tags["usfby"];
    }
    if (keyExists(plugin->tags, "year")) {
        plugin->info->date = plugin->tags["year"];
    }
    if (keyExists(plugin->tags, "volume")) {
        plugin->info->volumeAmplificationStr = plugin->tags["volume"];
    }
    if (keyExists(plugin->tags, "genre")) {
        plugin->info->genre = plugin->tags["genre"];
    }
    if (keyExists(plugin->tags, "comment")) {
        plugin->info->comments = plugin->tags["comment"];
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLazyusf2 *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);
    usf_render_resampled(plugin->lazyState, static_cast<short *>(buffer), size, plugin->waveformat.frequency);
    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);
    usf_shutdown(plugin->lazyState);
    usf_clear(plugin->lazyState);
    psf_load(plugin->info->filePath.c_str(), &plugin->psfFileSystem, 0x21, pluginLazyusf2::UsfLoad, plugin, nullptr,
             nullptr, 0, nullptr, nullptr);
    usf_set_hle_audio(plugin->lazyState, 1);
    usf_set_compare(plugin->lazyState, plugin->loaderState.enablecompare);
    usf_set_fifo_full(plugin->lazyState, plugin->loaderState.enablefifofull);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginLazyusf2 *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        if (plugin->length > 0) {
            *length = plugin->length;
        } else {
            *length = -1;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
