#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "fmod_errors.h"
#include "info.h"
#include "../ken/KDMENG.c"
#include "plugins.h"

/* TODO: std::string ReplayKen::GetInfo() const
    {
        std::string info;
        info = "2 channels\n";
        if (m_mediaType.ext == eExtension::_kdm)
            info += "Ken's Digital Music";
        else if (m_mediaType.ext == eExtension::_ksm)
            info += "Ken's Adlib Music";
        else if (m_mediaType.ext == eExtension::_sm)
            info += "Ken's CT-640 Music";
        else
            info += "Ken's 4-note Music";
        info += "\nKen Silverman";
        return info;
    } */

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);


FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_webken_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
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

class pluginKdm
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginKdm(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginKdm()
    {
        //delete some stuff
        delete[] myBuffer;
        myBuffer = 0;
    }

    signed short* myBuffer;
    Info* info;

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
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    auto plugin = new pluginKdm(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    //    char* smallBuffer;
    //    smallBuffer = new char[2];
    //    unsigned int bytesread;
    //    result = codec->fileseek(codec->filehandle,0,(char*)smallBuffer);

    //    result = codec->fileread(codec->filehandle,(char*)smallBuffer,2,&bytesread,0);


    //    if(!(smallBuffer[0]=='a' && smallBuffer[1]=='y') && !(smallBuffer[0]=='y' && smallBuffer[1]=='m'))
    //    {
    //        delete smallBuffer;
    //        return FMOD_ERR_FORMAT;
    //    }

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);


    plugin->myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    initkdmeng();

    int length = kdmload (plugin->info->filename.data());

    if (length < 0)
    {
        delete plugin->myBuffer;
        return FMOD_ERR_FORMAT;
    }


    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 1468;
    plugin->waveformat.lengthpcm = length / 1000 * plugin->waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->fileformat = "Ken's Digital Music";
    plugin->info->plugin = PLUGIN_webken;
    plugin->info->pluginName = PLUGIN_webken_NAME;
    plugin->info->setSeekable(true);


    kdmmusicon ();

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginKdm*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginKdm*>(codec->plugindata);
    kdmrendersound(static_cast<char*>(buffer), size << 2);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginKdm*>(codec->plugindata);
    kdmseek (position);
    return FMOD_OK;
}
