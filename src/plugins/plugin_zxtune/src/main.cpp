#include <cstring>
#include "apps/libzxtune/zxtune.h"
#include <iostream>
#include <string>
#include "info.h"
#include "fmod_errors.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_zxtune_NAME, // Name.
    0x00009000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
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

class pluginZxtune
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginZxtune(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginZxtune()
    {
        //delete some stuff
        ZXTune_DestroyPlayer(player);
        ZXTune_CloseModule(module);
        ZXTune_CloseData(data);
    }

    ZXTuneHandle data;
    ZXTuneHandle module;
    ZXTuneHandle player;
    Info* info;
    int posAfterSeek;
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
    auto* plugin = new pluginZxtune(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    /* Allocate space for buffer. */
    auto myBuffer = new signed short[filesize];

    //rewind file pointer
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    if (result != FMOD_OK)
    {
        return FMOD_ERR_FORMAT;
    }

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    if (result != FMOD_OK)
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->data = ZXTune_CreateData(myBuffer, filesize);
    plugin->module = ZXTune_OpenModule(plugin->data);
    plugin->player = ZXTune_CreatePlayer(plugin->module);

    delete [] myBuffer;

    if (!plugin->player)
    {
        return FMOD_ERR_FORMAT;
    }

    ZXTuneModuleInfo zxinfo;
    ZXTune_GetModuleInfo(plugin->module, &zxinfo);

    plugin->info->numChannels = zxinfo.Channels;
    plugin->info->numOrders = zxinfo.Positions;
    plugin->info->loopPosition = zxinfo.LoopPosition;
    plugin->info->loopFrame = zxinfo.LoopFrame;
    // plugin->info->numPatterns = zxinfo.Patterns;
    plugin->info->numFrames = zxinfo.Frames;
    plugin->info->initialTempo = zxinfo.InitialTempo;
    //    plugin->info->title = ZXTune_GetInfo(plugin->player,"Title");
    //    plugin->info->author = ZXTune_GetInfo(plugin->player,"Author");
    //    plugin->info->replay = ZXTune_GetInfo(plugin->player,"Program");
    //    plugin->info->comments = ZXTune_GetInfo(plugin->player,"Comment");

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    const uint64_t msecDuration = zxinfo.Frames * ZXTune_GetDuration(plugin->player) / 1000;
    plugin->waveformat.lengthpcm = (msecDuration / 1000.0) * plugin->waveformat.frequency;

    std::string type = ZXTune_GetInfo(plugin->player, "Type");

    if (type == "AS0")
    {
        plugin->info->fileformat = "ASC Sound Master v0.xx";
    }
    else if (type == "AY")
    {
        plugin->info->fileformat = "AY ZX Spectrum/Amstrad CPC";
    }
    else if (type == "ASC")
    {
        plugin->info->fileformat = "ASC Sound Master v1.xx-2.xx";
    }
    else if (type == "FTC")
    {
        plugin->info->fileformat = "Spectrum Fast Tracker";
    }
    else if (type == "GTR")
    {
        plugin->info->fileformat = "Global Tracker";
    }
    else if (type == "PSC")
    {
        plugin->info->fileformat = "Pro Sound Creator";
    }
    else if (type == "PSG")
    {
        plugin->info->fileformat = "Spectrum PSG";
    }
    else if (type == "PSM")
    {
        plugin->info->fileformat = "Pro Sound Maker";
    }
    else if (type == "PT1")
    {
        plugin->info->fileformat = "Spectrum Pro Tracker 1";
    }
    else if (type == "PT2")
    {
        plugin->info->fileformat = "Spectrum Pro Tracker 2";
    }
    else if (type == "PT3")
    {
        plugin->info->fileformat = "Spectrum Pro Tracker 3";
    }
    else if (type == "SQT")
    {
        plugin->info->fileformat = "SQ Tracker";
    }
    else if (type == "ST1")
    {
        plugin->info->fileformat = "Spectrum Sound Tracker 1";
    }
    else if (type == "ST3")
    {
        plugin->info->fileformat = "Spectrum Sound Tracker 3";
    }
    else if (type == "STC")
    {
        plugin->info->fileformat = "Compiled Spectrum Sound Tracker 1";
    }
    else if (type == "STP")
    {
        plugin->info->fileformat = "Spectrum Sound Tracker Pro 1";
    }
    else if (type == "TXT")
    {
        plugin->info->fileformat = "Vortextracker (Pro Tracker 3)";
    }
    else if (type == "TS")
    {
        plugin->info->fileformat = "TurboSound module for AY Emulator";
    }
    else if (type == "VTX")
    {
        plugin->info->fileformat = "Vortextracker";
    }
    else if (type == "YM") //played by st-sound
    {
        plugin->info->fileformat = "YM";
    }
    else if (type == "STR")
    {
        plugin->info->fileformat = "Sample Tracker";
    }
    else if (type == "CHI")
    {
        plugin->info->fileformat = "Spectrum Chip Tracker";
    }
    else if (type == "SQD")
    {
        plugin->info->fileformat = "SG Digital Tracker";
    }
    else if (type == "DMM")
    {
        plugin->info->fileformat = "Digital Music Maker";
    }
    else if (type == "PDT")
    {
        plugin->info->fileformat = "Prodigi Tracker";
    }
    else if (type == "DST")
    {
        plugin->info->fileformat = "Digital Studio for AY and Covox";
    }
    else if (type == "COP")
    {
        plugin->info->fileformat = "Sam Coupe E-Tracker";
    }
    else if (type == "TFE")
    {
        plugin->info->fileformat = "TFM Music Maker 1.3+";
    }
    else if (type == "TF0")
    {
        plugin->info->fileformat = "TFM Music Maker 0.1-1.2";
    }
    else if (type == "TFD")
    {
        plugin->info->fileformat = "TurboFM Dumped";
    }
    else if (type == "TFC")
    {
        plugin->info->fileformat = "TurboFM Compiled";
    }
    else
    {
        plugin->info->fileformat = "Unknown ZXTune";
        //plugin->info->fileformat =ZXTune_GetInfo(plugin->player,"Program")+"("+type+")";
    }

    plugin->posAfterSeek = 0;
    plugin->info->plugin = PLUGIN_zxtune;
    plugin->info->pluginName = PLUGIN_zxtune_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginZxtune*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginZxtune*>(codec->plugindata);

    int samples = ZXTune_RenderSound(plugin->player, buffer, size);
    if (samples == -1)
    {
        return FMOD_ERR_FORMAT;
    }

    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginZxtune*>(codec->plugindata);
    unsigned int pos = (position / 1000) * plugin->waveformat.frequency;
    plugin->posAfterSeek = ZXTune_SeekSound(plugin->player, pos);

    return FMOD_OK;
}

//TODO needed?
static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginZxtune*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = plugin->posAfterSeek;
    }

    return FMOD_OK;
}
