#include <iostream>
#include <stdio.h>
#include <string.h>
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


FMOD_CODEC_DESCRIPTION libopenmptcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD libopenmpt plugin", // Name.
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
    0 // Sound create callback (don't need it)
};

class libopenmptplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    libopenmptplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&libopenmptwaveformat, 0, sizeof(libopenmptwaveformat));
    }

    ~libopenmptplugin()
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
    FMOD_CODEC_WAVEFORMAT libopenmptwaveformat;
};

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &libopenmptcodec;
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

    libopenmptplugin* libopenmpt = new libopenmptplugin(codec);
    libopenmpt->info = (Info*)userexinfo->userdata;
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

    libopenmpt->myBuffer = new unsigned char[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, libopenmpt->myBuffer, filesize, &bytesread);

    try
    {
        Info* info = (Info*)userexinfo->userdata;


        //read config from disk
        string filename = libopenmpt->info->applicationPath + "/user/plugin/config/libopenmpt.cfg";
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
            {"play.at_end", "stop"},
            {"render.resampler.emulate_amiga", emulate_amiga_filter},
            {"render.resampler.emulate_amiga_type", amiga_filter},
            {"dither", dither},
            /*{ "play.at_end", "fadeout" },*/
        };
        libopenmpt->mod = nullptr;
        libopenmpt->mod = new openmpt::module_ext(libopenmpt->myBuffer, filesize, std::clog, ctls);


        libopenmpt->mod->set_render_param(openmpt::module::RENDER_STEREOSEPARATION_PERCENT, stereo_separation);
        libopenmpt->mod->set_render_param(openmpt::module::RENDER_INTERPOLATIONFILTER_LENGTH, interpolation_filter);

        libopenmpt->libopenmptwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
        libopenmpt->libopenmptwaveformat.channels = 2;
        libopenmpt->libopenmptwaveformat.frequency = 44100;
        libopenmpt->libopenmptwaveformat.pcmblocksize = (16 >> 3) * libopenmpt->libopenmptwaveformat.channels;
        libopenmpt->libopenmptwaveformat.lengthpcm = 0xffffffff;
        //libopenmpt->mod->get_duration_seconds()*libopenmpt->libopenmptwaveformat.frequency;

        codec->waveformat = &(libopenmpt->libopenmptwaveformat);
        codec->numsubsounds = 0;
        /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata = libopenmpt; /* user data value */

        info->numSamples = libopenmpt->mod->get_num_samples();
        info->numInstruments = libopenmpt->mod->get_num_instruments();
        info->title = libopenmpt->mod->get_metadata("title");

        info->comments = libopenmpt->mod->get_metadata("message_raw");


        info->numChannels = libopenmpt->mod->get_num_channels();
        info->numPatterns = libopenmpt->mod->get_num_patterns();
        info->numOrders = libopenmpt->mod->get_num_orders();
        //        info->restart = fc->mi.mod->rst;


        if (info->numSamples > 0)
        {
            std::vector<std::string> samplenames = libopenmpt->mod->get_sample_names();

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
                info->samplesSize[j] = libopenmpt->mod->get_mod_sample_size(j + 1);
                info->samplesLoopStart[j] = libopenmpt->mod->get_mod_sample_loopstart(j + 1);
                info->samplesLoopEnd[j] = libopenmpt->mod->get_mod_sample_loopend(j + 1);
                info->samplesVolume[j] = libopenmpt->mod->get_mod_sample_volume(j + 1) / 4;
                int finetune = libopenmpt->mod->get_mod_sample_finetune(j + 1);
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
            std::vector<std::string> instrumentnames = libopenmpt->mod->get_instrument_names();

            info->instruments = new string[info->numInstruments];

            for (int j = 0; j < info->numInstruments; j++)
            {
                info->instruments[j] = instrumentnames.at(j);
            }
        }

        info->fileformat = libopenmpt->mod->get_metadata("type_long");

        //get_current_channel_vu_mono seems to return 1.40317 for max volume for most formats, but max for protracker it is 1.11105
        if (info->fileformat.substr(0, 10) == "ProTracker" || info->fileformat.substr(0, 12) == "Soundtracker")
        {
            libopenmpt->maxVUMeter = 1.11105;
        }
        else
        {
            libopenmpt->maxVUMeter = 1.40317;
        }


        info->plugin = PLUGIN_libopenmpt;
        info->pluginName = PLUGIN_libopenmpt_NAME;
        info->setSeekable(true);
        //libopenmpt->vumeterBuffer = CreateQueue(22);
        return FMOD_OK;
    }
    catch (...)
    {
        delete libopenmpt->mod;
        libopenmpt->mod = nullptr;
        delete[] libopenmpt->myBuffer;
        return FMOD_ERR_FORMAT;
    }
}

