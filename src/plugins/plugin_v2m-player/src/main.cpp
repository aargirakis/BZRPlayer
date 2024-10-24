#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>
#include "fmod_errors.h"
#include "v2mplayer.h"
#include "v2mconv.h"
#include "sounddef.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_v2m_player_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginV2mPlayer
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginV2mPlayer(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginV2mPlayer()
    {
        //delete some stuff

        player->Close();
        delete player;
        delete[] convertedSong;
    }

    V2MPlayer* player;
    uint8_t* convertedSong;
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


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    Info* info = static_cast<Info*>(userexinfo->userdata);
    FMOD_RESULT result;

    string filename_lowercase = info->filename;
    std::transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(), ::tolower);
    if (filename_lowercase.substr(filename_lowercase.find_last_of(".") + 1) != "v2m" && filename_lowercase.substr(
        filename_lowercase.find_last_of(".") + 1) != "v2")
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    /* Allocate space for buffer. */
    unsigned char* myBuffer = new unsigned char[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    //read whole file to memory
    unsigned int bytesread;
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    auto* plugin = new pluginV2mPlayer(codec);


    sdInit();
    ssbase base;
    int version = CheckV2MVersion(myBuffer, filesize, base);
    if (version < 0)
    {
        delete [] myBuffer;
        return FMOD_ERR_FORMAT;
    }
    //cout << "v2m version: " << version << "\n";
    //flush(cout);


    int converted_length;
    ConvertV2M(myBuffer, filesize, &plugin->convertedSong, &converted_length);
    delete [] myBuffer;


    plugin->player = new V2MPlayer();
    plugin->player->Init();
    if (!plugin->player->Open(plugin->convertedSong))
    {
        return FMOD_ERR_FORMAT;
    }


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 4096;
    plugin->waveformat.lengthpcm = 0xffffffff;

    //cout << "length:" << plugin->player->Length() << "\n";
    //flush(cout);

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->fileformat = "Farbrausch V2M";
    info->setSeekable(false);
    info->plugin = PLUGIN_v2m_player;
    info->pluginName = PLUGIN_v2m_player_NAME;


    sS32* p;
    int pos = plugin->player->CalcPositions(&p);

    //    for(int i = 0;i<=pos;i++)
    //    {
    //        if(i%2==0)
    //        {
    //            cout << p[i] << " ms\n";

    //        }
    //        else
    //        {
    //            cout << "pos: " << std::dec << i << ": " << setfill('0') << setw(8) << std::hex << p[i] <<  std::dec << "\n";
    //        }

    //    }
    //    cout << "done\n";
    //    flush(cout);
    int length;
    if (pos % 2 == 0)
    {
        length = p[pos];
    }
    else
    {
        length = p[pos - 1];
    }
    delete[] p;


    plugin->waveformat.lengthpcm = ((1000 + length) * 2 / 1000.0) * plugin->waveformat.frequency;
    //add one extra second for reverb
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginV2mPlayer*>(codec->plugindata);

    if (plugin)
    {
        plugin->player->Stop();
        plugin->player->Close();
    }
    delete plugin;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginV2mPlayer*>(codec->plugindata);

    plugin->player->Render(static_cast<float*>(buffer), plugin->waveformat.pcmblocksize);

    if (size < plugin->waveformat.pcmblocksize)
    {
        *read = size;
    }
    else
    {
        *read = plugin->waveformat.pcmblocksize;
    }

    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginV2mPlayer*>(codec->plugindata);
    plugin->player->Play(position);
    return FMOD_OK;
}
