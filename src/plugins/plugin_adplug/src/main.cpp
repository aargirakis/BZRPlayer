#include "fmod_errors.h"
//#include <windows.h>
//#include "Logfile.h"

#include <adplug.h>
#include <emuopl.h>
#include <kemuopl.h>
#include <silentopl.h>

#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "info.h"
#include "plugins.h"

using namespace std;

//CLogFile *LogFile;
static int samples_left = 0;

FMOD_RESULT handle_error( const char* str )
{
    if ( str )
    {
        //CLogFile::getInstance()->Print( "Error: %s\n", str );
        return FMOD_ERR_INTERNAL;
    }
    else
        return FMOD_OK;
}
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION adplugcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD AdPlugin",			// Name.
    0x00010000,                         // Version 0xAAAABBBB   A = major, B = minor.
    1,                                  // force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS|FMOD_TIMEUNIT_SUBSONG,					// The time format we would like to accept into setposition/getposition.
    &open,                           // Open callback.
    &close,                          // Close callback.
    &read,                           // Read callback.
    &getlength,                      // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                    // Setposition callback.
    0,                                  // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                   // Sound create callback (don't need it)
};


class adplugin
{
    FMOD_CODEC_STATE *_codec;

public:
    adplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&adwaveformat, 0, sizeof(adwaveformat));
    }

    ~adplugin()
    {
        //delete some stuff
        delete player;
        delete opl;
    }
    CPlayer *	player;
    Copl*	opl;
    unsigned int m_remainingSamples;
    FMOD_CODEC_WAVEFORMAT adwaveformat;

};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &adplugcodec;
}

#ifdef __cplusplus
}
#endif



FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

    adplugin *ad = new adplugin(codec);
    Info* info = (Info*)userexinfo->userdata;

    FMOD_RESULT       result;
    ad->opl = new CKemuopl(44100,true,false);
    if (!ad->opl)
    {
        delete ad->opl;
        return FMOD_ERR_FORMAT;
    }

    cout << "adplug trying to load: " << info->filename << "\n";
    ad->player = CAdPlug::factory(info->filename,ad->opl);
    if (!ad->player)
    {
        delete ad->player;
        delete ad->opl;
        return FMOD_ERR_FORMAT;
    }


    //read config from disk


    string filename = info->applicationPath + "/user/plugin/config/adplug.cfg";
    ifstream ifs( filename.c_str() );
    string line;

    bool useDefaults = false;
    if(ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    int emulator = 0;
    int freq = 44100;
    bool stereo = true;
    ad->adwaveformat.channels = 1;

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
                        stereo = true;
                        ad->adwaveformat.channels = 2;
                    }
                    else
                    {
                        stereo = false;
                        ad->adwaveformat.channels = 2;
                    }
                }
                else if(word.compare("emulator")==0)
                {
                    emulator = atoi(value.c_str());
                }
            }
        }
        ifs.close();
    }

    //we have to create a new engine AGAIN with the new settings
    delete ad->opl;
    delete ad->player;
    //stereo plays twice as fast.........................???
    if(emulator==2)
    {
        ad->opl = new CKemuopl(freq,true,false);
    }
    else
    {
        ad->opl = new CEmuopl(freq,true,false);
    }
    if (!ad->opl)
    {
        delete ad->opl;
        return FMOD_ERR_FORMAT;
    }

    ad->player = CAdPlug::factory(info->filename,ad->opl);
    if (!ad->player)
    {
        delete ad->player;
        delete ad->opl;
        return FMOD_ERR_FORMAT;
    }

    ad->adwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    ad->adwaveformat.frequency    = freq;
    ad->adwaveformat.pcmblocksize   = (16 >> 3) * ad->adwaveformat.channels;
    ad->adwaveformat.lengthpcm    = ad->player->songlength()/1000*ad->adwaveformat.frequency;

    codec->waveformat   = &(ad->adwaveformat);
    codec->numsubsounds = 0;                    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds.
    codec->plugindata   = ad;                    // user data value

    info->fileformat = ad->player->gettype();
    info->artist = ad->player->getauthor();
    info->title = ad->player->gettitle();
    info->comments = ad->player->getdesc();
    info->numInstruments = ad->player->getinstruments();
    info->numPatterns=ad->player->getpatterns();
    info->numOrders=ad->player->getorders();

    info->instruments = new string[info->numInstruments];
    for(int j = 0; j<info->numInstruments; j++)
    {
        info->instruments[j] = ad->player->getinstrument(j);
    }
    info->plugin = PLUGIN_adplug;
    info->pluginName = PLUGIN_adplug_NAME;
    info->setSeekable(true);


    /*ad->player->update();
    samples_left = (int)(2 * ad->adwaveformat.frequency / ad->player->getrefresh());
 ad->player->rewind();
 ad->opl->init();*/
    cout << "adlib ok\n";
    return FMOD_OK;

}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
    delete (adplugin*)codec->plugindata;
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    adplugin* ad = (adplugin*)codec->plugindata;



        int real_len = size;
        for(;;)
        {

            //Here, somewhere, is a bug that makes the playing slower for a second in the beginning
            if (samples_left < real_len)
            {
                real_len -= samples_left;
                ad->player->update();
                samples_left = (int)(1 * ad->adwaveformat.frequency / ad->player->getrefresh());
            }
            else
            {
                ad->opl->update((short*)buffer,size);
                samples_left -= real_len;
                *read=size;
                return FMOD_OK;
            }
        }
