#include <cstring>
#include <iostream>
#include "fmod_errors.h"
#include "info.h"
#include "libsc68/sc68/sc68.h"
#include "file68/sc68/rsc68.h"
#include "plugins.h"

//CLogFile *LogFile;


/*void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        LogFile->Print("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}*/
/*FMOD_RESULT handle_error( const char* str )
{
    if ( str )
    {
        LogFile->Print( "Error: %s\n", str );
        return FMOD_ERR_INTERNAL;
    }
    else
        return FMOD_OK;
}*/
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);
FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_sc68_NAME, // Name.
    0x00012000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

static bool invalidFile = false;

class pluginSc68
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginSc68(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("sc68.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSc68()
    {
        //delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    sc68_t* sc68;
    sc68_init_t init68;
    sc68_create_t create68;
    sc68_music_info_t info;
    int currentSubsong;
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
    FMOD_RESULT result;
    auto plugin = new pluginSc68(codec);


    unsigned int bytesread;
    char* s;
    s = new char[16];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, s, 16, &bytesread);


    invalidFile = true;
    bool isSC68 = false;
    if (s[0] == 'S' && s[1] == 'C' && s[2] == '6' && s[3] == '8')
    {
        isSC68 = true;
        invalidFile = false;
    }
    else if (s[0] == 'I' && s[1] == 'C' && s[2] == 'E' && s[3] == '!')
    {
        isSC68 = false;
        invalidFile = false;
    }
    else if (s[12] == 'S' && s[13] == 'N' && s[14] == 'D' && s[15] == 'H')
    {
        isSC68 = false;
        invalidFile = false;
    }
    else
    {
        invalidFile = true;
    }
    delete[] s;

    if (invalidFile)
    {
        return FMOD_ERR_FORMAT;
    }


    Info* info = static_cast<Info*>(userexinfo->userdata);
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    /* Allocate space for buffer. */
    signed short* myBuffer = new signed short[filesize];


    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    //ERRCHECK(result);


    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    //ERRCHECK(result);

    memset(&plugin->init68, 0, sizeof(plugin->init68));

    sc68_init(&plugin->init68);

    string path = info->dataPath + SC68_DATA_DIR;
    rsc68_set_share(path.c_str());

    memset(&plugin->create68, 0, sizeof(plugin->create68));
    plugin->create68.emu68_debug = 1; // avoids sc68 crash
    plugin->sc68 = sc68_create(&plugin->create68);

    if (sc68_load_mem(plugin->sc68, (signed short*)myBuffer, filesize) < 0)
    {
        invalidFile = true;
        sc68_destroy(plugin->sc68);
        sc68_shutdown();
        delete[] myBuffer;
        cout << "FMOD_ERR_FORMAT load" << endl;
        return FMOD_ERR_FORMAT;
    }
    delete[] myBuffer;


    //just to get number of subsongs
    int infoError = sc68_music_info(plugin->sc68, &plugin->info, 1, 0);
    //No idea why I have to put track #2 in here to get the time?


    if (infoError)
    {
        //        std::cout << "music info error\n";
        //        std::flush(std::cout);
    }


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;


    unsigned int length = plugin->info.time_ms * plugin->waveformat.frequency * plugin->waveformat.channels;
    if (length > 0xfffff || length == 0)
    //if length > 4.6 hours (or 0) then set it to unlimited, some songs report a ridiculous large time
    {
        length = 0xffffffff;
    }
    plugin->waveformat.lengthpcm = length;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */


    plugin->currentSubsong = 1;
    sc68_play(plugin->sc68, plugin->currentSubsong, 0);

    info->plugin = PLUGIN_sc68;
    info->pluginName = PLUGIN_sc68_NAME;
    info->fileformat = "SC68";

    if (!isSC68)
    {
        info->fileformat = "SNDH";
    }
    info->author = plugin->info.author;
    info->composer = plugin->info.composer;
    info->replay = plugin->info.replay;
    info->hwname = plugin->info.hwname;
    info->title = plugin->info.title;
    info->rate = plugin->info.rate;
    info->address = plugin->info.addr;
    info->converter = plugin->info.converter ? plugin->info.converter : "";
    info->ripper = plugin->info.ripper ? plugin->info.ripper : "";
    info->numSubsongs = plugin->info.tracks;
    info->setSeekable(false);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginSc68*>(codec->plugindata);
    if (!invalidFile)
    {
        sc68_destroy(plugin->sc68);
        sc68_shutdown();
    }

    delete static_cast<pluginSc68*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginSc68*>(codec->plugindata);

    if (sc68_process(plugin->sc68, (signed short*)buffer, plugin->waveformat.pcmblocksize) == SC68_MIX_ERROR)
    {
        //cout << "FMOD_ERR_FORMAT play" << endl;
        //return FMOD_ERR_FORMAT;
    }
    *read = plugin->waveformat.pcmblocksize;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginSc68*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS)
    {
        //setting loop = 1 in api68_seek function will make it work
        //but seeking is so slow (like slowly winding it up, audible so it's pretty useless
        //int* seek = 0;
        //api68_seek(plugin->sc68, position,seek);
        sc68_play(plugin->sc68, plugin->currentSubsong, 0);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        sc68_play(plugin->sc68, position + 1, 0);
        sc68_music_info(plugin->sc68, &plugin->info, position + 1, 0);
        plugin->currentSubsong = position + 1;
        return FMOD_OK;
    }
    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginSc68*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        cout << "length: " << plugin->info.time_ms << endl;
        *length = plugin->info.time_ms;
        if (*length > 0xfffff || *length == 0)
        //if length > 4.6 hours (or 0) then set it to unlimited, some songs report a ridiculous large time
        {
            *length = 0xffffffff;
        }
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->info.tracks;
    }
    return FMOD_OK;
}
