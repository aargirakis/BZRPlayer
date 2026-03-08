#include <cstring>
#include <format>
#include <fstream>
#include <iterator>
#include <sstream>
#include "uade/eagleplayer.h"
#include "uade/uade.h"
#include "BaseSample.h"
#include "FileLoader.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

const string TYPE_PREFIX = "type: ";

unsigned int getLengthFromDatabase(const char *, int, const char *, const char *);

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_uade_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginUade {
    FMOD_CODEC_STATE *_codec;

public:
    pluginUade(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginUade() {
        //delete some stuff
        uade_cleanup_state(uadeState);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;

    Info *info;
    bool uade_songlengths_enabled;
    string uade_songlengthspath;
    uade_state *uadeState = nullptr;
    const struct uade_song_info *uadeSongInfo;
    unsigned int length = -1;
    bool isSeekSkipped = true;
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
    auto *plugin = new pluginUade(codec);

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize == 4294967295) //stream
    {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    auto *smallBuffer = new uint8_t[4];
    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, smallBuffer, 4, nullptr);

    // skip midi, riff and psf
    if (memcmp(smallBuffer, "MThd", 4) == 0 || memcmp(smallBuffer, "RIFF", 4) == 0 ||
        memcmp(smallBuffer, "PSF", 3) == 0) {
        delete[] smallBuffer;
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    delete[] smallBuffer;

    plugin->info = static_cast<Info *>(userexinfo->userdata);

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/uade.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    string frequency = "48000";
    string resampler = "sinc";
    int filter_emu = 1;
    string filter_mode = "a1200";
    int led_forced = 0;
    int led_state = 0;
    string panning = "0.5";
    string silence_timeout = "5";
    bool silence_timeout_enabled = true;
    plugin->uade_songlengths_enabled = true;
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "frequency") {
                    frequency = value;
                } else if (word == "resampler") {
                    resampler = value;
                } else if (word == "filterEmu") {
                    if (value == "true") {
                        filter_emu = 1;
                    } else {
                        filter_emu = 0;
                    }
                } else if (word == "filterMode") {
                    filter_mode = value;
                } else if (word == "ledForced") {
                    if (value == "auto") {
                        led_forced = 0;
                    } else if (value == "on") {
                        led_forced = 1;
                        led_state = 1;
                    } else {
                        led_forced = 1;
                        led_state = 0;
                    }
                } else if (word == "panning") {
                    int x = stoi(value);
                    panning = format("{}.{}", x / 10, x % 10);
                } else if (word == "silenceTimeout") {
                    silence_timeout = value;
                } else if (word == "silenceTimeoutEnabled") {
                    if (value == "true") {
                        silence_timeout_enabled = true;
                    } else {
                        silence_timeout_enabled = false;
                    }
                } else if (word == "continuousPlayback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                } else if (word == "uadeSonglengthsPath") {
                    plugin->uade_songlengthspath = value;
                } else if (word == "uadeSonglengthsEnabled") {
                    if (value == "true") {
                        plugin->uade_songlengths_enabled = true;
                    } else {
                        plugin->uade_songlengths_enabled = false;
                    }
                }
            }
        }
        ifs.close();
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = UADE_CHANNELS;
    plugin->waveformat.frequency = stoi(frequency);
    plugin->waveformat.pcmblocksize = 2048 * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    if (plugin->uade_songlengthspath.empty() || plugin->uade_songlengthspath == "/uade.md5") {
        plugin->uade_songlengthspath = plugin->info->dataPath + UADE_DATA_DIR + "/uade.md5";
    }

    uade_config *uadeConfig = uade_new_config();

    char uade_basedir[PATH_MAX];
    snprintf(uade_basedir, PATH_MAX, "%s%s", plugin->info->dataPath.c_str(), UADE_DATA_DIR);

    char uade_core[PATH_MAX];
    snprintf(uade_core, PATH_MAX, "%s%s/%s", plugin->info->libPath.c_str(), PLUGINS_DIR, UADE_CORE);

    uade_config_set_option(uadeConfig, UC_BASE_DIR, uade_basedir);
    uade_config_set_option(uadeConfig, UC_UADECORE_FILE, uade_core);
    uade_config_set_option(uadeConfig, UC_NO_CONTENT_DB, nullptr);
    uade_config_set_option(uadeConfig, UC_ONE_SUBSONG, nullptr);

    if (plugin->info->isContinuousPlaybackActive) {
        uade_config_set_option(uadeConfig, UC_NO_EP_END, nullptr);
        uade_config_set_option(uadeConfig, UC_DISABLE_TIMEOUTS, nullptr);
        uade_config_set_option(uadeConfig, UC_TIMEOUT_VALUE, "-1");
        uade_config_set_option(uadeConfig, UC_SUBSONG_TIMEOUT_VALUE, "-1");
        uade_config_set_option(uadeConfig, UC_SILENCE_TIMEOUT_VALUE, "-1");
    } else {
        uade_config_set_option(uadeConfig, UC_ENABLE_TIMEOUTS, nullptr);
        uade_config_set_option(uadeConfig, UC_SILENCE_TIMEOUT_VALUE,
                               silence_timeout_enabled ? silence_timeout.c_str() : "-1");
    }

    uade_config_set_option(uadeConfig, UC_FREQUENCY, frequency.c_str());
    uade_config_set_option(uadeConfig, UC_RESAMPLER, resampler.c_str());
    uade_config_set_option(uadeConfig, UC_FILTER_TYPE, filter_emu ? filter_mode.c_str() : "none");

    if (led_forced) {
        uade_config_set_option(uadeConfig, UC_FORCE_LED, led_state ? "on" : "off");
    }

    uade_config_set_option(uadeConfig, UC_PANNING_VALUE, panning.c_str());
    uade_config_set_option(uadeConfig, UC_NO_HEADPHONES, nullptr);

