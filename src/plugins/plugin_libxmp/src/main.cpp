#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "fmod.h"
#include "info.h"
#include "xmp.h"
#include "BaseRow.h"
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
    PLUGIN_libxmp_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS |
    FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODROW | FMOD_TIMEUNIT_MODPATTERN | FMOD_TIMEUNIT_MODPATTERN_INFO |
    FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS | FMOD_TIMEUNIT_MODVUMETER | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_SPEED |
    FMOD_TIMEUNIT_BPM, // The time format we would like to accept into setposition/getposition.
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
const char* NOTES[109] =
{
    "", "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
    "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
    "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
    "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
    "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
    "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};

class pluginLibxmp
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginLibxmp(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibxmp()
    {
        //delete some stuff

        delete[] myBuffer;

        //xmp_end_player(xmp);
        xmp_release_module(xmp); /* unload module */
        xmp_free_context(xmp); /* destroy the player context */
        myBuffer = 0;
    }

    signed short* myBuffer;
    Info* info;
    xmp_context xmp;
    struct xmp_module_info mi;
    struct xmp_frame_info fi;
    unsigned int subsong;

    FMOD_CODEC_WAVEFORMAT waveformat;
    bool isContinuousPlaybackActive;
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

    auto* plugin = new pluginLibxmp(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
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

    delete[] smallBuffer;

    plugin->myBuffer = new signed short[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, &bytesread);
    plugin->xmp = xmp_create_context();

    /* Load our module */
    if (xmp_load_module_from_memory(plugin->xmp, (signed short*)plugin->myBuffer, bytesread) != 0)
    //Doesn't work with prowizard songs
    //if (xmp_load_module(plugin->xmp, const_cast<char*>(plugin->info->filename.c_str())) != 0)
    {
        xmp_free_context(plugin->xmp); /* destroy the player context */
        delete[] plugin->myBuffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->info = static_cast<Info*>(userexinfo->userdata);


    //read config from disk
    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/libxmp.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    int freq = 44100;
    int channels = 2;
    int interpolation = XMP_INTERP_LINEAR;
    int stereoSeparation = 70;
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
                    if (value.compare("stereo") == 0)
                    {
                        channels = 2;
                    }
                    else
                    {
                        channels = 1;
                    }
                }
                else if (word.compare("resampling") == 0)
                {
                    if (value.compare("nearest") == 0)
                    {
                        interpolation = XMP_INTERP_NEAREST;
                    }
                    else if (value.compare("linear") == 0)
                    {
                        interpolation = XMP_INTERP_LINEAR;
                    }
                    else if (value.compare("cubic") == 0)
                    {
                        interpolation = XMP_INTERP_SPLINE;
                    }
                }
                else if (word.compare("stereo_separation") == 0)
                {
                    stereoSeparation = atoi(value.c_str());
                }
                else if (word.compare("continuous_playback") == 0)
                {
                    plugin->isContinuousPlaybackActive =
                        plugin->info->isPlayModeRepeatSongEnabled && value.compare("true") == 0;
                }
            }
        }
        ifs.close();
    }


    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = channels;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;


    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */


    if (channels == 1)
    {
        xmp_start_player(plugin->xmp, freq, XMP_FORMAT_MONO);
    }
    else
    {
        xmp_start_player(plugin->xmp, freq, 0);
    }

    xmp_get_module_info(plugin->xmp, &plugin->mi);
    xmp_get_frame_info(plugin->xmp, &plugin->fi);


    plugin->waveformat.lengthpcm = 0xffffffff;
    (plugin->fi.total_time / 1000.0) * freq;

    xmp_set_player(plugin->xmp,XMP_PLAYER_INTERP, interpolation);

    xmp_set_player(plugin->xmp,XMP_PLAYER_DSP,XMP_DSP_LOWPASS);

    xmp_set_player(plugin->xmp,XMP_PLAYER_MIX, stereoSeparation);


    plugin->info->title = plugin->mi.mod->name;

    if (plugin->mi.comment)
    {
        plugin->info->comments = plugin->mi.comment;
    }

    plugin->info->numPatterns = plugin->mi.mod->pat;
    plugin->info->numChannels = plugin->mi.mod->chn;
    plugin->info->numOrders = plugin->mi.mod->len;
    plugin->info->restart = plugin->mi.mod->rst;

    plugin->info->modVUMeters = new unsigned char[plugin->info->numChannels];


    int numSamples = plugin->mi.mod->ins;

    //    cout << "samples: " << plugin->mi.mod->smp << "\n";
    //    cout << "instrumens: " << plugin->mi.mod->ins << "\n";


    if (plugin->mi.mod->ins != plugin->mi.mod->smp)
    {
        numSamples = 0;
    }

    plugin->info->numSamples = numSamples;

    if (numSamples > 0)
    {
        plugin->info->samples = new string[numSamples];
        plugin->info->samplesSize = new unsigned int[numSamples];
        plugin->info->samplesLoopStart = new unsigned int[numSamples];
        plugin->info->samplesLoopEnd = new unsigned int[numSamples];
        plugin->info->samplesVolume = new unsigned short[numSamples];
        plugin->info->samplesFineTune = new signed int[numSamples];
        plugin->info->samplesData = new unsigned char*[numSamples];
        //        plugin->info->samples16Bit = new bool[numSamples];
        //        plugin->info->samplesStereo = new bool[numSamples];
        for (int j = 0; j < numSamples; j++)
        {
            plugin->info->samples[j] = plugin->mi.mod->xxi[j].name;
            plugin->info->samplesSize[j] = plugin->mi.mod->xxs[j].len;
            plugin->info->samplesLoopStart[j] = plugin->mi.mod->xxs[j].lps;
            plugin->info->samplesLoopEnd[j] = plugin->mi.mod->xxs[j].lpe;
            plugin->info->samplesData[j] = plugin->mi.mod->xxs[j].data;


            if (plugin->mi.mod->xxi[j].name && plugin->mi.mod->xxi[j].sub)
            {
                plugin->info->samplesVolume[j] = plugin->mi.mod->xxi[j].sub->vol * 4;
                plugin->info->samplesFineTune[j] = plugin->mi.mod->xxi[j].sub->fin;
            }
            else
            {
                plugin->info->samplesVolume[j] = 0;
                plugin->info->samplesFineTune[j] = 0; //plugin->mi.mod->xxi[j].sub->fin;
            }
        }
    }


    //    int numInstruments = plugin->mi.mod->ins;
    //    plugin->info->numInstruments = numInstruments;

    //    if(numInstruments>0)
    //    {
    //        plugin->info->samples = new string[numInstruments];
    //        plugin->info->samplesSize = new unsigned int[numInstruments];
    //        plugin->info->samplesLoopStart = new unsigned int[numInstruments];
    //        plugin->info->samplesLoopEnd = new unsigned int[numInstruments];
    //        plugin->info->samplesVolume = new unsigned short[numInstruments];
    //        plugin->info->samplesFineTune = new signed int[numInstruments];
    //        plugin->info->samples16Bit = new bool[numInstruments];
    //        plugin->info->samplesStereo = new bool[numInstruments];
    //        for(int j = 0; j<numInstruments; j++)
    //        {
    //            plugin->info->samples[j] = plugin->mi.mod->xxi[j].name;
    //            plugin->info->samplesSize[j] = plugin->mi.mod->xxi[j].len;
    //            plugin->info->samplesLoopStart[j] = plugin->mi.mod->xxi[j].lps;
    //            plugin->info->samplesLoopEnd[j] = plugin->mi.mod->xxi[j].lpe;
    //            plugin->info->samplesVolume = plugin->mi.mod->xxi[j].sub->vol;

    ////            lp->info->samplesVolume[j-1] = ModPlug_SampleVolume(lp->moddata,j);
    ////            lp->info->samplesFineTune[j-1] = ModPlug_SampleFineTune(lp->moddata,j);
    ////            lp->info->samples16Bit[j-1] = ModPlug_Sample16Bit(lp->moddata,j);
    ////            lp->info->samplesStereo[j-1] = ModPlug_SampleStereo(lp->moddata,j);
    ////            lp->info->samples[j-1] = c;
    //        }
    //    }

    plugin->subsong = 0;
    plugin->info->fileformat = plugin->mi.mod->type;
    plugin->info->plugin = PLUGIN_libxmp;
    plugin->info->pluginName = PLUGIN_libxmp_NAME;
    plugin->info->setSeekable(true);
    plugin->info->numSubsongs = plugin->mi.num_sequences;
    //delete c;

    return FMOD_OK;
}

