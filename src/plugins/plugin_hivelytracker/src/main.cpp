#include <stdio.h>
#include <string.h>
#include <string>
#include "fmod_errors.h"
#include "hvl_replay.h"
#include "info.h"
#include "BaseRow.h"
#include <iostream>
#include "plugins.h"

static int samples_left = 0;
FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        //exit(-1);
    }
}

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_hivelytracker_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_MUTE_VOICE | FMOD_TIMEUNIT_MODROW |
    FMOD_TIMEUNIT_MODPATTERN | FMOD_TIMEUNIT_MODORDER | FMOD_TIMEUNIT_MODPATTERN_INFO | FMOD_TIMEUNIT_MODVUMETER,
    // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    &getposition,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};
const char* NOTES[67] =
{
    "---", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
    "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
    "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "???"
};

class ahxplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    ahxplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("libmod.log");
        memset(&ahxwaveformat, 0, sizeof(ahxwaveformat));
    }

    ~ahxplugin()
    {
        //delete some stuff
        delete subsongslengths;
    }

    struct hvl_tune* m_tune;
    FMOD_CODEC_WAVEFORMAT ahxwaveformat;
    unsigned int* subsongslengths;
    Info* info;
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
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    ahxplugin* ahx = new ahxplugin(codec);
    ahx->info = (Info*)userexinfo->userdata;

    char id[4] = "";
    unsigned int bytesread;

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, id, 4, &bytesread);

    // HivelyTracker file
    if ((id[0] == 'H') && (id[1] == 'V') && (id[2] == 'L') && (id[3] < 2))
    {
        ahx->info->fileformat = "HivelyTracker";
    }
    else if ((id[0] == 'T') && (id[1] == 'H') && (id[2] == 'X') && (id[3] < 3))
    {
        ahx->info->fileformat = "AHX";
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }


    hvl_InitReplayer();


    //ahx->m_tune = hvl_LoadTune(const_cast<char*>(ahx->info->filename.c_str()), 44100, 1);

    FMOD_RESULT result;

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    unsigned char* myBuffer;
    myBuffer = new unsigned char[filesize];

    //rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    ERRCHECK(result);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);
    ERRCHECK(result);

    ahx->m_tune = hvl_ParseTune(myBuffer, filesize, 44100, 1);

    if (!ahx->m_tune)
    {
        delete myBuffer;
        return FMOD_ERR_FORMAT;
    }

    ahx->ahxwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    ahx->ahxwaveformat.channels = 2;
    ahx->ahxwaveformat.frequency = 44100;
    ahx->ahxwaveformat.pcmblocksize = 882; //(16 >> 3) * ahx->ahxwaveformat.channels;


    codec->waveformat = &ahx->ahxwaveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = ahx; /* user data value */


    int subsongs = ahx->m_tune->ht_SubsongNr;
    if (subsongs == 0)
    {
        subsongs = 1;
    }
    ahx->subsongslengths = new unsigned int[subsongs];
    for (int i = 0; i < subsongs; i++)
    {
        ahx->subsongslengths[i] = hvl_GetLen(ahx->m_tune) / 1000.0 * 44100;
    }

    hvl_InitSubsong(ahx->m_tune, 0);


    ahx->ahxwaveformat.lengthpcm = ahx->subsongslengths[0];
    ahx->info->title = ahx->m_tune->ht_Name;
    ahx->info->numChannels = ahx->m_tune->ht_Channels;
    ahx->info->numPatterns = ahx->m_tune->ht_TrackNr;
    ahx->info->modPatternRows = ahx->m_tune->ht_TrackLength;

    int numInstruments = ahx->m_tune->ht_InstrumentNr;
    ahx->info->numInstruments = numInstruments;
    ahx->info->numSubsongs = ahx->m_tune->ht_SubsongNr;
    ahx->info->numOrders = ahx->m_tune->ht_PositionNr;
    ahx->info->modPatternRestart = ahx->m_tune->ht_Restart;

    if (numInstruments > 0)
    {
        const unsigned char WAVELENGTH[6] =
        {
            4, 8, 10, 20, 40, 80
        };
        ahx->info->instruments = new string[numInstruments];
        ahx->info->instrumentsVolume = new unsigned char[numInstruments];
        ahx->info->instrumentsWavelen = new unsigned char[numInstruments];


        //        ahx->info->instrumentsFilterLowerLimit = new unsigned char[numInstruments];
        //        ahx->info->instrumentsFilterUpperLimit = new unsigned char[numInstruments];
        //        ahx->info->instrumentsFilterSpeed = new unsigned char[numInstruments];

        for (int j = 1; j <= numInstruments; j++)
        {
            ahx->info->instruments[j - 1] = ahx->m_tune->ht_Instruments[j].ins_Name;
            ahx->info->instrumentsVolume[j - 1] = ahx->m_tune->ht_Instruments[j].ins_Volume;
            ahx->info->instrumentsWavelen[j - 1] = WAVELENGTH[ahx->m_tune->ht_Instruments[j].ins_WaveLength];

            //            ahx->info->instrumentsFilterLowerLimit[j-1] = ahx->m_tune->ht_Instruments[j].ins_FilterLowerLimit;
            //            ahx->info->instrumentsFilterUpperLimit[j-1] = ahx->m_tune->ht_Instruments[j].ins_FilterUpperLimit;
            //            ahx->info->instrumentsFilterSpeed[j-1] = ahx->m_tune->ht_Instruments[j].ins_FilterSpeed;
        }
    }

    delete myBuffer;
    ahx->info->plugin = PLUGIN_hivelytracker;
    ahx->info->pluginName = PLUGIN_hivelytracker_NAME;
    ahx->info->setSeekable(true);


    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (ahx)
    {
        hvl_FreeTune(ahx->m_tune);
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    hvl_DecodeFrame(ahx->m_tune, (int8*)buffer, (int8*)buffer + 2, 4);
    if (size < ahx->ahxwaveformat.pcmblocksize)
    {
        *read = size;
    }
    else
    {
        *read = ahx->ahxwaveformat.pcmblocksize;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //hvl_GetLen can't be called during playing, so loop through all subsongs at load and getlength for
        //all songs and store them and return the current one here
        *length = ahx->subsongslengths[ahx->m_tune->ht_SongNum];

        return FMOD_OK;
    }

    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = ahx->m_tune->ht_SubsongNr;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MS)
    {
        hvl_Seek(ahx->m_tune, position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        hvl_InitSubsong(ahx->m_tune, position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < ahx->info->numChannels; i++)
        {
            int m = position >> i & 1;
            int mute = m == 0 ? 1 : 0;
            ahx->m_tune->ht_Voices[i].vc_TrackOn = mute;
        }
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALLBACK getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    ahxplugin* ahx = (ahxplugin*)codec->plugindata;

    if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = Front(ahx->m_tune->trackPosBuffer);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        *position = Front(ahx->m_tune->patternPosBuffer);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        //set the current track positions in the info struct and just return 0
        ahx->info->modTrackPositions.clear();
        for (int i = 0; i < ahx->m_tune->ht_Channels; i++)
        {
            ahx->info->modTrackPositions.push_back(
                ahx->m_tune->ht_Positions[Front(ahx->m_tune->patternPosBuffer)].pos_Track[i]);
        }
        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0
        for (vector<BaseRow*>::iterator itr = ahx->info->modRows.begin(); itr != ahx->info->modRows.end(); ++itr)
        {
            delete (*itr);
        }
        ahx->info->modRows.clear();
        for (int i = 0; i <= ahx->m_tune->ht_TrackNr; i++)
        {
            for (int j = 0; j < ahx->m_tune->ht_TrackLength; j++)
            {
                BaseRow* row = new BaseRow();
                uint8 note = ahx->m_tune->ht_Tracks[i][j].stp_Note;
                row->note = note;
                row->noteText = note == 0 ? "---" : NOTES[ahx->m_tune->ht_Tracks[i][j].stp_Note - 1];
                row->sample = ahx->m_tune->ht_Tracks[i][j].stp_Instrument;
                row->effect = ahx->m_tune->ht_Tracks[i][j].stp_FX;
                row->param = ahx->m_tune->ht_Tracks[i][j].stp_FXParam;
                row->effect2 = ahx->m_tune->ht_Tracks[i][j].stp_FXb;
                row->param2 = ahx->m_tune->ht_Tracks[i][j].stp_FXbParam;
                ahx->info->modRows.push_back(row);
            }
        }

        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        unsigned char* vumeters = new unsigned char[ahx->info->numChannels];
        hvl_GetChannelVolumes(ahx->m_tune, vumeters);

        ahx->info->modVUMeters = vumeters;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
