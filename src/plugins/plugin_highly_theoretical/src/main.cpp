#include <algorithm>
#include <cmath>
#ifndef WIN32
#include <filesystem>
#endif
#include "psflib.h"
#include "sega.h"
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
    PLUGIN_highly_theoretical_NAME, // name.
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

class pluginHighlyTheo {
    FMOD_CODEC_STATE *_codec;
    FILE *file;

public:
    pluginHighlyTheo(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginHighlyTheo() {
        // delete some stuff
        delete[] segaState;
    }

    static int InfoMetaPSF(void *context, const char *name, const char *value) {
        auto *plugin = static_cast<pluginHighlyTheo *>(context);
        if (!strncasecmp(name, "replaygain_", sizeof("replaygain_") - 1)) {
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
        const auto plugin = static_cast<pluginHighlyTheo *>(context);
        unsigned int filesize;
        FMOD_CODEC_FILE_SIZE(plugin->_codec, &filesize);

#ifdef WIN32
        plugin->file = fopen(uri, "rb");
#else
        string ext = filesystem::path(uri).filename().string();
        std::ranges::transform(ext, ext.begin(), ::tolower);

        if (ext.ends_with(".dsflib") || ext.ends_with(".ssflib")) {
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

    static int SdsfLoad(void *context, const uint8_t *exe, size_t exe_size, const uint8_t * /* reserved */,
                        size_t /* reserved_size */) {
        auto *plugin = static_cast<pluginHighlyTheo *>(context);
        if (exe_size < 4) return -1;

        uint8_t *dst = plugin->loaderState.data;

        if (plugin->loaderState.data_size < 4) {
            plugin->loaderState.data = dst = static_cast<uint8_t *>(malloc(exe_size));
            plugin->loaderState.data_size = exe_size;
            memcpy(dst, exe, exe_size);
            return 0;
        }

        uint32_t dst_start = get_le32(dst);
        uint32_t src_start = get_le32(exe);
        dst_start &= 0x7fffff;
        src_start &= 0x7fffff;
        size_t dst_len = plugin->loaderState.data_size - 4;
        size_t src_len = exe_size - 4;
        if (dst_len > 0x800000) dst_len = 0x800000;
        if (src_len > 0x800000) src_len = 0x800000;

        if (src_start < dst_start) {
            const uint32_t diff = dst_start - src_start;
            plugin->loaderState.data_size = dst_len + 4 + diff;
            plugin->loaderState.data = dst = static_cast<uint8_t *>(realloc(dst, plugin->loaderState.data_size));
            memmove(dst + 4 + diff, dst + 4, dst_len);
            memset(dst + 4, 0, diff);
            dst_len += diff;
            dst_start = src_start;
            set_le32(dst, dst_start);
        }
        if (src_start + src_len > dst_start + dst_len) {
            const size_t diff = src_start + src_len - (dst_start + dst_len);
            plugin->loaderState.data_size = dst_len + 4 + diff;
            plugin->loaderState.data = dst = static_cast<uint8_t *>(realloc(dst, plugin->loaderState.data_size));
            memset(dst + 4 + dst_len, 0, diff);
        }

        memcpy(dst + 4 + (src_start - dst_start), exe + 4, src_len);

        return 0;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    unordered_map<string, string> tags;
    int psfType = 0;
    unsigned int length = -1;
    uint8_t *segaState = nullptr;

    struct LoaderState {
        uint8_t *data = nullptr;
        size_t data_size = 0;

        void Clear() { new(this) LoaderState(); }

        ~LoaderState() { Clear(); }
    } loaderState = {};

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
    unsigned int bytesread;
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    auto *buffer = new uint8_t[4];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, 4, &bytesread);

    // skip midi and riff
    if (memcmp(buffer, "MThd", 4) == 0 || memcmp(buffer, "RIFF", 4) == 0) {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] buffer;

    auto *plugin = new pluginHighlyTheo(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    const auto psfType = psf_load(plugin->info->filename.c_str(), &plugin->psfFileSystem, 0, nullptr, nullptr,
                                  pluginHighlyTheo::InfoMetaPSF,
                                  plugin, 0, nullptr, nullptr);

    if (psfType != 0x11 && psfType != 0x12) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (psf_load(plugin->info->filename.c_str(), &plugin->psfFileSystem, static_cast<uint8_t>(psfType),
                 pluginHighlyTheo::SdsfLoad, plugin, nullptr, nullptr, 0, nullptr, nullptr) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->psfType = static_cast<uint8_t>(psfType);

    sega_init();

    plugin->segaState = new uint8_t[sega_get_state_size(plugin->psfType - 0x10)];

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    if (psfType == 0x12) {
        plugin->info->fileFormat = "Dreamcast (DSF)";
    } else {
        plugin->info->fileFormat = "Sega Saturn (SSF)";
    }

    plugin->info->plugin = PLUGIN_highly_theoretical;
    plugin->info->pluginName = PLUGIN_highly_theoretical_NAME;

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
    if (const string ripperTagKey = psfType == 0x11 ? "ssfby" : "dsfby"; keyExists(plugin->tags, ripperTagKey)) {
        plugin->info->ripper = plugin->tags[ripperTagKey];
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
    delete static_cast<pluginHighlyTheo *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginHighlyTheo *>(codec->plugindata);

    sega_execute(plugin->segaState, 0x7fffffff, static_cast<short *>(buffer), &size);
    *read = size;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginHighlyTheo *>(codec->plugindata);
    sega_clear_state(plugin->segaState, plugin->psfType - 0x10);
    sega_enable_dry(plugin->segaState, 1);
    sega_enable_dsp(plugin->segaState, 1);
    sega_enable_dsp_dynarec(plugin->segaState, 0);
    const uint32_t start = *reinterpret_cast<uint32_t *>(plugin->loaderState.data);
    uint32_t length = plugin->loaderState.data_size;
    if (const size_t maxLength = (plugin->psfType == 0x12) ? 0x800000 : 0x80000; (start + (length - 4)) > maxLength) {
        length = maxLength - start + 4;
    }

    sega_upload_program(plugin->segaState, plugin->loaderState.data, length);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginHighlyTheo *>(codec->plugindata);

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
