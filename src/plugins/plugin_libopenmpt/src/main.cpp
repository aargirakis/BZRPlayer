#include <iostream>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <vector>
#include <queue>
#include "libopenmpt.hpp"
#include "libopenmpt_ext.hpp"
#include "BaseRow.h"
#include <fmod_errors.h>
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK libopenmptopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK libopenmptclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK libopenmptread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK libopenmptgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK libopenmptsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                             FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK libopenmptgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);


FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libopenmpt_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS |
    FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN | FMOD_TIMEUNIT_MODPATTERN_INFO |
    FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS | FMOD_TIMEUNIT_MODVUMETER | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_SPEED |
    FMOD_TIMEUNIT_BPM, // The time format we would like to accept into setposition/getposition.
    &libopenmptopen, // Open callback.
    &libopenmptclose, // Close callback.
    &libopenmptread, // Read callback.
    &libopenmptgetlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &libopenmptsetposition, // Setposition callback.
    &libopenmptgetposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginLibopenmpt
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibopenmpt(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibopenmpt()
    {
        info->modRows.clear();

        for (vector<vector<BaseRow*>>::iterator itr = info->patterns.begin(); itr != info->patterns.end(); ++itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        info->patterns.clear();
        delete mod;
        mod = nullptr;
        delete[] myBuffer;
    }

    unsigned char* myBuffer;
    queue<unsigned char*> vumeterBuffer;
    queue<int> rowBuffer;
    queue<int> patternBuffer;
    queue<int> orderBuffer;
    double maxVUMeter;
    Info* info;
    openmpt::module_ext* mod;
    FMOD_CODEC_WAVEFORMAT waveformat;
    bool isContinuousPlaybackActive;
};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

FMOD_RESULT F_CALLBACK libopenmptopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;


    unsigned int bytesread;

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto* plugin = new pluginLibopenmpt(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);
    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }
    char* smallBuffer;
    smallBuffer = new char[4];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, smallBuffer, 4, &bytesread);


    if ((smallBuffer[0] == 'M' && smallBuffer[1] == 'T' && smallBuffer[2] == 'h' && smallBuffer[3] == 'd') ||
        smallBuffer[0] == 'R' && smallBuffer[1] == 'I' && smallBuffer[2] == 'F' && smallBuffer[3] == 'F')
    //it's a midi file
    {
        delete[] smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->myBuffer = new unsigned char[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);

    try
    {
        Info* info = static_cast<Info*>(userexinfo->userdata);


        //read config from disk
        string filename = plugin->info->applicationPath + USER_PLUGINS_CONFIG_DIR + "/libopenmpt.cfg";
        ifstream ifs(filename.c_str());
        string line;
        bool useDefaults = false;
        if (ifs.fail())
        {
            //The file could not be opened
            useDefaults = true;
        }

        //defaults
        int stereo_separation = 100;
        int interpolation_filter = 0;
        string emulate_amiga_filter = "1";
        string amiga_filter = "auto";
        string dither = "1";
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
                    if (word.compare("stereo_separation") == 0)
                    {
                        stereo_separation = atoi(value.c_str());
                    }
                    else if (word.compare("continuous_playback") == 0)
                    {
                        plugin->isContinuousPlaybackActive = info->isPlayModeRepeatSongEnabled && value.compare(
                            "true") == 0;
                    }
                    else if (word.compare("interpolation_filter") == 0)
                    {
                        interpolation_filter = atoi(value.c_str());
                    }
                    else if (word.compare("emulate_amiga_filter") == 0)
                    {
                        if (value.compare("true") == 0)
                        {
                            emulate_amiga_filter = "1";
                        }
                        else
                        {
                            emulate_amiga_filter = "0";
                        }
                    }
                    else if (word.compare("amiga_filter") == 0)
                    {
                        if (value.compare("a500") == 0)
                        {
                            amiga_filter = "a500";
                        }
                        else if (value.compare("a1200") == 0)
                        {
                            amiga_filter = "a1200";
                        }
                        else if (value.compare("unfiltered") == 0)
                        {
                            amiga_filter = "unfiltered";
                        }
                        else if (value.compare("auto") == 0)
                        {
                            amiga_filter = "auto";
                        }
                    }
                    else if (word.compare("dither") == 0)
                    {
                        if (value.compare("0") == 0)
                        {
                            dither = "0";
                        }
                        else if (value.compare("1") == 0)
                        {
                            dither = "1";
                        }
                        else if (value.compare("2") == 0)
                        {
                            dither = "2";
                        }
                        else if (value.compare("3") == 0)
                        {
                            dither = "3";
                        }
                    }
                }
            }
            ifs.close();
        }

        std::map<std::string, std::string> ctls
        {
            {"seek.sync_samples", "1"},
            {"play.at_end", plugin->isContinuousPlaybackActive ? "continue" : "stop"},
            {"render.resampler.emulate_amiga", emulate_amiga_filter},
            {"render.resampler.emulate_amiga_type", amiga_filter},
            {"dither", dither}
        };
        plugin->mod = nullptr;
        plugin->mod = new openmpt::module_ext(plugin->myBuffer, filesize, std::clog, ctls);

        plugin->mod->set_repeat_count(0); //it is 0 by default, and ignored with "continue" play mode
        plugin->mod->set_render_param(openmpt::module::RENDER_STEREOSEPARATION_PERCENT, stereo_separation);
        plugin->mod->set_render_param(openmpt::module::RENDER_INTERPOLATIONFILTER_LENGTH, interpolation_filter);

        plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
        plugin->waveformat.channels = 2;
        plugin->waveformat.frequency = 44100;
        plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
        plugin->waveformat.lengthpcm = 0xffffffff;
        //plugin->mod->get_duration_seconds()*plugin->waveformat.frequency;

        codec->waveformat = &(plugin->waveformat);
        codec->numsubsounds = 0;
        /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata = plugin; /* user data value */

        info->numSamples = plugin->mod->get_num_samples();
        info->numInstruments = plugin->mod->get_num_instruments();
        info->title = plugin->mod->get_metadata("title");

        info->comments = plugin->mod->get_metadata("message_raw");


        info->numChannels = plugin->mod->get_num_channels();
        info->numPatterns = plugin->mod->get_num_patterns();
        info->numOrders = plugin->mod->get_num_orders();
        //        info->restart = fc->mi.mod->rst;


        if (info->numSamples > 0)
        {
            std::vector<std::string> samplenames = plugin->mod->get_sample_names();

            info->samples = new string[info->numSamples];
            info->samplesSize = new unsigned int[info->numSamples];
            info->samplesLoopStart = new unsigned int[info->numSamples];
            info->samplesLoopEnd = new unsigned int[info->numSamples];
            info->samplesVolume = new unsigned short[info->numSamples];
            info->samplesFineTune = new signed int[info->numSamples];
            info->samplesData = new unsigned char*[info->numSamples];
            //        info->samples16Bit = new bool[numSamples];
            //        info->samplesStereo = new bool[numSamples];
            for (int j = 0; j < info->numSamples; j++)
            {
                info->samples[j] = samplenames.at(j);
                info->samplesSize[j] = plugin->mod->get_mod_sample_size(j + 1);
                info->samplesLoopStart[j] = plugin->mod->get_mod_sample_loopstart(j + 1);
                info->samplesLoopEnd[j] = plugin->mod->get_mod_sample_loopend(j + 1);
                info->samplesVolume[j] = plugin->mod->get_mod_sample_volume(j + 1) / 4;
                int finetune = plugin->mod->get_mod_sample_finetune(j + 1);
                if (finetune > 127)
                {
                    finetune -= 256;
                    finetune /= 16;
                }
                else
                {
                    finetune /= 16;
                }
                info->samplesFineTune[j] = finetune;

                //info->samplesData[j] = fc->mi.mod->xxs[j].data;
            }
        }

        if (info->numInstruments > 0)
        {
            std::vector<std::string> instrumentnames = plugin->mod->get_instrument_names();

            info->instruments = new string[info->numInstruments];

            for (int j = 0; j < info->numInstruments; j++)
            {
                info->instruments[j] = instrumentnames.at(j);
            }
        }

        info->fileformat = plugin->mod->get_metadata("type_long");

        //get_current_channel_vu_mono seems to return 1.40317 for max volume for most formats, but max for protracker it is 1.11105
        if (info->fileformat.substr(0, 10) == "ProTracker" || info->fileformat.substr(0, 12) == "Soundtracker")
        {
            plugin->maxVUMeter = 1.11105;
        }
        else
        {
            plugin->maxVUMeter = 1.40317;
        }


        info->plugin = PLUGIN_libopenmpt;
        info->pluginName = PLUGIN_libopenmpt_NAME;
        info->setSeekable(true);
        //plugin->vumeterBuffer = CreateQueue(22);
        return FMOD_OK;
    }
    catch (...)
    {
        delete plugin->mod;
        plugin->mod = nullptr;
        delete[] plugin->myBuffer;
        return FMOD_ERR_FORMAT;
    }
}

