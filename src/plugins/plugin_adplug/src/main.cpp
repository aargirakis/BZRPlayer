#include "fmod_errors.h"
#include <adplug.h>
#include <emuopl.h>
#include <kemuopl.h>
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

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_adplug_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_SUBSONG, // The time format we would like to accept into setposition/getposition.
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
    FMOD_CODEC_WAVEFORMAT waveformat;
    unsigned long samplesToWrite = 0;
    const int fixedBufferSize = 256;
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
    auto* plugin = new pluginAdplug(codec);
    Info* info = static_cast<Info*>(userexinfo->userdata);

    plugin->opl = new CKemuopl(44100, true, false);
    if (!plugin->opl)
    {
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }

    cout << "adplug trying to load: " << info->filename << "\n";
    plugin->player = CAdPlug::factory(info->filename, plugin->opl);
    if (!plugin->player)
    {
        delete plugin->player;
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }


    //read config from disk


    string filename = info->applicationPath + USER_PLUGINS_CONFIG_DIR + "/adplug.cfg";
    ifstream ifs(filename.c_str());
    string line;

    bool useDefaults = false;
    if (ifs.fail())
    {
        //The file could not be opened
        useDefaults = true;
    }

    int emulator = 0;
    int freq = 44100;
    bool stereo = true;
    plugin->waveformat.channels = 1;

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
                        stereo = true;
                        plugin->waveformat.channels = 2;
                    }
                    else
                    {
                        stereo = false;
                        plugin->waveformat.channels = 2;
                    }
                }
                else if (word.compare("emulator") == 0)
                {
                    emulator = atoi(value.c_str());
                }
            }
        }
        ifs.close();
    }

    //we have to create a new engine AGAIN with the new settings
    delete plugin->opl;
    delete plugin->player;
    //stereo plays twice as fast.........................???
    if (emulator == 2)
    {
        plugin->opl = new CKemuopl(freq, true, false);
    }
    else
    {
        plugin->opl = new CEmuopl(freq, true, false);
    }
    if (!plugin->opl)
    {
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }

    plugin->player = CAdPlug::factory(info->filename, plugin->opl);
    if (!plugin->player)
    {
        delete plugin->player;
        delete plugin->opl;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = plugin->player->songlength() / 1000 * plugin->waveformat.frequency;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds.
    codec->plugindata = plugin; // user data value

    info->fileformat = plugin->player->gettype();
    info->artist = plugin->player->getauthor();
    info->title = plugin->player->gettitle();
    info->comments = plugin->player->getdesc();
    info->numInstruments = plugin->player->getinstruments();
    info->numPatterns = plugin->player->getpatterns();
    info->numOrders = plugin->player->getorders();

    info->instruments = new string[info->numInstruments];
    for (int j = 0; j < info->numInstruments; j++)
    {
        info->instruments[j] = plugin->player->getinstrument(j);
    }
    info->plugin = PLUGIN_adplug;
    info->pluginName = PLUGIN_adplug_NAME;
    info->setSeekable(true);

    cout << "adlib ok\n";
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginAdplug*>(codec->plugindata);
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);

    while (plugin->samplesToWrite < plugin->fixedBufferSize)
    {
        plugin->player->update();
        plugin->samplesToWrite += 1 * plugin->waveformat.frequency / plugin->player->getrefresh();
    }

    plugin->opl->update(static_cast<short*>(buffer), plugin->fixedBufferSize);

    *read = plugin->fixedBufferSize;

    plugin->samplesToWrite -= plugin->fixedBufferSize;

    return FMOD_OK;

    //    unsigned int numSamples = size;
    //    auto maxSamples = numSamples;

    ////            if (m_hasEnded)
    ////            {
    ////                m_hasEnded = false;
    ////                return 0;
    ////            }

    //            auto remainingSamples = plugin->m_remainingSamples;
    //            while (numSamples > 0)
    //            {
    //                if (remainingSamples > 0)
    //                {
    //                    auto samplesToAdd = min(numSamples, remainingSamples);

    //                    //auto buf = reinterpret_cast<int16_t*>(output + samplesToAdd) - samplesToAdd * 2;
    //                    plugin->opl->update((short*)buffer, samplesToAdd);

    //                    remainingSamples -= samplesToAdd;
    //                    numSamples -= samplesToAdd;

    //                    //output = output->Convert(buf, samplesToAdd);
    //                }
    //                else if (plugin->player->update())
    //                {
    //                    remainingSamples = static_cast<uint32_t>(plugin->waveformat.frequency / plugin->player->getrefresh());
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
    //            plugin->m_remainingSamples = remainingSamples;

    //            *read = maxSamples - numSamples;
    //return FMOD_OK;
}

FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);

    //printf("lengthtype: %i ",lengthtype);

    if (lengthtype == FMOD_TIMEUNIT_MS)
    {
        *length = plugin->waveformat.lengthpcm;
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG)
    {
        *length = plugin->player->getsubsongs();
        return FMOD_OK;
    }
    else if (lengthtype == FMOD_TIMEUNIT_SUBSONG_MS)
    {
        *length = plugin->player->songlength();
        return FMOD_OK;
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginAdplug*>(codec->plugindata);
    if (postype == FMOD_TIMEUNIT_MS)
    {
        plugin->samplesToWrite = 0;
        plugin->player->seek(position);
        return FMOD_OK;
    }
    else if (postype == FMOD_TIMEUNIT_SUBSONG)
    {
        if (position < 0) position = 0;
        plugin->player->rewind(position);
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_FORMAT;
    }
}
