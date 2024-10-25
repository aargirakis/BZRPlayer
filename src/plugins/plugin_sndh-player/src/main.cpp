#include <stdio.h>
#include <string.h>
#include <string>
#include <queue>
#include "fmod_errors.h"
#include "info.h"
#include "SndhFile.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION tfmxcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "USF player plugin", // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_WAVEFORM,
    // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    &getposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class ahxplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    ahxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
    {
        delete sndh;
        //delete some stuff
    }


    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    Info* info;
    SndhFile* sndh;
    queue<uint32_t*> oscBuffer;
    unsigned int m_subsongIndex = 0;
    uint32_t m_hash = 0;

    int32_t GetTickCountFromSc68() const
    {
#define HBIT 32                         /* # of bit for hash     */
#define TBIT 6                          /* # of bit for track    */
#define WBIT 6                          /* # of bit for hardware */
#define FBIT (64-HBIT-TBIT-WBIT)        /* # of bit for frames   */
#define HFIX (32-HBIT)

#define TIMEDB_ENTRY(HASH,TRACK,FRAMES,FLAGS) \
                { 0x##HASH>>HFIX, TRACK-1, FLAGS, FRAMES }
#define E_EMPTY { 0,0,0,0 }

        typedef struct
        {
            unsigned int hash : HBIT; /* hash code              */
            unsigned int track : TBIT; /* track number (0-based) */
            unsigned int flags : WBIT; /* see enum               */
            unsigned int frames : FBIT; /* length in frames       */
        } dbentry_t;

#define STE 0
#define YM  0
#define TA  0
#define TB  0
#define TC  0
#define TD  0
#define NA  0

        static dbentry_t s_db[] = {
#           include "timedb.inc.h"
        };

        dbentry_t e;
        e.hash = m_hash >> HFIX;
        e.track = m_subsongIndex;
        if (auto* s = reinterpret_cast<dbentry_t*>(bsearch(&e, s_db, sizeof(s_db) / sizeof(dbentry_t),
                                                           sizeof(dbentry_t), [](const void* ea, const void* eb)
                                                           {
                                                               auto* a = reinterpret_cast<const dbentry_t*>(ea);
                                                               auto* b = reinterpret_cast<const dbentry_t*>(eb);

                                                               int v = a->hash - b->hash;
                                                               if (!v)
                                                                   v = a->track - b->track;
                                                               return v;
                                                           })))
            return s->frames;
        return 0;
    }

    void BuildHash(SndhFile* sndh)
    {
        // Hash taken from sc68
        uint32_t h = 0;
        int n = 32;
        const uint8_t* k = reinterpret_cast<const uint8_t*>(sndh->GetRawData());
        do
        {
            h += *k++;
            h += h << 10;
            h ^= h >> 6;
        }
        while (--n);

        n = sndh->GetRawDataSize();
        k = reinterpret_cast<const uint8_t*>(sndh->GetRawData());
        do
        {
            h += *k++;
            h += h << 10;
            h ^= h >> 6;
        }
        while (--n);
        m_hash = h;
    }
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
    return &tfmxcodec;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize < 8 || filesize > 1024 * 2048) //(2 mb)biggest sndh on modland is 1150960 bytes, don't know real max
    {
        return FMOD_ERR_FORMAT;
    }

    ahxplugin* ahx = new ahxplugin(codec);

    ahx->info = (Info*)userexinfo->userdata;
    int freq = 44100;

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    uint8_t* buffer;
    buffer = new uint8_t[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, filesize, &bytesread);
    ahx->sndh = new SndhFile();
    bool ok = ahx->sndh->Load(buffer, filesize, freq);
    if (!ok || !ahx->sndh->IsLoaded())
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] buffer;
    ahx->BuildHash(ahx->sndh);
    ahx->sndh->InitSubSong(1);


    int channels = 1;


    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    ahx->ahxwaveformat.channels = channels;
    ahx->ahxwaveformat.frequency = freq;
    ahx->ahxwaveformat.pcmblocksize = (16 >> 3) * ahx->ahxwaveformat.channels;
    ahx->ahxwaveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */

    ahx->info->numChannels = 4;
    ahx->info->plugin = PLUGIN_sndh_player;
    ahx->info->pluginName = PLUGIN_sndh_player_NAME;
    ahx->info->fileformat = "SNDH";
    //ahx->info->waveformDisplay = new uint32_t[25600];
    //memset(ahx->info->waveformDisplay, 0, 25600 * sizeof(ahx->info->waveformDisplay));
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
    uint32_t* osc = new uint32_t[18000];
    memset(osc, 0, 18000 * sizeof(osc));
    ahx->sndh->AudioRender((int16_t*)buffer, size, osc);
    ahx->oscBuffer.push(osc);
    ahx->info->waveformDisplay = ahx->oscBuffer.front();
    if (ahx->oscBuffer.size() >= 60)
    {
        uint32_t* o = ahx->oscBuffer.front();
        delete[] o;
        ahx->oscBuffer.pop();
    }
    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MS)
    {
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        ahx->m_subsongIndex = position;
        ahx->sndh->InitSubSong(ahx->m_subsongIndex + 1);
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        SndhFile::SubSongInfo subsongInfo;
        ahx->sndh->GetSubsongInfo(ahx->m_subsongIndex + 1, subsongInfo);

        if (subsongInfo.musicAuthor != nullptr)
        {
            ahx->info->artist = subsongInfo.musicAuthor;
        }
        if (subsongInfo.musicName != nullptr)
        {
            ahx->info->title = subsongInfo.musicName;
        }
        if (subsongInfo.ripper != nullptr)
        {
            ahx->info->ripper = subsongInfo.ripper;
        }
        if (subsongInfo.converter != nullptr)
        {
            ahx->info->converter = subsongInfo.converter;
        }
        if (subsongInfo.year != nullptr)
        {
            ahx->info->date = subsongInfo.year;
        }

        ahx->info->clockSpeed = subsongInfo.playerTickRate;
        unsigned int ticks = subsongInfo.playerTickCount;
        if (ticks == 0)
        {
            ticks = ahx->GetTickCountFromSc68();
        }
        unsigned int milliseconds = (ticks * subsongInfo.samplePerTick) / (ahx->ahxwaveformat.frequency / 1000);
        *length = milliseconds;
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = ahx->sndh->GetSubsongCount();
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_WAVEFORM)
    {
        return FMOD_OK;
    }
}
