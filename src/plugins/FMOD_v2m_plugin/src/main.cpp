#include <string.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include "fmod_errors.h"
#include "src/v2mplayer.h"
#include "src/v2mconv.h"
#include "src/sounddef.h"
#include "src/libv2.h"
#include "info.h"

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION tfmxcodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "V2M player plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    1,                                   // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,	              // The time format we would like to accept into setposition/getposition.
    &open,                             // Open callback.
    &close,                            // Close callback.
    &read,                             // Read callback.
    0,                                   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};

class v2mplugin
{
	FMOD_CODEC_STATE *_codec;

	public:
	v2mplugin(FMOD_CODEC_STATE *codec)
	{
		_codec = codec;
		memset(&v2mwaveformat, 0, sizeof(v2mwaveformat));
	}

	~v2mplugin()
	{
		//delete some stuff

       player->Close();
       delete player;
       delete[] convertedSong;


	}
    V2MPlayer* player;
    uint8_t* convertedSong;
	FMOD_CODEC_WAVEFORMAT v2mwaveformat;
	
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
    return &tfmxcodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{
	


    Info* info = (Info*)userexinfo->userdata;
	FMOD_RESULT result;

    string filename_lowercase = info->filename;
    std::transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(), ::tolower);
    if(filename_lowercase.substr(filename_lowercase.find_last_of(".") + 1) != "v2m" && filename_lowercase.substr(filename_lowercase.find_last_of(".") + 1) != "v2")
    {
        return FMOD_ERR_FORMAT;
    }

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
	/* Allocate space for buffer. */
    unsigned char* myBuffer = new unsigned char[filesize];

	//rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec,0,0);

	//read whole file to memory
    unsigned int bytesread;
    result = FMOD_CODEC_FILE_READ(codec,myBuffer,filesize,&bytesread);

	v2mplugin *v2m = new v2mplugin(codec);


    sdInit();
    int version = CheckV2MVersion(myBuffer, filesize);
    if (version < 0)
    {
        delete [] myBuffer;
        return FMOD_ERR_FORMAT;
    }
    //cout << "v2m version: " << version << "\n";
    //flush(cout);



    int converted_length;
    ConvertV2M(myBuffer, filesize, &v2m->convertedSong, &converted_length);
    delete [] myBuffer;


    v2m->player = new V2MPlayer();
    v2m->player->Init();
    if(!v2m->player->Open(v2m->convertedSong))
    {
        return FMOD_ERR_FORMAT;
    }


    v2m->v2mwaveformat.format       = FMOD_SOUND_FORMAT_PCMFLOAT;
    v2m->v2mwaveformat.channels     = 2;
    v2m->v2mwaveformat.frequency    = 44100;
    v2m->v2mwaveformat.pcmblocksize   = 4096;
    v2m->v2mwaveformat.lengthpcm    = 0xffffffff;

    //cout << "length:" << v2m->player->Length() << "\n";
    //flush(cout);
	
	codec->waveformat   = &v2m->v2mwaveformat;
	codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
	codec->plugindata   = v2m;                    /* user data value */

    info->fileformat = "Farbrausch V2M";
    info->setSeekable(false);
	info->plugin = "v2m";


    sS32* p;
    int pos = v2m->player->CalcPositions(&p);

//    for(int i = 0;i<=pos;i++)
//    {
//        if(i%2==0)
//        {
//            cout << p[i] << " ms\n";

//        }
//        else
//        {
//            cout << "pos: " << std::dec << i << ": " << setfill('0') << setw(8) << std::hex << p[i] <<  std::dec << "\n";
//        }

//    }
//    cout << "done\n";
//    flush(cout);
    int length;
    if(pos%2==0)
    {
        length=p[pos];
    }
    else
    {
        length=p[pos-1];
    }
    delete[] p;


    v2m->v2mwaveformat.lengthpcm    = ((1000+length)*2/1000.0)*v2m->v2mwaveformat.frequency; //add one extra second for reverb
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE *codec)
{
	v2mplugin* v2m = (v2mplugin*)codec->plugindata;

	if (v2m)
	{
        v2m->player->Stop();
        v2m->player->Close();
	}
    delete v2m;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
	v2mplugin* v2m = (v2mplugin*)codec->plugindata;

    v2m->player->Render((float*)buffer,v2m->v2mwaveformat.pcmblocksize);

    if(size<v2m->v2mwaveformat.pcmblocksize)
    {
        *read=size;
    }
    else
    {
        *read=v2m->v2mwaveformat.pcmblocksize;
    }

    return FMOD_OK;
}




FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{

    v2mplugin* v2m = (v2mplugin*)codec->plugindata;
    v2m->player->Play(position);
	return FMOD_OK;
}

