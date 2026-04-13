#include <cstring>
#include <fstream>
#include "BaseSample.h"
#include "FileLoader.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_flod_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN | FMOD_TIMEUNIT_MODPATTERN_INFO,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    nullptr,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    &getPosition,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginFlod {
    FMOD_CODEC_STATE *_codec;

public:
    pluginFlod(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginFlod() {
        //delete some stuf
        delete player;
        player = nullptr;
    }

    AmigaPlayer *player = nullptr;
    Info *info;

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
    auto plugin = new pluginFlod(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/flod.cfg";
    ifstream ifs(filename.c_str());
    int force = 0;
    int forcePlayer = 0;
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    bool clockspeedNTSC = false;
    int filter = AmigaFilter::FORCE_OFF;
    bool model1200 = true;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "model") {
                    if (value == "500") {
                        model1200 = false;
                    } else if (value == "1200") {
                        model1200 = true;
                    }
                } else if (word == "filter") {
                    if (value == "on") {
                        filter = AmigaFilter::FORCE_ON;
                    } else if (value == "off") {
                        filter = AmigaFilter::FORCE_OFF;
                    } else if (value == "auto") {
                        filter = AmigaFilter::AUTOMATIC;
                    }
                } else if (word == "clockspeed") {
                    if (value == "ntsc") {
                        clockspeedNTSC = true;
                    } else {
                        clockspeedNTSC = false;
                    }
                } else if (word == "force") {
                    force = atoi(value.c_str());
                } else if (word == "player") {
                    forcePlayer = atoi(value.c_str());
                }
            }
        }
        ifs.close();
    }

    auto fileLoader = new FileLoader();

    fileLoader->setForcePlayer(forcePlayer);

    plugin->player = fileLoader->load(plugin->info->fileBuffer, plugin->info->filesize, plugin->info->filePath.c_str());

    if (!plugin->player) {
        // delete some stuff
        delete plugin->player;
        delete plugin;
        delete fileLoader;
        return FMOD_ERR_FORMAT;
    }

    delete fileLoader;

    plugin->info = static_cast<Info *>(userexinfo->userdata);
    //plugin->player->play();
    plugin->player->setVersion(force);
    plugin->player->setNTSC(clockspeedNTSC);
    plugin->player->amiga->setModel(model1200);
    plugin->player->amiga->setFilter(filter);
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->numSubsongs = plugin->player->getSubsongsCount();
    plugin->info->fileFormat = plugin->player->format;
    plugin->info->plugin = PLUGIN_flod;
    plugin->info->pluginName = PLUGIN_flod_NAME;
    plugin->info->setSeekable(false);

    plugin->info->hasTitle = true;
    plugin->info->numChannels = plugin->player->getChannels();
    plugin->player->getTitle(plugin->info->title);
    if (vector<BaseSample *> samples = plugin->player->getSamples(); !samples.empty()) {
        plugin->info->numSamples = static_cast<int>(samples.size());
        plugin->info->samples = new string[plugin->info->numSamples];
        plugin->info->samplesSize = new unsigned int[plugin->info->numSamples];
        //            plugin->info->samplesLoopStart = new unsigned int[plugin->info->numSamples];
        //            plugin->info->samplesLoopLength = new unsigned int[plugin->info->numSamples];
        plugin->info->samplesVolume = new unsigned short[plugin->info->numSamples];

        //            int loopStart = 0;
        for (int j = 0; j < plugin->info->numSamples; j++) {
            if (samples[j]) {
                plugin->info->samples[j] = samples[j]->name;
                plugin->info->samplesSize[j] = samples[j]->length;
                ////                    plugin->info->samplesLoopLength[j] = samples[j]->length-samples[j]->repeat;
                ////                    plugin->info->samplesLoopStart[j] = samples[j]->repeat;
                plugin->info->samplesVolume[j] = samples[j]->volume;
            }
        }
    } else {
        plugin->info->numSamples = 0;
    }

    plugin->player->play();

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginFlod *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto plugin = static_cast<pluginFlod *>(codec->plugindata);
    if (plugin->player->amiga->isCompleted()) {
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }
    plugin->player->mixer(buffer, size >> 2);
    *read = size >> 2;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto plugin = static_cast<pluginFlod *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODROW) {
        *position = plugin->player->getCurrentRow();
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MODPATTERN) {
        *position = plugin->player->getCurrentPattern();
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO) {
        // set the mod pattern (notes etc.) in the info struct and just return 0
        plugin->player->getModRows(plugin->info->modRows);

        //const vector<AmigaRow*>& b = plugin->player->getModRows();
        *position = 0;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
