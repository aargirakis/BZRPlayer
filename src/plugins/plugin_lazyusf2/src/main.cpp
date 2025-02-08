#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "usf.h"
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
    PLUGIN_lazyusf2_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class ahxplugin
{
    FMOD_CODEC_STATE* _codec;
    FILE* file;

public:
    ahxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
    {
        if (m_lazyState)
        {
            usf_shutdown(m_lazyState);
            delete[] m_lazyState;
        }
    }

    static int InfoMetaPSF(void* context, const char* name, const char* value)
    {
        auto* This = reinterpret_cast<ahxplugin*>(context);
		if (!_stricmp(name, "_enablecompare"))
        {
            This->m_loaderState.enablecompare = 1;
        }
        else if (!_stricmp(name, "_enablefifofull"))
        {
            This->m_loaderState.enablefifofull = 1;
        }
        else if (!_strnicmp(name, "replaygain_", sizeof("replaygain_") - 1))
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
				{
                    This->m_length = length;
					
				}
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
            //This->m_hasLib = true;
        }
        else if (name[0] == '_')
        {
        }
        else
        {
            This->m_tags[name] = value;
        }
        return 0;
    }

    static void* OpenPSF(void* context, const char* uri)
    {
        auto s = reinterpret_cast<ahxplugin*>(context);
        unsigned int filesize;
        FMOD_CODEC_FILE_SIZE(s->_codec, &filesize);
        s->file = fopen(uri, "rb");
        return s->file;
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
        auto* This = reinterpret_cast<ahxplugin*>(context);
        if (exe && exe_size > 0)
            return -1;

        return usf_upload_section(This->m_lazyState, reserved, reserved_size);
    }

    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
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

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{	
    FMOD_RESULT result;
    ahxplugin* ahx = new ahxplugin(codec);
    ahx->info = (Info*)userexinfo->userdata;
	
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
	
    auto psfType = psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0x21, nullptr, nullptr, ahx->InfoMetaPSF,
                            ahx, 0, nullptr, nullptr);
    if (psfType == 0x21)
    {
        auto extPos = ahx->info->filename.find_last_of('.');
        if (extPos == std::string::npos || _stricmp(ahx->info->filename.c_str() + extPos + 1, "usflib") != 0)
        {
			if (ahx->m_lazyState == nullptr)
			{
				ahx->m_lazyState = new uint8_t[usf_get_state_size()];
			}
            usf_clear(ahx->m_lazyState);
            if (psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, uint8_t(psfType), ahx->UsfLoad, ahx,
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


    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    ahx->ahxwaveformat.channels = channels;
    ahx->ahxwaveformat.frequency = freq;
    ahx->ahxwaveformat.pcmblocksize = 2;
    ahx->ahxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */

    ahx->info->plugin = PLUGIN_lazyusf2;
    ahx->info->pluginName = PLUGIN_lazyusf2_NAME;
    ahx->info->fileformat = "N64";

    if (keyExists(ahx->m_tags, "title"))
    {
        ahx->info->title = ahx->m_tags["title"];
    }
    if (keyExists(ahx->m_tags, "artist"))
    {
        ahx->info->artist = ahx->m_tags["artist"];
    }
    if (keyExists(ahx->m_tags, "game"))
    {
        ahx->info->game = ahx->m_tags["game"];
    }
    if (keyExists(ahx->m_tags, "copyright"))
    {
        ahx->info->copyright = ahx->m_tags["copyright"];
    }
    if (keyExists(ahx->m_tags, "psfby"))
    {
        ahx->info->ripper = ahx->m_tags["psfby"];
    }
    if (keyExists(ahx->m_tags, "year"))
    {
        ahx->info->date = ahx->m_tags["year"];
    }
    if (keyExists(ahx->m_tags, "volume"))
    {
        ahx->info->volumeAmplificationStr = ahx->m_tags["volume"];
    }
    if (keyExists(ahx->m_tags, "genre"))
    {
        ahx->info->system = ahx->m_tags["genre"];
    }
    if (keyExists(ahx->m_tags, "comment"))
    {
        ahx->info->comments = ahx->m_tags["comment"];
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    delete ahx;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
	ahxplugin* ahx = (ahxplugin*)codec->plugindata;
	usf_render_resampled(ahx->m_lazyState, (short*)buffer, size, ahx->ahxwaveformat.frequency);
	*read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
	usf_shutdown(ahx->m_lazyState);
    usf_clear(ahx->m_lazyState);
	psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0x21, ahx->UsfLoad, ahx, nullptr, nullptr, 0, nullptr, nullptr);
	usf_set_hle_audio(ahx->m_lazyState, 1);
	usf_set_compare(ahx->m_lazyState, ahx->m_loaderState.enablecompare);
	usf_set_fifo_full(ahx->m_lazyState, ahx->m_loaderState.enablefifofull);
    return FMOD_OK;
}
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_MS)
    {
        if (ahx->m_length > 0)
        {
            *length = ahx->m_length;
        }
        else
        {
            *length = 0xffffffff;
        }
        ahx->info->numSubsongs = 1;
        return FMOD_OK;
    }
}
