#include <cstdio>
#include <cstring>
#include <string>
#include <zconf.h>
#include <zlib.h>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "state.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_vio2sf_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginVio2sf
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    pluginVio2sf(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVio2sf()
    {
        //delete some stuff
        state_deinit(&m_ndsState);
    }

    static int InfoMetaPSF(void* context, const char* name, const char* value)
    {
        auto* plugin = static_cast<pluginVio2sf*>(context);
        if (!_strnicmp(name, "replaygain_", sizeof("replaygain_") - 1))
        {
        }
        else if (!_stricmp(name, "length"))
        {
            auto getDigit = [](const char*& value)
            {
                bool isDigit = false;
                int32_t digit = 0;
                while (*value && (*value < '0' || *value > '9'))
                    ++value;
                while (*value && *value >= '0' && *value <= '9')
                {
                    isDigit = true;
                    digit = digit * 10 + *value - '0';
                    ++value;
                }
                if (isDigit)
                    return digit;
                else
                    return -1;
            };
            auto length = getDigit(value);
            if (length >= 0)
            {
                while (*value && *value != ':' && *value != '.')
                    ++value;
                if (*value)
                {
                    if (*value == ':')
                    {
                        auto d = getDigit(++value);
                        if (d >= 0)
                            length = length * 60 + Clamp(d, 0, 59);
                    }
                    length *= 1000;
                    while (*value && *value != '.')
                        ++value;
                    if (*value == '.')
                    {
                        auto d = getDigit(++value);
                        if (d >= 0)
                            length += Clamp(d, 0, 999);
                    }
                }
                else
                    length *= 1000;

                if (length > 0)
                    plugin->m_length = length;
            }
        }
        else if (!_stricmp(name, "fade"))
        {
        }
        else if (!_stricmp(name, "utf8"))
        {
        }
        else if (!_stricmp(name, "_lib"))
        {
            //plugin->m_hasLib = true;
        }
        else if (name[0] == '_')
        {
        }
        else
        {
            plugin->m_tags[name] = value;
        }
        return 0;
    }

    static void* OpenPSF(void* context, const char* uri)
    {
        auto plugin = static_cast<pluginVio2sf*>(context);
        unsigned int filesize;
        FMOD_CODEC_FILE_SIZE(plugin->_codec, &filesize);
        plugin->file = fopen(uri, "rb");
        return plugin->file;
    }

    static size_t ReadPSF(void* buffer, size_t size, size_t count, void* handle)
    {
        return fread(buffer, size, count, static_cast<FILE*>(handle));
    }

    static int SeekPSF(void* handle, int64_t offset, int whence)
    {
        return fseek(static_cast<FILE*>(handle), offset, whence);
    }

    static int ClosePSF(void* handle)
    {
        return fclose(static_cast<FILE*>(handle));
    }

    static long TellPSF(void* handle)
    {
        return ftell(static_cast<FILE*>(handle));
    }

    int static TwosfLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved,
                         size_t reserved_size)
    {
        auto* plugin = static_cast<pluginVio2sf*>(context);
        if (exe_size >= 8)
        {
            if (plugin->TwosfLoadMap(plugin, 0, exe, (unsigned)exe_size))
                return -1;
        }

        if (reserved_size)
        {
            size_t resv_pos = 0;
            if (reserved_size < 16)
                return -1;
            while (resv_pos + 12 < reserved_size)
            {
                unsigned save_size = get_le32(reserved + resv_pos + 4);
                unsigned save_crc = get_le32(reserved + resv_pos + 8);
                if (get_le32(reserved + resv_pos + 0) == 0x45564153)
                {
                    if (resv_pos + 12 + save_size > reserved_size)
                        return -1;
                    if (plugin->TwosfLoadMapz(plugin, 1, reserved + resv_pos + 12, save_size, save_crc))
                        return -1;
                }
                resv_pos += 12 + save_size;
            }
        }

        return 0;
    }

    int static TwosfLoadMap(void* context, int issave, const unsigned char* udata, unsigned usize)
    {
        auto* plugin = static_cast<pluginVio2sf*>(context);
        if (usize < 8) return -1;

        unsigned char* iptr;
        size_t isize;
        unsigned char* xptr;
        unsigned xsize = get_le32(udata + 4);
        unsigned xofs = get_le32(udata + 0);
        if (issave)
        {
            iptr = plugin->m_loaderState.state;
            isize = plugin->m_loaderState.state_size;
            plugin->m_loaderState.state = 0;
            plugin->m_loaderState.state_size = 0;
        }
        else
        {
            iptr = plugin->m_loaderState.rom;
            isize = plugin->m_loaderState.rom_size;
            plugin->m_loaderState.rom = 0;
            plugin->m_loaderState.rom_size = 0;
        }
        if (!iptr)
        {
            size_t rsize = xofs + xsize;
            if (!issave)
            {
                rsize -= 1;
                rsize |= rsize >> 1;
                rsize |= rsize >> 2;
                rsize |= rsize >> 4;
                rsize |= rsize >> 8;
                rsize |= rsize >> 16;
                rsize += 1;
            }
            iptr = static_cast<unsigned char*>(malloc(rsize + 10));
            if (!iptr)
                return -1;
            memset(iptr, 0, rsize + 10);
            isize = rsize;
        }
        else if (isize < xofs + xsize)
        {
            size_t rsize = xofs + xsize;
            if (!issave)
            {
                rsize -= 1;
                rsize |= rsize >> 1;
                rsize |= rsize >> 2;
                rsize |= rsize >> 4;
                rsize |= rsize >> 8;
                rsize |= rsize >> 16;
                rsize += 1;
            }
            xptr = static_cast<unsigned char*>(realloc(iptr, xofs + rsize + 10));
            if (!xptr)
            {
                free(iptr);
                return -1;
            }
            iptr = xptr;
            isize = rsize;
        }
        memcpy(iptr + xofs, udata + 8, xsize);
        if (issave)
        {
            plugin->m_loaderState.state = iptr;
            plugin->m_loaderState.state_size = isize;
        }
        else
        {
            plugin->m_loaderState.rom = iptr;
            plugin->m_loaderState.rom_size = isize;
        }
        return 0;
    }

    int static TwosfLoadMapz(void* context, int issave, const unsigned char* zdata, unsigned zsize, unsigned zcrc)
    {
        auto* plugin = static_cast<pluginVio2sf*>(context);
        int ret;
        int zerr;
        uLongf usize = 8;
        uLongf rsize = usize;
        unsigned char* udata;
        unsigned char* rdata;

        udata = static_cast<unsigned char*>(malloc(usize));
        if (!udata)
            return -1;

        while (Z_OK != (zerr = uncompress(udata, &usize, zdata, zsize)))
        {
            if (Z_MEM_ERROR != zerr && Z_BUF_ERROR != zerr)
            {
                free(udata);
                return -1;
            }
            if (usize >= 8)
            {
                usize = get_le32(udata + 4) + 8;
                if (usize < rsize)
                {
                    rsize += rsize;
                    usize = rsize;
                }
                else
                    rsize = usize;
            }
            else
            {
                rsize += rsize;
                usize = rsize;
            }
            rdata = static_cast<unsigned char*>(realloc(udata, usize));
            if (!rdata)
            {
                free(udata);
                return -1;
            }
            udata = rdata;
        }

        rdata = static_cast<unsigned char*>(realloc(udata, usize));
        if (!rdata)
        {
            free(udata);
            return -1;
        }

        if (0)
        {
            uLong ccrc = crc32(crc32(0L, Z_NULL, 0), rdata, static_cast<uInt>(usize));
            if (ccrc != zcrc)
                return -1;
        }

        ret = TwosfLoadMap(plugin, issave, rdata, static_cast<unsigned>(usize));
        free(rdata);
        return ret;
    }


    FMOD_CODEC_WAVEFORMAT waveformat;
    Info* info;
    std::unordered_map<std::string, std::string> m_tags;
    int32_t ms_interpolation = 0;
    uint64_t m_length;
    NDS_state m_ndsState = {};

    struct LoaderState
    {
        uint8_t* rom = nullptr;
        uint8_t* state = nullptr;
        size_t rom_size = 0;
        size_t state_size = 0;

        int initial_frames = -1;
        int sync_type = 0;
        int clockdown = 0;
        int arm9_clockdown_level = 0;
        int arm7_clockdown_level = 0;

        void Clear()
        {
            if (rom) free(rom);
            if (state) free(state);
            new(this) LoaderState();
        }

        ~LoaderState() { Clear(); }
    } m_loaderState = {};

    const psf_file_callbacks m_psfFileSystem = {
        "\\/|:",
        this,
        OpenPSF,
        ReadPSF,
        SeekPSF,
        ClosePSF,
        TellPSF
    };

