#include "BaseSample.h"
#include "FileLoader.h"
#include <cstring>
#include <cstdio>
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
#include "uade.h"
#include "uae.h"
#include "songinfo.h"
#include "xs_length.h"
#ifdef __cplusplus
}
#endif

unsigned int getLengthFromDatabase(const char*, int, const char*);
int test_ac1d(unsigned char* data);
int test_pru1(unsigned char* data);
int test_fcm(unsigned char* data);
int test_skyt(unsigned char* data);
int test_p40a(unsigned char* data);
int test_p40b(unsigned char* data);
int test_p41a(unsigned char* data);
string getMD5(const char* sidfilename);
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_webuade_NAME, // Name.
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

class pluginWothkeUade
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginWothkeUade(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginWothkeUade()
    {
        //delete some stuff
    }

    FMOD_CODEC_WAVEFORMAT waveformat;

    Info* info;
    int led_forced;
    int led_state;
    int no_filter;
    int silence_timeout;
    bool silence_timeout_enabled;
    bool uade_songlengths_enabled;
    int currentSubsong;
    string uade_songlengthspath;
    string basefilename;
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
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }


    FMOD_RESULT result;
    auto* plugin = new pluginWothkeUade(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 2048 * 2;
    plugin->waveformat.lengthpcm = 0xffffffff;


    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    unsigned int bytesread;
    char* smallBuffer;
    smallBuffer = new char[4];
    unsigned char* myBuffer;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 4, &bytesread);
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
    if (filesize >= 62)
    {
        unsigned char* testBuffer;
        testBuffer = new unsigned char[62];
        result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        result = FMOD_CODEC_FILE_READ(codec, testBuffer, 62, &bytesread);

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
            filesize < (unsigned)(70 + dataSize))
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


    myBuffer = new unsigned char[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    char uade_basedir[1024];
    snprintf(uade_basedir, 1024, "%s/%s", plugin->info->dataPath.c_str(),UADE_DATA_DIR);

    int err = uade_boot(uade_basedir);
    if (err)
    {
        return FMOD_ERR_FORMAT;
    }

    err = uade_reset(plugin->waveformat.frequency, uade_basedir, const_cast<char*>(plugin->info->filename.c_str()), 1);
    if (err)
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned char* packBuffer;
    packBuffer = new unsigned char[2048];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, packBuffer, 2048, &bytesread);


    int acid = test_ac1d(packBuffer);
    int pru1 = test_pru1(packBuffer);
    int fcm = test_fcm(packBuffer);
    int skyt = test_skyt(packBuffer);
    int p40a = test_p40a(packBuffer);
    int p40b = test_p40b(packBuffer);
    int p41a = test_p41a(packBuffer);


    //read config from disk
    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/uade.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;

    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    plugin->led_forced = 0;
    plugin->led_state = 0;
    plugin->no_filter = 1;
    plugin->silence_timeout = 5;
    plugin->silence_timeout_enabled = true;
    plugin->uade_songlengths_enabled = true;
    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("led_forced") == 0)
                {
                    if (value.compare("auto") == 0)
                    {
                        plugin->led_forced = 0;
                    }
                    else if (value.compare("on") == 0)
                    {
                        plugin->led_forced = 1;
                        plugin->led_state = 1;
                    }
                    else
                    {
                        plugin->led_forced = 1;
                        plugin->led_state = 0;
                    }
                }
                else if (word.compare("no_filter") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        plugin->no_filter = 1;
                    }
                    else
                    {
                        plugin->no_filter = 0;
                    }
                }
                else if (word.compare("silence_timeout") == 0)
                {
                    plugin->silence_timeout = atoi(value.c_str());
                }
                else if (word.compare("silence_timeout_enabled") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        plugin->silence_timeout_enabled = true;
                    }
                    else
                    {
                        plugin->silence_timeout_enabled = false;
                    }
                }
                else if (word.compare("uade_songlengths_path") == 0)
                {
                    plugin->uade_songlengthspath = value;
                }
                else if (word.compare("uade_songlengths_enabled") == 0)
                {
                    if (value.compare("true") == 0)
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

    if (plugin->uade_songlengthspath.empty() || plugin->uade_songlengthspath == "/uade.md5")
    {
        plugin->uade_songlengthspath = plugin->info->dataPath + UADE_DATA_DIR + "/uade.md5";
    }

    uade_state* state;
    state = get_uade_state();

    //    if (strcmp(state->ep->playername, "YM-2149") == 0)
    //    {
    //        return FMOD_ERR_FORMAT;
    //    }

    //    if (strcmp(state->ep->playername, "YM") == 0)
    //    {
    //        return FMOD_ERR_FORMAT;
    //    }

    state->config.led_forced = plugin->led_forced; //Force led
    state->config.led_state = plugin->led_state; //Forced led state
    state->config.no_filter = plugin->no_filter; //Turn filter emulation off.
    if (!plugin->silence_timeout_enabled)
    {
        state->config.silence_timeout = -1;
    }
    else
    {
        state->config.silence_timeout = plugin->silence_timeout;
    }

    state->config.use_timeouts = 1;
    state->config.timeout = -1;


    plugin->currentSubsong = -1;

    set_uade_state(state);


    plugin->info->numSamples = 0;
    plugin->info->plugin = PLUGIN_webuade;
    plugin->info->pluginName = PLUGIN_webuade_NAME;
    if (!acid)
    {
        plugin->info->fileformat = "AC1D Packer";
    }
    else if (!pru1)
    {
        plugin->info->fileformat = "Prorunner 1.0";
    }
    else if (!fcm)
    {
        plugin->info->fileformat = "FC-M Packer";
    }
    else if (!skyt)
    {
        plugin->info->fileformat = "SKYT Packer";
    }
    else if (!p40a)
    {
        plugin->info->fileformat = "The Player 4.0a";
    }
    else if (!p40b)
    {
        plugin->info->fileformat = "The Player 4.0b";
    }
    else if (!p41a)
    {
        plugin->info->fileformat = "The Player 4.1a";
    }
    else
    {
        plugin->info->fileformat = state->ep->playername;
    }


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
        unsigned int bytesread;
        char* d = new char[filesize];
        result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        result = FMOD_CODEC_FILE_READ(codec, d, filesize, &bytesread);

        auto* fileLoader = new FileLoader();

        AmigaPlayer* player = fileLoader->load((signed short*)d, filesize, plugin->info->filename.c_str());

        delete fileLoader;
        delete[] d;

        if (player)
        {
            std::vector<BaseSample*> samples;
            samples = player->getSamples();
            if (samples.size() > 0)
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

    plugin->info->md5New = getMD5(plugin->info->filename.c_str());
    plugin->info->setSeekable(false);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    uae_quit();
    auto* plugin = static_cast<pluginWothkeUade*>(codec->plugindata);
    delete plugin;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginWothkeUade*>(codec->plugindata);

    // trying to correctly handle tracks with/without length or with/without its sample files,
    // trying to skip unsupported/invalid track instead of a silent playback

    while (!get_missing_file() && !get_samples(buffer))
    {
        m68k_run_1();
    }

    if (get_quit() && get_missing_file())
    {
        return FMOD_ERR_FILE_NOTFOUND;
    }

    if (get_silence_detected())
    {
        return FMOD_ERR_FILE_EOF;
    }

    *read = plugin->waveformat.pcmblocksize;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginWothkeUade*>(codec->plugindata);
    if (plugin->currentSubsong == -1)
    {
        plugin->currentSubsong = get_min_subsongs();
    }
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = 1 + (get_max_subsongs() - get_min_subsongs());
        return FMOD_OK;
    }

    int sub = plugin->currentSubsong;
    if (get_min_subsongs() == 1)
    {
        sub = plugin->currentSubsong - 1;
    }

    int songLength;
    if (plugin->uade_songlengths_enabled)
    {
        songLength = getLengthFromDatabase(plugin->info->filename.c_str(), sub, plugin->uade_songlengthspath.c_str());
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
    auto* plugin = static_cast<pluginWothkeUade*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        char uade_basedir[1024];
        snprintf(uade_basedir, 1024, "%s/%s", plugin->info->dataPath.c_str(),UADE_DATA_DIR);

        int err = uade_reset(plugin->waveformat.frequency, uade_basedir, const_cast<char*>(plugin->info->filename.c_str()), 0);

        uade_state* state;
        state = get_uade_state();

        state->config.led_forced = plugin->led_forced; //Force led
        state->config.led_state = plugin->led_state; //Forced led state
        state->config.no_filter = plugin->no_filter; //Turn filter emulation off.
        if (!plugin->silence_timeout_enabled)
        {
            state->config.silence_timeout = -1;
        }
        else
        {
            state->config.silence_timeout = plugin->silence_timeout;
        }
        state->config.use_timeouts = 1;
        state->config.timeout = -1;
        set_uade_state(state);
        set_subsong(plugin->currentSubsong);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (get_max_subsongs() - get_min_subsongs() == 0)
        {
            return FMOD_OK;
        }
        if (position > (get_max_subsongs() - get_min_subsongs()))
        {
            position = get_min_subsongs();
        }
        else
        {
            position = get_min_subsongs() + position;
        }

        plugin->currentSubsong = position;
        change_subsong(position);
        return FMOD_OK;
    }
    return FMOD_OK;
}

