#include <format>
#include <fstream>
#include "sidplayfp.h"
#include "SidTune.h"
#include "SidInfo.h"
#include "residfp.h"
#include "sidid.h"
#include "sidemu.h"
#include "MUS.h"
#include "SidDatabase.h"
#include "fmod_errors.h"
#include "plugins.h"
#include "info.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

unsigned int getLengthFromDb(const string &databasePath, const string &md5, unsigned int subsong);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libsidplayfp_NAME, // Name.
    0x00012100, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS,
    // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    &getPosition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

constexpr uint8_t voicesPerSidChip = 4;

class pluginLibsidplayfp {
    FMOD_CODEC_STATE *_codec;

public:
    pluginLibsidplayfp(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    static char *loadRom(const char *path, size_t romSize) {
        char *buffer = nullptr;
        ifstream is(path, ios::binary);
        if (is.good()) {
            buffer = new char[romSize];
            is.read(buffer, romSize);
        } else {
            cout << "failed loading rom: " << path << endl;
        }
        is.close();
        return buffer;
    }

    ~pluginLibsidplayfp() {
        delete tune;
        delete [] kernal;
        delete [] basic;
        delete [] chargen;
    }

    Info *info;
    SidTune *tune = nullptr;
    unsigned int subsongs;
    ReSIDfpBuilder *rs;
    sidplayfp *player;
    char *kernal;
    char *basic;
    char *chargen;
    string hvscSonglengthsFile;
    unsigned int seekPosition;
    unsigned int maxVoices;
    bool *mutePtr;
    bool hvscSonglengthsDataBaseEnabled;
    bool isSeeking = false;
    unsigned int length = 0;

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

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize > 1024 * 96) //96kb, biggest sid in hvsc is about 63 kb
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned int bytesread;
    auto *myBuffer = new uint8_t[filesize];

    //rewind file pointer
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    if (memcmp(myBuffer, "PSID", 4) != 0 && memcmp(myBuffer, "RSID", 4) != 0) {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    auto *plugin = new pluginLibsidplayfp(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string kernal_filename = plugin->info->dataPath + KERNAL_BIN_DATA_PATH;
    string basic_filename = plugin->info->dataPath + BASIC_BIN_DATA_PATH;
    string characters_filename = plugin->info->dataPath + CHARACTERS_BIN_DATA_PATH;

    plugin->kernal = pluginLibsidplayfp::loadRom(kernal_filename.c_str(), 8192);
    plugin->basic = pluginLibsidplayfp::loadRom(basic_filename.c_str(), 8192);
    plugin->chargen = pluginLibsidplayfp::loadRom(characters_filename.c_str(), 4096);

    plugin->player = new sidplayfp();

    plugin->player->setRoms((const uint8_t *) plugin->kernal, (const uint8_t *) plugin->basic,
                            (const uint8_t *) plugin->chargen);

    plugin->rs = new ReSIDfpBuilder("Demo");
    // Get the number of SIDs supported by the engine

    plugin->rs->create(plugin->player->info().maxsids());

    // Check if builder is ok
    if (!plugin->rs->getStatus()) {
        delete[] myBuffer;
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    //read sidid
    string sididCfg = plugin->info->dataPath + SIDID_CFG_DATA_PATH;
    plugin->info->songPlayer = identifyBufferFromConfig(sididCfg.c_str(), myBuffer, filesize);

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/libsidplayfp.cfg";
    ifstream ifs(filename.c_str());

    bool useDefaults = false;
    if (ifs.fail()) {
        //The file could not be opened
        useDefaults = true;
    }

    unsigned int freq = 44100;
    bool filter = true;
    SidConfig::playback_t playback = SidConfig::STEREO;
    plugin->waveformat.channels = 2;

    SidConfig::sid_model_t defaultSidModel = SidConfig::MOS6581;
    SidConfig::c64_model_t c64Model = SidConfig::PAL;
    SidConfig::sampling_method_t samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
    bool forceSidModel = false;
    bool forcec64Model = false;

    plugin->hvscSonglengthsDataBaseEnabled = true;
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "frequency") {
                    freq = atoi(value.c_str());
                } else if (word == "playback") {
                    if (value == "left") //old, just for compability
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    } else if (value == "mono") {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    } else if (value == "stereo") {
                        playback = SidConfig::STEREO;
                        plugin->waveformat.channels = 2;
                    } else if (value == "right") //old, just for compability
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    }
                } else if (word == "sampling_method") {
                    if (value == "interpolate") {
                        samplingMethod = SidConfig::INTERPOLATE;
                    } else if (value == "resample/interpolate") {
                        samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
                    }
                } else if (word == "clock_speed") {
                    if (value == "correct") {
                        forcec64Model = false;
                    } else if (value == "pal") {
                        c64Model = SidConfig::PAL;
                        forcec64Model = true;
                    } else if (value == "ntsc") {
                        c64Model = SidConfig::NTSC;
                        forcec64Model = true;
                    } else if (value == "old_ntsc") {
                        c64Model = SidConfig::OLD_NTSC;
                        forcec64Model = true;
                    } else if (value == "drean") {
                        c64Model = SidConfig::DREAN;
                        forcec64Model = true;
                    }
                } else if (word == "sid_model") {
                    if (value == "correct") {
                        forceSidModel = false;
                    } else if (value == "mos6581") {
                        defaultSidModel = SidConfig::MOS6581;
                        forceSidModel = true;
                    } else if (value == "mos8580") {
                        defaultSidModel = SidConfig::MOS8580;
                        forceSidModel = true;
                    }
                } else if (word == "sid_filter") {
                    if (value == "true") {
                        filter = true;
                    } else {
                        filter = false;
                    }
                } else if (word == "hvsc_songlengths_path") {
                    plugin->hvscSonglengthsFile = value;
                } else if (word == "hvsc_songlengths_enabled") {
                    if (value == "true") {
                        plugin->hvscSonglengthsDataBaseEnabled = true;
                    } else {
                        plugin->hvscSonglengthsDataBaseEnabled = false;
                    }
                } else if (word == "continuous_playback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    if (plugin->hvscSonglengthsFile.empty()) {
        plugin->hvscSonglengthsFile = plugin->info->dataPath + HVSC_SONGLENGTHS_PATH;
    }

