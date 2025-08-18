#include <cstring>
#include <string>
#include "fmod_errors.h"

extern "C" {
#include "jxs.h"
#include "jaytrax.h"
}

#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_jaytrax_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
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

class pluginJaytrax
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginJaytrax(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginJaytrax()
    {
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    JT1Player* jay;
    JT1Song* song;
    uint8_t* myBuffer;
    unsigned int length;
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


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    Info* info = static_cast<Info*>(userexinfo->userdata);

    auto* plugin = new pluginJaytrax(codec);

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    plugin->myBuffer = new uint8_t[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    bool isErr = jxsfile_readSongMem(plugin->myBuffer, filesize, &plugin->song) != 0;
    delete[] plugin->myBuffer;
    if (isErr)
    {
        return FMOD_ERR_FORMAT;
    }

	plugin->jay = jaytrax_init();
    plugin->jay->song = plugin->song;

    jaytrax_changeSubsong(plugin->jay, 0);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    info->plugin = PLUGIN_jaytrax;
    info->pluginName = PLUGIN_jaytrax_NAME;
    info->fileformat = "Jaytrax";
    info->setSeekable(false);

    return FMOD_OK;
}

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginJaytrax*>(codec->plugindata);
	if(plugin!=nullptr)
	{
		jaytrax_free(plugin->jay);
	}
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginJaytrax*>(codec->plugindata);

    jaytrax_renderChunk(plugin->jay, static_cast<int16_t *>(buffer), static_cast<int32_t>(size), plugin->waveformat.frequency);

    *read = size;
    return FMOD_OK;
}


FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginJaytrax*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        //        jaytrax_stopSong(plugin->jay);
        //        jaytrax_loadSong(plugin->jay,plugin->jay->song);
        //        jaytrax_continueSong(plugin->jay);
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->jay->subsongNr = position;
        plugin->length = jaytrax_getLength(plugin->jay, position, 1, plugin->waveformat.frequency)
                         / plugin->waveformat.frequency * 1000;

        jaytrax_changeSubsong(plugin->jay, position);

        return FMOD_OK;
    }
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginJaytrax*>(codec->plugindata);
    if (lengthtype == FMOD_TIMEUNIT_MS)
    {
        *length = -1;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->jay->song->nrofsongs;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = plugin->length;
    }
    return FMOD_OK;
}
