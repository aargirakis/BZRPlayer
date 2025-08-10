#include "AmigaPlayer.h"
#include "AmigaFilter.h"
#include "BaseSample.h"
#include "FileLoader.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALL fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_flod_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN |
    FMOD_TIMEUNIT_MODPATTERN_INFO, // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    &fcgetlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    &fcgetposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginFlod
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginFlod(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
        player = 0;
        //decoder=NULL;
    }

    ~pluginFlod()
    {
        //delete some stuf
        delete[] myBuffer;
        myBuffer = 0;
        if (player) delete player;
        player = 0;
    }

    AmigaPlayer* player;
    signed short* myBuffer;
    Info* info;

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
FMOD_RESULT F_CALL fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    auto plugin = new pluginFlod(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    unsigned int bytesread;
    unsigned int filesize;

    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize < 30)
    {
        return FMOD_ERR_FORMAT;
    }
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }

    char* smallBuffer;
    smallBuffer = new char[30];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 30, &bytesread);

    bool isSoundmon1 = false;
    cout << "flod: " << smallBuffer[26] << smallBuffer[27] << smallBuffer[28] << smallBuffer[29] << "\n";
    isSoundmon1 = (smallBuffer[26] == 'B' && smallBuffer[27] == 'P' &&
        smallBuffer[28] == 'S' && smallBuffer[29] == 'M');

    delete[] smallBuffer;
    if (!isSoundmon1)
    {
        return FMOD_ERR_FORMAT;
    }
    plugin->myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);
    //read config from disk

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/flod.cfg";
    ifstream ifs(filename.c_str());
    string line;
    int force = 0;
    int forcePlayer = 0;
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    bool clockspeedNTSC = false;
    int filter = AmigaFilter::FORCE_OFF;
    bool model1200 = true;

    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of("=");

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("model") == 0)
                {
                    if (value.compare("500") == 0)
                    {
                        model1200 = false;
                    }
                    else if (value.compare("1200") == 0)
                    {
                        model1200 = true;
                    }
                }
                else if (word.compare("filter") == 0)
                {
                    if (value.compare("on") == 0)
                    {
                        filter = AmigaFilter::FORCE_ON;
                    }
                    else if (value.compare("off") == 0)
                    {
                        filter = AmigaFilter::FORCE_OFF;
                    }
                    else if (value.compare("auto") == 0)
                    {
                        filter = AmigaFilter::AUTOMATIC;
                    }
                }
                else if (word.compare("clockspeed") == 0)
                {
                    if (value.compare("ntsc") == 0)
                    {
                        clockspeedNTSC = true;
                    }
                    else
                    {
                        clockspeedNTSC = false;
                    }
                }
                else if (word.compare("force") == 0)
                {
                    force = atoi(value.c_str());
                }
                else if (word.compare("player") == 0)
                {
                    forcePlayer = atoi(value.c_str());
                }
            }
        }
        ifs.close();
    }

    auto fileLoader = new FileLoader();

    fileLoader->setForcePlayer(forcePlayer);

    plugin->player = fileLoader->load((signed short*)plugin->myBuffer, filesize, plugin->info->filename.c_str());


    if (!plugin->player)
    {
        //delete some stuf
        delete[] plugin->myBuffer;
        if (plugin->player) delete plugin->player;
        delete fileLoader;
        return FMOD_ERR_FORMAT;
    }


    delete fileLoader;


    plugin->info = static_cast<Info*>(userexinfo->userdata);
    //plugin->player->play();
    plugin->player->setVersion(force);
    plugin->player->setNTSC(clockspeedNTSC);
    plugin->player->amiga->setModel(model1200);
    plugin->player->amiga->setFilter(filter);
    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = 0xffffffff;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->fileformat = plugin->player->format;
    plugin->info->plugin = PLUGIN_flod;
    plugin->info->pluginName = PLUGIN_flod_NAME;
    plugin->info->setSeekable(false);


    plugin->info->hasTitle = true;
    plugin->info->numChannels = plugin->player->getChannels();
    plugin->player->getTitle(plugin->info->title);
    std::vector<BaseSample*> samples = plugin->player->getSamples();
    if (samples.size() > 0)
    {
        plugin->info->numSamples = samples.size();
        plugin->info->samples = new string[plugin->info->numSamples];
        plugin->info->samplesSize = new unsigned int[plugin->info->numSamples];
        //            plugin->info->samplesLoopStart = new unsigned int[plugin->info->numSamples];
        //            plugin->info->samplesLoopLength = new unsigned int[plugin->info->numSamples];
        plugin->info->samplesVolume = new unsigned short[plugin->info->numSamples];

        //            int loopStart = 0;
        for (int j = 0; j < plugin->info->numSamples; j++)
        {
            if (samples[j])
            {
                plugin->info->samples[j] = samples[j]->name;
                plugin->info->samplesSize[j] = samples[j]->length;
                ////                    plugin->info->samplesLoopLength[j] = samples[j]->length-samples[j]->repeat;
                ////                    plugin->info->samplesLoopStart[j] = samples[j]->repeat;
                plugin->info->samplesVolume[j] = samples[j]->volume;
            }
        }
    }
    else
    {
        plugin->info->numSamples = 0;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL fcclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginFlod*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALL fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto plugin = static_cast<pluginFlod*>(codec->plugindata);
    if (plugin->player->amiga->isCompleted())
    {
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }
    plugin->player->mixer(buffer, size >> 2);
    *read = size >> 2;

    return FMOD_OK;
}

FMOD_RESULT F_CALL fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginFlod*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS)
    {
        plugin->player->play();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->player->selectSong(position);
        plugin->player->play();
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALL fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto plugin = static_cast<pluginFlod*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS || lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = plugin->waveformat.lengthpcm;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->player->getSubsongsCount();
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALL fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginFlod*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = plugin->player->getCurrentRow();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *position = plugin->player->getCurrentPattern();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0
        plugin->player->getModRows(plugin->info->modRows);

        //const std::vector<AmigaRow*>& b = plugin->player->getModRows();
        *position = 0;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