string getMD5(const char* sidfilename)
{
    xs_md5hash_t hash;
    int error = xs_get_hash(sidfilename, hash);

    if (error) //there was an error calculating the hash
    {
        cout << "getMD5 Error calculating hash: " << error << "\n";
        return 0;
    }


    string hashStr;
    for (int i = 0; i < 16; i++)
    {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        hashStr += hex;
    }
    return hashStr;
}

unsigned int getLengthFromDatabase(const char* sidfilename, int subsong, const char* database)
{
    unsigned int length = 0;
    xs_md5hash_t hash;
    int error = xs_get_hash(sidfilename, hash);


    if (error) //there was an error calculating the hash
    {
        cout << "getLengthFromDatabase Error calculating hash: " << error << "\n";
        return 0;
    }


    string hashStr;
    for (int i = 0; i < 16; i++)
    {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        hashStr += hex;
    }

    //CLogFile::getInstance()->Print(LOGINFO,"MD5 for sid file: %s",hashStr.toStdString().c_str());

    string filename = database;

    ifstream ifs(filename);

    if (ifs.fail())
    {
        //The file could not be opened
        cout << "Couldn't open UADE songlengths file: " << filename << "\n";
        return 0;
    }

    string line;
    while (getline(ifs, line))
    {
        if (!line.substr(0, 32).compare(hashStr))
        {
            //we found it
            int j = line.find_first_of("=");
            if (j == -1)
            {
                cout << "Formatting error in SID songlengths file!\n";
                return 0;
            }
            line = line.substr(j + 1);
            stringstream ss(line);
            istream_iterator<std::string> begin(ss);
            istream_iterator<std::string> end;
            vector<std::string> vstrings(begin, end);

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
                cout << "Formatting error in SID songlengths file!\n";
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
        cout << "Couldn't find song length for: " << sidfilename << ". Hash <" << hashStr << ">\n";
    }
    return length;
}

int test_ac1d(unsigned char* data)
{
    int i;
    /* test #1 */
    if (data[2] != 0xac || data[3] != 0x1d)
        return -1;

    /* test #2 */
    if (data[0] > 0x7f)
        return -1;

    /* test #4 */
    for (i = 0; i < 31; i++)
    {
        if (data[10 + 8 * i] > 0x0f)
            return -1;
    }

    /* test #5 */
    for (i = 0; i < 128; i++)
    {
        if (data[768 + i] > 0x7f)
            return -1;
    }

    return 0;
}

int test_pru1(uint8* data)
{
    if (data[1080] != 'S' || data[1081] != 'N' || data[1082] != 'T' || data[1083] != '.')
        return -1;

    /* test 2 */
    if (data[951] != 0x7f)
        return -1;

    /* test 3 */
    if (data[950] > 0x7f)
        return -1;

    return 0;
}

int test_fcm(uint8* data)
{
    int j;

    /* "FC-M" : ID of FC-M packer */
    if (data[0] != 'F' || data[1] != 'C' || data[2] != '-' || data[3] != 'M')
        return -1;

    /* test 1 */
    if (data[4] != 0x01)
        return -1;

    /* test 2 */
    if (data[5] != 0x00)
        return -1;

    /* test 3 */
    for (j = 0; j < 31; j++)
    {
        if (data[37 + 8 * j] > 0x40)
            return -1;
    }

    return 0;
}

int test_skyt(uint8* data)
{
    int i;

    /* test 2 */
    for (i = 0; i < 31; i++)
    {
        if (data[8 * i + 4] > 0x40)
            return -1;
    }

    if (data[256] != 'S' || data[257] != 'K' || data[258] != 'Y' || data[259] != 'T')
        return -1;

    return 0;
}

int test_p40a(uint8* data)
{
    if (data[0] != 'P' || data[1] != '4' || data[2] != '0' || data[3] != 'A')
        return -1;
    return 0;
}

int test_p40b(uint8* data)
{
    if (data[0] != 'P' || data[1] != '4' || data[2] != '0' || data[3] != 'B')
        return -1;
    return 0;
}

int test_p41a(uint8* data)
{
    if (data[0] != 'P' || data[1] != '4' || data[2] != '1' || data[3] != 'A')
        return -1;
    return 0;
}
