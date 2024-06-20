#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "fmod_errors.h"
#include "info.h"
#include "kdmeng.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);


FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "FMOD FLOD plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    0,                                   // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,	 // The time format we would like to accept into setposition/getposition.
    &fcopen,                             // Open callback.
    &fcclose,                            // Close callback.
    &fcread,                             // Read callback.
    0,                        // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
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
        delete m_player;
        myBuffer=0;
    }
    signed short* myBuffer;
    Info* info;
    kdmeng* m_player;

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

//    char* smallBuffer;
//    smallBuffer = new char[2];
//    unsigned int bytesread;
//    result = codec->fileseek(codec->filehandle,0,(char*)smallBuffer);

//    result = codec->fileread(codec->filehandle,(char*)smallBuffer,2,&bytesread,0);


//    if(!(smallBuffer[0]=='a' && smallBuffer[1]=='y') && !(smallBuffer[0]=='y' && smallBuffer[1]=='m'))
//    {
//        delete smallBuffer;
//        return FMOD_ERR_FORMAT;
//    }

    unsigned int bytesread;
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);



    fc->myBuffer = new signed short[filesize];

    result = FMOD_CODEC_FILE_SEEK(codec,0,0);
    result = FMOD_CODEC_FILE_READ(codec,fc->myBuffer,filesize,&bytesread);

    fc->m_player = new kdmeng( 44100, 2, 2 );

    unsigned found = fc->info->filename.find_last_of("/\\");

    long length = fc->m_player->load((signed short*)fc->myBuffer,filesize,fc->info->filename.substr(0,found+1).c_str());

    if ( !length )
    {
        delete fc->myBuffer;
        delete fc->m_player;
        return FMOD_ERR_FORMAT;
    }



    fc->info = (Info*)userexinfo->userdata;

    fc->fcwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels     = 2;
    fc->fcwaveformat.frequency    = 44100;
    fc->fcwaveformat.pcmblocksize   = 1468;
    fc->fcwaveformat.lengthpcm = length/1000*fc->fcwaveformat.frequency;

    codec->waveformat   = &(fc->fcwaveformat);
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = fc;                    /* user data value */

    fc->info->fileformat = "Ken's Digital Music";
    fc->info->plugin = PLUGIN_kdm;
    fc->info->pluginName = PLUGIN_kdm_NAME;
    fc->info->setSeekable(true);

    int numSamples = fc->m_player->getNumwaves();
    int numTracks = fc->m_player->getNumtracks();
    fc->info->numSamples = numSamples;
    fc->info->numPatterns = numTracks;





    if(numSamples>0)
    {
        fc->info->samplesSize = new unsigned int[numSamples];
        fc->info->samplesLoopStart = new unsigned int[numSamples];
        fc->info->samplesLoopLength = new unsigned int[numSamples];
        fc->info->samplesFineTune = new signed int[numSamples];
        fc->info->samples = new string[numSamples];
        char* c = new char[17];
        for(int j = 0; j<numSamples; j++)
        {
            fc->info->samplesSize[j] = fc->m_player->getInstsize(j);
            fc->info->samplesLoopStart[j] = fc->m_player->getInstrepstart(j);
            fc->info->samplesLoopLength[j] = fc->m_player->getInstreplength(j);
            fc->info->samplesFineTune[j] = fc->m_player->getInstfinetune(j);
            fc->m_player->getInstname(j,c);
            fc->info->samples[j] = c;
        }
        delete c;
    }

    if(numTracks>0)
    {
        fc->info->instruments = new string[numTracks];
        fc->info->instrumentsNumber = new char[numTracks];
        fc->info->instrumentsQuantize = new char[numTracks];
        fc->info->instrumentsVolume1 = new unsigned char[numTracks];
        fc->info->instrumentsVolume2 = new unsigned char[numTracks];


        char* c = new char[17];
        for(int j = 0; j<numTracks; j++)
        {
            int instrIdx = fc->m_player->getTrackInstrument(j);
            fc->info->instrumentsNumber[j] = instrIdx+1;
            fc->info->instruments[j] = fc->info->samples[instrIdx];
            fc->info->instrumentsQuantize[j] = fc->m_player->getTrackQuantize(j);
            fc->info->instrumentsVolume1[j] = fc->m_player->getTrackVolume1(j);
            fc->info->instrumentsVolume2[j] = fc->m_player->getTrackVolume2(j);
        }
        delete c;
    }

    fc->m_player->musicon();

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
    fc->m_player->rendersound(buffer,size<<2);
    *read=size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    fc->m_player->seek( position );
    return FMOD_OK;
}



