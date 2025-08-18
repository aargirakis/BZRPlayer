#include <cstdio>
#include <cstring>
#include <string>
#include "fmod_errors.h"
#include "hvl_replay.h"
#include "info.h"
#include "BaseRow.h"
#include <fstream>
#include "plugins.h"

static int samples_left = 0;
FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype);

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
    nullptr // Sound create callback (don't need it)
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

class pluginHivelyTracker
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginHivelyTracker(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("libmod.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginHivelyTracker()
    {
        //delete some stuff
        delete subsongslengths;
    }

    struct hvl_tune* m_tune;
    FMOD_CODEC_WAVEFORMAT waveformat;
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

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    auto* plugin = new pluginHivelyTracker(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    char id[4] = "";
    unsigned int bytesread;

    FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    FMOD_CODEC_FILE_READ(codec, id, 4, &bytesread);

    // HivelyTracker file
    if ((id[0] == 'H') && (id[1] == 'V') && (id[2] == 'L') && (id[3] < 2))
    {
        plugin->info->fileformat = "HivelyTracker";
    }
    else if ((id[0] == 'T') && (id[1] == 'H') && (id[2] == 'X') && (id[3] < 3))
    {
        plugin->info->fileformat = "AHX";
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }

    //read config from disk
    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/hivelytracker.cfg";
    ifstream ifs(filename.c_str());
    string line;
    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    uint32 defstereo = 4;
    plugin->info->isContinuousPlaybackActive = false;

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
                    defstereo = atoi(value.c_str());
                }
                else if (word.compare("continuous_playback") == 0)
                {
                    plugin->info->isContinuousPlaybackActive = plugin->info->isPlayModeRepeatSongEnabled && value.compare(
                        "true") == 0;
                }
            }
        }
        ifs.close();
    }

    hvl_InitReplayer();


    //plugin->m_tune = hvl_LoadTune(const_cast<char*>(plugin->info->filename.c_str()), 44100, 1);

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

    plugin->m_tune = hvl_ParseTune(myBuffer, filesize, 44100, defstereo);

    if (!plugin->m_tune)
    {
        delete myBuffer;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 882; //(16 >> 3) * plugin->waveformat.channels;


    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */


    int subsongs = plugin->m_tune->ht_SubsongNr;
    if (subsongs == 0)
    {
        subsongs = 1;
    }
    plugin->subsongslengths = new unsigned int[subsongs];
    for (int i = 0; i < subsongs; i++)
    {
        hvl_InitSubsong(plugin->m_tune, i);
        plugin->subsongslengths[i] = hvl_GetLen(plugin->m_tune);
    }

    plugin->waveformat.lengthpcm = -1;
    plugin->info->title = plugin->m_tune->ht_Name;
    plugin->info->numChannels = plugin->m_tune->ht_Channels;
    plugin->info->numPatterns = plugin->m_tune->ht_TrackNr;
    plugin->info->modPatternRows = plugin->m_tune->ht_TrackLength;

    int numInstruments = plugin->m_tune->ht_InstrumentNr;
    plugin->info->numInstruments = numInstruments;
    plugin->info->numSubsongs = plugin->m_tune->ht_SubsongNr;
    plugin->info->numOrders = plugin->m_tune->ht_PositionNr;
    plugin->info->modPatternRestart = plugin->m_tune->ht_Restart;

    if (numInstruments > 0)
    {
        const unsigned char WAVELENGTH[6] =
        {
            4, 8, 10, 20, 40, 80
        };
        plugin->info->instruments = new string[numInstruments];
        plugin->info->instrumentsVolume = new unsigned char[numInstruments];
        plugin->info->instrumentsWavelen = new unsigned char[numInstruments];


        //        plugin->info->instrumentsFilterLowerLimit = new unsigned char[numInstruments];
        //        plugin->info->instrumentsFilterUpperLimit = new unsigned char[numInstruments];
        //        plugin->info->instrumentsFilterSpeed = new unsigned char[numInstruments];

        for (int j = 1; j <= numInstruments; j++)
        {
            plugin->info->instruments[j - 1] = plugin->m_tune->ht_Instruments[j].ins_Name;
            plugin->info->instrumentsVolume[j - 1] = plugin->m_tune->ht_Instruments[j].ins_Volume;
            plugin->info->instrumentsWavelen[j - 1] = WAVELENGTH[plugin->m_tune->ht_Instruments[j].ins_WaveLength];

            //            plugin->info->instrumentsFilterLowerLimit[j-1] = plugin->m_tune->ht_Instruments[j].ins_FilterLowerLimit;
            //            plugin->info->instrumentsFilterUpperLimit[j-1] = plugin->m_tune->ht_Instruments[j].ins_FilterUpperLimit;
            //            plugin->info->instrumentsFilterSpeed[j-1] = plugin->m_tune->ht_Instruments[j].ins_FilterSpeed;
        }
    }

    delete myBuffer;
    plugin->info->plugin = PLUGIN_hivelytracker;
    plugin->info->pluginName = PLUGIN_hivelytracker_NAME;
    plugin->info->setSeekable(true);


    return FMOD_OK;
}

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    auto* plugin = static_cast<pluginHivelyTracker*>(codec->plugindata);
    if (plugin)
    {
        hvl_FreeTune(plugin->m_tune);
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginHivelyTracker*>(codec->plugindata);
    hvl_DecodeFrame(plugin->m_tune, (int8*)buffer, (int8*)buffer + 2, 4);
    if (size < plugin->waveformat.pcmblocksize)
    {
        *read = size;
    }
    else
    {
        *read = plugin->waveformat.pcmblocksize;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginHivelyTracker*>(codec->plugindata);
    if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS || lengthtype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //hvl_GetLen can't be called during playing, so loop through all subsongs at load and getlength for
        //all songs and store them and return the current one here
        *length = plugin->subsongslengths[plugin->m_tune->ht_SongNum];

        return FMOD_OK;
    }

    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->m_tune->ht_SubsongNr;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_OK;
}


FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto plugin = static_cast<pluginHivelyTracker*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS)
    {
        hvl_Seek(plugin->m_tune, position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        hvl_InitSubsong(plugin->m_tune, position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MUTE_VOICE)
    {
        //position is a mask
        for (int i = 0; i < plugin->info->numChannels; i++)
        {
            int m = position >> i & 1;
            int mute = m == 0 ? 1 : 0;
            plugin->m_tune->ht_Voices[i].vc_TrackOn = mute;
        }
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

FMOD_RESULT F_CALL getposition(FMOD_CODEC_STATE* codec, unsigned int* position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginHivelyTracker*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MODROW)
    {
        *position = Front(plugin->m_tune->trackPosBuffer);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODORDER)
    {
        *position = Front(plugin->m_tune->patternPosBuffer);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN)
    {
        //set the current track positions in the info struct and just return 0
        plugin->info->modTrackPositions.clear();
        for (int i = 0; i < plugin->m_tune->ht_Channels; i++)
        {
            plugin->info->modTrackPositions.push_back(
                plugin->m_tune->ht_Positions[Front(plugin->m_tune->patternPosBuffer)].pos_Track[i]);
        }
        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODPATTERN_INFO)
    {
        //set the mod pattern (notes etc.) in the info struct and just return 0
        for (vector<BaseRow*>::iterator itr = plugin->info->modRows.begin(); itr != plugin->info->modRows.end(); ++itr)
        {
            delete (*itr);
        }
        plugin->info->modRows.clear();
        for (int i = 0; i <= plugin->m_tune->ht_TrackNr; i++)
        {
            for (int j = 0; j < plugin->m_tune->ht_TrackLength; j++)
            {
                auto* row = new BaseRow();
                uint8 note = plugin->m_tune->ht_Tracks[i][j].stp_Note;
                row->note = note;
                row->noteText = note == 0 ? "---" : NOTES[plugin->m_tune->ht_Tracks[i][j].stp_Note - 1];
                row->sample = plugin->m_tune->ht_Tracks[i][j].stp_Instrument;
                row->effect = plugin->m_tune->ht_Tracks[i][j].stp_FX;
                row->param = plugin->m_tune->ht_Tracks[i][j].stp_FXParam;
                row->effect2 = plugin->m_tune->ht_Tracks[i][j].stp_FXb;
                row->param2 = plugin->m_tune->ht_Tracks[i][j].stp_FXbParam;
                plugin->info->modRows.push_back(row);
            }
        }

        *position = 0;
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_MODVUMETER)
    {
        unsigned char* vumeters = new unsigned char[plugin->info->numChannels];
        hvl_GetChannelVolumes(plugin->m_tune, vumeters);

        plugin->info->modVUMeters = vumeters;
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
}