FMOD_RESULT F_CALLBACK libopenmptclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginLibopenmpt*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK libopenmptread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibopenmpt*>(codec->plugindata);
    plugin->mod->read_interleaved_stereo(plugin->waveformat.frequency, size, (short*)buffer);
    unsigned char* vumeters = new unsigned char[plugin->info->numChannels];


    for (int i = 0; i < plugin->info->numChannels; i++)
    {
        vumeters[i] = (plugin->mod->get_current_channel_vu_mono(i) / plugin->maxVUMeter) * 100;
    }

    if (plugin->vumeterBuffer.size() >= 70)
    {
        plugin->vumeterBuffer.pop();
        plugin->rowBuffer.pop();
        plugin->patternBuffer.pop();
        plugin->orderBuffer.pop();
    }
    plugin->vumeterBuffer.push(vumeters);
    plugin->rowBuffer.push(plugin->mod->get_current_row());
    plugin->patternBuffer.push(plugin->mod->get_current_pattern());
    plugin->orderBuffer.push(plugin->mod->get_current_order());

    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK libopenmptgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginLibopenmpt*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MS || lengthtype ==
        FMOD_TIMEUNIT_MUTE_VOICE)
    {
        if (plugin->isContinuousPlaybackActive)
        {
            *length = 0xffffffff;
        }
        else
        {
            *length = plugin->mod->get_duration_seconds() * 1000;
        }
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->mod->get_num_subsongs();
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALLBACK libopenmptsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                             FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibopenmpt*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //         //position is a mask
        //         for(int i = 0 ; i<plugin->info->numChannels ; i++)
        //         {
        //             #ifdef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
        //               openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>( plugin->mod->get_interface( openmpt::ext::interactive_id ) );

        //               if ( interactive ) {
        //                 interactive->set_channel_mute_status(i,(position >> i & 1));
        //                 cout << "muting channel : " << i << " " << (position >> i & 1) << endl;
        //                 cout << "muting channel mask: " << i << " " << (position) << endl;
        //                 flush(cout);
        //               } else {
        //                 // interface not available
        //               }
        //             #else
        //               // interface not available
        //             #endif

        //         }


        for (int i = 0; i < plugin->info->numChannels; i++)
        {
#ifdef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
            openmpt::ext::interactive* interactive = static_cast<openmpt::ext::interactive*>(plugin->mod->
                get_interface(openmpt::ext::interactive_id));

            if (interactive)
            {
                bool mute = false;
                if (plugin->info->mutedChannelsMask.at(i) == '0')
                {
                    mute = true;
                }
                interactive->set_channel_mute_status(i, mute);
                cout << "muting channel : " << i << " " << mute << endl;
                //cout << "muting channel mask: " << i << " " << (position) << endl;
            }
            else
            {
                // interface not available
            }
#else
            // interface not available
#endif
        }
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MS)
    {
        plugin->mod->set_position_seconds(position / 1000.0);
        return FMOD_OK;
    }

    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->mod->select_subsong(position);
        return FMOD_OK;
    }
    return FMOD_ERR_UNSUPPORTED;;
}

