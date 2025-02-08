#include <string>
#include "fmod_errors.h"
#include "info.h"
#include <iostream>
#include "plugins.h"
#include "../../src/files/include/files.h"

int AUDIO_Play_Flag;
char artist[20];
char style[20];
char SampleName[128][16][64];
int Midiprg[MAX_INSTRS];
char nameins[MAX_INSTRS][20];
extern char customPtkName[20];
extern Uint8* Mod_Memory;
int Chan_Midi_Prg[MAX_TRACKS];
char Chan_History_State[256][16];
int done = 0;

Uint32 STDCALL Mixer(Uint8* Buffer, Uint32 Len);

void Set_Default_Channels_Polyphony(void)
{
    for (int i = 0; i < MAX_TRACKS; i++)
    {
        Channels_Polyphony[i] = DEFAULT_POLYPHONY;
    }
}

int Alloc_Patterns_Pool(void);
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_protrekkr_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
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

class pluginProtrekkr
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginProtrekkr(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("libmod.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginProtrekkr()
    {
        delete[] buffer;
        if (loaded)
        {
            Ptk_Stop();
            Free_Samples();
            if (Mod_Memory) free(Mod_Memory);
            if (RawPatterns) free(RawPatterns);
        }
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    uint8_t* buffer;
    size_t filesize;
    bool loaded = false;
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
    Info* info = static_cast<Info*>(userexinfo->userdata);
    FMOD_RESULT result;

    int freq = 44100;
    int channels = 2;

    unsigned char* testBuffer;
    unsigned int bytesread;
    testBuffer = new unsigned char[7];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, testBuffer, 7, &bytesread);


    if (!(testBuffer[0] == 'P' && testBuffer[1] == 'R' && testBuffer[2] == 'O' && testBuffer[3] == 'T' && testBuffer[4]
            == 'R' && testBuffer[5] == 'E' && testBuffer[6] == 'K') &&
        !(testBuffer[0] == 'T' && testBuffer[1] == 'W' && testBuffer[2] == 'N' && testBuffer[3] == 'N' && testBuffer[4]
            == 'S' && testBuffer[5] == 'N' && testBuffer[6] == 'G'))
    {
        delete[] testBuffer;
        return FMOD_ERR_FORMAT;
    }
    delete[] testBuffer;

    auto* plugin = new pluginProtrekkr(codec);

    FMOD_CODEC_FILE_SIZE(codec, &plugin->filesize);

    memset(SampleName, 0, sizeof(SampleName));
    memset(nameins, 0, sizeof(nameins));


    Ptk_InitDriver();
    Alloc_Patterns_Pool();

    plugin->buffer = new uint8_t[plugin->filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->buffer, plugin->filesize, &bytesread);


    if (!Load_Ptk({plugin->buffer, plugin->filesize}))
    {
        return FMOD_ERR_FORMAT;
    }
    plugin->loaded = true;

    info->samples = new string[128];
    info->instruments = new string[128];


    for (int j = 0; j < 128; j++)
    {
        info->samples[j] = SampleName[j][0];
        info->instruments[j] = nameins[j];
    }


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = Calc_Length() / 1000 * freq;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->numSamples = 128;
    info->numInstruments = 128;
    info->numChannels = int(Song_Tracks);
    info->title = customPtkName;
    info->artist = artist;
    info->plugin = PLUGIN_protrekkr;
    info->pluginName = PLUGIN_protrekkr_NAME;
    info->fileformat = "Protrekkr";
    info->setSeekable(false);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginProtrekkr*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginProtrekkr*>(codec->plugindata);

    *read = Mixer(static_cast<Uint8*>(buffer), size >> 1);
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginProtrekkr*>(codec->plugindata);
    Ptk_Stop();
    Free_Samples();

    if (!Load_Ptk({plugin->buffer, plugin->filesize}))
    {
        return FMOD_ERR_FORMAT;
    }
    Ptk_Play();
    //pac_seek(plugin->pac_module,(position/1000)*plugin->waveformat.frequency,SEEK_SET);
    return FMOD_OK;
}
