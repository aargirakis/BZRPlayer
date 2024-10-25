#include "apps/libzxtune/zxtune.h"
#include <iostream>
#include <string>
#include "info.h"
#include "fmod_errors.h"
#include "plugins.h"

using namespace std;

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION libmodcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD zxtune Plugin", // Name.
    0x00009000, // Version 0xAAAABBBB   A = major, B = minor.
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

class zxtuneplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    zxtuneplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
    }

    ~zxtuneplugin()
    {
        //delete some stuff
        ZXTune_CloseModule(module);
        ZXTune_DestroyPlayer(player);
    }

    ZXTuneHandle module;
    ZXTuneHandle player;
    Info* info;
    int posAfterSeek;
    FMOD_CODEC_WAVEFORMAT zxwaveformat;
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
    return &libmodcodec;
}

#ifdef __cplusplus
}
#endif

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    zxtuneplugin* zx = new zxtuneplugin(codec);

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    /* Allocate space for buffer. */
    signed short* myBuffer = new signed short[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
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

    ZXTuneHandle data = ZXTune_CreateData(myBuffer, filesize);

    zx->module = ZXTune_OpenModule(data);
    zx->player = ZXTune_CreatePlayer(zx->module);


    delete [] myBuffer;
    if (!zx->player)
    {
        return FMOD_ERR_FORMAT;
    }


    zx->info = (Info*)userexinfo->userdata;

    ZXTuneModuleInfo zxinfo;
    ZXTune_GetModuleInfo(zx->module, &zxinfo);

    cout << "Title: " << ZXTune_GetInfo(zx->player, "Title") << "\n";
    //    cout << "Author: " << ZXTune_GetInfo(zx->player,"Author") << "\n";
    //    cout << "Program: " << ZXTune_GetInfo(zx->player,"Program") << "\n";
    //    cout << "Computer: " << ZXTune_GetInfo(zx->player,"Computer") << "\n";
    //    cout << "Date: " << ZXTune_GetInfo(zx->player,"Date") << "\n";
    //    cout << "Comment: " << ZXTune_GetInfo(zx->player,"Comment") << "\n";
    //    cout << "Version: " << ZXTune_GetInfo(zx->player,"Version") << "\n";
    //    cout << "CRC: " << ZXTune_GetInfo(zx->player,"CRC") << "\n";
    //    cout << "FixedCRC: " << ZXTune_GetInfo(zx->player,"FixedCRC") << "\n";
    //    cout << "FixedCRC: " << ZXTune_GetInfo(zx->player,"FixedCRC") << "\n";
    //    cout << "Size: " << ZXTune_GetInfo(zx->player,"Size") << "\n";
    //    cout << "Content: " << ZXTune_GetInfo(zx->player,"Content") << "\n";
    //    cout << "Container: " << ZXTune_GetInfo(zx->player,"Container") << "\n";
    //    cout << "Subpath: " << ZXTune_GetInfo(zx->player,"Subpath") << "\n";
    //    cout << "Extension: " << ZXTune_GetInfo(zx->player,"Extension") << "\n";
    //    cout << "Filename: " << ZXTune_GetInfo(zx->player,"Filename") << "\n";
    //    cout << "Path: " << ZXTune_GetInfo(zx->player,"Path") << "\n";
    //    cout << "Fullpath: " << ZXTune_GetInfo(zx->player,"Fullpath") << "\n";


    zx->info->numChannels = zxinfo.Channels;
    zx->info->numOrders = zxinfo.Positions;
    zx->info->loopPosition = zxinfo.LoopPosition;
    zx->info->loopFrame = zxinfo.LoopFrame;
    zx->info->numPatterns = zxinfo.Patterns;
    zx->info->numFrames = zxinfo.Frames;
    zx->info->initialTempo = zxinfo.InitialTempo;
    //    zx->info->title = ZXTune_GetInfo(zx->player,"Title");
    //    zx->info->author = ZXTune_GetInfo(zx->player,"Author");
    //    zx->info->replay = ZXTune_GetInfo(zx->player,"Program");
    //    zx->info->comments = ZXTune_GetInfo(zx->player,"Comment");


    zx->zxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    zx->zxwaveformat.channels = 2;
    zx->zxwaveformat.frequency = 44100;
    zx->zxwaveformat.pcmblocksize = (16 >> 3) * zx->zxwaveformat.channels;


    codec->waveformat = &(zx->zxwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = zx; /* user data value */

    zx->zxwaveformat.lengthpcm = 0xffffffff;
    const uint64_t msecDuration = zxinfo.Frames * ZXTune_GetDuration(zx->player) / 1000;
    zx->zxwaveformat.lengthpcm = (msecDuration / 1000.0) * zx->zxwaveformat.frequency;


    std::string type = ZXTune_GetInfo(zx->player, "Type");


    if (type == "AS0")
    {
        zx->info->fileformat = "ASC Sound Master v0.xx";
    }
    else if (type == "AY")
    {
        zx->info->fileformat = "AY ZX Spectrum/Amstrad CPC";
    }
    else if (type == "ASC")
    {
        zx->info->fileformat = "ASC Sound Master v1.xx-2.xx";
    }
    else if (type == "FTC")
    {
        zx->info->fileformat = "Spectrum Fast Tracker";
    }
    else if (type == "GTR")
    {
        zx->info->fileformat = "Global Tracker";
    }
    else if (type == "PSC")
    {
        zx->info->fileformat = "Pro Sound Creator";
    }
    else if (type == "PSG")
    {
        zx->info->fileformat = "Spectrum PSG";
    }
    else if (type == "PSM")
    {
        zx->info->fileformat = "Pro Sound Maker";
    }
    else if (type == "PT1")
    {
        zx->info->fileformat = "Spectrum Pro Tracker 1";
    }
    else if (type == "PT2")
    {
        zx->info->fileformat = "Spectrum Pro Tracker 2";
    }
    else if (type == "PT3")
    {
        zx->info->fileformat = "Spectrum Pro Tracker 3";
    }
    else if (type == "SQT")
    {
        zx->info->fileformat = "SQ Tracker";
    }
    else if (type == "ST1")
    {
        zx->info->fileformat = "Spectrum Sound Tracker 1";
    }
    else if (type == "ST3")
    {
        zx->info->fileformat = "Spectrum Sound Tracker 3";
    }
    else if (type == "STC")
    {
        zx->info->fileformat = "Compiled Spectrum Sound Tracker 1";
    }
    else if (type == "STP")
    {
        zx->info->fileformat = "Spectrum Sound Tracker Pro 1";
    }
    else if (type == "TXT")
    {
        zx->info->fileformat = "Vortextracker (Pro Tracker 3)";
    }
    else if (type == "TS")
    {
        zx->info->fileformat = "TurboSound module for AY Emulator";
    }
    else if (type == "VTX")
    {
        zx->info->fileformat = "Vortextracker";
    }
    else if (type == "YM") //played by st-sound
    {
        zx->info->fileformat = "YM";
    }
    else if (type == "STR")
    {
        zx->info->fileformat = "Sample Tracker";
    }
    else if (type == "CHI")
    {
        zx->info->fileformat = "Spectrum Chip Tracker";
    }
    else if (type == "SQD")
    {
        zx->info->fileformat = "SG Digital Tracker";
    }
    else if (type == "DMM")
    {
        zx->info->fileformat = "Digital Music Maker";
    }
    else if (type == "PDT")
    {
        zx->info->fileformat = "Prodigi Tracker";
    }
    else if (type == "DST")
    {
        zx->info->fileformat = "Digital Studio for AY and Covox";
    }
    else if (type == "COP")
    {
        zx->info->fileformat = "Sam Coupe E-Tracker";
    }
    else if (type == "TFE")
    {
        zx->info->fileformat = "TFM Music Maker 1.3+";
    }
    else if (type == "TF0")
    {
        zx->info->fileformat = "TFM Music Maker 0.1-1.2";
    }
    else if (type == "TFD")
    {
        zx->info->fileformat = "TurboFM Dumped";
    }
    else if (type == "TFC")
    {
        zx->info->fileformat = "TurboFM Compiled";
    }
    else
    {
        zx->info->fileformat = "Unknown ZXTune";
        //zx->info->fileformat =ZXTune_GetInfo(zx->player,"Program")+"("+type+")";
    }

    zx->posAfterSeek = 0;
    zx->info->plugin = PLUGIN_zxtune;
    zx->info->pluginName = PLUGIN_zxtune_NAME;
    zx->info->setSeekable(true);

    //    cout << "zxtune length: " << msecDuration <<  endl;
    //    cout << "zxtune numChannels: " << zx->info->numChannels <<  endl;
    //    cout << "zxtune numOrders: " << zx->info->numOrders <<  endl;
    //    cout << "zxtune loopPosition: " << zx->info->loopPosition <<  endl;
    //    cout << "zxtune loopFrame: " << zx->info->loopFrame <<  endl;
    //    cout << "zxtune numPatterns: " << zx->info->numPatterns <<  endl;
    //    cout << "zxtune initialTempo: " << zx->info->initialTempo <<  endl;
    //    cout << "zxtune fileformat: " << zx->info->fileformat <<  endl;

    //    flush(cout);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    zxtuneplugin* zx = static_cast<zxtuneplugin*>(codec->plugindata);
    delete zx;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    //cout << "read" <<   endl;
    zxtuneplugin* zx = static_cast<zxtuneplugin*>(codec->plugindata);
    //cout << "zxtune read: " << size <<  endl;
    //flush(cout);

    int err = ZXTune_RenderSound(zx->player, buffer, size);
    //    if (err)
    //    {
    //        cout << "returned bytes: " <<  err << endl;
    //    }
    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    zxtuneplugin* zx = static_cast<zxtuneplugin*>(codec->plugindata);
    unsigned int pos = (position / 1000) * zx->zxwaveformat.frequency;
    zx->posAfterSeek = ZXTune_SeekSound(zx->player, pos);
    //cout << "setposition" <<   endl;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    //cout << "getlength" <<   endl;
    zxtuneplugin* zx = static_cast<zxtuneplugin*>(codec->plugindata);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    zxtuneplugin* zx = static_cast<zxtuneplugin*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = zx->posAfterSeek;
    }
    //cout << "getposition" <<   endl;
    return FMOD_OK;
}