#ifndef NDEBUG
    uade_config_set_option(uadeConfig, UC_VERBOSE, nullptr);
#endif

    plugin->uadeState = uade_new_state(uadeConfig);

    if (plugin->uadeState == nullptr) {
        return FMOD_ERR_FORMAT;
    }

#ifndef NDEBUG
    uade_enable_uadecore_log_collection(plugin->uadeState);
#endif

    auto myBuffer = new uint8_t[filesize];

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, nullptr);

    if (uade_play_from_buffer(plugin->info->filename.c_str(), myBuffer, filesize, -1, plugin->uadeState) <= 0) {
        cout << "Can not play " << plugin->info->filename << endl;
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->uadeSongInfo = uade_get_song_info(plugin->uadeState);

    if (const int subsong = plugin->info->currentSubsong + plugin->uadeSongInfo->subsongs.min;
        subsong > plugin->uadeSongInfo->subsongs.min) {
        /*
         * uade seems buggy, since
         * uade_seek(UADE_SEEK_SUBSONG_RELATIVE, 0, subsong, plugin->uadeState)
         * cause audio issue (see #616)
         * so in order to change subsong uade_play_from_buffer() is invoked again
         */

        uade_stop(plugin->uadeState);

        if (uade_play_from_buffer(plugin->info->filename.c_str(), myBuffer, filesize, subsong, plugin->uadeState) <=
            0) {
            cout << "Can not play " << plugin->info->filename << " subsong " << subsong << endl;
            delete[] myBuffer;
            return FMOD_ERR_FORMAT;
        }
    }

    delete[] myBuffer;

    if (plugin->uadeSongInfo->formatname[0]) {
        string str(plugin->uadeSongInfo->formatname);
        plugin->info->fileformat = str.starts_with(TYPE_PREFIX)
                                       ? str.substr(TYPE_PREFIX.length())
                                       : plugin->uadeSongInfo->formatname;
    } else {
        plugin->info->fileformat = plugin->uadeSongInfo->detectioninfo.ep->playername;
    }

    if (plugin->uadeSongInfo->modulename[0]) {
        plugin->info->title = plugin->uadeSongInfo->modulename;
    }

    plugin->info->numSubsongs = 1 + (plugin->uadeSongInfo->subsongs.max - plugin->uadeSongInfo->subsongs.min);
    plugin->info->numSamples = 0;
    plugin->info->plugin = PLUGIN_uade;
    plugin->info->pluginName = PLUGIN_uade_NAME;

    if (plugin->info->fileformat == "BenDaglish" || plugin->info->fileformat == "DeltaMusic1.3" || plugin->info->
        fileformat ==
        "DeltaMusic2.0" || plugin->info->fileformat == "DavidWhittaker"
        || plugin->info->fileformat == "Fred" || plugin->info->fileformat == "Infogrames" || plugin->info->fileformat ==
        "JasonBrooke" || plugin->info->fileformat == "JochenHippel"
        || plugin->info->fileformat == "JochenHippel-CoSo" || plugin->info->fileformat == "Mugician" || plugin->info->
        fileformat ==
        "MugicianII"
        || plugin->info->fileformat == "RobHubbard" || plugin->info->fileformat == "RichardJoseph" || plugin->info->
        fileformat ==
        "SIDMon1.0" || plugin->info->fileformat == "SIDMon2.0"
        || plugin->info->fileformat == "PaulShields") {
        //read samples
        auto d = new uint8_t[filesize];
        FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        FMOD_CODEC_FILE_READ(codec, d, filesize, nullptr);

        auto *fileLoader = new FileLoader();

        AmigaPlayer *player = fileLoader->load(d, filesize, plugin->info->filename.c_str());

        delete fileLoader;
        delete[] d;

        if (player) {
            vector<BaseSample *> samples;
            samples = player->getSamples();
            if (!samples.empty()) {
                plugin->info->numSamples = samples.size();
                plugin->info->samples = new string[plugin->info->numSamples];
                plugin->info->samplesSize = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesLoopStart = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesLoopLength = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesVolume = new unsigned short[plugin->info->numSamples];

                for (int j = 0; j < plugin->info->numSamples; j++) {
                    if (samples[j]) {
                        plugin->info->samples[j] = samples[j]->name;
                        plugin->info->samplesSize[j] = samples[j]->length;
                        plugin->info->samplesVolume[j] = samples[j]->volume;
                    }
                }
            }

            delete player;
        }
    }

    plugin->info->md5 = plugin->uadeSongInfo->modulemd5;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginUade *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginUade *>(codec->plugindata);

    const ssize_t renderedBytes = uade_read(
        buffer, plugin->waveformat.pcmblocksize * UADE_BYTES_PER_FRAME, plugin->uadeState);

    /* read variable must be set before checking notifications,
     * in order inform fmod how many audio bytes still left to play in case of song end notification
     * otherwise, tracks that ends too early (e.g. last subsong of Turrican 2 mdat.world_1)
     * will continue play forever.
     * moreover, its value must be renderedBytes and not fixed (ie plugin->waveformat.pcmblocksize),
     * otherwise, after a song end notification the audio bytes still left to play will include
     * (part of) looped track data, producing a final audio pop.
     */
    *read = static_cast<unsigned int>(renderedBytes / UADE_BYTES_PER_FRAME);

    uade_notification un = {};
    while (uade_read_notification(&un, plugin->uadeState)) {
        switch (un.type) {
            case UADE_NOTIFICATION_MESSAGE:
                cout << "Amiga message: " << un.msg << endl;
                break;
            case UADE_NOTIFICATION_SONG_END:
                cout << (un.song_end.happy ? "" : "bad ") << "song end" << ": " << un.song_end.reason << endl;

                //FMOD_ERR_FILE_EOF is used here in order to let fmod playing the remaining audio bytes
                return FMOD_ERR_FILE_EOF;

            default:
                cout << "Unknown libuade notification" << endl;
                break;
        }

        uade_cleanup_notification(&un);
    }

    // should never happen (unless the file is just deleted instants before)
    if (renderedBytes < 0) {
        return FMOD_ERR_FILE_NOTFOUND;
    }

    /*
     * renderedBytes should never reach value 0 when uade.md5 provides the song length
     * however this check *might* be still useful if no UADE_NOTIFICATION_SONG_END is received
     */
    if (renderedBytes == 0) {
        return FMOD_ERR_FORMAT;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    auto *plugin = static_cast<pluginUade *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        if (!plugin->uade_songlengths_enabled) {
            *length = -1;
        } else {
            if (plugin->length == -1) {
                plugin->length = getLengthFromDatabase(plugin->info->filename.c_str(), plugin->info->currentSubsong,
                                                       plugin->uadeSongInfo->modulemd5,
                                                       plugin->uade_songlengthspath.c_str());
            }

            *length = plugin->length;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    auto *plugin = static_cast<pluginUade *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        if (plugin->isSeekSkipped) {
            plugin->isSeekSkipped = false;
            return FMOD_OK;
        }

        uade_seek(UADE_SEEK_SUBSONG_RELATIVE, position / 1000.0, plugin->uadeSongInfo->subsongs.cur, plugin->uadeState);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

unsigned int getLengthFromDatabase(const char *filename, int subsong, const char *md5, const char *database) {
    unsigned int length = 0;

    string filenameFromDb = database;

    ifstream ifs(filenameFromDb);

    if (ifs.fail()) {
        //The file could not be opened
        cout << "Couldn't open UADE songlengths file: " << filenameFromDb << endl;
        return 0;
    }

    string line;
    while (getline(ifs, line)) {
        if (line.substr(0, 32) == md5) {
            //we found it
            int j = line.find_first_of("=");
            if (j == -1) {
                cout << "Formatting error in uade songlengths file: " << filenameFromDb << endl;
                return 0;
            }

            line = line.substr(j + 1);
            stringstream ss(line);
            istream_iterator<string> begin(ss);
            istream_iterator<string> end;
            vector vstrings(begin, end);

            if (subsong >= vstrings.size()) {
                cout << "Subsong length not found" << endl;
                return 0;
            }

            line = vstrings.at(subsong);

            int i = line.find_first_of(":");
            if (i == -1) {
                cout << "Formatting error in uade songlengths file: " << filenameFromDb << endl;
                return 0;
            }

            int msk = line.find_first_of(".");

            int ms = 0;
            if (msk != -1) //There are milliseconds
            {
                ms = atoi(line.substr(msk + 1).c_str());
                if (string str_ms = line.substr(msk + 1); str_ms.size() == 2) {
                    ms *= 10;
                } else if (str_ms.size() == 1) {
                    ms *= 100;
                }
            }

            int sec = atoi(line.substr(i + 1, i + 3).c_str()) * 1000;

            int min = atoi(line.substr(0, i).c_str()) * 1000 * 60;
            length = min + sec + ms;
            break;
        }
    }

    if (length == 0) {
        cout << "Couldn't find song length for: " << filename << " [Hash: " << md5 << "]" << endl;
    }

    return length;
}
