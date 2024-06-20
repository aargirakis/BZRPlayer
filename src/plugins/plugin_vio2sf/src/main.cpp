#include <stdio.h>
#include <string.h>
#include <string>
#include <zconf.h>
#include <zlib.h>
#include "fmod_errors.h"
#include "info.h"
#include "main.h"
#include "psflib.h"
#include "state.h"
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
    &getlength,                                   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
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
        state_deinit(&m_ndsState);
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

        int static TwosfLoad(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved, size_t reserved_size)
            {
                auto* This = reinterpret_cast<ahxplugin*>(context);
                if (exe_size >= 8)
                {
                    if (This->TwosfLoadMap(This, 0, exe, (unsigned)exe_size))
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
                            if (This->TwosfLoadMapz(This, 1, reserved + resv_pos + 12, save_size, save_crc))
                                return -1;
                        }
                        resv_pos += 12 + save_size;
                    }
                }

                return 0;
            }

            int static TwosfLoadMap(void* context, int issave, const unsigned char* udata, unsigned usize)
            {
                auto* This = reinterpret_cast<ahxplugin*>(context);
                if (usize < 8) return -1;

                unsigned char* iptr;
                size_t isize;
                unsigned char* xptr;
                unsigned xsize = get_le32(udata + 4);
                unsigned xofs = get_le32(udata + 0);
                if (issave)
                {
                    iptr = This->m_loaderState.state;
                    isize = This->m_loaderState.state_size;
                    This->m_loaderState.state = 0;
                    This->m_loaderState.state_size = 0;
                }
                else
                {
                    iptr = This->m_loaderState.rom;
                    isize = This->m_loaderState.rom_size;
                    This->m_loaderState.rom = 0;
                    This->m_loaderState.rom_size = 0;
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
                    iptr = (unsigned char*)malloc(rsize + 10);
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
                    xptr = (unsigned char*)realloc(iptr, xofs + rsize + 10);
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
                    This->m_loaderState.state = iptr;
                    This->m_loaderState.state_size = isize;
                }
                else
                {
                    This->m_loaderState.rom = iptr;
                    This->m_loaderState.rom_size = isize;
                }
                return 0;
            }

            int static TwosfLoadMapz(void* context, int issave, const unsigned char* zdata, unsigned zsize, unsigned zcrc)
            {
                auto* This = reinterpret_cast<ahxplugin*>(context);
                int ret;
                int zerr;
                uLongf usize = 8;
                uLongf rsize = usize;
                unsigned char* udata;
                unsigned char* rdata;

                udata = (unsigned char*)malloc(usize);
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
                    rdata = (unsigned char*)realloc(udata, usize);
                    if (!rdata)
                    {
                        free(udata);
                        return -1;
                    }
                    udata = rdata;
                }

                rdata = (unsigned char*)realloc(udata, usize);
                if (!rdata)
                {
                    free(udata);
                    return -1;
                }

                if (0)
                {
                    uLong ccrc = crc32(crc32(0L, Z_NULL, 0), rdata, (uInt)usize);
                    if (ccrc != zcrc)
                        return -1;
                }

                ret = TwosfLoadMap(This, issave, rdata, (unsigned)usize);
                free(rdata);
                return ret;
            }


    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
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

                void Clear() { if (rom) free(rom); if (state) free(state); new (this) LoaderState(); }

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

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &tfmxcodec;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

    ahxplugin *ahx = new ahxplugin(codec);

    ahx->info = (Info*)userexinfo->userdata;

    unsigned int bytesread;
    FMOD_RESULT       result;
    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    uint8_t* buffer;
    buffer = new uint8_t[4];
    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,buffer,4,&bytesread);

    if((buffer[0]=='M' && buffer[1]=='T' && buffer[2]=='h' && buffer[3]=='d') || (buffer[0]=='R' && buffer[1]=='I' && buffer[2]=='F' && buffer[3]=='F')) //it's a midi file
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    ahx->m_length = 0xffffffff;
    if(psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0x24, nullptr, nullptr, ahx->InfoMetaPSF, ahx, 0, nullptr, nullptr))
    {
        auto extPos = ahx->info->filename.find_last_of('.');
        if (extPos == std::string::npos || _stricmp(ahx->info->filename.c_str() + extPos + 1, "2sflib") != 0)
        {
            if (psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0x24, ahx->TwosfLoad, ahx, nullptr, nullptr, 0, nullptr, nullptr) >= 0)
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

    ahx->ahxwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    ahx->ahxwaveformat.channels     = channels;
    ahx->ahxwaveformat.frequency    = freq;
    ahx->ahxwaveformat.pcmblocksize   = (16 >> 3) * ahx->ahxwaveformat.channels;
    ahx->ahxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat   = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = ahx;                    /* user data value */

    ahx->info->fileformat = "Nintendo DS";
    ahx->info->plugin = PLUGIN_vio2sf;
    ahx->info->pluginName = PLUGIN_vio2sf_NAME;

    if(keyExists(ahx->m_tags,"title"))
    {
        ahx->info->title =ahx->m_tags["title"];
    }
    if(keyExists(ahx->m_tags,"artist"))
    {
        ahx->info->artist =ahx->m_tags["artist"];
    }
    if(keyExists(ahx->m_tags,"game"))
    {
        ahx->info->game =ahx->m_tags["game"];
    }
    if(keyExists(ahx->m_tags,"copyright"))
    {
        ahx->info->copyright =ahx->m_tags["copyright"];
    }
    if(keyExists(ahx->m_tags,"psfby"))
    {
        ahx->info->ripper =ahx->m_tags["psfby"];
    }
    if(keyExists(ahx->m_tags,"year"))
    {
        ahx->info->date =ahx->m_tags["year"];
    }
    if(keyExists(ahx->m_tags,"volume"))
    {
        ahx->info->volumeAmplificationStr =ahx->m_tags["volume"];
    }
    if(keyExists(ahx->m_tags,"genre"))
    {
        ahx->info->system =ahx->m_tags["genre"];
    }
    if(keyExists(ahx->m_tags,"comment"))
    {
        ahx->info->comments =ahx->m_tags["comment"];
    }
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
    state_render(&ahx->m_ndsState, (s16*)buffer, size);
    *read=size;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    psf_load(ahx->info->filename.c_str(), &ahx->m_psfFileSystem, 0x24, ahx->TwosfLoad, ahx, ahx->InfoMetaPSF, ahx, 0, nullptr, nullptr);
    state_deinit(&ahx->m_ndsState);
    state_init(&ahx->m_ndsState);
    ahx->m_ndsState.dwInterpolation = 0;
    ahx->m_ndsState.dwChannelMute = 0;
    ahx->m_ndsState.initial_frames = ahx->m_loaderState.initial_frames;
    ahx->m_ndsState.sync_type = ahx->m_loaderState.sync_type;
    ahx->m_ndsState.arm7_clockdown_level = ahx->m_loaderState.arm7_clockdown_level;
    ahx->m_ndsState.arm9_clockdown_level = ahx->m_loaderState.arm9_clockdown_level;

    if (ahx->m_loaderState.rom)
                state_setrom(&ahx->m_ndsState, ahx->m_loaderState.rom, (u32)ahx->m_loaderState.rom_size, 1);

    state_loadstate(&ahx->m_ndsState, ahx->m_loaderState.state, (u32)ahx->m_loaderState.state_size);

    return FMOD_OK;

}
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if(lengthtype==FMOD_TIMEUNIT_MS)
    {
        if(ahx->m_length>0)
        {
                *length = ahx->m_length;
        }
        else
        {
                *length = 0xffffffff;
        }
        ahx->info->numSubsongs=1;
        return FMOD_OK;
    }

}
