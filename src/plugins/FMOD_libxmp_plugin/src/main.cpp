#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "fmod.h"

#include "info.h"
#include "xmp.h"
#include "BaseRow.h"



FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD FLOD plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    0,                                   // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS|FMOD_TIMEUNIT_MS_REAL|FMOD_TIMEUNIT_SUBSONG|FMOD_TIMEUNIT_SUBSONG_MS|FMOD_TIMEUNIT_MUTE_VOICE|FMOD_TIMEUNIT_MODROW|FMOD_TIMEUNIT_MODPATTERN|FMOD_TIMEUNIT_MODPATTERN_INFO|FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS|FMOD_TIMEUNIT_MODVUMETER|FMOD_TIMEUNIT_MODORDER|FMOD_TIMEUNIT_SPEED|FMOD_TIMEUNIT_BPM,     // The time format we would like to accept into setposition/getposition.
    &fcopen,                             // Open callback.
    &fcclose,                            // Close callback.
    &fcread,                             // Read callback.
    &fcgetlength,                        // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition,                      // Setposition callback.
    &fcgetposition,                      // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};
const char* NOTES[109] =
{
    "",  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
    "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
    "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
    "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
    "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
    "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9"
};
class fcplugin
{
    FMOD_CODEC_STATE *_codec;

public:
    fcplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));

    }

    ~fcplugin()
    {
        //delete some stuff

        delete[] myBuffer;

        //xmp_end_player(xmp);
        xmp_release_module(xmp); /* unload module */
        xmp_free_context(xmp); /* destroy the player context */
        myBuffer=0;

    }
    signed short* myBuffer;
    Info* info;
    xmp_context xmp;
    struct xmp_module_info mi;
    struct xmp_frame_info fi;
    unsigned int subsong;

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

