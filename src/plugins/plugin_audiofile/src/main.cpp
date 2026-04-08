#include <cstring>
#include <audiofile.h>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_audiofile_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    nullptr,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginAudiofile {
    FMOD_CODEC_STATE *_codec;

public:
    pluginAudiofile(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAudiofile() {
        // delete some stuff
        afCloseFile(file);
    }

    AFfilehandle file;
    int seek_to_time = -1;

    FMOD_CODEC_WAVEFORMAT waveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin. This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C! C++ functions will be mangled incorrectly and not load in fmod.
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
    const auto plugin = new pluginAudiofile(codec);
    const auto info = static_cast<Info *>(userexinfo->userdata);

    plugin->file = afOpenFile(info->filename.c_str(), "r", nullptr);

    if (!plugin->file) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    switch (afGetFileFormat(plugin->file, nullptr)) {
        case AF_FILE_UNKNOWN:
            info->fileFormat = "Unknown Audio File Library";
            break;
        case AF_FILE_RAWDATA:
            info->fileFormat = "Audio File Library Raw Data";
            break;
        case AF_FILE_AIFFC:
            info->fileFormat = "AIFFC";
            break;
        case AF_FILE_AIFF:
            info->fileFormat = "AIFF";
            break;
        case AF_FILE_NEXTSND:
            info->fileFormat = "Next snd";
            break;
        case AF_FILE_WAVE:
            info->fileFormat = "Wave";
            break;
        case AF_FILE_BICSF:
            info->fileFormat = "Berkeley";
            break;
        case AF_FILE_AVR:
            info->fileFormat = "Audio Visual Research";
            break;
        case AF_FILE_IFF_8SVX:
            info->fileFormat = "Amiga IFF/8SVX";
            break;
        case AF_FILE_NIST_SPHERE:
            info->fileFormat = "NIST SPHERE";
            break;
        case AF_FILE_VOC:
            info->fileFormat = "Creative Voice File";
            delete plugin;
            return FMOD_ERR_FORMAT;
        case AF_FILE_SAMPLEVISION:
            info->fileFormat = "SampleVision";
            break;
        default:
            // should not happen
            delete plugin;
            return FMOD_ERR_FORMAT;
    }

    const int channels = afGetChannels(plugin->file, AF_DEFAULT_TRACK);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;

    afSetVirtualSampleFormat(plugin->file, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, plugin->waveformat.format * 8);

    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = static_cast<int>(afGetRate(plugin->file, AF_DEFAULT_TRACK));
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * channels;
    plugin->waveformat.lengthpcm = static_cast<unsigned int>(afGetFrameCount(plugin->file, AF_DEFAULT_TRACK));

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value
    info->plugin = PLUGIN_audiofile;
    info->pluginName = PLUGIN_audiofile_NAME;
    info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginAudiofile *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginAudiofile *>(codec->plugindata);

    if (plugin->seek_to_time >= 0) {
        afSeekFrame(plugin->file, AF_DEFAULT_TRACK, plugin->seek_to_time / 1000 * plugin->waveformat.frequency);
        plugin->seek_to_time = -1;
    }

    *read = afReadFrames(plugin->file, AF_DEFAULT_TRACK, buffer, static_cast<int>(codec->waveformat->pcmblocksize));
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginAudiofile *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->seek_to_time = static_cast<int>(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
