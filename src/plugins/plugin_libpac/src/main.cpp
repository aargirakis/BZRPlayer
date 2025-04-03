#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <iostream>
#include "fmod_errors.h"
#include "pacP.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);


FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libpac_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    &getposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLibpac
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibpac(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("libmod.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibpac()
    {
        if (pac_module != nullptr)
        {
            pac_exit();
            pac_module = nullptr;
        }
        unlink(tempFilename.c_str());
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
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

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
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
    Info* info = static_cast<Info*>(userexinfo->userdata);

    int freq = 44100;
    int channels = 2;

    pac_exit();
    if (pac_init(freq, 16, channels) != 0)
    {
        return FMOD_ERR_FORMAT;
    }


    pac_disable(PAC_MODE_DEFAULT);
    auto plugin = new pluginLibpac(codec);
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
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    plugin->tempFilename = info->tempPath + "/bzr_tempfile.tmp";
    bool ok = fmemopen(myBuffer, filesize, "r", plugin->tempFilename.c_str());

    if (ok && (plugin->pac_module = pac_open(plugin->tempFilename.c_str())) == nullptr)
    {
        delete[] myBuffer;
        //plugin->pac_module = NULL;
        unlink(plugin->tempFilename.c_str());
        delete plugin;
        return FMOD_ERR_FORMAT;
    }
    delete[] myBuffer;

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = pac_length(plugin->pac_module);


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */


    info->numChannels = pac_num_channels(plugin->pac_module);
    info->numPatterns = pac_num_sheets(plugin->pac_module);
    info->numOrders = pac_num_positions(plugin->pac_module);
    info->title = pac_title(plugin->pac_module);
    info->plugin = PLUGIN_libpac;
    info->pluginName = PLUGIN_libpac_NAME;
    info->fileformat = "SBStudio PAC";
    info->setSeekable(true);

    int numSamples = pac_num_samples(plugin->pac_module);
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
            struct pac_sound* sample = pac_sample(plugin->pac_module, j);
            info->samples[j - 1] = sample->name;
            info->samples16Bit[j - 1] = sample->bits == 16 ? true : false;
            info->samplesSize[j - 1] = sample->length;
            info->samplesLoopStart[j - 1] = sample->loopstart;
            info->samplesLoopEnd[j - 1] = sample->loopend;
            info->samplesVolume[j - 1] = sample->volume;
            info->samplesFineTune[j - 1] = sample->tune;
        }
    }

    //cout << "sample name " << pac_sample_title(plugin->pac_module,2);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginLibpac*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibpac*>(codec->plugindata);
    pac_read(plugin->pac_module, buffer, size);
    *read = size >> 2;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibpac*>(codec->plugindata);
    pac_seek(plugin->pac_module, (position / 1000) * plugin->waveformat.frequency,SEEK_SET);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibpac*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = (pac_tell(plugin->pac_module) / 44100.0) * 1000;
    }
    return FMOD_OK;
}
