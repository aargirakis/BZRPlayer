#include <cstdio>
#include <cstring>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "Array.inl.h"
#include "Array.h"
#include "qsound.h"
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
    PLUGIN_highly_quixotic_NAME, // Name.
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

class pluginHighlyQ
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    pluginHighlyQ(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginHighlyQ()
    {
        //delete some stuff
        delete[] m_qsoundState;
    }

    static int InfoMetaPSF(void* context, const char* name, const char* value)
    {
        auto* plugin = static_cast<pluginHighlyQ*>(context);
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
        auto plugin = static_cast<pluginHighlyQ*>(context);
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

    static int QsoundLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* /*reserved*/,
                          size_t /*reserved_size*/)
    {
        auto* plugin = static_cast<pluginHighlyQ*>(context);
        for (;;)
        {
            char s[4];
            if (exe_size < 11) break;
            memcpy(s, exe, 3);
            exe += 3;
            exe_size -= 3;
            s[3] = 0;
            uint32_t dataofs = *(uint32_t*)exe;
            exe += 4;
            exe_size -= 4;
            uint32_t datasize = *(uint32_t*)exe;
            exe += 4;
            exe_size -= 4;
            if (datasize > exe_size)
                return -1;

            {
                char* section = s;
                uint32_t start = dataofs;
                const uint8_t* data = exe;
                uint32_t size = datasize;

                core::Array<uint8_t>* pArray = NULL;
                core::Array<valid_range>* pArrayValid = NULL;
                uint32_t maxsize = 0x7FFFFFFF;

                if (!strcmp(section, "KEY"))
                {
                    pArray = &plugin->m_aKey;
                    pArrayValid = &plugin->m_aKeyValid;
                    maxsize = 11;
                }
                else if (!strcmp(section, "Z80"))
                {
                    pArray = &plugin->m_aZ80ROM;
                    pArrayValid = &plugin->m_aZ80ROMValid;
                }
                else if (!strcmp(section, "SMP"))
                {
                    pArray = &plugin->m_aSampleROM;
                    pArrayValid = &plugin->m_aSampleROMValid;
                }
                else
                {
                    return -1;
                }

                if ((start + size) < start)
                {
                    return -1;
                }

                uint32_t newsize = start + size;
                uint32_t oldsize = pArray->NumItems();
                if (newsize > maxsize)
                {
                    return -1;
                }

                if (newsize > oldsize)
                {
                    pArray->Resize(newsize);
                    memset(pArray->Items() + oldsize, 0, newsize - oldsize);
                }

                memcpy(pArray->Items() + start, data, size);

                oldsize = uint32_t(pArrayValid->NumItems());
                pArrayValid->Resize(oldsize + 1);
                valid_range& range = (pArrayValid->Items())[oldsize];
                range.start = start;
                range.size = size;
            }

            exe += datasize;
            exe_size -= datasize;
        }

        return 0;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info* info;
    std::unordered_map<std::string, std::string> m_tags;
    uint8_t* m_qsoundState = nullptr;
    uint64_t m_length;
    core::Array<uint8_t> m_aKey;
    core::Array<valid_range> m_aKeyValid;
    core::Array<uint8_t> m_aZ80ROM;
    core::Array<valid_range> m_aZ80ROMValid;
    core::Array<uint8_t> m_aSampleROM;
    core::Array<valid_range> m_aSampleROMValid;

    struct LoaderState
    {
        uint8_t* data = nullptr;
        size_t data_size = 0;

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
    auto* plugin = new pluginHighlyQ(codec);

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
    if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x41, nullptr, nullptr, plugin->InfoMetaPSF, plugin, 0,
                 nullptr, nullptr))
    {
        auto extPos = plugin->info->filename.find_last_of('.');
        if (extPos == std::string::npos || _stricmp(plugin->info->filename.c_str() + extPos + 1, "qsflib") != 0)
        {
            if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x41, plugin->QsoundLoad, plugin, nullptr,
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
    plugin->m_qsoundState = new uint8_t[qsound_get_state_size()];
    qsound_init();

    int freq = 24038;
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

    plugin->info->fileformat = "Capcom QSound";

    plugin->info->plugin = PLUGIN_highly_quixotic;
    plugin->info->pluginName = PLUGIN_highly_quixotic_NAME;

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
    auto* plugin = static_cast<pluginHighlyQ*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginHighlyQ*>(codec->plugindata);
    unsigned int numSamples;

    if (qsound_execute(plugin->m_qsoundState, 0x7fffffff, static_cast<short*>(buffer), &size) <= 0)
    {
        *read = 0;
    }
    else
    {
        *read = size;
    }


    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    pluginHighlyQ* plugin = static_cast<pluginHighlyQ*>(codec->plugindata);
    qsound_clear_state(plugin->m_qsoundState);

    if (plugin->m_aKey.NumItems() == 11)
    {
        uint8_t* ptr = plugin->m_aKey.Items();
        uint32_t swap_key1 = *reinterpret_cast<uint32_t*>(ptr + 0);
        uint32_t swap_key2 = *reinterpret_cast<uint32_t*>(ptr + 4);
        uint32_t addr_key = *reinterpret_cast<uint32_t*>(ptr + 8);
        uint8_t xor_key = *(ptr + 10);
        qsound_set_kabuki_key(plugin->m_qsoundState, swap_key1, swap_key2, static_cast<uint16_t>(addr_key), xor_key);
    }
    else
    {
        qsound_set_kabuki_key(plugin->m_qsoundState, 0, 0, 0, 0);
    }
    qsound_set_z80_rom(plugin->m_qsoundState, plugin->m_aZ80ROM.Items(), uint32_t(plugin->m_aZ80ROM.NumItems()));
    qsound_set_sample_rom(plugin->m_qsoundState, plugin->m_aSampleROM.Items(), uint32_t(plugin->m_aSampleROM.NumItems()));
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginHighlyQ*>(codec->plugindata);

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
