#include <cstdio>
#include <cstring>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "usf.h"
#include "plugins.h"

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

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
    &getlength,   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLazyusf2
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    pluginLazyusf2(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLazyusf2()
    {
        if (m_lazyState)
        {
            usf_shutdown(m_lazyState);
            delete[] m_lazyState;
        }
    }

    static int InfoMetaPSF(void* context, const char* name, const char* value)
    {
        auto* plugin = static_cast<pluginLazyusf2*>(context);
		if (!strcasecmp(name, "_enablecompare"))
        {
            plugin->m_loaderState.enablecompare = 1;
        }
        else if (!strcasecmp(name, "_enablefifofull"))
        {
            plugin->m_loaderState.enablefifofull = 1;
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

    static void* OpenPSF(void* context, const char* uri)
    {
        auto plugin = static_cast<pluginLazyusf2*>(context);
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

    static int UsfLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved,
                       size_t reserved_size)
    {
        auto* plugin = static_cast<pluginLazyusf2*>(context);
        if (exe && exe_size > 0)
            return -1;

        return usf_upload_section(plugin->m_lazyState, reserved, reserved_size);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info* info;
    std::unordered_map<std::string, std::string> m_tags;
    uint64_t m_length=0;
    uint8_t* m_lazyState = nullptr;

        struct LoaderState
        {
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

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{	
    FMOD_RESULT result;
    auto* plugin = new pluginLazyusf2(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

	unsigned int bytesread;

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
	delete[] buffer;

    auto psfType = psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x21, nullptr, nullptr, plugin->InfoMetaPSF,
                            plugin, 0, nullptr, nullptr);
    if (psfType == 0x21)
    {
        auto extPos = plugin->info->filename.find_last_of('.');
        if (extPos == std::string::npos || strcasecmp(plugin->info->filename.c_str() + extPos + 1, "usflib") != 0)
        {
			if (plugin->m_lazyState == nullptr)
			{
				plugin->m_lazyState = new uint8_t[usf_get_state_size()];
			}
            usf_clear(plugin->m_lazyState);
            if (psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, uint8_t(psfType), plugin->UsfLoad, plugin,
                         nullptr, nullptr, 0, nullptr, nullptr) >= 0)
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
    plugin->waveformat.pcmblocksize = 2;
    plugin->waveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->plugin = PLUGIN_lazyusf2;
    plugin->info->pluginName = PLUGIN_lazyusf2_NAME;
    plugin->info->fileformat = "N64";

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
    auto* plugin = static_cast<pluginLazyusf2*>(codec->plugindata);

    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
	auto* plugin = static_cast<pluginLazyusf2*>(codec->plugindata);
	usf_render_resampled(plugin->m_lazyState, static_cast<short*>(buffer), size, plugin->waveformat.frequency);
	*read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLazyusf2*>(codec->plugindata);
	usf_shutdown(plugin->m_lazyState);
    usf_clear(plugin->m_lazyState);
	psf_load(plugin->info->filename.c_str(), &plugin->m_psfFileSystem, 0x21, plugin->UsfLoad, plugin, nullptr, nullptr, 0, nullptr, nullptr);
	usf_set_hle_audio(plugin->m_lazyState, 1);
	usf_set_compare(plugin->m_lazyState, plugin->m_loaderState.enablecompare);
	usf_set_fifo_full(plugin->m_lazyState, plugin->m_loaderState.enablefifofull);
    return FMOD_OK;
}
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginLazyusf2*>(codec->plugindata);

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
