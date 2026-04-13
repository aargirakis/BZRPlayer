#include <cstring>
#include <fstream>
#include "Music_Emu.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_game_music_emu_NAME, // name.
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

class pluginGme {
    FMOD_CODEC_STATE *_codec;

public:
    pluginGme(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginGme() {
        // delete some stuff
        gme_delete(emu);
    }

    Music_Emu *emu = nullptr;
    FMOD_CODEC_WAVEFORMAT waveformat;
    gme_info_t *gmeInfo;
    Info *info;
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
    auto plugin = new pluginGme(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/gme.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    int freq = 44100;
    bool ignore_silence = false;
    double treble = 0;
    double bass = 15;
    double stereoDepth = 0.0;
    gme_equalizer_t eq = {treble, bass};
    float tempo = 1.0;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "frequency") {
                    freq = atoi(value.c_str());
                } else if (word == "ignore_silence") {
                    if (value == "yes") {
                        ignore_silence = true;
                    } else if (value == "no") {
                        ignore_silence = false;
                    }
                } else if (word == "tempo") {
                    tempo = atof(value.c_str()) / 100.0;
                } else if (word == "treble") {
                    treble = atoi(value.c_str());
                } else if (word == "bass") {
                    bass = atoi(value.c_str());
                } else if (word == "stereo_depth") {
                    stereoDepth = atof(value.c_str()) / 100.0;
                }
            }
        }
        ifs.close();
    }

    gme_type_t file_type = gme_identify_extension(gme_identify_header(plugin->info->fileBuffer));

    // sap format is played by plugin_asap
    if (!file_type || file_type->extension_ == string(gme_sap_type->extension_)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->info->fileFormat = file_type->system;
    plugin->emu = gme_new_emu(file_type, freq);

    if (gme_load_data(plugin->emu, plugin->info->fileBuffer, static_cast<long>(plugin->info->filesize))) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (gme_track_info(plugin->emu, &plugin->gmeInfo, plugin->info->currentSubsong)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (!plugin->gmeInfo) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    gme_ignore_silence(plugin->emu, !ignore_silence);

    gme_set_tempo(plugin->emu, tempo);

    eq.bass = bass;
    eq.treble = treble;
    gme_set_equalizer(plugin->emu, &eq);
    gme_set_stereo_depth(plugin->emu, stereoDepth);

    if (gme_start_track(plugin->emu, plugin->info->currentSubsong)) {
        return FMOD_ERR_FORMAT;
    }

    plugin->info->title = plugin->gmeInfo->song;
    plugin->info->artist = plugin->gmeInfo->author;
    plugin->info->copyright = plugin->gmeInfo->copyright;
    plugin->info->comments = plugin->gmeInfo->comment;
    plugin->info->system = plugin->gmeInfo->system;
    plugin->info->game = plugin->gmeInfo->game;
    plugin->info->ripper = plugin->gmeInfo->dumper;
    plugin->info->plugin = PLUGIN_game_music_emu;
    plugin->info->pluginName = PLUGIN_game_music_emu_NAME;
    plugin->info->setSeekable(true);
    plugin->info->numChannels = gme_voice_count(plugin->emu);
    plugin->info->numSubsongs = gme_track_count(plugin->emu);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginGme *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto plugin = static_cast<pluginGme *>(codec->plugindata);
    plugin->emu->play(size << 1, static_cast<signed short *>(buffer));

    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto plugin = static_cast<pluginGme *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        if (gme_seek(plugin->emu, static_cast<int>(position))) {
            return FMOD_ERR_FORMAT;
        }
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        gme_mute_voices(plugin->emu, 0); // unmutes all voices
        //gme_mute_voices( plugin->emu, -1 ); // mutes all voices
        // position is a mask
        gme_mute_voices(plugin->emu, static_cast<int>(position));
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto plugin = static_cast<pluginGme *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        // If file doesn't have overall length, it might have intro and loop lengths
        if (plugin->gmeInfo->length <= 0)
            plugin->gmeInfo->length = plugin->gmeInfo->intro_length + plugin->gmeInfo->loop_length * 2;

        if (plugin->gmeInfo->length > 0) {
            *length = plugin->gmeInfo->length;
        } else {
            *length = -1;
        }

        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
