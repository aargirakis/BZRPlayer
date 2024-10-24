#include "fmod_errors.h"
#include "plugins.h"
#include "sidplayfp.h"
#include "SidTune.h"
#include "SidTuneInfo.h"
#include "SidInfo.h"
#include "residfp.h"
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include "info.h"
#include "sidid.h"
#include "sidemu.h"
#include "MUS.h"

using namespace std;


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        //exit(-1);
    }
}

FMOD_RESULT F_CALLBACK sidopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK sidclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK sidread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK sidsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK sidgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK sidgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);
unsigned int getLengthFromSIDDatabase(string databasefile, bool newDatabaseVersion, string sidfilename, int subsong);
string md5_new;
string md5_old;

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libsidplayfp_NAME, // Name.
    0x00012100, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS,
    // The time format we would like to accept into setposition/getposition.
    &sidopen, // Open callback.
    &sidclose, // Close callback.
    &sidread, // Read callback.
    &sidgetlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &sidsetposition, // Setposition callback.
    &sidgetposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLibsidplayfp
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibsidplayfp(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        tune = NULL;

        memset(&waveformat, 0, sizeof(waveformat));
    }

    char* loadRom(const char* path, size_t romSize)
    {
        char* buffer = 0;
        std::ifstream is(path, std::ios::binary);
        if (is.good())
        {
            buffer = new char[romSize];
            is.read(buffer, romSize);
        }
        else
        {
            //cout << "failed rom loading\n";
            //flush(cout);
        }
        is.close();
        return buffer;
    }

    ~pluginLibsidplayfp()
    {
        if (tune) delete tune;
        if (kernal) delete [] kernal;
        if (basic) delete [] basic;
        if (chargen) delete [] chargen;
    }

    Info* info;
    SidTune* tune;
    int subsongs;
    ReSIDfpBuilder* rs;
    sidplayfp* player;
    char* kernal;
    char* basic;
    char* chargen;
    string hvscSonglengthsFile;
    unsigned int seekPosition;
    bool mute[9];

    bool hvscSonglengthsDataBaseEnabled;
    bool isContinuousPlaybackActive;


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

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

