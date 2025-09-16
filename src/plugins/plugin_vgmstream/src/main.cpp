extern "C" {
#include "vgmstream.h"
}

#include <iostream>
#include "fmod_errors.h"
#include "info.h"
#include "../app/plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_vgmstream_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
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

class pluginVgmstream
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginVgmstream(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVgmstream()
    {
        //delete some stuff
    }

    VGMSTREAM* vgmstream;
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


static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto* plugin = new pluginVgmstream(codec);

    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->vgmstream = NULL;
    plugin->vgmstream = init_vgmstream(plugin->info->filename.c_str());
    if (!plugin->vgmstream)
    {
        return FMOD_ERR_FORMAT;
    }

    plugin->vgmstream->loop_flag = 0;

    /* will we be able to play it? */
    if (plugin->vgmstream->channels <= 0)
    {
        close_vgmstream(plugin->vgmstream);
        plugin->vgmstream = NULL;
        return FMOD_ERR_FORMAT;
    }

    int loop_count = 1;
    int fade_seconds = 0;
    int fade_delay_seconds = 0;
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = plugin->vgmstream->channels;
    plugin->waveformat.frequency = plugin->vgmstream->sample_rate;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm =
        get_vgmstream_play_samples(loop_count, fade_seconds, fade_delay_seconds, plugin->vgmstream);

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    char description[128];

    if (plugin->vgmstream->meta_type == meta_FFMPEG || plugin->vgmstream->meta_type == meta_FFMPEG_faulty) {
        get_vgmstream_coding_description(plugin->vgmstream, description, sizeof(description));
    } else {
        get_vgmstream_meta_description(plugin->vgmstream, description, sizeof(description));
    }

    plugin->info->fileformat = description;
    plugin->info->plugin = PLUGIN_vgmstream;
    plugin->info->pluginName = PLUGIN_vgmstream_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    //pluginVgmstream* plugin = static_cast<plugin*>(codec->plugindata);
    //close_vgmstream(plugin->vgmstream);
    delete static_cast<pluginVgmstream*>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginVgmstream*>(codec->plugindata);
    render_vgmstream2(static_cast<sample_t*>(buffer), size, plugin->vgmstream);
    *read = size;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginVgmstream*>(codec->plugindata);

    auto seek_sample = static_cast<int32_t>(position * 0.001 * plugin->vgmstream->sample_rate);
    seek_vgmstream(plugin->vgmstream, seek_sample);
    return FMOD_OK;
}
