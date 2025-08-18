#include "fmod_errors.h"
#include <adplug.h>
#include <emuopl.h>
#include <kemuopl.h>
#include <nemuopl.h>
#include <wemuopl.h>
#include <surroundopl.h>
#include <fstream>
#include <cstring>
#include "info.h"
#include "plugins.h"

using namespace std;

//CLogFile *LogFile;

FMOD_RESULT handle_error(const char* str)
{
    if (str)
    {
        //CLogFile::getInstance()->Print( "Error: %s\n", str );
        return FMOD_ERR_INTERNAL;
    }
    else
        return FMOD_OK;
}

FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_adplug_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG | FMOD_TIMEUNIT_SUBSONG_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getlength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};


class pluginAdplug
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginAdplug(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginAdplug()
    {
        //delete some stuff
        delete player;
        delete opl;
    }

    CPlayer* player;
    Copl* opl;
    Info* info;
    FMOD_CODEC_WAVEFORMAT waveformat;
    unsigned long samplesToWrite = 0;
    unsigned int currentSubsong = -1;
    unsigned int songlength = -1;
    bool isSongEndReached = false;
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
    auto* plugin = new pluginAdplug(codec);
    plugin->info = static_cast<Info*>(userexinfo->userdata);

    //read config from disk
    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/adplug.cfg";
    ifstream ifs(filename.c_str());
    string line;

    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    int emulator = 2;
    int freq = 44100;
    int bits = 16; //TODO didn't added to preferences yet since 8bit still sounds too loud (distorted)
    plugin->waveformat.channels = 2;
    bool harmonic = true;
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults)
    {
        while (getline(ifs, line))
        {
            int i = line.find_first_of('=');

            if (i != -1)
            {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word.compare("emulator") == 0)
                {
                    emulator = atoi(value.c_str());
                }
                else if (word.compare("frequency") == 0)
                {
                    freq = atoi(value.c_str());
                }
                else if (word.compare("playback") == 0)
                {
                    int playback = atoi(value.c_str());
                    if (playback == 0) // mono
                    {
                        plugin->waveformat.channels = 1;
                        harmonic = false;
                    }
                    else if (playback == 1) // stereo
                    {
                        plugin->waveformat.channels = 2;
                        harmonic = false;
                    }
                    else if (playback == 2) // surround (default)
                    {
                        plugin->waveformat.channels = 2;
                        harmonic = true;
                    }
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

    switch (emulator)
    {
        case 0: // Tatsuyuki Satoh (left channel only when stereo)
        if (harmonic)
        {
            COPLprops a = {};
            COPLprops b = {};
            a.use16bit = b.use16bit = bits == 16;
            a.stereo = b.stereo = false;
            a.opl = new CEmuopl(freq, a.use16bit, a.stereo);
            b.opl = new CEmuopl(freq, b.use16bit, b.stereo);
            plugin->opl = new CSurroundopl(&a, &b, bits == 16);
        }
        else plugin->opl = new CEmuopl(freq, bits == 16, plugin->waveformat.channels == 2);
        break;

    case 1:
        // Ken Silverman (only supports one instance so does not work properly in surround mode in old versions of the adplug library)
        if (harmonic)
        {
            COPLprops a = {};
            COPLprops b = {};
            a.use16bit = b.use16bit = bits == 16;
            a.stereo = b.stereo = false;
            a.opl = new CKemuopl(freq, a.use16bit, a.stereo);
            b.opl = new CKemuopl(freq, b.use16bit, b.stereo);
            plugin->opl = new CSurroundopl(&a, &b, bits == 16);
        }
        else plugin->opl = new CKemuopl(freq, bits == 16, plugin->waveformat.channels == 2);
        break;

    case 2: // Woody (DOSBox)
    default:
        if (harmonic)
        {
            COPLprops a = {};
            COPLprops b = {};
            a.use16bit = b.use16bit = bits == 16;
            a.stereo = b.stereo = false;
            a.opl = new CWemuopl(freq, a.use16bit, a.stereo);
            b.opl = new CWemuopl(freq, b.use16bit, b.stereo);
            plugin->opl = new CSurroundopl(&a, &b, bits == 16);
        }
        else plugin->opl = new CWemuopl(freq, bits == 16, plugin->waveformat.channels == 2);
        break;

    case 3: // Nuked OPL3 (only works in stereo 16 bits)
        if (harmonic)
        {
            COPLprops a = {};
            COPLprops b = {};
            a.use16bit = b.use16bit = true; // Nuked only supports 16-bit
            a.stereo = b.stereo = true; // Nuked only supports stereo
            a.opl = new CNemuopl(freq);
            b.opl = new CNemuopl(freq);
            plugin->opl = new CSurroundopl(&a, &b, bits == 16); // SurroundOPL can convert to 8-bit though
        }
        else plugin->opl = new CNemuopl(freq);
        plugin->waveformat.channels = 2;
        bits = 16;
        break;
    }

    if (!plugin->opl)
    {
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }

    plugin->player = CAdPlug::factory(plugin->info->filename, plugin->opl);
    if (!plugin->player)
    {
        delete plugin->player;
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = bits == 16? FMOD_SOUND_FORMAT_PCM16 : FMOD_SOUND_FORMAT_PCM8;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (bits >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds.
    codec->plugindata = plugin; // user data value

    plugin->info->fileformat = plugin->player->gettype();
    plugin->info->artist = plugin->player->getauthor();
    plugin->info->title = plugin->player->gettitle();
    plugin->info->comments = plugin->player->getdesc();
    plugin->info->numInstruments = plugin->player->getinstruments();
    plugin->info->numPatterns = plugin->player->getpatterns();
    plugin->info->numOrders = plugin->player->getorders();

    plugin->info->instruments = new string[plugin->info->numInstruments];
    for (int j = 0; j < plugin->info->numInstruments; j++)
    {
        plugin->info->instruments[j] = plugin->player->getinstrument(j);
    }

    plugin->info->plugin = PLUGIN_adplug;
    plugin->info->pluginName = PLUGIN_adplug_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginAdplug*>(codec->plugindata);
    return FMOD_OK;
}

FMOD_RESULT F_CALL read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);

    if (plugin->currentSubsong != -1 && plugin->songlength == -1)
    {
        // songlength function does a rewind internally: call it only before start playing
        plugin->songlength = static_cast<unsigned int>(plugin->player->songlength(static_cast<int>(plugin->currentSubsong)));
    }

    if (plugin->isSongEndReached && plugin->samplesToWrite == 0 && !plugin->info->isContinuousPlaybackActive)
    {
        return FMOD_ERR_FILE_EOF;
    }

    while (plugin->samplesToWrite < plugin->waveformat.pcmblocksize)
    {
        plugin->isSongEndReached = !plugin->player->update();
        plugin->samplesToWrite += static_cast<unsigned long>(plugin->waveformat.frequency / plugin->player->getrefresh());
    }

    plugin->opl->update(static_cast<short*>(buffer), static_cast<int>(plugin->waveformat.pcmblocksize));
    *read = plugin->waveformat.pcmblocksize;
    plugin->samplesToWrite -= plugin->waveformat.pcmblocksize;

    return FMOD_OK;
}

FMOD_RESULT F_CALL getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->player->getsubsongs();
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = plugin->songlength;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALL setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        plugin->samplesToWrite = 0;
        plugin->player->seek(position);
        return FMOD_OK;
    }

    if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        plugin->currentSubsong = position;
        return FMOD_OK;
    }

    return FMOD_ERR_FORMAT;
}