//__declspec(dllexport) FMOD_CODEC_DESCRIPTION* F_STDCALL _FMODGetCodecDescription()
//{
//    return &fccodec;
//}

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &fccodec;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

    FMOD_RESULT       result;

    fcplugin *fc = new fcplugin(codec);
    fc->info = (Info*)userexinfo->userdata;

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if(filesize==4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }


    char* smallBuffer;
    smallBuffer = new char[4];

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,smallBuffer,4,&bytesread);



    if((smallBuffer[0]=='M' && smallBuffer[1]=='T' && smallBuffer[2]=='h' && smallBuffer[3]=='d') || smallBuffer[0]=='R' && smallBuffer[1]=='I' && smallBuffer[2]=='F' && smallBuffer[3]=='F') //it's a midi file
    {

        delete[] smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] smallBuffer;

    fc->myBuffer = new signed short[filesize];
    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,fc->myBuffer,filesize,&bytesread);
    fc->xmp = xmp_create_context();

    /* Load our module */
    if (xmp_load_module_from_memory(fc->xmp,(signed short*)fc->myBuffer,bytesread) != 0) //Doesn't work with prowizard songs
    //if (xmp_load_module(fc->xmp, const_cast<char*>(fc->info->filename.c_str())) != 0)
    {
        xmp_free_context(fc->xmp); /* destroy the player context */
        delete[] fc->myBuffer;
        return FMOD_ERR_FORMAT;
    }

    fc->info = (Info*)userexinfo->userdata;



    //read config from disk
    string filename = fc->info->applicationPath + "/data/plugin/config/libxmp.cfg";
    ifstream ifs( filename.c_str() );
    string line;
    bool useDefaults = false;
    if(ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    int freq = 44100;
    int channels = 2;
    int interpolation=XMP_INTERP_LINEAR;
    int stereoSeparation=70;
    if(!useDefaults)
    {
        while( getline( ifs , line) )
        {
            int i = line.find_first_of("=");

            if(i!=-1)
            {
                string word = line.substr(0,i);
                string value = line.substr(i+1);
                if(word.compare("frequency")==0)
                {
                    freq = atoi(value.c_str());
                }
                else if(word.compare("playback")==0)
                {
                    if(value.compare("stereo")==0)
                    {
                        channels = 2;
                    }
                    else
                    {
                        channels = 1;
                    }
                }
                else if(word.compare("resampling")==0)
                {
                    if(value.compare("nearest")==0)
                    {
                        interpolation=XMP_INTERP_NEAREST;
                    }
                    else if(value.compare("linear")==0)
                    {
                        interpolation=XMP_INTERP_LINEAR;
                    }
                    else if(value.compare("cubic")==0)
                    {
                        interpolation=XMP_INTERP_SPLINE;
                    }
                }
                else if(word.compare("stereo_separation")==0)
                {
                    stereoSeparation = atoi(value.c_str());
                }
            }
        }
        ifs.close();
    }


    fc->fcwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels     = channels;
    fc->fcwaveformat.frequency    = freq;
    fc->fcwaveformat.pcmblocksize   = (16 >> 3) * fc->fcwaveformat.channels;


    codec->waveformat   = &(fc->fcwaveformat);
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = fc;                    /* user data value */


    if(channels==1)
    {
        xmp_start_player(fc->xmp, freq, XMP_FORMAT_MONO);
    }
    else
    {
        xmp_start_player(fc->xmp, freq, 0);
    }

    xmp_get_module_info(fc->xmp, &fc->mi);
    xmp_get_frame_info(fc->xmp, &fc->fi);



    fc->fcwaveformat.lengthpcm = 0xffffffff;(fc->fi.total_time/1000.0)*freq;

    xmp_set_player(fc->xmp,XMP_PLAYER_INTERP,interpolation);

    xmp_set_player(fc->xmp,XMP_PLAYER_DSP,XMP_DSP_LOWPASS);

    xmp_set_player(fc->xmp,XMP_PLAYER_MIX,stereoSeparation);





    fc->info->title = fc->mi.mod->name;

    if(fc->mi.comment)
    {
        fc->info->comments = fc->mi.comment;
    }

    fc->info->numPatterns = fc->mi.mod->pat;
    fc->info->numChannels = fc->mi.mod->chn;
    fc->info->numOrders = fc->mi.mod->len;
    fc->info->restart = fc->mi.mod->rst;

    fc->info->modVUMeters = new unsigned char[fc->info->numChannels];


    int numSamples =  fc->mi.mod->ins;

//    cout << "samples: " << fc->mi.mod->smp << "\n";
//    cout << "instrumens: " << fc->mi.mod->ins << "\n";


    if(fc->mi.mod->ins!=fc->mi.mod->smp)
    {
        numSamples=0;
    }

    fc->info->numSamples = numSamples;

    if(numSamples>0)
    {

        fc->info->samples = new string[numSamples];
        fc->info->samplesSize = new unsigned int[numSamples];
        fc->info->samplesLoopStart = new unsigned int[numSamples];
        fc->info->samplesLoopEnd = new unsigned int[numSamples];
        fc->info->samplesVolume = new unsigned short[numSamples];
        fc->info->samplesFineTune = new signed int[numSamples];
        fc->info->samplesData = new unsigned char*[numSamples];
        //        fc->info->samples16Bit = new bool[numSamples];
        //        fc->info->samplesStereo = new bool[numSamples];
        for(int j = 0; j<numSamples; j++)
        {
            fc->info->samples[j] = fc->mi.mod->xxi[j].name;
            fc->info->samplesSize[j] = fc->mi.mod->xxs[j].len;
            fc->info->samplesLoopStart[j] = fc->mi.mod->xxs[j].lps;
            fc->info->samplesLoopEnd[j] = fc->mi.mod->xxs[j].lpe;
            fc->info->samplesData[j] = fc->mi.mod->xxs[j].data;


            if(fc->mi.mod->xxi[j].name && fc->mi.mod->xxi[j].sub)
            {
                fc->info->samplesVolume[j] = fc->mi.mod->xxi[j].sub->vol*4;
                fc->info->samplesFineTune[j] = fc->mi.mod->xxi[j].sub->fin;
            }
            else
            {
                fc->info->samplesVolume[j] = 0;
                fc->info->samplesFineTune[j] = 0;//fc->mi.mod->xxi[j].sub->fin;
            }
        }
    }



    //    int numInstruments = fc->mi.mod->ins;
    //    fc->info->numInstruments = numInstruments;

    //    if(numInstruments>0)
    //    {
    //        fc->info->samples = new string[numInstruments];
    //        fc->info->samplesSize = new unsigned int[numInstruments];
    //        fc->info->samplesLoopStart = new unsigned int[numInstruments];
    //        fc->info->samplesLoopEnd = new unsigned int[numInstruments];
    //        fc->info->samplesVolume = new unsigned short[numInstruments];
    //        fc->info->samplesFineTune = new signed int[numInstruments];
    //        fc->info->samples16Bit = new bool[numInstruments];
    //        fc->info->samplesStereo = new bool[numInstruments];
    //        for(int j = 0; j<numInstruments; j++)
    //        {
    //            fc->info->samples[j] = fc->mi.mod->xxi[j].name;
    //            fc->info->samplesSize[j] = fc->mi.mod->xxi[j].len;
    //            fc->info->samplesLoopStart[j] = fc->mi.mod->xxi[j].lps;
    //            fc->info->samplesLoopEnd[j] = fc->mi.mod->xxi[j].lpe;
    //            fc->info->samplesVolume = fc->mi.mod->xxi[j].sub->vol;

    ////            lp->info->samplesVolume[j-1] = ModPlug_SampleVolume(lp->moddata,j);
    ////            lp->info->samplesFineTune[j-1] = ModPlug_SampleFineTune(lp->moddata,j);
    ////            lp->info->samples16Bit[j-1] = ModPlug_Sample16Bit(lp->moddata,j);
    ////            lp->info->samplesStereo[j-1] = ModPlug_SampleStereo(lp->moddata,j);
    ////            lp->info->samples[j-1] = c;
    //        }
    //    }

    fc->subsong = 0;
    fc->info->fileformat = fc->mi.mod->type;
    fc->info->plugin = "libxmp";
    fc->info->setSeekable(true);
    fc->info->numSubsongs = fc->mi.num_sequences;
    //delete c;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec)
{

    delete (fcplugin*)codec->plugindata;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    xmp_play_buffer(fc->xmp,buffer,size<<2,1);

    *read=size;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    if(postype==FMOD_TIMEUNIT_MUTE_VOICE)
    {

    }
    if(postype==FMOD_TIMEUNIT_MS)
    {
        xmp_seek_time(fc->xmp, position);
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_SUBSONG)
    {
        fc->subsong = position;
        int startPattern = fc->mi.seq_data[position].entry_point;
        //xmp_restart_module(fc->xmp);
        xmp_set_position(fc->xmp, startPattern);
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for(int i = 0 ; i<fc->info->numChannels ; i++)
        {
            bool mute = false;
            if(fc->info->mutedChannelsMask.at(i)=='0')
            {
                mute = true;
            }
            xmp_channel_mute(fc->xmp, i, mute);

        }


        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_ERR_UNSUPPORTED;


}

FMOD_RESULT F_CALLBACK fcgetlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if(lengthtype==FMOD_TIMEUNIT_SUBSONG_MS || lengthtype==FMOD_TIMEUNIT_MUTE_VOICE)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *length = fc->fi.total_time;
        return FMOD_OK;
    }
    else if(lengthtype==FMOD_TIMEUNIT_SUBSONG)
    {
        *length = fc->mi.num_sequences;
        return FMOD_OK;
    }

    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }

}

