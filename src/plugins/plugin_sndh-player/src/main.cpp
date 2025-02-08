#include <cstdio>
#include <cstring>
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

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_sndh_player_NAME, // Name.
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
    nullptr // Sound create callback (don't need it)
};

class pluginSndhPlayer
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginSndhPlayer(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSndhPlayer()
    {
        delete sndh;
        //delete some stuff
    }


    FMOD_CODEC_WAVEFORMAT waveformat;
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
    return &codecDescription;
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

    auto* plugin = new pluginSndhPlayer(codec);

    plugin->info = static_cast<Info*>(userexinfo->userdata);
    int freq = 44100;

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    uint8_t* buffer;
    buffer = new uint8_t[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, buffer, filesize, &bytesread);
    plugin->sndh = new SndhFile();
    bool ok = plugin->sndh->Load(buffer, filesize, freq);
    if (!ok || !plugin->sndh->IsLoaded())
    {
        delete[] buffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] buffer;
    plugin->BuildHash(plugin->sndh);
    plugin->sndh->InitSubSong(1);


    int channels = 1;


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->numChannels = 4;
    plugin->info->plugin = PLUGIN_sndh_player;
    plugin->info->pluginName = PLUGIN_sndh_player_NAME;
    plugin->info->fileformat = "SNDH";
    //plugin->info->waveformDisplay = new uint32_t[25600];
    //memset(plugin->info->waveformDisplay, 0, 25600 * sizeof(plugin->info->waveformDisplay));
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginSndhPlayer*>(codec->plugindata);

    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginSndhPlayer*>(codec->plugindata);
    auto* osc = new uint32_t[18000];
    memset(osc, 0, 18000 * sizeof(osc));
    plugin->sndh->AudioRender(static_cast<int16_t*>(buffer), size, osc);
    plugin->oscBuffer.push(osc);
    plugin->info->waveformDisplay = plugin->oscBuffer.front();
    if (plugin->oscBuffer.size() >= 60)
    {
        uint32_t* o = plugin->oscBuffer.front();
        delete[] o;
        plugin->oscBuffer.pop();
    }
    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginSndhPlayer*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->m_subsongIndex = position;
        plugin->sndh->InitSubSong(plugin->m_subsongIndex + 1);
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginSndhPlayer*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        SndhFile::SubSongInfo subsongInfo;
        plugin->sndh->GetSubsongInfo(plugin->m_subsongIndex + 1, subsongInfo);

        if (subsongInfo.musicAuthor != nullptr)
        {
            plugin->info->artist = subsongInfo.musicAuthor;
        }
        if (subsongInfo.musicName != nullptr)
        {
            plugin->info->title = subsongInfo.musicName;
        }
        if (subsongInfo.ripper != nullptr)
        {
            plugin->info->ripper = subsongInfo.ripper;
        }
        if (subsongInfo.converter != nullptr)
        {
            plugin->info->converter = subsongInfo.converter;
        }
        if (subsongInfo.year != nullptr)
        {
            plugin->info->date = subsongInfo.year;
        }

        plugin->info->clockSpeed = subsongInfo.playerTickRate;
        unsigned int ticks = subsongInfo.playerTickCount;
        if (ticks == 0)
        {
            ticks = plugin->GetTickCountFromSc68();
        }
        unsigned int milliseconds = (ticks * subsongInfo.samplePerTick) / (plugin->waveformat.frequency / 1000);
        *length = milliseconds;
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->sndh->GetSubsongCount();
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginSndhPlayer*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_WAVEFORM)
    {
        return FMOD_OK;
    }
}
