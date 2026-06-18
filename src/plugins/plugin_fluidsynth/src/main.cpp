#include "fluidsynth.h"
#include "fluidsynth/settings.h"
#include "fluidsynth/synth.h"
#include "midi_sequencer.hpp"
#include "midi_sequencer_impl.hpp"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static constexpr unsigned int rate = 44100;
static constexpr unsigned int channels = 2;
static constexpr unsigned int pcmFloatSize = sizeof(uint32_t);
static constexpr unsigned int pcmBlockSize = channels * pcmFloatSize;
static constexpr unsigned int samplesToWrite = 256;

FMOD_CODEC_DESCRIPTION codec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_fluidsynth_NAME, // name.
    0x00012300, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginFluidsynth {
    FMOD_CODEC_STATE *_codec;

public:
    pluginFluidsynth(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginFluidsynth() {
        if (player) {
            fluid_player_stop(player);
            delete_fluid_player(player);
        }

        if (synth) delete_fluid_synth(synth);
        if (settings) delete_fluid_settings(settings);
    }

    Info *info;
    FMOD_CODEC_WAVEFORMAT waveformat;
    fluid_settings_t *settings = nullptr;
    fluid_synth_t *synth = nullptr;
    fluid_player_t *player = nullptr;

    BW_MidiSequencer *sequencer = nullptr;
    BW_MidiRtInterface *sequencerInterface = nullptr;
};

#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codec;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    auto *plugin = new pluginFluidsynth(codec);

    plugin->settings = new_fluid_settings();

    if (plugin->settings == nullptr) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->synth = new_fluid_synth(plugin->settings);

    if (plugin->synth == nullptr) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    // for gm.sf2 only (mimics fmod defaults)
    fluid_synth_reverb_on(plugin->synth, -1, false);
    fluid_synth_chorus_on(plugin->synth, -1, false);
    fluid_synth_set_gain(plugin->synth, 0.8f);

    plugin->info = static_cast<Info *>(userexinfo->userdata);

    //TODO load gm.sf2 from mem not path (thru new_fluid_defsfloader?)

    if (const string gmSf2Path = plugin->info->dataPath + GM_SF2_PATH;
        fluid_synth_sfload(plugin->synth, gmSf2Path.c_str(), 1) == FLUID_FAILED) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->sequencer = new BW_MidiSequencer();
    plugin->sequencerInterface = new BW_MidiRtInterface;

    memset(plugin->sequencer, 0, sizeof(BW_MidiRtInterface));

    plugin->sequencerInterface->onDebugMessage= nullptr;
    // plugin->sequencerInterface->rtUserData = this;
    // plugin->sequencerInterface->rt_noteOn  = rtNoteOn;
    // plugin->sequencerInterface->rt_noteOff = rtNoteOff;
    // plugin->sequencerInterface->rt_noteAfterTouch = rtNoteAfterTouch;
    // plugin->sequencerInterface->rt_channelAfterTouch = rtChannelAfterTouch;
    // plugin->sequencerInterface->rt_controllerChange = rtControllerChange;
    // plugin->sequencerInterface->rt_patchChange = rtPatchChange;
    // plugin->sequencerInterface->rt_pitchBend = rtPitchBend;
    // plugin->sequencerInterface->rt_systemExclusive = rtSysEx;
    //
    // plugin->sequencerInterface->onPcmRender = playSynth;
    // plugin->sequencerInterface->onPcmRender_userData = this;
    //

    plugin->sequencerInterface->pcmSampleRate = rate;
    plugin->sequencerInterface->pcmFrameSize = pcmBlockSize;

    // sequencerInterface->rt_deviceSwitch = rtDeviceSwitch;
    // sequencerInterface->rt_currentDevice = rtCurrentDevice;

    plugin->sequencer->setInterface(plugin->sequencerInterface);
    plugin->sequencer->setDeviceMask(
        BW_MidiSequencer::Device_GeneralMidi | BW_MidiSequencer::Device_SoundMasterII |
        BW_MidiSequencer::Device_GravisUltrasound);

    if (!plugin->sequencer->loadMIDI(plugin->info->fileBuffer, plugin->info->filesize)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = rate;
    plugin->waveformat.pcmblocksize = pcmBlockSize;
    plugin->waveformat.lengthpcm = -1;
    //plugin->waveformat.lengthpcm = static_cast<unsigned int>(length / 1000.0L * plugin->waveformat.frequency);;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->artist = "";
    plugin->info->title = "";
    plugin->info->fileFormat = "";

    //TODO display extended metadata

    plugin->info->plugin = PLUGIN_fluidsynth;
    plugin->info->pluginName = PLUGIN_fluidsynth_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginFluidsynth *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginFluidsynth *>(codec->plugindata);

    // if (fluid_player_get_status(plugin->player) != FLUID_PLAYER_PLAYING) {
    //     return FMOD_ERR_FILE_EOF;
    // }
    //
    // if (fluid_synth_write_float(plugin->synth, samplesToWrite, buffer, 0, 2, buffer, 1, 2) != FLUID_OK) {
    //     //TODO
    //     return FMOD_ERR_FILE_EOF;
    // }
    //
    // *read = samplesToWrite;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto const *plugin = static_cast<pluginFluidsynth *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginFluidsynth *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
