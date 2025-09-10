#include <gme.h>
#include "Music_Emu.h"
#include <blargg_endian.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include "fmod.h"
#include "info.h"
#include "plugins.h"

using namespace std;


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        //LogFile->Print("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

FMOD_RESULT handle_error(const char* str)
{
    if (str)
    {
        return FMOD_ERR_INTERNAL;
    }
    else
        return FMOD_OK;
}

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);
FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_game_music_emu_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_TEMPO | FMOD_TIMEUNIT_MUTE_VOICE,
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

class pluginGme
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginGme(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("gameemu.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginGme()
    {
        //delete some stuff
        gme_delete(emu);
    }

    Music_Emu* emu;
    FMOD_CODEC_WAVEFORMAT waveformat;
    gme_info_t* info;
    int track;
    Info* bzrInfo;
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


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    auto plugin = new pluginGme(codec);


    //read config from disk

    plugin->bzrInfo = static_cast<Info*>(userexinfo->userdata);
    string filename = plugin->bzrInfo->userPath + PLUGINS_CONFIG_DIR + "/gameemu.cfg";
    ifstream ifs(filename.c_str());
    string line;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    int freq = 44100;
    bool ignore_silence = false;
    double treble = 0;
    long bass = 15;
    double stereoDepth = 0.0;
    gme_equalizer_t eq = {treble, bass};


    float tempo = 1.0;

    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("frequency") == 0)
                {
                    freq = atoi(value.c_str());
                }
                else if (word.compare("ignore_silence") == 0)
                {
                    if (value.compare("yes") == 0)
                    {
                        ignore_silence = true;;
                    }
                    else if (value.compare("no") == 0)
                    {
                        ignore_silence = false;
                    }
                }

                else if (word.compare("tempo") == 0)
                {
                    tempo = atof(value.c_str()) / 100.0;
                }
                else if (word.compare("treble") == 0)
                {
                    treble = atoi(value.c_str());
                }
                else if (word.compare("bass") == 0)
                {
                    bass = atoi(value.c_str());
                }
                else if (word.compare("stereo_depth") == 0)
                {
                    stereoDepth = atof(value.c_str()) / 100.0;
                }
            }
        }
        ifs.close();
    }


    //check what music format the file is
    unsigned int bytesread;
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    char header[4] = "";

    result = FMOD_CODEC_FILE_READ(codec, header, 4, &bytesread);

    gme_type_t file_type = gme_identify_extension(gme_identify_header(header));

    // sap format is played by plugin_asap
    if (!file_type || file_type->extension_ == std::string(gme_sap_type->extension_))
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->bzrInfo->fileformat = file_type->system;
    plugin->emu = gme_new_emu(file_type, freq);

    /* Allocate space for buffer. */
    signed short* myBuffer = new signed short[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    ERRCHECK(result);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    ERRCHECK(result);

    handle_error(gme_load_data(plugin->emu, myBuffer, filesize));
    delete [] myBuffer;

    plugin->track = 0;

    handle_error(gme_track_info(plugin->emu, &plugin->info, plugin->track));

    if (!plugin->info) {
        return FMOD_ERR_INTERNAL;
    }

    /* Get and print main info for track */

    /*printf( "System   : %s\n", info.system );
    printf( "Game     : %s\n", info.game );
    printf( "Author   : %s\n", info.author );
    printf( "Copyright: %s\n", info.copyright );
    printf( "Comment  : %s\n", info.comment );
    printf( "Dumper   : %s\n", info.dumper );
    printf( "Tracks   : %d\n", (int) info.track_count );
    printf( "\n" );
    printf( "Track    : %d\n", (int) track + 1 );
    printf( "Name     : %s\n", info.song );
    printf( "Length   : %ld:%02ld",
                    (long) info.length / 1000 / 60, (long) info.length / 1000 % 60 );
    if ( info.loop_length != 0 )
            printf( " (endless)" );*/

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    //// If file doesn't have overall length, it might have intro and loop lengths
    //   if ( plugin->info.length <= 0 )
    //       plugin->info.length = plugin->info.intro_length + plugin->info.loop_length * 2;

    //if(plugin->info.length>0)
    //{
    //	plugin->waveformat.lengthpcm    = plugin->info.length/1000*plugin->waveformat.frequency;
    //}

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    gme_ignore_silence(plugin->emu, !ignore_silence);

    gme_set_tempo(plugin->emu, tempo);

    eq.bass = bass;
    eq.treble = treble;
    gme_set_equalizer(plugin->emu, &eq);
    gme_set_stereo_depth(plugin->emu, stereoDepth);

    handle_error(gme_start_track(plugin->emu, plugin->track));

    plugin->bzrInfo->title = plugin->info->song;
    plugin->bzrInfo->artist = plugin->info->author;
    plugin->bzrInfo->copyright = plugin->info->copyright;
    plugin->bzrInfo->comments = plugin->info->comment;
    plugin->bzrInfo->system = plugin->info->system;
    plugin->bzrInfo->game = plugin->info->game;
    plugin->bzrInfo->dumper = plugin->info->dumper;
    plugin->bzrInfo->plugin = PLUGIN_game_music_emu;
    plugin->bzrInfo->pluginName = PLUGIN_game_music_emu_NAME;
    plugin->bzrInfo->setSeekable(true);
    plugin->bzrInfo->numChannels = gme_voice_count(plugin->emu);
    plugin->bzrInfo->numSubsongs = (int)gme_track_count(plugin->emu);


    return FMOD_OK;
}

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginGme*>(codec->plugindata);
    //delete LogFile;
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginGme*>(codec->plugindata);
    plugin->emu->play(size << 1, static_cast<signed short*>(buffer));

    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginGme*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS)
    {
        return handle_error(gme_seek(plugin->emu, position));
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        plugin->track = position;
        return handle_error(gme_start_track(plugin->emu, position));
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_TEMPO)
    {
        //return handle_error( gme_start_track( plugin->emu, position ) );
        if (position < 0) position = 0;
        double rate = (double)position / 250.0;
        //printf("\n\nrate: %f\n",rate);
        gme_set_tempo(plugin->emu, rate);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        gme_mute_voices(plugin->emu, 0); //unmutes all voices
        //gme_mute_voices( plugin->emu, -1 ); //mutes all voices
        //position is a mask
        gme_mute_voices(plugin->emu, position);
        return FMOD_ERR_UNSUPPORTED;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto plugin = static_cast<pluginGme*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        handle_error(gme_track_info(plugin->emu, &plugin->info, plugin->track));
        plugin->bzrInfo->title=plugin->info->song;

        // If file doesn't have overall length, it might have intro and loop lengths
        if (plugin->info->length <= 0)
            plugin->info->length = plugin->info->intro_length + plugin->info->loop_length * 2;

        if (plugin->info->length > 0)
        {
            *length = plugin->info->length;
        }
        else
        {
            *length = -1;
        }
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        handle_error(gme_track_info(plugin->emu, &plugin->info, plugin->track));
        plugin->bzrInfo->title=plugin->info->song;
        *length = (int)gme_track_count(plugin->emu);
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginGme*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        *position = plugin->emu->current_track();
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
