extern "C" {
#include "vgmstream.h"
}

#include <iostream>
#include "fmod_errors.h"
#include "info.h"
#include "../app/plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_vgmstream_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    0,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    fcplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
    }

    ~fcplugin()
    {
        //delete some stuff
    }

    VGMSTREAM* vgmstream;
    Info* info;

    FMOD_CODEC_WAVEFORMAT fcwaveformat;
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


FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    fcplugin* fc = new fcplugin(codec);

    fc->info = (Info*)userexinfo->userdata;

    fc->vgmstream = NULL;
    fc->vgmstream = init_vgmstream(fc->info->filename.c_str());
    if (!fc->vgmstream)
    {
        return FMOD_ERR_FORMAT;
    }

    fc->vgmstream->loop_flag = 0;

    /* will we be able to play it? */
    if (fc->vgmstream->channels <= 0)
    {
        close_vgmstream(fc->vgmstream);
        fc->vgmstream = NULL;
        return FMOD_ERR_FORMAT;
    }

    int loop_count = 1;
    int fade_seconds = 0;
    int fade_delay_seconds = 0;
    fc->fcwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels = fc->vgmstream->channels;
    fc->fcwaveformat.frequency = fc->vgmstream->sample_rate;
    fc->fcwaveformat.pcmblocksize = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm =
        get_vgmstream_play_samples(loop_count, fade_seconds, fade_delay_seconds, fc->vgmstream);

    codec->waveformat = &(fc->fcwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = fc; /* user data value */

    fc->info->plugin = PLUGIN_vgmstream;
    fc->info->pluginName = PLUGIN_vgmstream_NAME;
    fc->info->setSeekable(true);

    char description[128];
    get_vgmstream_meta_description(fc->vgmstream, description, sizeof(description));
    fc->info->fileformat = description;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    //fcplugin* fc = (fcplugin*)codec->plugindata;
    //close_vgmstream(fc->vgmstream);
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    render_vgmstream((sample_t*)buffer, size, fc->vgmstream);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    int32_t seek_sample = (int32_t)(position * 0.001 * fc->vgmstream->sample_rate);
    seek_vgmstream(fc->vgmstream, seek_sample);
    return FMOD_OK;
}
