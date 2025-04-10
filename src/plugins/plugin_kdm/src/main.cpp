#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "fmod_errors.h"
#include "info.h"
#include "kdmeng.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_kdm_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginKdm
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginKdm(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginKdm()
    {
        //delete some stuff
        delete[] myBuffer;
        //delete m_player;
        myBuffer = 0;
    }

    signed short* myBuffer;
    Info* info;
    //kdmeng* m_player;

    FMOD_CODEC_WAVEFORMAT waveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/

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

    auto plugin = new pluginKdm(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    //    char* smallBuffer;
    //    smallBuffer = new char[2];
    //    unsigned int bytesread;
    //    result = codec->fileseek(codec->filehandle,0,(char*)smallBuffer);

    //    result = codec->fileread(codec->filehandle,(char*)smallBuffer,2,&bytesread,0);


    //    if(!(smallBuffer[0]=='a' && smallBuffer[1]=='y') && !(smallBuffer[0]=='y' && smallBuffer[1]=='m'))
    //    {
    //        delete smallBuffer;
    //        return FMOD_ERR_FORMAT;
    //    }

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);


    plugin->myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

  //  plugin->m_player = new kdmeng(44100, 2, 2);

    int length = kdmload(plugin->info->filename.data());

    if (length < 0)
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned found = plugin->info->filename.find_last_of("/\\");

    long length = plugin->m_player->load((signed short*)plugin->myBuffer, filesize,
                                     plugin->info->filename.substr(0, found + 1).c_str());

    if (!length)
    {
        delete plugin->myBuffer;
        delete plugin->m_player;
        return FMOD_ERR_FORMAT;
    }


    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 1468;
    plugin->waveformat.lengthpcm = length / 1000 * plugin->waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->fileformat = "Ken's Digital Music";
    plugin->info->plugin = PLUGIN_kdm;
    plugin->info->pluginName = PLUGIN_kdm_NAME;
    plugin->info->setSeekable(true);

    int numSamples = plugin->m_player->getNumwaves();
    int numTracks = plugin->m_player->getNumtracks();
    plugin->info->numSamples = numSamples;
    plugin->info->numPatterns = numTracks;


    if (numSamples > 0)
    {
        plugin->info->samplesSize = new unsigned int[numSamples];
        plugin->info->samplesLoopStart = new unsigned int[numSamples];
        plugin->info->samplesLoopLength = new unsigned int[numSamples];
        plugin->info->samplesFineTune = new signed int[numSamples];
        plugin->info->samples = new string[numSamples];
        char* c = new char[17];
        for (int j = 0; j < numSamples; j++)
        {
            plugin->info->samplesSize[j] = plugin->m_player->getInstsize(j);
            plugin->info->samplesLoopStart[j] = plugin->m_player->getInstrepstart(j);
            plugin->info->samplesLoopLength[j] = plugin->m_player->getInstreplength(j);
            plugin->info->samplesFineTune[j] = plugin->m_player->getInstfinetune(j);
            plugin->m_player->getInstname(j, c);
            plugin->info->samples[j] = c;
        }
        delete c;
    }

    if (numTracks > 0)
    {
        plugin->info->instruments = new string[numTracks];
        plugin->info->instrumentsNumber = new char[numTracks];
        plugin->info->instrumentsQuantize = new char[numTracks];
        plugin->info->instrumentsVolume1 = new unsigned char[numTracks];
        plugin->info->instrumentsVolume2 = new unsigned char[numTracks];


        char* c = new char[17];
        for (int j = 0; j < numTracks; j++)
        {
            int instrIdx = plugin->m_player->getTrackInstrument(j);
            plugin->info->instrumentsNumber[j] = instrIdx + 1;
            plugin->info->instruments[j] = plugin->info->samples[instrIdx];
            plugin->info->instrumentsQuantize[j] = plugin->m_player->getTrackQuantize(j);
            plugin->info->instrumentsVolume1[j] = plugin->m_player->getTrackVolume1(j);
            plugin->info->instrumentsVolume2[j] = plugin->m_player->getTrackVolume2(j);
        }
        delete c;
    }

    plugin->m_player->musicon();

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginKdm*>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginKdm*>(codec->plugindata);
    plugin->m_player->rendersound(buffer, size << 2);
    *read = size;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginKdm*>(codec->plugindata);
    plugin->m_player->seek(position);
    return FMOD_OK;
}
