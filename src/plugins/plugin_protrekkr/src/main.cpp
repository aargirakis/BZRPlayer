#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include "info.h"
#include <iostream>
#include "plugins.h"
#include "../../release/distrib/replay/lib/include/replay.h"
#include "../../src/files/include/files.h"

int AUDIO_Play_Flag;

char artist[20];
char style[20];
char SampleName[128][16][64];
char Midiprg[128];
char nameins[128][20];
extern char replayerPtkName[20];
extern Uint8* Mod_Memory;
int Chan_Midi_Prg[MAX_TRACKS];
char Chan_History_State[256][MAX_TRACKS];

int done = 0; // set to TRUE if decoding has finished
Uint32 STDCALL Mixer(Uint8* Buffer, Uint32 Len);
// Reset the channels polyphonies  to their default state
void Set_Default_Channels_Polyphony(void)
{
    int i;

    for (i = 0; i < MAX_TRACKS; i++)
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

FMOD_CODEC_DESCRIPTION tfmxcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "TFMX player plugin", // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    0,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class ahxplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    ahxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("libmod.log");
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
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

    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    uint8_t* buffer;
    unsigned int filesize;
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
    return &tfmxcodec;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    Info* info = (Info*)userexinfo->userdata;
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

    ahxplugin* ahx = new ahxplugin(codec);

    FMOD_CODEC_FILE_SIZE(codec, &ahx->filesize);

    memset(SampleName, 0, sizeof(SampleName));
    memset(nameins, 0, sizeof(nameins));


    Ptk_InitDriver();
    Alloc_Patterns_Pool();

    ahx->buffer = new uint8_t[ahx->filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, ahx->buffer, ahx->filesize, &bytesread);


    if (!Load_Ptk({ahx->buffer, ahx->filesize}))
    {
        return FMOD_ERR_FORMAT;
    }
    ahx->loaded = true;

    info->samples = new string[128];
    info->instruments = new string[128];


    for (int j = 0; j < 128; j++)
    {
        info->samples[j] = SampleName[j][0];
        info->instruments[j] = nameins[j];
    }


    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    ahx->ahxwaveformat.channels = channels;
    ahx->ahxwaveformat.frequency = freq;
    ahx->ahxwaveformat.pcmblocksize = (16 >> 3) * ahx->ahxwaveformat.channels;
    ahx->ahxwaveformat.lengthpcm = Calc_Length() / 1000 * freq;


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */

    info->numSamples = 128;
    info->numInstruments = 128;
    info->numChannels = int(Songtracks);
    info->title = replayerPtkName;
    info->artist = artist;
    info->plugin = PLUGIN_protrekkr;
    info->pluginName = PLUGIN_protrekkr_NAME;
    info->fileformat = "Protrekkr";
    info->setSeekable(false);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    delete (ahxplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    *read = Mixer((Uint8*)buffer, size >> 1);
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    Ptk_Stop();
    Free_Samples();

    if (!Load_Ptk({ahx->buffer, ahx->filesize}))
    {
        return FMOD_ERR_FORMAT;
    }
    Ptk_Play();
    //pac_seek(ahx->pac_module,(position/1000)*ahx->ahxwaveformat.frequency,SEEK_SET);
    return FMOD_OK;
}
