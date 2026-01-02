extern "C" {
#include "api_internal.h"
#include "coding.h"
}

#include "fmod_errors.h"
#include "info.h"
#include "../app/plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

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

class pluginVgmstream {
    FMOD_CODEC_STATE *_codec;

public:
    pluginVgmstream(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVgmstream() {
        //delete some stuff
        libvgmstream_free(libvgmstream);
        libvgmstream = nullptr;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    libvgmstream_t *libvgmstream = nullptr;
    Info *info;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    auto *smallBuffer = new uint8_t[16];
    FMOD_CODEC_FILE_READ(codec, smallBuffer, 16, nullptr);

    /* skip gm.dls:
     * plugin_vgmstream has higher prio than FMOD_SOUND_TYPE_MIDI,
     * thus when plugin_vgmstream will fail on midi loading
     * FMOD will try to invoke plugin_vgmstream again for loading gm.dls too
    */
    if (memcmp(smallBuffer, "RIFF\x0c\x80" "4\0DLS colh", 16) == 0) {
        delete[] smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] smallBuffer;

    auto *plugin = new pluginVgmstream(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->libvgmstream = libvgmstream_init();

    if (!plugin->libvgmstream) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    libvgmstream_config_t config = {};
    config.ignore_loop = true;

    libvgmstream_setup(plugin->libvgmstream, &config);

    auto libsf = libstreamfile_open_from_stdio(plugin->info->filename.c_str());
    if (!libsf) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (libvgmstream_open_stream(plugin->libvgmstream, libsf, 0) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    switch (plugin->libvgmstream->format->sample_format) {
        default:
        case LIBVGMSTREAM_SFMT_PCM16:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
            break;
        case LIBVGMSTREAM_SFMT_PCM24:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM24;
            break;
        case LIBVGMSTREAM_SFMT_PCM32:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM32;
            break;
        case LIBVGMSTREAM_SFMT_FLOAT:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
            break;
    }

    plugin->waveformat.channels = plugin->libvgmstream->format->channels;
    plugin->waveformat.frequency = plugin->libvgmstream->format->sample_rate;
    plugin->waveformat.lengthpcm = static_cast<unsigned int>(plugin->libvgmstream->format->stream_samples);

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->samplerate = plugin->waveformat.frequency;
    plugin->info->bitRate = plugin->libvgmstream->format->stream_bitrate;
    plugin->info->numChannels = plugin->waveformat.channels;

    if (const auto priv = static_cast<libvgmstream_priv_t *>(plugin->libvgmstream->priv);
        priv->vgmstream->coding_type != coding_FFmpeg) {
        plugin->info->fileformat = plugin->libvgmstream->format->meta_name;
    } else {
        if (priv->vgmstream->meta_type == meta_FFMPEG || priv->vgmstream->meta_type == meta_FFMPEG_faulty) {
            plugin->info->fileformat = get_ffmpeg_format_long_name(
                static_cast<ffmpeg_codec_data *>(priv->vgmstream->codec_data));
        } else {
            plugin->info->fileformat = plugin->libvgmstream->format->meta_name;
        }

        const AVDictionaryEntry *tags = ffmpeg_get_all_metadata(
            static_cast<ffmpeg_codec_data *>(priv->vgmstream->codec_data));

        int i = 0;
        while (tags[i].key != nullptr) {
            plugin->info->metadata.emplace_back(tags[i].key, tags[i].value);
            i++;
        }
    }

    plugin->info->fileformatSpecific = plugin->libvgmstream->format->codec_name;
    plugin->info->plugin = PLUGIN_vgmstream;
    plugin->info->pluginName = PLUGIN_vgmstream_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginVgmstream *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginVgmstream *>(codec->plugindata);

    if (plugin->libvgmstream->decoder->done) {
        // TODO needed? FMOD_ERR_FORMAT?
        return FMOD_ERR_FILE_EOF;
    }

    if (libvgmstream_fill(plugin->libvgmstream, buffer, static_cast<int>(size)) < 0) {
        // TODO needed? FMOD_ERR_FILE_EOF?
        return FMOD_ERR_FORMAT;
    }

    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginVgmstream *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        const auto seek_sample = static_cast<int32_t>(position * 0.001 * plugin->libvgmstream->format->sample_rate);
        libvgmstream_seek(plugin->libvgmstream, seek_sample);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
