#include "StSoundLibrary.h"
#include <cstring>
#include <iostream>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libstsound_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr, // Sound create callback (don't need it)
    nullptr
};

class pluginLibstsound
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibstsound(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibstsound()
    {
        //delete some stuff
        ymMusicDestroy(pMusic);
        delete[] myBuffer;
    }

    YMMUSIC* pMusic;
    FMOD_CODEC_WAVEFORMAT waveformat;
    signed char* myBuffer;
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


static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    auto* plugin = new pluginLibstsound(codec);
    Info* info = static_cast<Info*>(userexinfo->userdata);
    plugin->pMusic = ymMusicCreate();
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    unsigned int bytesread;
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }
    plugin->myBuffer = new signed char[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    if (!ymMusicLoadMemory(plugin->pMusic, static_cast<signed char*>(plugin->myBuffer), filesize))
    {
        ymMusicDestroy(plugin->pMusic);
        delete plugin->myBuffer;
        return FMOD_ERR_FORMAT;
    }
    cout << "we got ym!\n";
    flush(cout);

    ymMusicInfo_t yminfo;
    ymMusicGetInfo(plugin->pMusic, &yminfo);


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;
    if (yminfo.musicTimeInMs > 0)
    {
        plugin->waveformat.lengthpcm = yminfo.musicTimeInMs / 1000 * plugin->waveformat.frequency;
    }


    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    ymMusicSetLoopMode(plugin->pMusic,YMTRUE);


    info->artist = yminfo.pSongAuthor;
    info->title = yminfo.pSongName;
    info->comments = yminfo.pSongComment;
    info->songPlayer = yminfo.pSongPlayer;
    info->songType = yminfo.pSongType;
    info->comments = yminfo.pSongComment;
    info->numSamples = 0;
    info->plugin = PLUGIN_libstsound;
    info->pluginName = PLUGIN_libstsound_NAME;


    plugin->waveformat.channels = 2;
    if (info->songType == "MIX1")
    {
        plugin->waveformat.channels = 1;
    }

    info->fileformat = yminfo.pSongType;
    info->setSeekable(false);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto plugin = static_cast<pluginLibstsound*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibstsound*>(codec->plugindata);

    int nbSample = size;
    ymMusicCompute(plugin->pMusic, static_cast<ymsample*>(buffer), nbSample);
    *read = size;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibstsound*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        ymMusicStop(plugin->pMusic);
        ymMusicPlay(plugin->pMusic);
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_OK;
}
