#include <climits>
#include <cstring>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "plugins.h"
#include "psf2fs.h"
#include "psx.h"
#include "iop.h"
#include "bios.h"
#include "hebios.h"
#include "r3000.h"

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_highly_experimental_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginHighlyExp
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    pluginHighlyExp(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginHighlyExp()
    {
        //delete some stuff
        if (m_psf2fs)
        {
            psf2fs_delete(m_psf2fs);
        }
        delete[] m_psxState;
    }


    static int InfoMetaPSF(void* context, const char* name, const char* value)
    {
        auto* plugin = static_cast<pluginHighlyExp*>(context);
        if (!strcasecmp(name, "_refresh"))
        {
            sscanf(value, "%u", &plugin->m_loaderState.refresh);
        }
        else if (!strncasecmp(name, "replaygain_", sizeof("replaygain_") - 1))
        {
        }
        else if (!strcasecmp(name, "length"))
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
                {
                    plugin->m_length = length;
                }
            }
        }
        else if (!strcasecmp(name, "fade"))
        {
        }
        else if (!strcasecmp(name, "utf8"))
        {
        }
        else if (!strcasecmp(name, "_lib"))
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

    static int PsfLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* /*reserved*/,
                       size_t /*reserved_size*/)
    {
        auto* plugin = static_cast<pluginHighlyExp*>(context);
        auto psx = (psxexe_hdr_t*)exe;

        if (exe_size < 0x800)
            return -1;
        if (exe_size > UINT_MAX)
            return -1;

        uint32_t addr = get_le32(&psx->exec.t_addr);
        uint32_t size = (uint32_t)exe_size - 0x800;

        addr &= 0x1fffff;
        if ((addr < 0x10000) || (size > 0x1f0000) || (addr + size > 0x200000))
            return -1;

        void* pIOP = psx_get_iop_state(plugin->m_psxState);
        iop_upload_to_ram(pIOP, addr, exe + 0x800, size);

        if (!plugin->m_loaderState.refresh)
        {
            if (!strncasecmp((const char*)exe + 113, "Japan", 5))
                plugin->m_loaderState.refresh = 60;
            else if (!strncasecmp((const char*)exe + 113, "Europe", 6))
                plugin->m_loaderState.refresh = 50;
            else if (!strncasecmp((const char*)exe + 113, "North America", 13))
                plugin->m_loaderState.refresh = 60;
        }

        if (plugin->m_loaderState.first)
        {
            void* pR3000 = iop_get_r3000_state(pIOP);
            r3000_setreg(pR3000, R3000_REG_PC, get_le32(&psx->exec.pc0));
            r3000_setreg(pR3000, R3000_REG_GEN + 29, get_le32(&psx->exec.s_ptr));
            plugin->m_loaderState.first = false;
        }

        return 0;
    }

    static void* OpenPSF(void* context, const char* uri)
    {
        auto plugin = static_cast<pluginHighlyExp*>(context);
        unsigned int filesize;
        FMOD_CODEC_FILE_SIZE(plugin->_codec, &filesize);
        plugin->file = fopen(uri, "rb");
        return plugin->file;
    }

    static size_t ReadPSF(void* buffer, size_t size, size_t count, void* handle)
    {
        return fread(buffer, size, count, (FILE*)handle);
    }

    static int SeekPSF(void* handle, int64_t offset, int whence)
    {
        return fseek((FILE*)handle, offset, whence);
    }

    static int ClosePSF(void* handle)
    {
        return fclose((FILE*)handle);
    }

    static long TellPSF(void* handle)
    {
        return ftell((FILE*)handle);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    uint8_t* m_psxState = nullptr;
    void* m_psf2fs = nullptr;
    int psfType = 0;
    Info* info;
    std::unordered_map<std::string, std::string> m_tags;
    uint64_t m_length;

    struct LoaderState
    {
        bool first = true;
        uint32_t refresh = 0;

        void Clear() { new(this) LoaderState(); }

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
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto* plugin = new pluginHighlyExp(codec);

    plugin->info = static_cast<Info*>(userexinfo->userdata);

    unsigned int bytesread;

    FMOD_RESULT result;

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

    int freq = 44100;
    int channels = 2;

    plugin->psfType = psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0, nullptr, nullptr, plugin->InfoMetaPSF,
                            plugin, 1, nullptr, nullptr);

    if (plugin->psfType != 1 && plugin->psfType != 2)
    {
        return FMOD_ERR_FORMAT;
    }
    auto extPos = plugin->info->filename.find_last_of('.');
    if (extPos == std::string::npos || strcasecmp(plugin->info->filename.c_str() + extPos + 1,
                                                plugin->psfType == 0x1 ? "psflib" : "psf2lib") != 0)
    {
        bios_set_image(hebios, HEBIOS_SIZE);
        psx_init();

        plugin->m_psxState = new uint8_t[psx_get_state_size(uint8_t(plugin->psfType))];
        psx_clear_state(plugin->m_psxState, uint8_t(plugin->psfType));

        if (plugin->psfType == 2)
        {
            plugin->m_psf2fs = psf2fs_create();
        }

        if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, uint8_t(plugin->psfType),
                     plugin->psfType == 1 ? plugin->PsfLoad : psf2fs_load_callback, plugin->psfType == 1 ? plugin : plugin->m_psf2fs,
                     nullptr, nullptr, 0, nullptr, nullptr) >= 0)
        {
            //m_psfType = uint8_t(psfType);
            //m_mediaType.ext = psfType == 0x1 ? m_hasLib ? eExtension::_minipsf : eExtension::_psf : m_hasLib ? eExtension::_minipsf2: eExtension::_psf2;
            //m_subsongs.Add({ 0, uint32_t(m_length) });
            //cout << "psfLoad success!\n";
            //flush(cout);
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


    if (plugin->m_psxState == nullptr)
    {
        plugin->m_psxState = new uint8_t[psx_get_state_size(plugin->psfType)];

        if (plugin->psfType == 2)
        {
            plugin->m_psf2fs = psf2fs_create();
        }
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = 48000;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = plugin->m_length * plugin->waveformat.frequency / 1000; //0xffffffff;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->plugin = PLUGIN_highly_experimental;
    plugin->info->pluginName = PLUGIN_highly_experimental_NAME;
    if (plugin->psfType == 1)
    {
        plugin->waveformat.frequency = 44100;
        plugin->info->fileformat = "PS1";
    }
    else if (plugin->psfType == 2)
    {
        plugin->info->fileformat = "PS2";
    }
    plugin->info->numSamples = 0;

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

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginHighlyExp*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginHighlyExp*>(codec->plugindata);
    unsigned int numSamples;
    int maybeerror = psx_execute(plugin->m_psxState, 0x7fffffff, (short*)buffer, &size, 0);
    //    cout << "numSamples: " << numSamples << "\n";
    //    cout << "maybeerror: " << maybeerror << "\n";
    //    cout << "size: " << size << "\n";
    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginHighlyExp*>(codec->plugindata);

    psx_clear_state(plugin->m_psxState, static_cast<uint8_t>(plugin->psfType));
    if (plugin->m_psf2fs)
    {
        psf2fs_delete(plugin->m_psf2fs);
        plugin->m_psf2fs = psf2fs_create();
    }
    plugin->m_loaderState.Clear();
    psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, uint8_t(plugin->psfType),
             plugin->psfType == 1 ? plugin->PsfLoad : psf2fs_load_callback, plugin->psfType == 1 ? plugin : plugin->m_psf2fs, nullptr,
             nullptr, 0, nullptr, nullptr);
    if (plugin->m_loaderState.refresh)
    {
        psx_set_refresh(plugin->m_psxState, plugin->m_loaderState.refresh);
    }
    if (plugin->psfType == 2)
    {
        struct
        {
            static int EMU_CALL virtual_readfile(void* context, const char* path, int offset, char* buffer, int length)
            {
                return psf2fs_virtual_readfile(context, path, offset, buffer, length);
            }
        } cb;
        psx_set_readfile(plugin->m_psxState, cb.virtual_readfile, plugin->m_psf2fs);
    }
    void* pIOP = psx_get_iop_state(plugin->m_psxState);
    iop_set_compat(pIOP, IOP_COMPAT_HARSH);

    return FMOD_OK;
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginHighlyExp*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS || lengthtype == FMOD_TIMEUNIT_MS_REAL)
    {
        if (plugin->m_length > 0)
        {
            *length = plugin->m_length;
        }
        else
        {
            *length = 0xffffffff;
        }
        *length = 0xffffffff;
        return FMOD_OK;
    }
}