    plugin->tune = new SidTune(myBuffer, filesize);
    // Check if the tune is valid
    if (!plugin->tune->getStatus()) {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] myBuffer;

    plugin->rs->filter(filter);

    SidConfig cfg;

    cfg.frequency = freq;
    cfg.playback = playback;
    cfg.forceSidModel = forceSidModel;
    cfg.forceC64Model = forcec64Model;
    cfg.defaultSidModel = defaultSidModel;
    cfg.defaultC64Model = c64Model;
    cfg.samplingMethod = samplingMethod;
    cfg.fastSampling = false;
    cfg.sidEmulation = plugin->rs;

    if (!plugin->player->config(cfg)) {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    const SidTuneInfo *s = plugin->tune->getInfo();

    plugin->info->initAddr = s->initAddr();
    plugin->info->loadAddr = s->loadAddr();
    plugin->info->playAddr = s->playAddr();
    plugin->info->songSpeed = s->songSpeed();

    plugin->subsongs = s->songs();
    plugin->tune->selectSong(0);
    plugin->player->load(plugin->tune);
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.frequency = static_cast<int>(cfg.frequency);
    plugin->waveformat.pcmblocksize = 128 * plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1; //infinite length

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    codec->plugindata = plugin; //user data value

    plugin->info->numChannels = voicesPerSidChip * s->sidChips();

    plugin->maxVoices = voicesPerSidChip * plugin->player->info().maxsids();

    plugin->mutePtr = new bool[plugin->maxVoices];

    if (s->numberOfInfoStrings() == 3) {
        plugin->info->title = s->infoString(0);
        plugin->info->artist = s->infoString(1);
        plugin->info->copyright = s->infoString(2);
    }

    string comments;
    for (int i = 0; i < s->numberOfCommentStrings(); i++) {
        comments += '\n' + string(s->commentString(i));
    }

    plugin->info->comments = comments;
    plugin->info->numSamples = 0;

    switch (s->clockSpeed()) {
        case SidTuneInfo::CLOCK_UNKNOWN:
            plugin->info->clockSpeed = 0;
            break;
        case SidTuneInfo::CLOCK_PAL:
            plugin->info->clockSpeed = 1;
            break;
        case SidTuneInfo::CLOCK_NTSC:
            plugin->info->clockSpeed = 2;
            break;
        case SidTuneInfo::CLOCK_ANY:
            plugin->info->clockSpeed = 3;
            break;
    }

    string sidModel;

    switch (s->sidModel(0)) {
        case SidTuneInfo::SIDMODEL_6581:
            sidModel = "6581";
            break;
        case SidTuneInfo::SIDMODEL_8580:
            sidModel = "8580";
            break;
        case SidTuneInfo::SIDMODEL_ANY:
            sidModel = "Any";
            break;
        case SidTuneInfo::SIDMODEL_UNKNOWN:
        default:
            sidModel = "Unknown";
    }

    plugin->info->chips = format("{} (x{})", sidModel, s->sidChips());

    switch (s->compatibility()) {
        case SidTuneInfo::COMPATIBILITY_C64:
            plugin->info->compatibility = 0;
            break;
        case SidTuneInfo::COMPATIBILITY_PSID:
            plugin->info->compatibility = 1;
            break;
        case SidTuneInfo::COMPATIBILITY_R64:
            plugin->info->compatibility = 2;
            break;
        case SidTuneInfo::COMPATIBILITY_BASIC:
            plugin->info->compatibility = 3;
            break;
    }

    if (strcmp(s->formatString(), "C64 Sidplayer format (MUS)") != 0 && strcmp(
            s->formatString(), "C64 Stereo Sidplayer format (MUS+STR)") != 0) {
        plugin->info->md5 = plugin->tune->createMD5New();
    }

    plugin->info->startSubSong = static_cast<int>(s->startSong());
    plugin->info->numSubsongs = static_cast<int>(plugin->subsongs);
    plugin->info->fileformatSpecific = s->formatString();
    plugin->info->plugin = PLUGIN_libsidplayfp;
    plugin->info->pluginName = PLUGIN_libsidplayfp_NAME;
    plugin->info->fileformat = "C64 SID";
    plugin->info->setSeekable(true);

    plugin->seekPosition = 0;

    for (int i = 0; i < plugin->maxVoices; i++) {
        plugin->mutePtr[i] = false;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);
    //    bool skipClick=true;
    //    if(skipClick)
    //    {
    //        if(plugin->player->timeMs()==0)
    //        {
    //            do
    //            {
    //                plugin->player->play((short int*)buffer,size<<1);
    //            }
    //            while(plugin->player->timeMs()<10);
    //        }
    //    }

    unsigned int toRead;

    if (plugin->isSeeking) {
        if (plugin->player->timeMs() < plugin->seekPosition) {
            /*
             * the current way playback & seeking are implemented leads to inaccurate seeking position:
             * higher is the number of rendered samples (per each fmod read) during seeking
             * and higher will be the difference between actual vs expected seeking position.
             * in order to fix the seeking position accuracy issue here
             * the minimum possible number of samples are rendered during the seeking (which is less than 1msec).
             * a better way would be to calculate the number of samples left for arriving to the desired position,
             * but this needs a whole redesign
             */
            plugin->player->play(static_cast<short int *>(buffer), 16 * plugin->waveformat.channels);
            toRead = 16;
        } else {
            for (int i = 0; i < plugin->maxVoices; i++) {
                plugin->player->mute(i / voicesPerSidChip, i % voicesPerSidChip, plugin->mutePtr[i]);
            }

            for (int i = 0; i < plugin->player->info().maxsids(); i++) {
                plugin->player->filter(i, true);
            }

            plugin->player->fastForward(100);
            plugin->isSeeking = false;
            toRead = 0;
        }
    } else {
        plugin->player->play(static_cast<short int *>(buffer), plugin->waveformat.pcmblocksize);
        toRead = plugin->waveformat.pcmblocksize / plugin->waveformat.channels;
    }

    *read = toRead;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS) {
        if (position == 0) {
            if (plugin->player->timeMs() != 0) {
                plugin->player->load(plugin->tune);
            }
        } else {
            for (int i = 0; i < plugin->maxVoices; i++) {
                plugin->player->mute(i / voicesPerSidChip, i % voicesPerSidChip, true);
            }

            for (int i = 0; i < plugin->player->info().maxsids(); i++) {
                plugin->player->filter(i, false);
            }

            plugin->seekPosition = position;

            if (position <= plugin->player->timeMs()) {
                plugin->player->load(plugin->tune);
            }

            plugin->player->fastForward(100 * 32);
            plugin->isSeeking = true;
        }

        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG) {
        plugin->tune->selectSong(position + 1);
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        for (int i = 0; i < plugin->maxVoices; i++) {
            plugin->mutePtr[i] = false;
        }
        //position is a mask
        for (int i = 0; i < plugin->maxVoices; i++) {
            plugin->player->mute(i / voicesPerSidChip, i % voicesPerSidChip, position >> i & 1);
            plugin->mutePtr[i] = position >> i & 1;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLibsidplayfp *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = plugin->waveformat.lengthpcm;
        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MS) {
        if (!plugin->hvscSonglengthsDataBaseEnabled) {
            *length = -1;
        } else {
            if (plugin->length == 0) {
                plugin->length = getLengthFromDb(plugin->hvscSonglengthsFile, plugin->info->md5,
                                                 plugin->tune->getInfo()->currentSong());
            }
            *length = plugin->length;
        }

        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG) {
        *length = plugin->subsongs;
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_SUBSONG_MS) {
        *position = plugin->player->timeMs();
        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_SUBSONG) {
        const SidTuneInfo *s = plugin->tune->getInfo();
        *position = s->currentSong();
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

unsigned int getLengthFromDb(const string &databasePath, const string &md5, unsigned int subsong) {
    const auto sidDb = new SidDatabase(); //TODO sidDatabase->close() && Delete

    if (!sidDb->open(databasePath.c_str())) {
        cout << sidDb->error() << " [" << databasePath.c_str() << "]" << endl;
        return -1;
    }

    const unsigned int length = sidDb->lengthMs(md5.c_str(), subsong);

    delete sidDb;

    if (length == 0) {
        return -1;
    }

    return length;
}
