#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include <pacP.h>
#include "info.h"
#include <iostream>
#include "plugins.h"

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
    &getposition,
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
        if (pac_module != nullptr)
        {
            pac_exit();
            pac_module = nullptr;
        }
        unlink(tempFilename.c_str());
    }

    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    struct pac_module* pac_module;
    string tempFilename;
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


bool fmemopen(void* buf, size_t size, const char* mode, string filename)
{
    FILE* f = fopen(filename.c_str(), "wb");
    if (NULL == f)
        return NULL;

    fwrite(buf, size, 1, f);
    fclose(f);
    return true;
}

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    Info* info = (Info*)userexinfo->userdata;

    int freq = 44100;
    int channels = 2;

    pac_exit();
    if (pac_init(freq, 16, channels) != 0)
    {
        return FMOD_ERR_FORMAT;
    }


    pac_disable(PAC_MODE_DEFAULT);
    ahxplugin* ahx = new ahxplugin(codec);
    pac_enable(PAC_MODE_DEFAULT);

    pac_enable(PAC_MODE_DEFAULT);
    char* smallBuffer;
    smallBuffer = new char[4];
    unsigned char* myBuffer;
    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    myBuffer = new unsigned char[filesize];
    FMOD_RESULT result;

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 4, &bytesread);

    if (smallBuffer[0] != 'P' || smallBuffer[1] != 'A' || smallBuffer[2] != 'C' || smallBuffer[3] != 'G')
    {
        delete[] smallBuffer;
        delete ahx;
        return FMOD_ERR_FORMAT;
    }

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    ahx->tempFilename = info->tempPath + "/bzr_tempfile.tmp";
    bool ok = fmemopen(myBuffer, filesize, "r", ahx->tempFilename.c_str());

    if (ok && (ahx->pac_module = pac_open(ahx->tempFilename.c_str())) == nullptr)
    {
        delete[] myBuffer;
        //ahx->pac_module = NULL;
        unlink(ahx->tempFilename.c_str());
        delete ahx;
        return FMOD_ERR_FORMAT;
    }
    delete[] myBuffer;

    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    ahx->ahxwaveformat.channels = channels;
    ahx->ahxwaveformat.frequency = freq;
    ahx->ahxwaveformat.pcmblocksize = (16 >> 3) * ahx->ahxwaveformat.channels;
    ahx->ahxwaveformat.lengthpcm = pac_length(ahx->pac_module);


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */


    info->numChannels = pac_num_channels(ahx->pac_module);
    info->numPatterns = pac_num_sheets(ahx->pac_module);
    info->numOrders = pac_num_positions(ahx->pac_module);
    info->title = pac_title(ahx->pac_module);
    info->plugin = PLUGIN_libpac;
    info->pluginName = PLUGIN_libpac_NAME;
    info->fileformat = "SBStudio PAC";
    info->setSeekable(true);

    int numSamples = pac_num_samples(ahx->pac_module);
    info->numSamples = numSamples;

    if (numSamples > 0)
    {
        info->samples = new string[numSamples];
        info->samples16Bit = new bool[numSamples];
        info->samplesSize = new unsigned int[numSamples];
        info->samplesLoopStart = new unsigned int[numSamples];
        info->samplesLoopEnd = new unsigned int[numSamples];
        info->samplesVolume = new unsigned short[numSamples];
        info->samplesFineTune = new int[numSamples];

        for (int j = 1; j <= numSamples; j++)
        {
            struct pac_sound* sample = pac_sample(ahx->pac_module, j);
            info->samples[j - 1] = sample->name;
            info->samples16Bit[j - 1] = sample->bits == 16 ? true : false;
            info->samplesSize[j - 1] = sample->length;
            info->samplesLoopStart[j - 1] = sample->loopstart;
            info->samplesLoopEnd[j - 1] = sample->loopend;
            info->samplesVolume[j - 1] = sample->volume;
            info->samplesFineTune[j - 1] = sample->tune;
        }
    }

    //cout << "sample name " << pac_sample_title(ahx->pac_module,2);


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
    pac_read(ahx->pac_module, buffer, size);
    *read = size >> 2;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    pac_seek(ahx->pac_module, (position / 1000) * ahx->ahxwaveformat.frequency,SEEK_SET);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = (pac_tell(ahx->pac_module) / 44100.0) * 1000;
    }
    return FMOD_OK;
}