FMOD_RESULT F_CALLBACK sidopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    auto* plugin = new pluginLibsidplayfp(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    plugin->tune = 0;
    plugin->kernal = 0;
    plugin->basic = 0;
    plugin->chargen = 0;
	for(int i=0;i<9;i++)
	{
		plugin->mute[i]=false;
	}

    plugin->seekPosition = 0;


    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize > 1024 * 96) //96kb, biggest sid in hvsc is about 63 kb
    {
        return FMOD_ERR_FORMAT;
    }
    unsigned int bytesread;


    unsigned char* myBuffer;
    myBuffer = new unsigned char[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    ERRCHECK(result);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    ERRCHECK(result);

    uint_least32_t voice3Index;
    if (!((myBuffer[0] == 'P' && myBuffer[1] == 'S' && myBuffer[2] == 'I' && myBuffer[3] == 'D') || (myBuffer[0] == 'R'
        && myBuffer[1] == 'S' && myBuffer[2] == 'I' && myBuffer[3] == 'D')) && !libsidplayfp::detect(
        &myBuffer[0], filesize, voice3Index))
    {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    Info* info = static_cast<Info*>(userexinfo->userdata);

    plugin->player = new sidplayfp();

    string kernal_filename = info->applicationPath + KERNAL_BIN_DATA_PATH;
    string basic_filename = info->applicationPath + BASIC_BIN_DATA_PATH;
    string characters_filename = info->applicationPath + CHARACTERS_BIN_DATA_PATH;

    plugin->kernal = plugin->loadRom(kernal_filename.c_str(), 8192);
    plugin->basic = plugin->loadRom(basic_filename.c_str(), 8192);
    plugin->chargen = plugin->loadRom(characters_filename.c_str(), 4096);
    plugin->player->setRoms((const uint8_t*)plugin->kernal, (const uint8_t*)plugin->basic, (const uint8_t*)plugin->chargen);

    plugin->rs = new ReSIDfpBuilder("Demo");
    // Get the number of SIDs supported by the engine
    unsigned int maxsids = (plugin->player->info()).maxsids();
    plugin->rs->create(maxsids);

    // Check if builder is ok
    if (!plugin->rs->getStatus())
    {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    //read sidid
    string sidid_filename = info->applicationPath + SIDID_CFG_DATA_PATH;
    if (!readconfig(sidid_filename.c_str()))
    {
        std::string myString(identify(myBuffer, filesize), MAX_PATHNAME);
        info->songPlayer = myString;
    }

    //read config from disk
    string filename = info->applicationPath + USER_PLUGINS_CONFIG_DIR + "/libsidplayfp.cfg";
    ifstream ifs(filename.c_str());
    string line;

    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
        cout << "failed to open libsidplayfp.cfg";
    }

    int freq = 44100;
    bool filter = true;
    SidConfig::playback_t playback = SidConfig::STEREO;
    plugin->waveformat.channels = 2;

    SidConfig::sid_model_t sidModel = SidConfig::MOS6581;
    SidConfig::c64_model_t c64Model = SidConfig::PAL;
    SidConfig::sampling_method_t samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
    bool forceSidModel = false;
    bool forcec64Model = false;

    plugin->hvscSonglengthsDataBaseEnabled = true;
    plugin->isContinuousPlaybackActive = false;

    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("frequency") == 0)
                {
                    freq = atoi(value.c_str());
                }
                else if (word.compare("playback") == 0)
                {
                    if (value.compare("left") == 0) //old, just for compability
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    }
                    else if (value.compare("mono") == 0)
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    }
                    else if (value.compare("stereo") == 0)
                    {
                        playback = SidConfig::STEREO;
                        plugin->waveformat.channels = 2;
                    }
                    else if (value.compare("right") == 0) //old, just for compability
                    {
                        playback = SidConfig::MONO;
                        plugin->waveformat.channels = 1;
                    }
                }
                else if (word.compare("sampling_method") == 0)
                {
                    if (value.compare("interpolate") == 0)
                    {
                        samplingMethod = SidConfig::INTERPOLATE;
                    }
                    else if (value.compare("resample/interpolate") == 0)
                    {
                        samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
                    }
                }

                else if (word.compare("clock_speed") == 0)
                {
                    if (value.compare("correct") == 0)
                    {
                        forcec64Model = false;
                    }
                    else if (value.compare("pal") == 0)
                    {
                        c64Model = SidConfig::PAL;
                        forcec64Model = true;
                    }
                    else if (value.compare("ntsc") == 0)
                    {
                        c64Model = SidConfig::NTSC;
                        forcec64Model = true;
                    }
                    else if (value.compare("old_ntsc") == 0)
                    {
                        c64Model = SidConfig::OLD_NTSC;
                        forcec64Model = true;
                    }
                    else if (value.compare("drean") == 0)
                    {
                        c64Model = SidConfig::DREAN;
                        forcec64Model = true;
                    }
                }
                else if (word.compare("sid_model") == 0)
                {
                    if (value.compare("correct") == 0)
                    {
                        forceSidModel = false;
                    }
                    else if (value.compare("mos6581") == 0)
                    {
                        sidModel = SidConfig::MOS6581;
                        forceSidModel = true;
                    }
                    else if (value.compare("mos8580") == 0)
                    {
                        sidModel = SidConfig::MOS8580;
                        forceSidModel = true;
                    }
                }
                else if (word.compare("sid_filter") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        filter = true;
                    }
                    else
                    {
                        filter = false;
                    }
                }
                else if (word.compare("hvsc_songlengths_path") == 0)
                {
                    plugin->hvscSonglengthsFile = value;
                }
                else if (word.compare("hvsc_songlengths_enabled") == 0)
                {
                    if (value.compare("true") == 0)
                    {
                        plugin->hvscSonglengthsDataBaseEnabled = true;
                    }
                    else
                    {
                        plugin->hvscSonglengthsDataBaseEnabled = false;
                    }
                }
                else if (word.compare("continuous_playback") == 0)
                {
                    plugin->isContinuousPlaybackActive = info->isPlayModeRepeatSongEnabled && value.compare("true") == 0;
                }
            }
        }
        ifs.close();
    }

    if (plugin->hvscSonglengthsFile.empty())
    {
        plugin->hvscSonglengthsFile = plugin->info->applicationPath + HVSC_SONGLENGTHS_DATA_PATH;
    }
    else if (plugin->hvscSonglengthsFile == HVSC_SONGLENGTHS_DATA_PATH || plugin->hvscSonglengthsFile == HVSC_SONGLENGTHS_USER_PATH)
    {
        plugin->hvscSonglengthsFile = plugin->info->applicationPath + plugin->hvscSonglengthsFile;
    }


    plugin->tune = new SidTune(myBuffer, filesize);
    // Check if the tune is valid
    if (!plugin->tune->getStatus())
    {
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
    cfg.defaultSidModel = sidModel;
    cfg.defaultC64Model = c64Model;
    cfg.samplingMethod = samplingMethod;
    cfg.fastSampling = false;
    cfg.sidEmulation = plugin->rs;

    if (!plugin->player->config(cfg))
    {
        delete[] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    const SidTuneInfo* s = plugin->tune->getInfo();

    info->initAddr = s->initAddr();
    info->loadAddr = s->loadAddr();
    info->playAddr = s->playAddr();
    info->songSpeed = s->songSpeed();


    plugin->subsongs = s->songs();

    plugin->tune->selectSong(1);
    plugin->player->load(plugin->tune);
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.frequency = cfg.frequency;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = 0xffffffff; //inifite length

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    codec->plugindata = plugin; //user data value


    info->numChannels = 3*s->sidChips();
    if (s->numberOfInfoStrings() == 3)
    {
        info->title = s->infoString(0);
        info->artist = s->infoString(1);
        info->copyright = s->infoString(2);
    }
    string comments;
    for (int i = 0; i < s->numberOfCommentStrings(); i++)
    {
        comments = comments + '\n' + string(s->commentString(i));
    }
    info->comments = comments;
    info->numSamples = 0;
    switch (s->clockSpeed())
    {
    case SidTuneInfo::CLOCK_UNKNOWN:
        info->clockSpeed = 0;
        break;
    case SidTuneInfo::CLOCK_PAL:
        info->clockSpeed = 1;
        break;
    case SidTuneInfo::CLOCK_NTSC:
        info->clockSpeed = 2;
        break;
    case SidTuneInfo::CLOCK_ANY:
        info->clockSpeed = 3;
        break;
    }
    switch (s->sidModel(0))
    {
    case SidTuneInfo::SIDMODEL_UNKNOWN:
        info->sidModel = 0;
        break;
    case SidTuneInfo::SIDMODEL_6581:
        info->sidModel = 1;
        break;
    case SidTuneInfo::SIDMODEL_8580:
        info->sidModel = 2;
        break;
    case SidTuneInfo::SIDMODEL_ANY:
        info->sidModel = 3;
        break;
    }

    switch (s->compatibility())
    {
    case SidTuneInfo::COMPATIBILITY_C64:
        info->compatibility = 0;
        break;
    case SidTuneInfo::COMPATIBILITY_PSID:
        info->compatibility = 1;
        break;
    case SidTuneInfo::COMPATIBILITY_R64:
        info->compatibility = 2;
        break;
    case SidTuneInfo::COMPATIBILITY_BASIC:
        info->compatibility = 3;
        break;
    }


    if (strcmp(s->formatString(), "C64 Sidplayer format (MUS)") != 0 && strcmp(
        s->formatString(), "C64 Stereo Sidplayer format (MUS+STR)") != 0)
    {
        md5_new = plugin->tune->createMD5New();
        md5_old = plugin->tune->createMD5();
    }
    else
    {
        md5_new = "";
        md5_old = "";
    }


    info->startSubSong = s->startSong();
    info->numSubsongs = plugin->subsongs;
    info->fileformatSpecific = s->formatString();
    info->md5New = md5_new;
    info->md5Old = md5_old;


    info->setSeekable(true);
    info->plugin = PLUGIN_libsidplayfp;
    info->pluginName = PLUGIN_libsidplayfp_NAME;
    info->fileformat = "C64 SID";
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK sidread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibsidplayfp*>(codec->plugindata);
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
    plugin->player->play((short int*)buffer, size << 1);
    if (plugin->player->timeMs() >= plugin->seekPosition)
    {
        plugin->player->fastForward(100);
		for(int i=0;i<9;i++)
		{
			plugin->player->mute(0, i, plugin->mute[i]);
		}

    }
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK sidsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibsidplayfp*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        if (position > plugin->player->timeMs())
        {
			for(int i=0;i<9;i++)
			{
				plugin->player->mute(0, i, true);
			}
            plugin->seekPosition = position;
            plugin->player->fastForward(3000);
        }
        else if (position == 0)
        {
            plugin->player->load(plugin->tune);
        }
        else if (position <= plugin->player->timeMs())
        {
			for(int i=0;i<9;i++)
			{
				plugin->player->mute(0, i, true);
			}
            plugin->seekPosition = position;
            plugin->player->load(plugin->tune);
            plugin->player->fastForward(3000);
        }

        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->tune->selectSong(position + 1);
        plugin->player->load(plugin->tune);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
		for(int i=0;i<9;i++)
		{
			plugin->mute[i]=false;
		}
        //position is a mask
        for (int i = 0; i < plugin->info->numChannels; i++)
        {
            plugin->player->mute(i / 3, i % 3, (position >> i & 1));
            plugin->mute[i] = position >> i & 1;
        }
        return FMOD_OK;
    }
    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALLBACK sidclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginLibsidplayfp*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK sidgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginLibsidplayfp*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        *length = plugin->waveformat.lengthpcm;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MS)
    {
        string databasefile = plugin->hvscSonglengthsFile;
        const SidTuneInfo* s = plugin->tune->getInfo();
        unsigned int sidLength = 0;
        if (plugin->hvscSonglengthsDataBaseEnabled && !plugin->isContinuousPlaybackActive)
        {
            sidLength = getLengthFromSIDDatabase(databasefile, true, plugin->info->filename, s->currentSong());
        }
        *length = sidLength;

        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->subsongs;
        return FMOD_OK;
    }
}