private:
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto* plugin = new pluginVio2sf(codec);

    plugin->info = static_cast<Info*>(userexinfo->userdata);

    unsigned int bytesread;
    FMOD_RESULT result;
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    uint8_t* buffer;
    buffer = new uint8_t[4];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, 4, &bytesread);

    if ((buffer[0] == 'M' && buffer[1] == 'T' && buffer[2] == 'h' && buffer[3] == 'd') || (buffer[0] == 'R' && buffer[1]
        == 'I' && buffer[2] == 'F' && buffer[3] == 'F')) //it's a midi file
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->m_length = 0xffffffff;
    if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x24, nullptr, nullptr, plugin->InfoMetaPSF, plugin, 0,
                 nullptr, nullptr))
    {
        auto extPos = plugin->info->filename.find_last_of('.');
        if (extPos == std::string::npos || _stricmp(plugin->info->filename.c_str() + extPos + 1, "2sflib") != 0)
        {
            if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x24, plugin->TwosfLoad, plugin, nullptr,
                         nullptr, 0, nullptr, nullptr) >= 0)
            {
            }
            else
            {
                return FMOD_ERR_FORMAT;
            }
        }
        else
        {
            return FMOD_ERR_FORMAT;
        }
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }

    int freq = 44100;
    int channels = 2;

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->fileformat = "Nintendo DS";
    plugin->info->plugin = PLUGIN_vio2sf;
    plugin->info->pluginName = PLUGIN_vio2sf_NAME;

    if (keyExists(plugin->m_tags, "title"))
    {
        plugin->info->title = plugin->m_tags["title"];
    }
    if (keyExists(plugin->m_tags, "artist"))
    {
        plugin->info->artist = plugin->m_tags["artist"];
    }
    if (keyExists(plugin->m_tags, "game"))
    {
        plugin->info->game = plugin->m_tags["game"];
    }
    if (keyExists(plugin->m_tags, "copyright"))
    {
        plugin->info->copyright = plugin->m_tags["copyright"];
    }
    if (keyExists(plugin->m_tags, "psfby"))
    {
        plugin->info->ripper = plugin->m_tags["psfby"];
    }
    if (keyExists(plugin->m_tags, "year"))
    {
        plugin->info->date = plugin->m_tags["year"];
    }
    if (keyExists(plugin->m_tags, "volume"))
    {
        plugin->info->volumeAmplificationStr = plugin->m_tags["volume"];
    }
    if (keyExists(plugin->m_tags, "genre"))
    {
        plugin->info->system = plugin->m_tags["genre"];
    }
    if (keyExists(plugin->m_tags, "comment"))
    {
        plugin->info->comments = plugin->m_tags["comment"];
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginVio2sf*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginVio2sf*>(codec->plugindata);
    state_render(&plugin->m_ndsState, static_cast<s16*>(buffer), size);
    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginVio2sf*>(codec->plugindata);

    psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x24, plugin->TwosfLoad, plugin, plugin->InfoMetaPSF, plugin, 0,
             nullptr, nullptr);
    state_deinit(&plugin->m_ndsState);
    state_init(&plugin->m_ndsState);
    plugin->m_ndsState.dwInterpolation = 0;
    plugin->m_ndsState.dwChannelMute = 0;
    plugin->m_ndsState.initial_frames = plugin->m_loaderState.initial_frames;
    plugin->m_ndsState.sync_type = plugin->m_loaderState.sync_type;
    plugin->m_ndsState.arm7_clockdown_level = plugin->m_loaderState.arm7_clockdown_level;
    plugin->m_ndsState.arm9_clockdown_level = plugin->m_loaderState.arm9_clockdown_level;

    if (plugin->m_loaderState.rom)
        state_setrom(&plugin->m_ndsState, plugin->m_loaderState.rom, (u32)plugin->m_loaderState.rom_size, 1);

    state_loadstate(&plugin->m_ndsState, plugin->m_loaderState.state, (u32)plugin->m_loaderState.state_size);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginVio2sf*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS)
    {
        if (plugin->m_length > 0)
        {
            *length = plugin->m_length;
        }
        else
        {
            *length = 0xffffffff;
        }
        plugin->info->numSubsongs = 1;
        return FMOD_OK;
    }
}
