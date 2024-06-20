#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "usf.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION tfmxcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "USF player plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    1,                                   // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,	              // The time format we would like to accept into setposition/getposition.
    &open,                             // Open callback.
    &close,                            // Close callback.
    &read,                             // Read callback.
    0,                                   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};

class ahxplugin
{
    FMOD_CODEC_STATE *_codec;
    FILE* file;

public:
    ahxplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
    {

        //delete some stuff
    }
    static int InfoMetaPSF(void* context, const char* name, const char* value)
            {
                auto* This = reinterpret_cast<ahxplugin*>(context);
                if (!_strnicmp(name, "replaygain_", sizeof("replaygain_") - 1))
                {}
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
                            This->m_length = length;
                    }
                }
                else if (!_stricmp(name, "fade"))
                {}
                else if (!_stricmp(name, "utf8"))
                {}
                else if (!_stricmp(name, "_lib"))
                {
                    //This->m_hasLib = true;
                }
                else if (name[0] == '_')
                {}
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
            FMOD_CODEC_FILE_SIZE(s->_codec,&filesize);
            s->file = fopen(uri,"rb");
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

        static int UsfLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved, size_t reserved_size)
        {
            auto* This = reinterpret_cast<ahxplugin*>(context);
            if (exe && exe_size > 0)
                return -1;

            return usf_upload_section(This->m_lazyState, reserved, reserved_size);
        }
    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    Info* info;
    std::unordered_map<std::string, std::string> m_tags;
    uint64_t m_length;
    uint8_t* m_lazyState = nullptr;
    struct LoaderState
            {
                uint8_t* data = nullptr;
                size_t data_size = 0;

                void Clear() { new (this) LoaderState(); }

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

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &tfmxcodec;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{


    FMOD_RESULT       result;
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
//    if (filesize < 8 || filesize > 1024*2048) //(2 mb)biggest sndh on modland is 1150960 bytes, don't know real max
//    {
//        return FMOD_ERR_FORMAT;
//    }

    ahxplugin *ahx = new ahxplugin(codec);

    ahx->info = (Info*)userexinfo->userdata;


    result = FMOD_CODEC_FILE_SEEK(codec,0,0);



    ahx->m_length = 0xffffffff;
    auto psfType = psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0, nullptr, nullptr, ahx->InfoMetaPSF, ahx, 0, nullptr, nullptr);
    if (psfType == 0x21)
    {
        auto extPos = ahx->info->filename.find_last_of('.');
        if (extPos == std::string::npos || _stricmp(ahx->info->filename.c_str() + extPos + 1, "usflib") != 0)
        {
            ahx->m_lazyState = new uint8_t[usf_get_state_size()];
            usf_clear(ahx->m_lazyState);
            if (psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, uint8_t(psfType), ahx->UsfLoad, ahx, nullptr, nullptr, 0, nullptr, nullptr) >= 0)
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
cout << "usf ok\n";

    int freq = 44100;
    int channels = 2;


    ahx->ahxwaveformat.format       = FMOD_SOUND_FORMAT_PCMFLOAT;
    ahx->ahxwaveformat.channels     = channels;
    ahx->ahxwaveformat.frequency    = freq;
    ahx->ahxwaveformat.pcmblocksize   = 2;
    ahx->ahxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat   = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = ahx;                    /* user data value */

    ahx->info->plugin = PLUGIN_lazyusf2;
    ahx->info->pluginName = PLUGIN_lazyusf2_NAME;
    ahx->info->fileformat = "N64";

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    delete ahx;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    *read=size;
    return FMOD_OK;
}
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if(postype==FMOD_TIMEUNIT_MS)
    {
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_SUBSONG)
    {
        //ahx->m_subsongIndex = position;
        //ahx->sndh->InitSubSong(ahx->m_subsongIndex+1);
        return FMOD_OK;
    }
    return FMOD_OK;

}
