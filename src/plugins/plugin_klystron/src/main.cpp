#include <iostream>
#include <stdio.h>
#include <string.h>
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
    0,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &klystronsetposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class klystronplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    klystronplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&klystronwaveformat, 0, sizeof(klystronwaveformat));
    }

    ~klystronplugin()
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
    FMOD_CODEC_WAVEFORMAT klystronwaveformat;
};

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

    klystronplugin* klystron = new klystronplugin(codec);

    klystron->myBuffer = new signed short[filesize];

    klystron->player = KSND_CreatePlayerUnregistered(44100);
    if (!klystron->player)
    {
        cout << "Error creating Klystron Player!\n";
        flush(cout);
        delete klystron;
        return FMOD_ERR_FORMAT;
    }

    //klystron->song = KSND_LoadSongFromMemory(klystron->player, klystron->myBuffer, filesize);
    klystron->song = KSND_LoadSong(klystron->player, info->filename.c_str());

    if (!klystron->song)
    {
        delete klystron;
        return FMOD_ERR_FORMAT;
    }

    int numPatternRows = KSND_GetSongLength(klystron->song);
    cout << "Song length: " << (KSND_GetPlayTime(klystron->song, numPatternRows) / 1000.0) << "\n";
    flush(cout);
    klystron->klystronwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    klystron->klystronwaveformat.channels = 2;
    klystron->klystronwaveformat.frequency = 44100;
    klystron->klystronwaveformat.pcmblocksize = (16 >> 3) * klystron->klystronwaveformat.channels;
    klystron->klystronwaveformat.lengthpcm = (KSND_GetPlayTime(klystron->song, numPatternRows) / 1000.0) * klystron->
        klystronwaveformat.frequency;

    codec->waveformat = &(klystron->klystronwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = klystron; /* user data value */

    klystron->songinfo = new KSongInfo();
    KSND_GetSongInfo(klystron->song, klystron->songinfo);

    info->title = klystron->songinfo->song_title;
    info->numInstruments = klystron->songinfo->n_instruments;
    info->numChannels = klystron->songinfo->n_channels;
    info->numPatterns = numPatternRows;

    if (info->numInstruments > 0)
    {
        info->instruments = new string[info->numInstruments];
        for (int j = 0; j < info->numInstruments; j++)
        {
            info->instruments[j] = klystron->songinfo->instrument_name[j];
        }
    }

    KSND_PlaySong(klystron->player, klystron->song, 0);

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
    delete (klystronplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK klystronread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    klystronplugin* klystron = (klystronplugin*)codec->plugindata;
    KSND_FillBuffer(klystron->player, (short int*)buffer, size << 2);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK klystronsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                           FMOD_TIMEUNIT postype)
{
    klystronplugin* klystron = (klystronplugin*)codec->plugindata;
    //mus_set_song(&klystron->mus, &klystron->song, 0);
    KSND_PlaySong(klystron->player, klystron->song, 0);
    return FMOD_OK;
}
