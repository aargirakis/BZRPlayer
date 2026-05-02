#include <cstring>
#include <list>
#include "decoder.h"
#include "organya.h"
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
    PLUGIN_organya_decoder_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODVUMETER,
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

class pluginOrganya {
    FMOD_CODEC_STATE *_codec;

public:
    pluginOrganya(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginOrganya() {
        // delete some stuff
        if (tune) org_decoder_destroy(tune);
    }

    org_decoder_t *tune = nullptr;
    Info *info;
    int queueSize;
    list<unsigned char *> vuMeterBuffer;
    FMOD_CODEC_WAVEFORMAT waveformat;
    static constexpr int sampleRate = 44100;
};

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
    auto *plugin = new pluginOrganya(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);
    plugin->queueSize = 18000;
    ifstream is;
    //is.imbue(locale("en_US.UTF8"));
    is.open(plugin->info->filePath, ios::binary);

    const string samples_path = plugin->info->dataPath + ORGANYA_DATA_DIR;
    plugin->tune = org_decoder_create(is, samples_path.c_str(), 1);
    is.close();

    if (!plugin->tune) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->tune->state.sample_rate = pluginOrganya::sampleRate;

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = pluginOrganya::sampleRate;
    plugin->waveformat.pcmblocksize = 4;
    plugin->waveformat.lengthpcm = static_cast<unsigned int>(org_decoder_get_total_samples(plugin->tune));

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->fileFormat = "Organya";
    plugin->info->plugin = PLUGIN_organya_decoder;
    plugin->info->pluginName = PLUGIN_organya_decoder_NAME;
    plugin->info->setSeekable(true);
    plugin->info->numChannels = 16;
    org_decoder_seek_sample(plugin->tune, 0);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginOrganya *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginOrganya *>(codec->plugindata);

    const auto samplesDecoded = org_decode_samples(plugin->tune, static_cast<short *>(buffer),
                                                   plugin->waveformat.pcmblocksize);

    unsigned char vuMeters[16];
    //unsigned char const* vuMetersMean = new unsigned char[16];
    constexpr double maxVuMeter = 32767 / 4;

    for (int i = 0; i < 16; i++) {
        const unsigned char newValue = org_decoder_vumeter_channel(plugin->tune, i) / maxVuMeter * 100;
        vuMeters[i] = newValue;
    }

    if (plugin->vuMeterBuffer.size() >= plugin->queueSize) {
        plugin->vuMeterBuffer.pop_front();
    }

    plugin->vuMeterBuffer.push_back(vuMeters);

    *read = static_cast<int>(samplesDecoded);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginOrganya *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        // position should be in samples, not ms
        org_decoder_seek_sample(plugin->tune, position * pluginOrganya::sampleRate / 1000);
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        // position is a mask
        for (int i = 0; i < 16; i++) {
            const unsigned int m = position >> i & 1;
            const bool mute = m != 0;
            org_decoder_mute_channel(plugin->tune, i, mute);
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginOrganya *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODVUMETER) {
        unsigned int vuMetersMean[16] = {};
        int iteraterCount = 0;
        constexpr int everyNth = 4000;

        for (const auto &it: plugin->vuMeterBuffer) {
            for (int i = 0; i < 16; i++) {
                if (iteraterCount == 0) {
                    vuMetersMean[i] += it[i];
                }
            }

            iteraterCount++;

            if (iteraterCount == everyNth) {
                iteraterCount = 0;
            }
        }

        for (unsigned int &i: vuMetersMean) {
            i = i / (plugin->queueSize / everyNth);
        }

        unsigned char vuMetersDone[16];

        for (int i = 0; i < 16; i++) {
            vuMetersDone[i] = vuMetersMean[i];
        }

        //plugin->info->modVuMeters = plugin->vuMeterBuffer.front();
        plugin->info->modVuMeters = vuMetersDone;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
