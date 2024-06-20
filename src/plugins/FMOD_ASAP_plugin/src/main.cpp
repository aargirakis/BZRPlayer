
#include <string>
#include <string.h>
#include "info.h"
#include <stdio.h>
#include <iostream>
#include "fmod_errors.h"

#include "asap.h"


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);
FMOD_CODEC_DESCRIPTION codec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "ASAP Player Plugin",			// Name.
    0x00012300,                         // Version 0xAAAABBBB   A = major, B = minor.
    1,                                  // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS|FMOD_TIMEUNIT_SUBSONG|FMOD_TIMEUNIT_MUTE_VOICE,					// The time format we would like to accept into setposition/getposition.
    &open,                           // Open callback.
    &close,                          // Close callback.
    &read,                           // Read callback.
    &getlength,                      // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                    // Setposition callback.
    &getposition,                    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                // Sound create callback (don't need it)
};
class plugin
{
    FMOD_CODEC_STATE *_codec;
public:
    plugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~plugin()
    {
        //delete some stuff
        delete[] module;
        ASAP_Delete(asap);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    ASAP *asap;
    const ASAPInfo *asap_info;
    unsigned char* module;
    int current_song;
    unsigned int mask;

};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &codec;
}

#ifdef __cplusplus
}
#endif



FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
    Info* info = (Info*)userexinfo->userdata;

    plugin *tp = new plugin(codec);



    unsigned int module_len;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &module_len);


    tp->module = new unsigned char[module_len];

    FMOD_CODEC_FILE_SEEK(codec,0,0);
    FMOD_CODEC_FILE_READ(codec,tp->module,module_len,&bytesread);


        tp->asap = ASAP_New();
        if (!ASAP_Load(tp->asap, info->filename.c_str(), tp->module, module_len))
        {
            ASAP_Delete(tp->asap);
            delete[] tp->module;

            return FMOD_ERR_FORMAT;
        }

        int song;
        int duration;

        tp->asap_info = ASAP_GetInfo(tp->asap);
        song = ASAPInfo_GetDefaultSong(tp->asap_info);
        tp->current_song = song;
        duration = ASAPInfo_GetDuration(tp->asap_info, song);

        if (!ASAP_PlaySong(tp->asap, song, duration))
        {
            ASAP_Delete(tp->asap);
            delete tp;
            return FMOD_ERR_FORMAT;
        }

        tp->waveformat.format       = FMOD_SOUND_FORMAT_PCM16;
        tp->waveformat.channels     = ASAPInfo_GetChannels(tp->asap_info);
        tp->waveformat.frequency    = 44100;
        tp->waveformat.pcmblocksize   = (16 >> 3) * tp->waveformat.channels;
        tp->waveformat.lengthpcm = 0xffffffff;

    //    if(duration!=-1)
    //    {
    //        tp->waveformat.lengthpcm = duration/1000*tp->waveformat.frequency;
    //    }

        codec->waveformat   = &tp->waveformat;
        codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata   = tp;                    /* user data value */


        info->author=ASAPInfo_GetAuthor(tp->asap_info);
        info->title=ASAPInfo_GetTitle(tp->asap_info);
        info->numChannels=8;
        info->date = ASAPInfo_GetDate(tp->asap_info);
        info->clockSpeed =  !ASAPInfo_IsNtsc(tp->asap_info);


        info->fileformat = "Unknown ASAP";
        info->plugin = "ASAP";
        info->setSeekable(true);

        const char* modulext = ASAPInfo_GetOriginalModuleExt(tp->asap_info,tp->module, module_len);
        if(modulext==NULL)
        {
            info->fileformat="Slight Atari Player";
        }
        else
        {
            info->fileformat = ASAPInfo_GetExtDescription(ASAPInfo_GetOriginalModuleExt(tp->asap_info,tp->module, module_len));
        }

        int numberOfInstruments = 0;
    //    const char *s = ASAPInfo_GetInstrumentName(tp->asap_info, tp->module, module_len, 0);
    //    if (s != NULL)
    //    {
    //        do
    //        {
    //            s = ASAPInfo_GetInstrumentName(tp->asap_info, tp->module, module_len,numberOfInstruments);
    //            numberOfInstruments++;
    //        }
    //        while (s != NULL);
    //    }

    //    numberOfInstruments--;
    //    if(numberOfInstruments>0)
    //    {
    //        info->numInstruments = numberOfInstruments;
    //        info->instruments = new string[numberOfInstruments];

    //        const int INSTRUMENT_NAME_MAX_LENGTH = 64;//TODO Check what's the max length, I just took a reasonable number
    //        char name[INSTRUMENT_NAME_MAX_LENGTH];

    //        for(int i = 0; i < numberOfInstruments; i++)
    //        {
    //            s = ASAPInfo_GetInstrumentName(tp->asap_info, tp->module, module_len,i);
    //            sprintf(name,s);
    //            info->instruments[i] = name;
    //        }
    //    }
    //    tp->mask = 0;
        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
    {
        plugin* tp = (plugin*)codec->plugindata;
        delete tp;
        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
    {
        plugin* tp = (plugin*)codec->plugindata;
        *read= ASAP_Generate(tp->asap, (unsigned char*)buffer, size<<tp->waveformat.channels, ASAPSampleFormat_S16_L_E);
        return FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
    {
        plugin* tp = (plugin*)codec->plugindata;

        if(lengthtype==FMOD_TIMEUNIT_SUBSONG_MS || lengthtype==FMOD_TIMEUNIT_MUTE_VOICE)
        {
            *length = ASAPInfo_GetDuration(tp->asap_info,tp->current_song);
            return FMOD_OK;
        }
        else if(lengthtype==FMOD_TIMEUNIT_SUBSONG)
        {
            *length = ASAPInfo_GetSongs(tp->asap_info);
            return FMOD_OK;
        }
        else
        {
            return FMOD_OK;
        }
    }


    FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
    {
        plugin* tp = (plugin*)codec->plugindata;
        if(postype==FMOD_TIMEUNIT_MS)
        {
            if(position>=0)
            {
                if(ASAP_Seek(tp->asap,position))
                {
                    ASAP_MutePokeyChannels(tp->asap,tp->mask);
                    return FMOD_OK;
                }
            }
        }
        else if(postype==FMOD_TIMEUNIT_SUBSONG)
        {
            if(position<0) position = 0;
            int duration = ASAPInfo_GetDuration(tp->asap_info,position);
            ASAP_PlaySong(tp->asap, position, duration);
            ASAP_MutePokeyChannels(tp->asap,tp->mask);
            tp->current_song = position;
            return FMOD_OK;
        }
        else if(postype==FMOD_TIMEUNIT_MUTE_VOICE)
        {
            //mutes voices
            //position is a mask
            ASAP_MutePokeyChannels(tp->asap,position);
            tp->mask = position;
            return FMOD_ERR_UNSUPPORTED;
        }
        return FMOD_ERR_UNSUPPORTED;
    }
    FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype)
    {
        plugin* tp = (plugin*)codec->plugindata;
        if(postype==FMOD_TIMEUNIT_SUBSONG)
        {
            *position = tp->current_song;
            return FMOD_OK;
        }
        else
        {
            return FMOD_OK;
        }
    }