FMOD_RESULT F_CALLBACK libopenmptgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibopenmpt*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        plugin->info->modVUMeters = plugin->vumeterBuffer.front();
        //cout << "get position " <<  ": " << (int)plugin->info->modVUMeters[0] << endl;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = plugin->rowBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *position = plugin->patternBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        *position = plugin->orderBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SPEED)
    {
        *position = plugin->mod->get_current_speed();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_BPM)
    {
        *position = plugin->mod->get_current_estimated_bpm();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0

        plugin->info->modRows.clear();

        for (vector<vector<BaseRow*>>::iterator itr = plugin->info->patterns.begin(); itr != plugin->info->
             patterns.end(); ++itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        plugin->info->patterns.clear();

        for (int i = 0; i < plugin->mod->get_num_patterns(); i++)
        {
            int numberRows = plugin->mod->get_pattern_num_rows(i);
            vector<BaseRow*> modRows(numberRows * plugin->info->numChannels);
            int counter = 0;
            for (int j = 0; j < numberRows; j++)
            {
                for (int k = 0; k < plugin->info->numChannels; k++)
                {
                    auto row = new BaseRow();
                    //int trackidx = fc->mi.mod->xxp[i]->index[k];
                    row->note = plugin->mod->
                                            get_pattern_row_channel_command(i, j, k, plugin->mod->command_note);
                    row->sample = plugin->mod->get_pattern_row_channel_command(
                        i, j, k, plugin->mod->command_instrument);
                    int effect = plugin->mod->get_pattern_row_channel_command(
                        i, j, k, plugin->mod->command_effect);
                    if (effect > 0)
                    {
                        effect--;
                    }
                    if (effect == 0x12)
                    {
                        effect = 0x0e;
                    }
                    else if (effect == 0x10)
                    {
                        effect = 0x0f;
                    }


                    row->effect = effect;
                    row->param = plugin->mod->get_pattern_row_channel_command(
                        i, j, k, plugin->mod->command_parameter);
                    row->effect2 = plugin->mod->get_pattern_row_channel_command(
                        i, j, k, plugin->mod->command_volumeffect);
                    row->param2 = 0;
                    row->vol = plugin->mod->get_pattern_row_channel_command(
                        i, j, k, plugin->mod->command_volume);

                    modRows[counter] = row;
                    counter++;
                }
            }
            plugin->info->patterns.push_back(modRows);
        }
        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = plugin->mod->get_position_seconds() * 1000;
        return FMOD_OK;
    }

    return FMOD_OK;
}
