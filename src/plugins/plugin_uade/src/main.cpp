#include "BaseSample.h"
#include "FileLoader.h"
#include <cstring>
#include <cstdio>
#include <format>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <uade/eagleplayer.h>
#include <uade/uade.h>
#ifdef __cplusplus
}
#endif

const string TYPE_PREFIX = "type: ";
unsigned int getLengthFromDatabase(const char*, int, const char*, const char*);

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_uade_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginUade
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginUade(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginUade()
    {
        //delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;

    Info* info;
    bool uade_songlengths_enabled;
    bool isContinuousPlaybackActive;
    int currentSubsong;
    string uade_songlengthspath;
    uade_state *uadeState;
    const struct uade_song_info *uadeSongInfo;
    unsigned int filesize;
    unsigned char *myBufferP;
    bool setPositionWithTimeunitSubSongHasBeenInvoked = false;
    unsigned int totalSkippedBytes = 0;
    bool isUadeSeekInvocationAllowed = false;
};

#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto *plugin = new pluginUade(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    FMOD_CODEC_FILE_SIZE(codec, &plugin->filesize);
    if (plugin->filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    char* smallBuffer;
    smallBuffer = new char[4];
    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, smallBuffer, 4, nullptr);
    if (((smallBuffer[0] == 'M' && smallBuffer[1] == 'T' && smallBuffer[2] == 'h' && smallBuffer[3] == 'd') ||
            smallBuffer[0] == 'R' && smallBuffer[1] == 'I' && smallBuffer[2] == 'F' && smallBuffer[3] == 'F')
        //it's a midi file
        || (smallBuffer[0] == 'P' && smallBuffer[1] == 'S' && smallBuffer[2] == 'F') //it's a psf file
    )
    {
        delete[] smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] smallBuffer;

    //check if it's a IMPlay Song Format (UADE might pick it up as "Images Music System"
    if (plugin->filesize >= 62)
    {
        unsigned char* testBuffer;
        testBuffer = new unsigned char[62];
        FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        FMOD_CODEC_FILE_READ(codec, testBuffer, 62, nullptr);

        unsigned char majorVersion = testBuffer[0];
        unsigned char minorVersion = testBuffer[1];
        unsigned int tuneId = testBuffer[2] + testBuffer[3] * 256 + testBuffer[4] * 256 * 256 + testBuffer[5] * 256 *
            256 * 256;
        unsigned char tickBeat = testBuffer[36];
        unsigned char beatMeasure = testBuffer[37];

        unsigned int totalTick = testBuffer[38] + testBuffer[39] + testBuffer[40] + testBuffer[41];
        unsigned int dataSize = testBuffer[42] + testBuffer[43] * 256 + testBuffer[44] * 256 * 256 + testBuffer[45] *
            256 * 256 * 256;
        unsigned int nrCommand = testBuffer[46] + testBuffer[47] * 256 + testBuffer[48] * 256 * 256 + testBuffer[49] *
            256 * 256 * 256;


        // validate header and data size
        if (majorVersion != 1 ||
            minorVersion != 0 ||
            tuneId != 0 ||
            tickBeat == 0 ||
            beatMeasure == 0 ||
            totalTick == 0 ||
            dataSize == 0 ||
            nrCommand == 0 ||
            plugin->filesize < (70 + dataSize))
        {
            //This is "probably" not IMPlay Song Format
        }
        else
        {
            delete[] testBuffer;
            return FMOD_ERR_FORMAT;
        }
        delete[] testBuffer;
    }

    //check if it's a IMPlay Song Format
    //    majorVersion = static_cast<uint8_t>(f->readInt(1));
    //	minorVersion = static_cast<uint8_t>(f->readInt(1));
    //	uint32_t tuneId = static_cast<uint32_t>(f->readInt(4));
    //	f->readString(tuneName, TUNE_NAME_SIZE);
    //	tickBeat = static_cast<uint8_t>(f->readInt(1));
    //	uint8_t beatMeasure = static_cast<uint8_t>(f->readInt(1));
    //	uint32_t totalTick = static_cast<uint32_t>(f->readInt(4));
    //	dataSize = static_cast<uint32_t>(f->readInt(4));
    //	uint32_t nrCommand = static_cast<uint32_t>(f->readInt(4));
    //	f->seek(FILLER_SIZE, binio::Add);
    //	soundMode = static_cast<uint8_t>(f->readInt(1));
    //	pitchBRange = static_cast<uint8_t>(f->readInt(1));
    //	basicTempo = static_cast<uint16_t>(f->readInt(2));
    //	f->seek(FILLER_SIZE, binio::Add);

    //	// validate header and data size
    //	if (majorVersion != 1 ||
    //		minorVersion != 0 ||
    //		tuneId != 0 ||
    //		tickBeat == 0 ||
    //		beatMeasure == 0 ||
    //		totalTick == 0 ||
    //		dataSize == 0 ||
    //		nrCommand == 0 ||
    //		fp.filesize(f) < (unsigned)(HEADER_LEN + dataSize))

    auto myBuffer = new unsigned char[plugin->filesize];
    plugin->myBufferP = myBuffer;

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, myBuffer, plugin->filesize, nullptr);

    auto const *info = static_cast<Info *>(userexinfo->userdata);

    //read config from disk
    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/uade.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail())
    {
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
    plugin->isContinuousPlaybackActive = false;

    if (!useDefaults)
    {
        string line;
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "frequency")
                {
                    frequency = value;
                }
                else if (word == "resampler")
                {
                    resampler = value;
                }
                else if (word == "filter_emu")
                {
                    if (value == "true")
                    {
                        filter_emu = 1;
                    }
                    else
                    {
                        filter_emu = 0;
                    }
                }
                else if (word == "filter_mode")
                {
                    filter_mode = value;
                }
                else if (word == "led_forced")
                {
                    if (value == "auto")
                    {
                        led_forced = 0;
                    }
                    else if (value == "on")
                    {
                        led_forced = 1;
                        led_state = 1;
                    }
                    else
                    {
                        led_forced = 1;
                        led_state = 0;
                    }
                }
                else if (word == "panning")
                {
                    int x = stoi(value);
                    panning = std::format("{}.{}", x / 10, x % 10);
                }
                else if (word == "silence_timeout")
                {
                    silence_timeout = value;
                }
                else if (word == "silence_timeout_enabled")
                {
                    if (value == "true")
                    {
                        silence_timeout_enabled = true;
                    }
                    else
                    {
                        silence_timeout_enabled = false;
                    }
                }
                else if (word == "continuous_playback")
                {
                    plugin->isContinuousPlaybackActive = info->isPlayModeRepeatSongEnabled && value == "true";
                }
                else if (word == "uade_songlengths_path")
                {
                    plugin->uade_songlengthspath = value;
                }
                else if (word == "uade_songlengths_enabled")
                {
                    if (value == "true")
                    {
                        plugin->uade_songlengths_enabled = true;
                    }
                    else
                    {
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
    plugin->waveformat.lengthpcm = 0xffffffff;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    if (plugin->uade_songlengthspath.empty() || plugin->uade_songlengthspath == "/uade.md5")
    {
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

    if (plugin->isContinuousPlaybackActive) {
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

    plugin->currentSubsong = -1;

    if (uade_play_from_buffer(plugin->info->filename.c_str(), myBuffer, plugin->filesize, plugin->currentSubsong,
                              plugin->uadeState) <= 0) {
        cout << "Can not play " << plugin->info->filename << endl;
        return FMOD_ERR_FORMAT;
    }

    plugin->uadeSongInfo = uade_get_song_info(plugin->uadeState);

    if (plugin->uadeSongInfo->formatname[0]) {
        string str(plugin->uadeSongInfo->formatname);
        plugin->info->fileformat = str.starts_with(TYPE_PREFIX)?
                                       str.substr(TYPE_PREFIX.length()) : plugin->uadeSongInfo->formatname;
    } else {
        plugin->info->fileformat = plugin->uadeSongInfo->detectioninfo.ep->playername;
    }

    if (plugin->uadeSongInfo->modulename[0]) {
        plugin->info->title = plugin->uadeSongInfo->modulename;
    }

    plugin->info->numSamples = 0;
    plugin->info->plugin = PLUGIN_uade;
    plugin->info->pluginName = PLUGIN_uade_NAME;

    if (plugin->info->fileformat == "BenDaglish" || plugin->info->fileformat == "DeltaMusic1.3" || plugin->info->fileformat ==
        "DeltaMusic2.0" || plugin->info->fileformat == "DavidWhittaker"
        || plugin->info->fileformat == "Fred" || plugin->info->fileformat == "Infogrames" || plugin->info->fileformat ==
        "JasonBrooke" || plugin->info->fileformat == "JochenHippel"
        || plugin->info->fileformat == "JochenHippel-CoSo" || plugin->info->fileformat == "Mugician" || plugin->info->fileformat ==
        "MugicianII"
        || plugin->info->fileformat == "RobHubbard" || plugin->info->fileformat == "RichardJoseph" || plugin->info->fileformat ==
        "SIDMon1.0" || plugin->info->fileformat == "SIDMon2.0"
        || plugin->info->fileformat == "PaulShields"
    )
    {
        //read samples
        auto d = new char[plugin->filesize];
        FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        FMOD_CODEC_FILE_READ(codec, d, plugin->filesize, nullptr);

        auto* fileLoader = new FileLoader();

        AmigaPlayer* player = fileLoader->load((signed short*)d, plugin->filesize, plugin->info->filename.c_str());

        delete fileLoader;
        delete[] d;

        if (player)
        {
            std::vector<BaseSample*> samples;
            samples = player->getSamples();
            if (!samples.empty())
            {
                plugin->info->numSamples = samples.size();
                plugin->info->samples = new string[plugin->info->numSamples];
                plugin->info->samplesSize = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesLoopStart = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesLoopLength = new unsigned int[plugin->info->numSamples];
                plugin->info->samplesVolume = new unsigned short[plugin->info->numSamples];

                for (int j = 0; j < plugin->info->numSamples; j++)
                {
                    if (samples[j])
                    {
                        plugin->info->samples[j] = samples[j]->name;
                        plugin->info->samplesSize[j] = samples[j]->length;
                        plugin->info->samplesVolume[j] = samples[j]->volume;
                    }
                }
            }

            delete player;
        }
    }

    plugin->info->md5New = plugin->uadeSongInfo->modulemd5;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginUade*>(codec->plugindata);
    uade_cleanup_state(plugin->uadeState);
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto *plugin = static_cast<pluginUade *>(codec->plugindata);

    /*
     * when track playback starts fmod pre-buffers some audio bytes,
     * however this happens before setposition with FMOD_TIMEUNIT_SUBSONG postype is invoked,
     * so before bzr2 can be aware of any subsong neither change it:
     * this leads to breaking those scenario where a track has first subsong immediately ending (e.g. rjp.ingame_1),
     * causing bzr2 to play only the first subsong instead of going to the next one.
     * the following workaround forces the uade playback
     * to start only after setposition with FMOD_TIMEUNIT_SUBSONG postype has been invoked
     * (skipping the fmod pre-buffering meanwhile).
     */
    if (!plugin->setPositionWithTimeunitSubSongHasBeenInvoked) {
        *read = 8;
        plugin->totalSkippedBytes += *read;
        return FMOD_OK;
    }

    plugin->isUadeSeekInvocationAllowed = true;

    ssize_t renderedBytes = uade_read(
        buffer, plugin->waveformat.pcmblocksize * UADE_BYTES_PER_FRAME, plugin->uadeState);

    /* read variable must be set before checking notifications,
     * in order inform fmod how many audio bytes still left to play in case of song end notification
     * otherwise, tracks that ends too early (e.g. last subsong of Turrican 2 mdat.world_1)
     * will continue play forever.
     * moreover, its value must be renderedBytes and not fixed (ie plugin->waveformat.pcmblocksize),
     * otherwise, after a song end notification the audio bytes still left to play will include
     * (part of) looped track data, producing a final audio pop.
     */
    *read = static_cast<unsigned int>(renderedBytes) / UADE_BYTES_PER_FRAME;

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

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginUade*>(codec->plugindata);
    if (plugin->currentSubsong == -1)
    {
        plugin->currentSubsong = plugin->uadeSongInfo->subsongs.min;
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = 1 + (plugin->uadeSongInfo->subsongs.max - plugin->uadeSongInfo->subsongs.min);
        return FMOD_OK;
    }

    int sub = plugin->currentSubsong;
    if (plugin->uadeSongInfo->subsongs.min == 1)
    {
        sub = plugin->currentSubsong - 1;
    }

    unsigned int songLength;
    if (plugin->uade_songlengths_enabled && !plugin->isContinuousPlaybackActive)
    {
        songLength = getLengthFromDatabase(plugin->info->filename.c_str(), sub, plugin->uadeSongInfo->modulemd5,
                                           plugin->uade_songlengthspath.c_str());
    }
    else
    {
        songLength = 0;
    }

    *length = songLength;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginUade*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        /*
         * issue #616 workaround:
         *
         * uade_seek might be used for initial subsong setting,
         * introduces it audio pops, also when invoked before any uade_read:
         * so, in order to avoid any audio pop when playback starts,
         * uade_stop & uade_play_from_buffer are invoked for initial subsong setting.
         *
         * after the subsong has been set uade_seek invocations must be allowed again
         * (for catching user's playback seeking actions).
         *
         * two different strategies can be used:
         *
         * 1) allow it after the second setposition (with FMOD_TIMEUNIT_MS) invocation (using a counter)
         *    the first invocation (coming from a fmod internal call) happens before fmod pre-buffering stage
         *    and the second one (coming from bzr2 code) happens after the subsong has been already set
         *
         * 2) allow it after fmod pre-buffering stage (using a bool)
         *    since it means the subsong has been already set
         *
         * the 2nd strategy has been preferred since it is more straightforward
         * and does not strictly depend on non-plugin audio code details
         */
        if (plugin->isUadeSeekInvocationAllowed) {
            uade_seek(UADE_SEEK_SUBSONG_RELATIVE, static_cast<double>(position) / 1000, plugin->currentSubsong,
                      plugin->uadeState);
        }

        return FMOD_OK;
    }

    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->setPositionWithTimeunitSubSongHasBeenInvoked = true;

        if (plugin->uadeSongInfo->subsongs.max - plugin->uadeSongInfo->subsongs.min == 0)
        {
            return FMOD_OK;
        }
        if (position > plugin->uadeSongInfo->subsongs.max - plugin->uadeSongInfo->subsongs.min)
        {
            position = plugin->uadeSongInfo->subsongs.min;
        }
        else
        {
            position = plugin->uadeSongInfo->subsongs.min + position;
        }

        plugin->currentSubsong = position;

        uade_stop(plugin->uadeState);
        uade_play_from_buffer(plugin->info->filename.c_str(), plugin->myBufferP, plugin->filesize,
                              plugin->currentSubsong,
                              plugin->uadeState);

        return FMOD_OK;
    }
    return FMOD_OK;
}

unsigned int getLengthFromDatabase(const char* filename, int subsong, const char* md5, const char* database)
{
    unsigned int length = 0;

    //CLogFile::getInstance()->Print(LOGINFO,"MD5 for file: %s",hashStr.toStdString().c_str());

    string filenameFromDb = database;

    ifstream ifs(filenameFromDb);

    if (ifs.fail())
    {
        //The file could not be opened
        cout << "Couldn't open UADE songlengths file: " << filenameFromDb << "\n";
        return 0;
    }

    string line;
    while (getline(ifs, line))
    {
        if (!line.substr(0, 32).compare(md5))
        {
            //we found it
            int j = line.find_first_of("=");
            if (j == -1)
            {
                cout << "Formatting error in uade songlengths file!\n";
                return 0;
            }
            line = line.substr(j + 1);
            stringstream ss(line);
            istream_iterator<std::string> begin(ss);
            istream_iterator<std::string> end;
            vector vstrings(begin, end);

            if (subsong >= vstrings.size())
            {
                cout << "Subsong length not found\n";
                flush(cout);
                return 0;
            }

            line = vstrings.at(subsong);
            //cout << "length (" << subsong << "): " << line << "\n";
            //flush(cout);
            int i = line.find_first_of(":");
            if (i == -1)
            {
                cout << "Formatting error in uade songlengths file!\n";
                return 0;
            }

            int msk = line.find_first_of(".");

            int ms = 0;
            if (msk != -1) //There are milliseconds
            {
                ms = atoi(line.substr(msk + 1).c_str());
                string str_ms = line.substr(msk + 1);
                if (str_ms.size() == 2)
                {
                    ms *= 10;
                }
                else if (str_ms.size() == 1)
                {
                    ms *= 100;
                }
            }

            int sec = atoi(line.substr(i + 1, i + 3).c_str()) * 1000;

            int min = atoi(line.substr(0, i).c_str()) * 1000 * 60;
            length = min + sec + ms;
            break;
        }
    }

    if (length == 0)
    {
        cout << "Couldn't find song length for: " << filename << ". Hash <" << md5 << ">\n";
    }
    return length;
}