//    unsigned int numSamples = size;
//    auto maxSamples = numSamples;

////            if (m_hasEnded)
////            {
////                m_hasEnded = false;
////                return 0;
////            }

//            auto remainingSamples = ad->m_remainingSamples;
//            while (numSamples > 0)
//            {
//                if (remainingSamples > 0)
//                {
//                    auto samplesToAdd = min(numSamples, remainingSamples);

//                    //auto buf = reinterpret_cast<int16_t*>(output + samplesToAdd) - samplesToAdd * 2;
//                    ad->opl->update((short*)buffer, samplesToAdd);

//                    remainingSamples -= samplesToAdd;
//                    numSamples -= samplesToAdd;

//                    //output = output->Convert(buf, samplesToAdd);
//                }
//                else if (ad->player->update())
//                {
//                    remainingSamples = static_cast<uint32_t>(ad->adwaveformat.frequency / ad->player->getrefresh());
//                    //m_isStuck = 0;
//                }
////                else if (m_isStuck == 1)
////                {
////                    m_player->rewind(m_subsongIndex);
////                    m_isStuck++;
////                }
//                else
//                {
//                    //m_isStuck++;
//                    //m_hasEnded = m_isStuck > 1 || numSamples != maxSamples;
//                    break;
//                }
//            }
//            ad->m_remainingSamples = remainingSamples;

//            *read = maxSamples - numSamples;
            return FMOD_OK;

}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype)
{
    adplugin* ad = (adplugin*)codec->plugindata;

    //printf("lengthtype: %i ",lengthtype);

    if(lengthtype==FMOD_TIMEUNIT_MS)
    {
        *length = ad->adwaveformat.lengthpcm;
        return FMOD_OK;
    }
    else if(lengthtype==FMOD_TIMEUNIT_SUBSONG)
    {
        *length = ad->player->getsubsongs();
        return FMOD_OK;
    }
    else if(lengthtype==FMOD_TIMEUNIT_SUBSONG_MS)
    {

        *length = ad->player->songlength();
        return FMOD_OK;
    }
    return FMOD_OK;
    
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{

    adplugin* ad = (adplugin*)codec->plugindata;
    if(postype==FMOD_TIMEUNIT_MS)
    {
        ad->m_remainingSamples=0;
        ad->player->seek(position);
        return FMOD_OK;
    }
    else if(postype==FMOD_TIMEUNIT_SUBSONG)
    {
        if(position<0) position = 0;
        ad->player->rewind(position);
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }
}


