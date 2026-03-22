#include <cstring>
#include "kdmeng.h"
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
    PLUGIN_kdm_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // the time format we would like to accept into setposition/getposition
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    nullptr, // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &setPosition, // setposition callback
    nullptr, // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginKdm {
    FMOD_CODEC_STATE *_codec;

public:
    pluginKdm(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginKdm() {
        // delete some stuff
        delete[] myBuffer;
        delete player;
        myBuffer = nullptr;
    }

    uint8_t *myBuffer;
    Info *info;
    kdmeng *player;

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
    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    const auto plugin = new pluginKdm(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->myBuffer = new uint8_t[filesize];

    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    constexpr int sampleRate = 44100;

    plugin->player = new kdmeng(sampleRate, 2, 2);

    const unsigned found = plugin->info->filename.find_last_of("/\\");

    const long length = plugin->player->load(plugin->myBuffer, filesize,
                                               plugin->info->filename.substr(0, found + 1).c_str());

    if (!length) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = sampleRate;
    plugin->waveformat.pcmblocksize = 1468;
    plugin->waveformat.lengthpcm = static_cast<unsigned int>(length / 1000.0L * plugin->waveformat.frequency);

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    plugin->info->fileFormat = "Ken's Digital Music";
    plugin->info->plugin = PLUGIN_kdm;
    plugin->info->pluginName = PLUGIN_kdm_NAME;
    plugin->info->setSeekable(true);

    const int numSamples = plugin->player->getNumwaves();
    const int numTracks = plugin->player->getNumtracks();
    plugin->info->numSamples = numSamples;
    plugin->info->numPatterns = numTracks;

    if (numSamples > 0) {
        plugin->info->samplesSize = new unsigned int[numSamples];
        plugin->info->samplesLoopStart = new unsigned int[numSamples];
        plugin->info->samplesLoopLength = new unsigned int[numSamples];
        plugin->info->samplesFineTune = new signed int[numSamples];
        plugin->info->samples = new string[numSamples];

        const auto c = new char[17];

        for (int j = 0; j < numSamples; j++) {
            plugin->info->samplesSize[j] = plugin->player->getInstsize(j);
            plugin->info->samplesLoopStart[j] = plugin->player->getInstrepstart(j);
            plugin->info->samplesLoopLength[j] = plugin->player->getInstreplength(j);
            plugin->info->samplesFineTune[j] = plugin->player->getInstfinetune(j);
            plugin->player->getInstname(j, c);
            plugin->info->samples[j] = c;
        }

        delete c;
    }

    if (numTracks > 0) {
        plugin->info->instruments = new string[numTracks];
        plugin->info->instrumentsNumber = new char[numTracks];
        plugin->info->instrumentsQuantize = new char[numTracks];
        plugin->info->instrumentsVolume1 = new unsigned char[numTracks];
        plugin->info->instrumentsVolume2 = new unsigned char[numTracks];

        const auto c = new char[17];

        for (int j = 0; j < numTracks; j++) {
            const int instrIdx = plugin->player->getTrackInstrument(j);
            plugin->info->instrumentsNumber[j] = instrIdx + 1;
            plugin->info->instruments[j] = plugin->info->samples[instrIdx];
            plugin->info->instrumentsQuantize[j] = plugin->player->getTrackQuantize(j);
            plugin->info->instrumentsVolume1[j] = plugin->player->getTrackVolume1(j);
            plugin->info->instrumentsVolume2[j] = plugin->player->getTrackVolume2(j);
        }

        delete c;
    }

    plugin->player->musicon();

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginKdm *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto plugin = static_cast<pluginKdm *>(codec->plugindata);
    plugin->player->rendersound(buffer, size << 2);
    *read = size;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginKdm *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->player->seek(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
