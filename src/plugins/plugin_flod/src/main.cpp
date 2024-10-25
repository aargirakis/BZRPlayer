#include "AmigaPlayer.h"
#include "AmigaFilter.h"
#include "BaseSample.h"
#include "FileLoader.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD FLOD plugin", // Name.
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
    0 // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    fcplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
        player = 0;
        //decoder=NULL;
    }

    ~fcplugin()
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

    FMOD_CODEC_WAVEFORMAT fcwaveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &fccodec;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    fcplugin* fc = new fcplugin(codec);
    fc->info = (Info*)userexinfo->userdata;

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
    fc->myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, fc->myBuffer, filesize, &bytesread);
    //read config from disk

    string filename = fc->info->applicationPath + "/user/plugin/config/flod.cfg";
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

    FileLoader* fileLoader = new FileLoader();

    fileLoader->setForcePlayer(forcePlayer);

    fc->player = fileLoader->load((signed short*)fc->myBuffer, filesize, fc->info->filename.c_str());


    if (!fc->player)
    {
        //delete some stuf
        delete[] fc->myBuffer;
        if (fc->player) delete fc->player;
        delete fileLoader;
        return FMOD_ERR_FORMAT;
    }


    delete fileLoader;


    fc->info = (Info*)userexinfo->userdata;
    //fc->player->play();
    fc->player->setVersion(force);
    fc->player->setNTSC(clockspeedNTSC);
    fc->player->amiga->setModel(model1200);
    fc->player->amiga->setFilter(filter);
    fc->fcwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels = 2;
    fc->fcwaveformat.frequency = 44100;
    fc->fcwaveformat.pcmblocksize = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm = 0xffffffff;

    codec->waveformat = &(fc->fcwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = fc; /* user data value */

    fc->info->fileformat = fc->player->format;
    fc->info->plugin = PLUGIN_flod;
    fc->info->pluginName = PLUGIN_flod_NAME;
    fc->info->setSeekable(false);


    fc->info->hasTitle = true;
    fc->info->numChannels = fc->player->getChannels();
    fc->player->getTitle(fc->info->title);
    std::vector<BaseSample*> samples = fc->player->getSamples();
    if (samples.size() > 0)
    {
        fc->info->numSamples = samples.size();
        fc->info->samples = new string[fc->info->numSamples];
        fc->info->samplesSize = new unsigned int[fc->info->numSamples];
        //            fc->info->samplesLoopStart = new unsigned int[fc->info->numSamples];
        //            fc->info->samplesLoopLength = new unsigned int[fc->info->numSamples];
        fc->info->samplesVolume = new unsigned short[fc->info->numSamples];

        //            int loopStart = 0;
        for (int j = 0; j < fc->info->numSamples; j++)
        {
            if (samples[j])
            {
                fc->info->samples[j] = samples[j]->name;
                fc->info->samplesSize[j] = samples[j]->length;
                ////                    fc->info->samplesLoopLength[j] = samples[j]->length-samples[j]->repeat;
                ////                    fc->info->samplesLoopStart[j] = samples[j]->repeat;
                fc->info->samplesVolume[j] = samples[j]->volume;
            }
        }
    }
    else
    {
        fc->info->numSamples = 0;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    if (fc->player->amiga->isCompleted())
    {
        return FMOD_ERR_FILE_COULDNOTSEEK;
    }
    fc->player->mixer(buffer, size >> 2);
    *read = size >> 2;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MS)
    {
        fc->player->play();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        fc->player->selectSong(position);
        fc->player->play();
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_MS || lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = fc->fcwaveformat.lengthpcm;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = fc->player->getSubsongsCount();
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = fc->player->getCurrentRow();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *position = fc->player->getCurrentPattern();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0
        fc->player->getModRows(fc->info->modRows);

        //const std::vector<AmigaRow*>& b = fc->player->getModRows();
        *position = 0;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