FMOD_RESULT F_CALLBACK fcgetposition(FMOD_CODEC_STATE *  codec, unsigned int *position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    if(postype==FMOD_TIMEUNIT_SUBSONG)
    {
            *position = fc->subsong;
            return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MODROW)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.row;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MODPATTERN)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.pattern;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MODORDER)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.pos;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_SPEED)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.speed;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_BPM)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.bpm;
        return FMOD_OK;
    }
    //this will return value too early because of no queue
    else if(postype==FMOD_TIMEUNIT_CURRENT_PATTERN_ROWS)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.num_rows;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MODVUMETER)
    {
        unsigned char* vumeters = new unsigned char[fc->info->numChannels];
        xmp_get_channel_volumes(fc->xmp,vumeters);
        fc->info->modVUMeters = vumeters;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0


        fc->info->modRows.clear();

        for (vector< vector<BaseRow*> >::iterator itr = fc->info->patterns.begin(); itr != fc->info->patterns.end(); ++itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        fc->info->patterns.clear();

        for( int i=0; i<fc->mi.mod->pat; i++ )
        {
            int numberRows = fc->mi.mod->xxp[i]->rows;
            vector<BaseRow*> modRows(numberRows*fc->info->numChannels);
            int counter = 0;
            for( int j=0; j<numberRows; j++ )
            {
                for( int k=0; k<fc->info->numChannels; k++ )
                {
                    BaseRow* row = new BaseRow();
                    int trackidx = fc->mi.mod->xxp[i]->index[k];

                    int note = fc->mi.mod->xxt[trackidx]->event[j].note;
                    row->note = note;
                    row->sample = fc->mi.mod->xxt[trackidx]->event[j].ins;
                    row->effect = fc->mi.mod->xxt[trackidx]->event[j].fxt;
                    row->param = fc->mi.mod->xxt[trackidx]->event[j].fxp;
                    row->effect2 = fc->mi.mod->xxt[trackidx]->event[j].f2t;
                    row->param2 = fc->mi.mod->xxt[trackidx]->event[j].f2p;
                    row->vol = fc->mi.mod->xxt[trackidx]->event[j].vol-1;
                    if(fc->info->fileformat=="Composer 669")
                    {
                        row->vol /=4;
                    }
                    modRows[counter] = row;
                    counter++;
                }
            }
            fc->info->patterns.push_back(modRows);
        }
        *position = 0;
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_MS_REAL)
    {
        xmp_get_frame_info(fc->xmp, &fc->fi);
        *position = fc->fi.time;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
