#include <cstring>
#include "asap.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_asap_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_MUTE_VOICE,
    // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    &getPosition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginAsap {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAsap(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAsap() {
        //delete some stuff
        delete[] myBuffer;
        ASAP_Delete(asap);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    ASAP *asap;
    const ASAPInfo *asap_info;
    uint8_t *myBuffer;
    int current_song;
    unsigned int mask = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codec;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto *plugin = new pluginAsap(codec);

    plugin->myBuffer = new uint8_t[filesize];

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    auto *info = static_cast<Info *>(userexinfo->userdata);

    plugin->asap = ASAP_New();

    if (!ASAP_Load(plugin->asap, info->filename.c_str(), plugin->myBuffer, static_cast<int>(filesize))) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->asap_info = ASAP_GetInfo(plugin->asap);

    int song = ASAPInfo_GetDefaultSong(plugin->asap_info);
    plugin->current_song = song;

    if (const int duration = ASAPInfo_GetDuration(plugin->asap_info, song); !
        ASAP_PlaySong(plugin->asap, song, duration)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = ASAPInfo_GetChannels(plugin->asap_info);
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->author = ASAPInfo_GetAuthor(plugin->asap_info);
    info->title = ASAPInfo_GetTitle(plugin->asap_info);
    info->numChannels = 8;
    info->date = ASAPInfo_GetDate(plugin->asap_info);
    info->clockSpeed = !ASAPInfo_IsNtsc(plugin->asap_info);
    info->fileformat = "Unknown ASAP";
    info->plugin = PLUGIN_asap;
    info->pluginName = PLUGIN_asap_NAME;
    info->setSeekable(true);

    if (ASAPInfo_GetOriginalModuleExt(plugin->asap_info, plugin->myBuffer, static_cast<int>(filesize)) == nullptr) {
        info->fileformat = "Slight Atari Player";
    } else {
        info->fileformat = ASAPInfo_GetExtDescription(
            ASAPInfo_GetOriginalModuleExt(plugin->asap_info, plugin->myBuffer, static_cast<int>(filesize)));
    }

    //int numberOfInstruments = 0;
    //    const char *s = ASAPInfo_GetInstrumentName(plugin->asap_info, plugin->myBuffer, filesize, 0);
    //    if (s != NULL)
    //    {
    //        do
    //        {
    //            s = ASAPInfo_GetInstrumentName(plugin->asap_info, plugin->myBuffer, filesize,numberOfInstruments);
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
    //            s = ASAPInfo_GetInstrumentName(plugin->asap_info, plugin->myBuffer, filesize,i);
    //            sprintf(name,s);
    //            info->instruments[i] = name;
    //        }
    //    }
    //    plugin->mask = 0;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginAsap *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginAsap *>(codec->plugindata);

    int bufferedBytes = ASAP_Generate(plugin->asap, static_cast<unsigned char *>(buffer),
                                      static_cast<int>(size) << plugin->waveformat.channels, ASAPSampleFormat_S16_L_E);

    if (bufferedBytes <= 0) {
        return FMOD_ERR_FILE_EOF;
    }

    *read = bufferedBytes;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto const *plugin = static_cast<pluginAsap *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = ASAPInfo_GetDuration(plugin->asap_info, plugin->current_song);
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG) {
        *length = ASAPInfo_GetSongs(plugin->asap_info);
    }

    return FMOD_OK;
}


static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginAsap *>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS) {
        if (ASAP_Seek(plugin->asap, static_cast<int>(position))) {
            ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(plugin->mask));
            return FMOD_OK;
        }
    } else if (postype == FMOD_TIMEUNIT_SUBSONG) {
        int duration = ASAPInfo_GetDuration(plugin->asap_info, static_cast<int>(position));
        ASAP_PlaySong(plugin->asap, static_cast<int>(position), duration);
        plugin->current_song = static_cast<int>(position);
        return FMOD_OK;
    } else if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        //mutes voices
        //position is a mask
        ASAP_MutePokeyChannels(plugin->asap, static_cast<int>(position));
        plugin->mask = position;
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    auto const *plugin = static_cast<pluginAsap *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_SUBSONG) {
        *position = plugin->current_song;
    }

    return FMOD_OK;
}
