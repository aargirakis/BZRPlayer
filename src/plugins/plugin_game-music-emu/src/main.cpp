#include <gme.h>
#include "Music_Emu.h"
#include <blargg_endian.h>
#include <iostream>
#include <fstream>
#include <string.h>
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

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);
FMOD_CODEC_DESCRIPTION gamecodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD Game Music Player Plugin", // Name.
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
    0 // Sound create callback (don't need it)
};

class gameplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    gameplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("gameemu.log");
        memset(&gpwaveformat, 0, sizeof(gpwaveformat));
    }

    ~gameplugin()
    {
        //delete some stuff
        gme_delete(emu);
    }

    Music_Emu* emu;
    FMOD_CODEC_WAVEFORMAT gpwaveformat;
    gme_info_t* info;
    int track;
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
    return &gamecodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    gameplugin* gp = new gameplugin(codec);


    //read config from disk

    //Info* info = new Info();
    Info* info = (Info*)userexinfo->userdata;
    string filename = info->applicationPath + "/user/plugin/config/gameemu.cfg";
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

    info->fileformat = file_type->system;
    gp->emu = gme_new_emu(file_type, freq);

    /* Allocate space for buffer. */
    signed short* myBuffer = new signed short[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    ERRCHECK(result);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    ERRCHECK(result);

    handle_error(gme_load_data(gp->emu, myBuffer, filesize));
    delete [] myBuffer;

    gp->track = 0;

    handle_error(gme_track_info(gp->emu, &gp->info, gp->track));

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

    gp->gpwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    gp->gpwaveformat.channels = 2;
    gp->gpwaveformat.frequency = freq;
    gp->gpwaveformat.pcmblocksize = (16 >> 3) * gp->gpwaveformat.channels;
    gp->gpwaveformat.lengthpcm = 0xffffffff;

    //// If file doesn't have overall length, it might have intro and loop lengths
    //   if ( gp->info.length <= 0 )
    //       gp->info.length = gp->info.intro_length + gp->info.loop_length * 2;

    //if(gp->info.length>0)
    //{
    //	gp->gpwaveformat.lengthpcm    = gp->info.length/1000*gp->gpwaveformat.frequency;
    //}

    codec->waveformat = &(gp->gpwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = gp; /* user data value */

    gme_ignore_silence(gp->emu, !ignore_silence);

    gme_set_tempo(gp->emu, tempo);

    eq.bass = bass;
    eq.treble = treble;
    gme_set_equalizer(gp->emu, &eq);
    gme_set_stereo_depth(gp->emu, stereoDepth);

    handle_error(gme_start_track(gp->emu, gp->track));

    info->title = gp->info->song;
    info->artist = gp->info->author;
    info->copyright = gp->info->copyright;
    info->comments = gp->info->comment;
    info->system = gp->info->system;
    info->game = gp->info->game;
    info->dumper = gp->info->dumper;
    info->plugin = PLUGIN_game_music_emu;
    info->pluginName = PLUGIN_game_music_emu_NAME;
    info->setSeekable(true);
    info->numChannels = gme_voice_count(gp->emu);
    info->numSubsongs = (int)gme_track_count(gp->emu);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    gameplugin* gp = (gameplugin*)codec->plugindata;
    delete (gameplugin*)codec->plugindata;
    //delete LogFile;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    gameplugin* gp = (gameplugin*)codec->plugindata;
    gp->emu->play(size << 1, (signed short*)buffer);

    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    gameplugin* gp = (gameplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MS)
    {
        return handle_error(gme_seek(gp->emu, position));
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        gp->track = position;
        return handle_error(gme_start_track(gp->emu, position));
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_TEMPO)
    {
        //return handle_error( gme_start_track( gp->emu, position ) );
        if (position < 0) position = 0;
        double rate = (double)position / 250.0;
        //printf("\n\nrate: %f\n",rate);
        gme_set_tempo(gp->emu, rate);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        gme_mute_voices(gp->emu, 0); //unmutes all voices
        //gme_mute_voices( gp->emu, -1 ); //mutes all voices
        //position is a mask
        gme_mute_voices(gp->emu, position);
        return FMOD_ERR_UNSUPPORTED;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    gameplugin* gp = (gameplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        handle_error(gme_track_info(gp->emu, &gp->info, gp->track));
        // If file doesn't have overall length, it might have intro and loop lengths
        if (gp->info->length <= 0)
            gp->info->length = gp->info->intro_length + gp->info->loop_length * 2;

        if (gp->info->length > 0)
        {
            *length = gp->info->length;
        }
        else
        {
            *length = 0xffffffff;
        }
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = (int)gme_track_count(gp->emu);
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    gameplugin* gp = (gameplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        *position = gp->emu->current_track();
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
