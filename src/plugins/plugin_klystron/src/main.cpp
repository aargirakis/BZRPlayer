#include <iostream>
#include <cstring>
#ifdef __cplusplus
extern "C" {
#endif

#include "lib/ksnd.h"
#include "snd/music.h"
#ifdef __cplusplus
}
#endif

#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK klystronopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK klystronclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK klystronread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK klystronsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                           FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_klystron_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &klystronopen, // Open callback.
    &klystronclose, // Close callback.
    &klystronread, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &klystronsetposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginKlystron
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginKlystron(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginKlystron()
    {
        if (song) KSND_FreeSong(song);
        if (player) KSND_FreePlayer(player);
        delete songinfo;
        delete[] myBuffer;
    }

    signed short* myBuffer;
    KPlayer* player;
    KSongInfo* songinfo;
    KSong* song;
    FMOD_CODEC_WAVEFORMAT waveformat;
};

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

FMOD_RESULT F_CALLBACK klystronopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    Info* info = static_cast<Info*>(userexinfo->userdata);
    FMOD_RESULT result;


    unsigned int bytesread;

    char smallBuffer[9];
    smallBuffer[8] = '\0';
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 8, &bytesread);

    if (strcmp(smallBuffer, MUS_SONG_SIG) != 0)
    {
        return FMOD_ERR_FORMAT;
    }

    auto* plugin = new pluginKlystron(codec);

    plugin->myBuffer = new signed short[filesize];

    plugin->player = KSND_CreatePlayerUnregistered(44100);
    if (!plugin->player)
    {
        cout << "Error creating Klystron Player!\n";
        flush(cout);
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    //plugin->song = KSND_LoadSongFromMemory(plugin->player, plugin->myBuffer, filesize);
    plugin->song = KSND_LoadSong(plugin->player, info->filename.c_str());

    if (!plugin->song)
    {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    int numPatternRows = KSND_GetSongLength(plugin->song);
    cout << "Song length: " << (KSND_GetPlayTime(plugin->song, numPatternRows) / 1000.0) << "\n";
    flush(cout);
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = (KSND_GetPlayTime(plugin->song, numPatternRows) / 1000.0) * plugin->
        waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->songinfo = new KSongInfo();
    KSND_GetSongInfo(plugin->song, plugin->songinfo);

    info->title = plugin->songinfo->song_title;
    info->numInstruments = plugin->songinfo->n_instruments;
    info->numChannels = plugin->songinfo->n_channels;
    info->numPatterns = numPatternRows;

    if (info->numInstruments > 0)
    {
        info->instruments = new string[info->numInstruments];
        for (int j = 0; j < info->numInstruments; j++)
        {
            info->instruments[j] = plugin->songinfo->instrument_name[j];
        }
    }

    KSND_PlaySong(plugin->player, plugin->song, 0);

    info->fileformat = "Klystron";
    info->plugin = PLUGIN_klystron;
    info->pluginName = PLUGIN_klystron_NAME;
    info->setSeekable(false);
    cout << "klystron open done\n";
    flush(cout);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK klystronclose(FMOD_CODEC_STATE* codec)
{
    cout << "klystronclose\n";
    flush(cout);
    delete static_cast<pluginKlystron*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK klystronread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginKlystron*>(codec->plugindata);
    KSND_FillBuffer(plugin->player, static_cast<short int*>(buffer), size << 2);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK klystronsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                           FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginKlystron*>(codec->plugindata);
    //mus_set_song(&plugin->mus, &plugin->song, 0);
    KSND_PlaySong(plugin->player, plugin->song, 0);
    return FMOD_OK;
}