FMOD_RESULT F_CALL fcclose(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginLibxmp*>(codec->plugindata);

    return FMOD_OK;
}

FMOD_RESULT F_CALL fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginLibxmp*>(codec->plugindata);
    xmp_play_buffer(plugin->xmp, buffer, size << 2, plugin->isContinuousPlaybackActive ? 0 : 1);

    *read = size;
    return FMOD_OK;
}

FMOD_RESULT F_CALL fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibxmp*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
    }
    if (postype == FMOD_TIMEUNIT_MS)
    {
        xmp_seek_time(plugin->xmp, position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->subsong = position;
        int startPattern = plugin->mi.seq_data[position].entry_point;
        //xmp_restart_module(plugin->xmp);
        xmp_set_position(plugin->xmp, startPattern);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < plugin->info->numChannels; i++)
        {
            bool mute = false;
            if (plugin->info->mutedChannelsMask.at(i) == '0')
            {
                mute = true;
            }
            xmp_channel_mute(plugin->xmp, i, mute);
        }


        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALL fcgetlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginLibxmp*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *length = plugin->isContinuousPlaybackActive ? 0xffffffff : plugin->fi.total_time;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->mi.num_sequences;
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}

FMOD_RESULT F_CALL fcgetposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginLibxmp*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        *position = plugin->subsong;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODROW)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.row;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.pattern;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.pos;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SPEED)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.speed;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_BPM)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.bpm;
        return FMOD_OK;
    }
    //this will return value too early because of no queue
    else if (postype == FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS)
    {
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.num_rows;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        unsigned char* vumeters = new unsigned char[plugin->info->numChannels];
        xmp_get_channel_volumes(plugin->xmp, vumeters);
        plugin->info->modVUMeters = vumeters;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0


        plugin->info->modRows.clear();

        for (vector<vector<BaseRow*>>::iterator itr = plugin->info->patterns.begin(); itr != plugin->info->patterns.end(); ++
             itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        plugin->info->patterns.clear();

        for (int i = 0; i < plugin->mi.mod->pat; i++)
        {
            int numberRows = plugin->mi.mod->xxp[i]->rows;
            vector<BaseRow*> modRows(numberRows * plugin->info->numChannels);
            int counter = 0;
            for (int j = 0; j < numberRows; j++)
            {
                for (int k = 0; k < plugin->info->numChannels; k++)
                {
                    auto row = new BaseRow();
                    int trackidx = plugin->mi.mod->xxp[i]->index[k];

                    int note = plugin->mi.mod->xxt[trackidx]->event[j].note;
                    row->note = note;
                    row->sample = plugin->mi.mod->xxt[trackidx]->event[j].ins;
                    row->effect = plugin->mi.mod->xxt[trackidx]->event[j].fxt;
                    row->param = plugin->mi.mod->xxt[trackidx]->event[j].fxp;
                    row->effect2 = plugin->mi.mod->xxt[trackidx]->event[j].f2t;
                    row->param2 = plugin->mi.mod->xxt[trackidx]->event[j].f2p;
                    row->vol = plugin->mi.mod->xxt[trackidx]->event[j].vol - 1;
                    if (plugin->info->fileformat == "Composer 669")
                    {
                        row->vol /= 4;
                    }
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
        xmp_get_frame_info(plugin->xmp, &plugin->fi);
        *position = plugin->fi.time;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
