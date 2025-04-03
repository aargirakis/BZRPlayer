#include <audiofile.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_audiofile_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};
static int seek_to_time = -1;

//CLogFile *LogFile;
class pluginAudiofile
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginAudiofile(FMOD_CODEC_STATE* codec)
    {
        //LogFile = new CLogFile("audiofile.log");
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAudiofile()
    {
        //delete some stuff
        afCloseFile(file);
        unlink(tempFilename.c_str());
    }

    AFfilehandle file;
    string tempFilename;

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

static bool invalidFile = false;

bool fmemopen(void* buf, size_t size, const char* mode, string filename)
{
    FILE* f = fopen(filename.c_str(), "wb");
    if (NULL == f)
        return NULL;

    fwrite(buf, size, 1, f);
    fclose(f);
    return true;
}

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto plugin = new pluginAudiofile(codec);
    double rate;
    int channels;
    unsigned char* myBuffer;
    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    myBuffer = new unsigned char[filesize];
    FMOD_RESULT result;

    Info* info = static_cast<Info*>(userexinfo->userdata);
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    plugin->tempFilename = info->tempPath + "/bzr_tempfile.tmp";
    bool ok = fmemopen(myBuffer, filesize, "r", plugin->tempFilename.c_str());
    delete[] myBuffer;
    plugin->file = afOpenFile(plugin->tempFilename.c_str(), "r", NULL);
    cout << "trying to open file: " << plugin->tempFilename.c_str() << "\n";

    if (!plugin->file)
    {
        cout << "failed open file: " << plugin->tempFilename.c_str() << "\n";
        unlink(plugin->tempFilename.c_str());
        invalidFile = true;
        return FMOD_ERR_FORMAT;
    }

    switch (afGetFileFormat(plugin->file,NULL))
    {
    case AF_FILE_UNKNOWN:
        info->fileformat = "Unknown Audio File Library";
        break;
    case AF_FILE_RAWDATA:
        info->fileformat = "Audio File Library Raw Data";
        break;
    case AF_FILE_AIFFC:
        info->fileformat = "AIFFC";
        break;
    case AF_FILE_AIFF:
        info->fileformat = "AIFF";
        break;
    case AF_FILE_NEXTSND:
        info->fileformat = "Next snd";
        break;
    case AF_FILE_WAVE:
        info->fileformat = "Wave";
        break;
    case AF_FILE_BICSF:
        info->fileformat = "Berkeley";
        break;
    case AF_FILE_AVR:
        info->fileformat = "Audio Visual Research";
        break;
    case AF_FILE_IFF_8SVX:
        info->fileformat = "Amiga IFF/8SVX";
        break;
    case AF_FILE_NIST_SPHERE:
        info->fileformat = "NIST SPHERE";
        break;
    case AF_FILE_VOC:
        info->fileformat = "Creative Voice File";
        return FMOD_ERR_FORMAT;
        break;
    case AF_FILE_SAMPLEVISION:
        info->fileformat = "SampleVision";
        break;
    default:
        //should not happen
        info->fileformat = "Unknown Audio File Library";
        return FMOD_ERR_FORMAT;
        break;
    }

    channels = afGetChannels(plugin->file, AF_DEFAULT_TRACK);
    rate = afGetRate(plugin->file, AF_DEFAULT_TRACK);
    afSetVirtualSampleFormat(plugin->file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 16);

    int frames = afGetFrameCount(plugin->file, AF_DEFAULT_TRACK);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = rate;
    plugin->waveformat.pcmblocksize = (16 >> 3) * channels;
    plugin->waveformat.lengthpcm = frames;


    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */
    info->plugin = PLUGIN_audiofile;
    info->pluginName = PLUGIN_audiofile_NAME;
    info->setSeekable(true);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    auto plugin = static_cast<pluginAudiofile*>(codec->plugindata);
    //	if(!invalidFile)
    //	{
    //		afCloseFile(plugin->file); //why does this crash...?
    //	}
    delete static_cast<pluginAudiofile*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    pluginAudiofile* plugin = static_cast<pluginAudiofile*>(codec->plugindata);

    if (seek_to_time >= 0)
    {
        afSeekFrame(plugin->file, AF_DEFAULT_TRACK,
                    (AFframecount)((seek_to_time / 1000) * plugin->waveformat.frequency));
        seek_to_time = -1;
    }

    int frames = afReadFrames(plugin->file, AF_DEFAULT_TRACK, (signed short*)buffer, codec->waveformat->pcmblocksize);

    *read = frames;
    //*read=size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    seek_to_time = position;
    return FMOD_OK;
}
