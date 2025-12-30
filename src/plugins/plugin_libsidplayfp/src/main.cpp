#include <filesystem>
#include <format>
#include <fstream>
#include "residfp.h"
#include "SidDatabase.h"
#include "sidemu.h"
#include "sidid.h"
#include "SidInfo.h"
#include "sidplayfp.h"
#include "SidTune.h"
#include "stil.h"
#include "fmod_errors.h"
#include "info.h"
#include "logger.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libsidplayfp_NAME, // name.
    0x00012100, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL | FMOD_TIMEUNIT_MUTE_VOICE,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    &getPosition,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
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
            logError(string("Failed loading rom ") + path, PLUGIN_libsidplayfp_NAME);
        }

        is.close();
        return buffer;
    }

    unsigned int getLengthFromDb(const string &databasePath, const string &md5, const unsigned int subsong) const {
        if (!sidDb) {
            logError("Unable to get HVSC Songlength database entry (STIL won't be available too)",
                     PLUGIN_libsidplayfp_NAME);
            return -1;
        }

        if (!sidDb->open(databasePath.c_str())) {
            logError(string("Error loading HVSC Songlength database (STIL won't be available too) from ")
                     + databasePath, PLUGIN_libsidplayfp_NAME);
            return -1;
        }

        const unsigned int length = sidDb->lengthMs(md5.c_str(), subsong);

        if (length == 0) {
            return -1;
        }

        return length;
    }

    unsigned int getTimeMs(const sidplayfp *player) const {
        const auto timeMs = player->timeMs();

        if (timeMs == 0 || timeMsOffset > timeMs) {
            return timeMs;
        }

        /*
         * try to match as much as possible the sid time elapsed with the one elapsed since the real playback start
         * in order to avoid both premature playback ending and skipping of very short tracks (with length <200ms)
         */
        return timeMs - timeMsOffset;
    }

    string getSidPathFromOpenedDb(const string &md5) const {
        if (!sidDb) {
            logError("Unable to get HVSC path entry (STIL won't be available too)", PLUGIN_libsidplayfp_NAME);
            return "";
        }

        return sidDb->sidPath(md5.c_str());
    }

    string getStilText(const string &baseDir, const string &stilPath, const string &buglistPath, const char *sidPath,
                       const int subsong) {
        stil = new STIL(stilPath.c_str(), buglistPath.c_str());

        stil->setBaseDir(baseDir.c_str());

        if (stil->getError() != 0) {
            logError(string("HVSC STIL error: ") + stil->getErrorStr(), PLUGIN_libsidplayfp_NAME);
            return "";
        }

        constexpr array fields = {
            STIL::STILField::name,
            STIL::STILField::author,
            STIL::STILField::title,
            STIL::STILField::artist,
            STIL::STILField::comment
        };

        const char *stilEntry = nullptr;
        string stilFinal;

        for (const auto field: fields) {
            stilEntry = stil->getEntry(sidPath, subsong, field);

            if (stilEntry) {
                const auto leadingWhiteSpacesCount = strspn(stilEntry, " ");
                stilFinal += string(stilEntry + leadingWhiteSpacesCount);
            }
        }

        stilEntry = stil->getEntry(sidPath, 0, STIL::STILField::comment);

        if (stilEntry) {
            stilFinal += stilEntry;
        }

        stilEntry = stil->getGlobalComment(sidPath);

        if (stilEntry) {
            stilFinal += stilEntry;
        }

        stilEntry = stil->getBug(sidPath);

        if (stilEntry) {
            const auto leadingWhiteSpacesCount = strspn(stilEntry, " ");
            stilFinal += string(stilEntry + leadingWhiteSpacesCount);
        }

        if (stilFinal.empty()) {
            return "";
        }

        constexpr string_view indentation = "\n         ";

        size_t pos = 0;

        while ((pos = stilFinal.find(indentation, pos)) != string::npos) {
            stilFinal.replace(pos, indentation.length(), " ");
            pos++;
        }

        return stilFinal.erase(stilFinal.find_last_not_of('\n') + 1);
    }

    ~pluginLibsidplayfp() {
        delete mutePtr;
        delete player;
        delete rs;
        delete sidDb;
        delete stil;
        delete tune;
        delete [] kernal;
        delete [] basic;
        delete [] chargen;
    }

    Info *info;
    SidTune *tune = nullptr;
    ReSIDfpBuilder *rs = nullptr;
    sidplayfp *player = nullptr;
    SidDatabase *sidDb = nullptr;
    STIL *stil = nullptr;
    char *kernal = nullptr;
    char *basic = nullptr;
    char *chargen = nullptr;
    string hvscFilesPath;
    unsigned int seekPosition;
    unsigned int maxVoices;
    bool *mutePtr = nullptr;
    bool hvscFilesEnabled;
    bool isSeeking = false;
    unsigned int length = 0;
    unsigned int timeMsOffset = 0;

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
    logDebug("Try", PLUGIN_libsidplayfp_NAME);

    auto *plugin = new pluginLibsidplayfp(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string filePathKernal = plugin->info->dataPath + KERNAL_BIN_DATA_PATH;
    string filePathBasic = plugin->info->dataPath + BASIC_BIN_DATA_PATH;
    string filePathCharacters = plugin->info->dataPath + CHARACTERS_BIN_DATA_PATH;

    plugin->kernal = pluginLibsidplayfp::loadRom(filePathKernal.c_str(), 8192);
    plugin->basic = pluginLibsidplayfp::loadRom(filePathBasic.c_str(), 8192);
    plugin->chargen = pluginLibsidplayfp::loadRom(filePathCharacters.c_str(), 4096);

    plugin->player = new sidplayfp();

    plugin->player->setRoms((const uint8_t *) plugin->kernal, (const uint8_t *) plugin->basic,
                            (const uint8_t *) plugin->chargen);

    plugin->rs = new ReSIDfpBuilder("Demo");

    // get the number of SIDs supported by the engine
    plugin->rs->create(plugin->player->info().maxsids());

    // check if builder is ok
    if (!plugin->rs->getStatus()) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGIN_CONFIGS_DIR "/" CONFIG_FILENAME;
    ifstream ifs(filename.c_str());

    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    unsigned int freq = 44100;
    bool filter = true;
    SidConfig::playback_t playback = SidConfig::STEREO;
    plugin->waveformat.channels = 2;

    SidConfig::sid_model_t defaultSidModel = SidConfig::MOS6581;
    SidConfig::c64_model_t c64Model = plugin->info->isSid ? SidConfig::PAL : SidConfig::NTSC;
    SidConfig::sampling_method_t samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
    bool forceSidModel = false;
    bool forcec64Model = false;

    plugin->hvscFilesEnabled = true;
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
                    if (value == "left") // old, just for compability
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    } else if (value == "mono") {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    } else if (value == "stereo") {
                        playback = SidConfig::STEREO;
                        plugin->waveformat.channels = 2;
                    } else if (value == "right") // old, just for compability
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
                } else if (word == "hvscFilesPath") {
                    plugin->hvscFilesPath = value;
                } else if (word == "hvscFilesEnabled") {
                    if (value == "true") {
                        plugin->hvscFilesEnabled = true;
                    } else {
                        plugin->hvscFilesEnabled = false;
                    }
                } else if (word == "continuousPlayback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    if (plugin->info->isSid) {
        plugin->tune = new SidTune(plugin->info->fileBuffer, static_cast<uint_least32_t>(plugin->info->filesize));
    } else {
        plugin->tune = new SidTune(nullptr, plugin->info->filePath.c_str(), nullptr);
    }

    // check if the tune is valid
    if (!plugin->tune->getStatus()) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

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
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    const SidTuneInfo *s = plugin->tune->getInfo();

    plugin->tune->selectSong(plugin->info->currentSubsong + 1);
    plugin->player->load(plugin->tune);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.frequency = static_cast<int>(cfg.frequency);
    plugin->waveformat.pcmblocksize = 128 * plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    codec->plugindata = plugin; // user data value

    plugin->info->initAddr = s->initAddr();
    plugin->info->loadAddr = s->loadAddr();
    plugin->info->playAddr = s->playAddr();

    plugin->info->numChannels = voicesPerSidChip * s->sidChips();

    plugin->maxVoices = voicesPerSidChip * plugin->player->info().maxsids();

    plugin->mutePtr = new bool[plugin->maxVoices];

    plugin->info->numSamples = 0;

    plugin->info->clockSpeedStr = plugin->player->info().speedString();

    vector<pair<int, int> > chipsPerModel;

    for (int i = 0; i < s->sidChips(); i++) {
        bool found = false;

        for (auto &[chips, model]: chipsPerModel) {
            if (model == s->sidModel(i)) {
                ++chips;
                found = true;
                break;
            }
        }

        if (!found) {
            chipsPerModel.emplace_back(1, s->sidModel(i));
        }
    }

    for (int i = 0; i < chipsPerModel.size(); i++) {
        if (i != 0) {
            plugin->info->chips += ", ";
        }

        plugin->info->chips += format("{} (x{})", [&chipsPerModel, &i] {
            switch (chipsPerModel[i].second) {
                case SidTuneInfo::SIDMODEL_6581:
                    return "6581";
                case SidTuneInfo::SIDMODEL_8580:
                    return "8580";
                case SidTuneInfo::SIDMODEL_ANY:
                    return "Any";
                case SidTuneInfo::SIDMODEL_UNKNOWN:
                default:
                    return "Unknown";
            }
        }(), chipsPerModel[i].first);
    }

    switch (s->compatibility()) {
        case SidTuneInfo::COMPATIBILITY_C64:
            plugin->info->compatibility = "C64 compatible";
            break;
        case SidTuneInfo::COMPATIBILITY_PSID:
            plugin->info->compatibility = "PSID specific";
            break;
        case SidTuneInfo::COMPATIBILITY_R64:
            plugin->info->compatibility = "Real C64 only";
            break;
        case SidTuneInfo::COMPATIBILITY_BASIC:
            plugin->info->compatibility = "Requires C64 Basic";
            break;
        default:
            plugin->info->compatibility = "Unknown";
    }

    if (plugin->info->isSid) {
        if (s->numberOfInfoStrings() == 3) {
            plugin->info->title = s->infoString(0);
            plugin->info->artist = s->infoString(1);
            plugin->info->copyright = s->infoString(2);
        }

        const string sididCfg = plugin->info->dataPath + SIDID_CFG_DATA_PATH;
        plugin->info->songPlayer = identifyBufferFromConfig(sididCfg.c_str(), plugin->info->fileBuffer,
                                                            static_cast<int>(plugin->info->filesize));

        plugin->info->md5 = plugin->tune->createMD5New();
    } else {
        string comments;
        for (int i = 0; i < s->numberOfCommentStrings(); i++) {
            comments += '\n' + string(s->commentString(i));
        }

        plugin->info->comments = comments;
    }

    plugin->info->numSubsongs = static_cast<int>(s->songs());

    if (plugin->info->numSubsongs > 1) {
        plugin->info->defaultSubsong = static_cast<int>(s->startSong());
    }

    plugin->info->fileFormat = s->formatString();
    plugin->info->plugin = PLUGIN_libsidplayfp;
    plugin->info->pluginName = PLUGIN_libsidplayfp_NAME;

    plugin->info->setSeekable(true);

    plugin->seekPosition = 0;

    for (int i = 0; i < plugin->maxVoices; i++) {
        plugin->mutePtr[i] = false;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLibsidplayfp *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);
    //    bool skipClick=true;
    //    if(skipClick)
    //    {
    //        if(plugin->getTimeMs(plugin->player) == 0)
    //        {
    //            do
    //            {
    //                plugin->player->play((short int*)buffer,size<<1);
    //            }
    //            while(plugin->getTimeMs(plugin->player) < 10);
    //        }
    //    }

    unsigned int toRead;

    if (plugin->isSeeking) {
        if (plugin->getTimeMs(plugin->player) < plugin->seekPosition) {
            /*
             * the current way playback & seeking are implemented leads to inaccurate seeking position:
             * higher is the number of rendered samples (per each fmod read) during seeking
             * and higher will be the difference between actual vs expected seeking position.
             * in order to fix the seeking position accuracy issue here
             * the minimum possible number of samples are rendered during the seeking (which is less than 1ms).
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
            if (plugin->getTimeMs(plugin->player) != 0) {
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

            if (position <= plugin->getTimeMs(plugin->player)) {
                plugin->player->load(plugin->tune);
            }

            plugin->player->fastForward(100 * 32);
            plugin->isSeeking = true;
        }

        return FMOD_OK;
    }
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        for (int i = 0; i < plugin->maxVoices; i++) {
            plugin->mutePtr[i] = false;
        }
        // position is a mask
        for (int i = 0; i < plugin->maxVoices; i++) {
            plugin->player->mute(i / voicesPerSidChip, i % voicesPerSidChip, position >> i & 1);
            plugin->mutePtr[i] = position >> i & 1;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        // this is the sid time elapsed for initial fmod pre-buffering
        if (plugin->timeMsOffset == 0) {
            plugin->timeMsOffset = plugin->player->timeMs();
        }

        if (!plugin->info->isSid || !plugin->hvscFilesEnabled) {
            *length = -1;
            return FMOD_OK;
        }

        if (plugin->length != 0) {
            *length = plugin->length;
            return FMOD_OK;
        }

        plugin->sidDb = new SidDatabase();

        if (plugin->hvscFilesPath.empty()) {
            plugin->hvscFilesPath = plugin->info->dataPath + SID_DATA_DIR;
        }

        string baseDir;
        string songlengthsPath;
        string stilPath;
        string buglistPath;

        if (plugin->hvscFilesPath.empty()) {
            baseDir = plugin->info->dataPath + SID_DATA_DIR;
            songlengthsPath = "/" HVSC_SONGLENGTHS_FILENAME;
            stilPath = "/" HVSC_STIL_FILENAME;
            buglistPath = "/" HVSC_BUGLIST_FILENAME;
        } else if (plugin->hvscFilesPath == plugin->info->dataPath + SID_DATA_DIR ||
                   plugin->hvscFilesPath == plugin->info->userPath + SID_DATA_DIR) {
            baseDir = plugin->hvscFilesPath;
            songlengthsPath = baseDir + "/" HVSC_SONGLENGTHS_FILENAME;
            stilPath = "/" HVSC_STIL_FILENAME;
            buglistPath = "/" HVSC_BUGLIST_FILENAME;
        } else {
            const filesystem::path p(plugin->hvscFilesPath);
            baseDir = p.parent_path().string();
            songlengthsPath = baseDir + "/DOCUMENTS/" HVSC_SONGLENGTHS_FILENAME;
            stilPath = "/DOCUMENTS/" HVSC_STIL_FILENAME;
            buglistPath = "/DOCUMENTS/" HVSC_BUGLIST_FILENAME;
        }

        plugin->length = plugin->getLengthFromDb(songlengthsPath,
                                                 plugin->info->md5,
                                                 plugin->info->currentSubsong + 1);

        *length = plugin->length;

        const string sidPath = plugin->getSidPathFromOpenedDb(plugin->info->md5);

        if (plugin->sidDb) {
            plugin->sidDb->close();
        }

        if (sidPath.empty()) {
            return FMOD_OK;
        }

        plugin->info->collectionEntry = sidPath;


        plugin->info->comments = plugin->getStilText(baseDir,
                                                     stilPath,
                                                     buglistPath,
                                                     sidPath.c_str(),
                                                     plugin->info->currentSubsong + 1);

        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getPosition(FMOD_CODEC_STATE *codec, unsigned int *position, FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginLibsidplayfp *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS_REAL) {
        *position = plugin->getTimeMs(plugin->player);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