FMOD_RESULT F_CALLBACK sidgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibsidplayfp*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *position = plugin->player->timeMs();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        const SidTuneInfo* s = plugin->tune->getInfo();
        *position = s->currentSong();
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}


unsigned int getLengthFromSIDDatabase(string databasefile, bool newDatabaseVersion, string sidfilename, int subsong)
{
    subsong--;
    unsigned int length = 0;

    string hashStr;
    if (newDatabaseVersion)
    {
        hashStr = md5_new;
    }
    else
    {
        hashStr = md5_old;
    }


    ifstream ifs(databasefile);

    if (ifs.fail())
    {
        //The file could not be opened
        cout << "Couldn't open sid lenghts file:" << databasefile.c_str() << "\n";
        flush(cout);
        return 0;
    }

    cout << "hash:" << hashStr.c_str() << "\n";
    cout << "filename:" << sidfilename.c_str() << "\n";
    flush(cout);

    string line;
    while (getline(ifs, line))
    {
        if (!line.substr(0, 32).compare(hashStr))
        {
            //we found it
            int j = line.find_first_of("=");
            if (j == -1)
            {
                cout << "Error in HVSC Songlengths file!\n";
                flush(cout);
                return 0;
            }
            line = line.substr(j + 1);


            istringstream iss(line);
            string token;

            int i = 0;
            bool found = false;

            while (getline(iss, token, ' ') && !found)
            {
                if (i == subsong)
                {
                    found = true;
                    break;
                }

                i++;
            }

            line = token;

            int k = line.find_first_of(":");

            if (k == -1)
            {
                std::cout << "Error in HVSC Songlengths file!\n";
                flush(cout);
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


            int sec = atoi(line.substr(k + 1, k + 3).c_str()) * 1000;

            int min = atoi(line.substr(0, k).c_str()) * 1000 * 60;

            length = min + sec + ms;

            break;
        }
    }

    if (length == 0)
    {
        //DebugWindow::instance()->addText("Couldn't find song length for SID: " + sidfilename.toLatin1() + ". Hash <" + hashStr.toLatin1() + ">");
    }

    return length;
}