FMOD_RESULT F_CALLBACK libopenmptclose(FMOD_CODEC_STATE* codec)
{
    delete (libopenmptplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK libopenmptread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    libopenmptplugin* libopenmpt = (libopenmptplugin*)codec->plugindata;
    libopenmpt->mod->read_interleaved_stereo(libopenmpt->libopenmptwaveformat.frequency, size, (short*)buffer);
    unsigned char* vumeters = new unsigned char[libopenmpt->info->numChannels];


    for (int i = 0; i < libopenmpt->info->numChannels; i++)
    {
        vumeters[i] = (libopenmpt->mod->get_current_channel_vu_mono(i) / libopenmpt->maxVUMeter) * 100;
    }

    if (libopenmpt->vumeterBuffer.size() >= 70)
    {
        libopenmpt->vumeterBuffer.pop();
        libopenmpt->rowBuffer.pop();
        libopenmpt->patternBuffer.pop();
        libopenmpt->orderBuffer.pop();
    }
    libopenmpt->vumeterBuffer.push(vumeters);
    libopenmpt->rowBuffer.push(libopenmpt->mod->get_current_row());
    libopenmpt->patternBuffer.push(libopenmpt->mod->get_current_pattern());
    libopenmpt->orderBuffer.push(libopenmpt->mod->get_current_order());

    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK libopenmptgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    libopenmptplugin* libopenmpt = (libopenmptplugin*)codec->plugindata;

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MS || lengthtype ==
        FMOD_TIMEUNIT_MUTE_VOICE)
    {
        *length = libopenmpt->mod->get_duration_seconds() * 1000;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = libopenmpt->mod->get_num_subsongs();
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
    libopenmptplugin* libopenmpt = (libopenmptplugin*)codec->plugindata;
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //         //position is a mask
        //         for(int i = 0 ; i<libopenmpt->info->numChannels ; i++)
        //         {
        //             #ifdef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
        //               openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>( libopenmpt->mod->get_interface( openmpt::ext::interactive_id ) );

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


        for (int i = 0; i < libopenmpt->info->numChannels; i++)
        {
#ifdef LIBOPENMPT_EXT_INTERFACE_INTERACTIVE
            openmpt::ext::interactive* interactive = static_cast<openmpt::ext::interactive*>(libopenmpt->mod->
                get_interface(openmpt::ext::interactive_id));

            if (interactive)
            {
                bool mute = false;
                if (libopenmpt->info->mutedChannelsMask.at(i) == '0')
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
        libopenmpt->mod->set_position_seconds(position / 1000.0);
        return FMOD_OK;
    }

    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        libopenmpt->mod->select_subsong(position);
        return FMOD_OK;
    }
    return FMOD_ERR_UNSUPPORTED;;
}

FMOD_RESULT F_CALLBACK libopenmptgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    libopenmptplugin* libopenmpt = (libopenmptplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        libopenmpt->info->modVUMeters = libopenmpt->vumeterBuffer.front();
        //cout << "get position " <<  ": " << (int)libopenmpt->info->modVUMeters[0] << endl;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = libopenmpt->rowBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        *position = libopenmpt->patternBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        *position = libopenmpt->orderBuffer.front();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SPEED)
    {
        *position = libopenmpt->mod->get_current_speed();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_BPM)
    {
        *position = libopenmpt->mod->get_current_estimated_bpm();
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0

        libopenmpt->info->modRows.clear();

        for (vector<vector<BaseRow*>>::iterator itr = libopenmpt->info->patterns.begin(); itr != libopenmpt->info->
             patterns.end(); ++itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        libopenmpt->info->patterns.clear();

        for (int i = 0; i < libopenmpt->mod->get_num_patterns(); i++)
        {
            int numberRows = libopenmpt->mod->get_pattern_num_rows(i);
            vector<BaseRow*> modRows(numberRows * libopenmpt->info->numChannels);
            int counter = 0;
            for (int j = 0; j < numberRows; j++)
            {
                for (int k = 0; k < libopenmpt->info->numChannels; k++)
                {
                    BaseRow* row = new BaseRow();
                    //int trackidx = fc->mi.mod->xxp[i]->index[k];
                    row->note = libopenmpt->mod->
                                            get_pattern_row_channel_command(i, j, k, libopenmpt->mod->command_note);
                    row->sample = libopenmpt->mod->get_pattern_row_channel_command(
                        i, j, k, libopenmpt->mod->command_instrument);
                    int effect = libopenmpt->mod->get_pattern_row_channel_command(
                        i, j, k, libopenmpt->mod->command_effect);
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
                    row->param = libopenmpt->mod->get_pattern_row_channel_command(
                        i, j, k, libopenmpt->mod->command_parameter);
                    row->effect2 = libopenmpt->mod->get_pattern_row_channel_command(
                        i, j, k, libopenmpt->mod->command_volumeffect);
                    row->param2 = 0;
                    row->vol = libopenmpt->mod->get_pattern_row_channel_command(
                        i, j, k, libopenmpt->mod->command_volume);

                    modRows[counter] = row;
                    counter++;
                }
            }
            libopenmpt->info->patterns.push_back(modRows);
        }
        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MS_REAL)
    {
        *position = libopenmpt->mod->get_position_seconds() * 1000;
        return FMOD_OK;
    }

    return FMOD_OK;
}
