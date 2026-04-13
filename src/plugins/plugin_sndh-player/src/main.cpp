#include <cstring>
#include <fstream>
#include <queue>
#include "SndhFile.h"
#include "timedb.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_sndh_player_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
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

class pluginSndhPlayer {
    FMOD_CODEC_STATE *_codec;

public:
    pluginSndhPlayer(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSndhPlayer() {
        delete sndh;
        // delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    SndhFile *sndh;
    queue<uint32_t *> oscBuffer;
    int songLength;
    uint32_t hash = 0;

    int32_t GetTickCountFromSc68() const {
        dbentry_t e;
        e.hash = hash >> HFIX;
        e.track = info->currentSubsong;
        if (auto const *s = static_cast<dbentry_t *>(bsearch(&e, s_db.data(), s_db.size(),
                                                             sizeof(dbentry_t), [](const void *ea, const void *eb) {
                                                                 auto *a = static_cast<const dbentry_t *>(ea);
                                                                 auto *b = static_cast<const dbentry_t *>(eb);

                                                                 int v = a->hash - b->hash;
                                                                 if (!v)
                                                                     v = a->track - b->track;
                                                                 return v;
                                                             })))
            return s->frames;
        return 0;
    }

    void BuildHash(SndhFile const *_sndh) {
        // hash taken from sc68
        uint32_t h = 0;
        int n = 32;
        const auto *k = static_cast<const uint8_t *>(_sndh->GetRawData());
        do {
            h += *k++;
            h += h << 10;
            h ^= h >> 6;
        } while (--n);

        n = _sndh->GetRawDataSize();
        k = static_cast<const uint8_t *>(_sndh->GetRawData());
        do {
            h += *k++;
            h += h << 10;
            h ^= h >> 6;
        } while (--n);
        hash = h;
    }
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
    auto *plugin = new pluginSndhPlayer(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->sndh = new SndhFile();

    constexpr int freq = 44100;

    if (bool ok = plugin->sndh->Load(plugin->info->fileBuffer, static_cast<int>(plugin->info->filesize), freq);
        !ok || !plugin->sndh->IsLoaded()) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->BuildHash(plugin->sndh);

    if (!plugin->sndh->InitSubSong(plugin->info->currentSubsong + 1)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/sndh-player.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "continuousPlayback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 1;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    SndhFile::SubSongInfo subsongInfo{};

    plugin->sndh->GetSubsongInfo(plugin->info->currentSubsong + 1, subsongInfo);

    if (subsongInfo.musicAuthor != nullptr) {
        plugin->info->artist = subsongInfo.musicAuthor;
    }
    if (subsongInfo.musicName != nullptr) {
        plugin->info->title = subsongInfo.musicName;
    }
    if (subsongInfo.ripper != nullptr) {
        plugin->info->ripper = subsongInfo.ripper;
    }
    if (subsongInfo.converter != nullptr) {
        plugin->info->converter = subsongInfo.converter;
    }
    if (subsongInfo.year != nullptr) {
        plugin->info->date = subsongInfo.year;
    }

    plugin->info->clockSpeed = subsongInfo.playerTickRate;

    unsigned int ticks = subsongInfo.playerTickCount;

    if (ticks == 0) {
        ticks = plugin->GetTickCountFromSc68();
    }

    plugin->songLength = ticks * subsongInfo.samplePerTick / (plugin->waveformat.frequency / 1000);

    plugin->info->numSubsongs = plugin->sndh->GetSubsongCount();
    plugin->info->numChannels = 4;
    plugin->info->plugin = PLUGIN_sndh_player;
    plugin->info->pluginName = PLUGIN_sndh_player_NAME;
    plugin->info->fileFormat = "SNDH";
    //plugin->info->waveformDisplay = new uint32_t[25600];
    //memset(plugin->info->waveformDisplay, 0, 25600 * sizeof(plugin->info->waveformDisplay));
    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginSndhPlayer *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginSndhPlayer *>(codec->plugindata);

    auto *osc = new uint32_t[18000];
    memset(osc, 0, 18000 * sizeof(*osc));

    plugin->sndh->AudioRender(static_cast<int16_t *>(buffer), static_cast<int>(size), osc);
    plugin->oscBuffer.push(osc);
    plugin->info->waveformDisplay = plugin->oscBuffer.front();

    if (plugin->oscBuffer.size() >= 60) {
        const uint32_t *o = plugin->oscBuffer.front();
        delete[] o;
        plugin->oscBuffer.pop();
    }

    *read = size;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginSndhPlayer *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->songLength; // TODO use lengthpcm?
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
