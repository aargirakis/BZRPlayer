#include "soundmanager.h"

#include <mainwindow.h>

#include "qdatetime.h"
#include "qdir.h"
#include <QFile>
#include <QDebug>
#include "plugins.h"

void SoundManager::Init(int device, QString outputfilename)
{
    result = FMOD_System_Create(&system, FMOD_VERSION);
    ERRCHECK(result);

    result = FMOD_Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_FILE, 0, "fmodlog.txt");
    ERRCHECK(result);
    unsigned int version;
    result = FMOD_System_GetVersion(system, &version, nullptr);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version,
               FMOD_VERSION);
    }

    currentDevice = device;
    printf("Setting ouput to: %i\n", currentDevice);
    FMOD_OUTPUTTYPE outputtype = static_cast<FMOD_OUTPUTTYPE>(device);
    result = FMOD_System_SetOutput(system, outputtype);
    ERRCHECK(result);

    if (outputtype != FMOD_OUTPUTTYPE_WAVWRITER)
    {
        printf("FMOD_System_Init: %i\n", currentDevice);
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, nullptr);
        ERRCHECK(result);
    }
    else
    {
        QString outputPath = outputfilename;

        QDir pathDir(userPath + "/recordings");
        if (!pathDir.exists())
        {
            QDir().mkdir(userPath + "/recordings");
        }

        outputPath = userPath + "/recordings/" + outputfilename;
        cout << "filename: " << outputPath.toStdString().c_str() << "\n";


        QDateTime date = QDateTime::currentDateTime();
        QString formattedTime = date.toString("yyyy.MM.dd - hh.mm.ss.ms");
        QString outputPathFile = outputPath + " " + formattedTime + ".wav";

        printf("FMOD_System_Init: WAVWRITER %i\n", currentDevice);

        cout << "filename complete: " << outputPathFile.toStdString().c_str() << "\n";
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, (char*)(outputPathFile.toStdString().c_str()));
        ERRCHECK(result);
        printf("WAVWRITER is set\n");
    }


    cout << "FMOD_System_CreateChannelGroup(system,"",&channelgroup)\n";
    FMOD_System_CreateChannelGroup(system, "", &channelgroup);
    ERRCHECK(result);

    cout << "FMOD_System_CreateDSPByType(system,FMOD_DSP_TYPE_NORMALIZE, &dspNormalizer)\n";
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_NORMALIZE, &dspNormalizer);
    ERRCHECK(result);

    cout << "FMOD_System_CreateDSPByType(system,FMOD.DSP_TYPE.FFT, &dspFFT)\n";
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FFT, &dspFFT);
    ERRCHECK(result);
    cout << "FMOD_DSP_SetActive(dspFFT,true)\n";
    FMOD_DSP_SetActive(dspFFT, true);
    ERRCHECK(result);
    cout << "FMOD_DSP_SetParameterInt(dspFFT,FMOD_DSP_FFT_WINDOW_HANNING,1024*2\n";
    result = FMOD_DSP_SetParameterInt(dspFFT, FMOD_DSP_FFT_WINDOW_HANNING, 16 * 2);
    ERRCHECK(result);
    cout << "FMOD_ChannelGroup_AddDSP(channelgroup,FMOD_CHANNELCONTROL_DSP_HEAD,dspFFT)\n";
    FMOD_ChannelGroup_AddDSP(channelgroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspFFT);
    ERRCHECK(result);

    cout << "FMOD_ChannelGroup_AddDSP(channelgroup,FMOD_CHANNELCONTROL_DSP_HEAD,dspNormalizer))\n";
    FMOD_ChannelGroup_AddDSP(channelgroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspNormalizer);
    ERRCHECK(result);

    cout << "FMOD_DSP_SetActive(dspNormalizer,true)\n";
    FMOD_DSP_SetActive(dspNormalizer, true);
    ERRCHECK(result);
}

void SoundManager::loadPluginChain() {
    if (PLUGIN_klystron_LIB != "")
    {
        loadPlugin(PLUGIN_klystron_LIB, 99999);
    }
    if (PLUGIN_libsidplayfp_LIB != "")
    {
        loadPlugin(PLUGIN_libsidplayfp_LIB, 0);
    }
    if (PLUGIN_libopenmpt_LIB != "")
    {
        loadPlugin(PLUGIN_libopenmpt_LIB, 0);
    }
    if (PLUGIN_highly_experimental_LIB != "")
    {
        loadPlugin(PLUGIN_highly_experimental_LIB, 1);
    }
    if (PLUGIN_highly_theoretical_LIB != "")
    {
        loadPlugin(PLUGIN_highly_theoretical_LIB, 1);
    }
    if (PLUGIN_lazyusf2_LIB != "")
    {
        loadPlugin(PLUGIN_lazyusf2_LIB, 1);
    }
    if (PLUGIN_highly_quixotic_LIB != "")
    {
        loadPlugin(PLUGIN_highly_quixotic_LIB, 1);
    }
    if (PLUGIN_vio2sf_LIB != "")
    {
        loadPlugin(PLUGIN_vio2sf_LIB, 1);
    }
    if (PLUGIN_protrekkr_LIB != "")
    {
        loadPlugin(PLUGIN_protrekkr_LIB, 1);
    }
    if (PLUGIN_hivelytracker_LIB != "")
    {
        loadPlugin(PLUGIN_hivelytracker_LIB, 1);
    }
    if (PLUGIN_libstsound_LIB != "")
    {
        loadPlugin(PLUGIN_libstsound_LIB, 1);
    }
    if (PLUGIN_flod_LIB != "")
    {
        loadPlugin(PLUGIN_flod_LIB, 1);
    }
    if (PLUGIN_sndh_player_LIB != "")
    {
        loadPlugin(PLUGIN_sndh_player_LIB, 1);
    }
    if (PLUGIN_furnace_LIB != "")
    {
        loadPlugin(PLUGIN_furnace_LIB, 1);
    }
    if (PLUGIN_uade_LIB != "")
    {
        loadPlugin(PLUGIN_uade_LIB, 1);
    }

    //loadPlugin("plugin_quartet.dll",1);

    if (PLUGIN_faad2_LIB != "")
    {
        loadPlugin(PLUGIN_faad2_LIB, 99999);
    }

    if (PLUGIN_zxtune_LIB != "")
    {
        loadPlugin(PLUGIN_zxtune_LIB, 99999);
    }

    if (PLUGIN_vgmplay_legacy_LIB != "")
    {
        loadPlugin(PLUGIN_vgmplay_legacy_LIB, 99999);
    }
    if (PLUGIN_libkss_LIB != "")
    {
        loadPlugin(PLUGIN_libkss_LIB, 99999);
    }
    if (PLUGIN_game_music_emu_LIB != "")
    {
        loadPlugin(PLUGIN_game_music_emu_LIB, 99999);
    }
    if (PLUGIN_asap_LIB != "")
    {
        loadPlugin(PLUGIN_asap_LIB, 99999);
    }

    if (PLUGIN_organya_decoder_LIB != "")
    {
        loadPlugin(PLUGIN_organya_decoder_LIB, 99999);
    }
    if (PLUGIN_sunvox_lib_LIB != "")
    {
        loadPlugin(PLUGIN_sunvox_lib_LIB, 99999);
    }
    if (PLUGIN_audiofile_LIB != "")
    {
        loadPlugin(PLUGIN_audiofile_LIB, 99999);
    }
    if (PLUGIN_sc68_LIB != "")
    {
        loadPlugin(PLUGIN_sc68_LIB, 99999);
    }
    if (PLUGIN_tunes98_plug_LIB != "")
    {
        loadPlugin(PLUGIN_tunes98_plug_LIB, 99999);
    }
    if (PLUGIN_kdm_LIB != "")
    {
        loadPlugin(PLUGIN_kdm_LIB, 99999);
    }
    if (PLUGIN_libpac_LIB != "")
    {
        loadPlugin(PLUGIN_libpac_LIB, 99999);
    }

    if (PLUGIN_libxmp_LIB != "")
    {
        loadPlugin(PLUGIN_libxmp_LIB, 99999);
    }

    if (PLUGIN_mdxmini_LIB != "")
    {
        loadPlugin(PLUGIN_mdxmini_LIB, 99999);
    }

    if (PLUGIN_adplug_LIB != "")
    {
        loadPlugin(PLUGIN_adplug_LIB, 99999);
    }
    if (PLUGIN_vgmstream_LIB != "")
    {
        loadPlugin(PLUGIN_vgmstream_LIB, 99999);
    }
    if (PLUGIN_audiodecoder_wsr_LIB != "")
    {
        loadPlugin(PLUGIN_audiodecoder_wsr_LIB, 99999);
    }
    if (PLUGIN_v2m_player_LIB != "")
    {
        loadPlugin(PLUGIN_v2m_player_LIB, 99999);
    }
    if (PLUGIN_jaytrax_LIB != "")
    {
        loadPlugin(PLUGIN_jaytrax_LIB, 99999);
    }

    //loadPlugin("plugin_wavpack.dll",99999);
}

int SoundManager::getSoundData(int channel)
{
    FMOD_DSP_PARAMETER_FFT* fft = 0;

    result = FMOD_DSP_GetParameterData(dspFFT, FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fft, nullptr, nullptr, 0);

    float val = 0;
    //for (int channel = 0; channel < fft->numchannels; channel++)
    {
        for (int bin = 0; bin < fft->length / 2; bin++)
        {
            val += fft->spectrum[channel][bin];
        }
    }
    //Clipping is probably over 5 (guessing?)
    //So clamp values over 5 to 5
    if (val > 5)
    {
        val = 5;
    }

    return val / 5 * 100;


    //    cout << "numchannels" << data->numchannels << "\n";
    //    flush(cout);
    //    cout << "length" << data->length << "\n";
    //    flush(cout);


    //result = FMOD_System_GetSoftwareFormat(system,&rate , nullptr, nullptr);
    //    nyquist = windowsize / 2;

    //                for (chan = 0; chan < 2; chan++)
    //                {
    //                    float average = 0.0f;
    //                    float power = 0.0f;

    //                    for (int i = 0; i < nyquist-1; ++i)
    //                    {
    //                        float hz = i * (rate * 0.5f) / (nyquist-1);
    //                        int index = i + (16384 * chan);

    //                        if (data[index] > 0.0001f) // aritrary cutoff to filter out noise
    //                        {
    //                            average += data[index] * hz;
    //                            power += data[index];
    //                        }
    //                    }

    //                    if (power > 0.001f)
    //                    {
    //                        freq[chan] = average / power;
    //                    }
    //                    else
    //                    {
    //                        freq[chan] = 0;
    //                    }
    //                }
    //                printf("%.02f %.02f\n", (freq[0]/16384)*100, (freq[1]/16384)*100);

    //    //FMOD_DSP *dsp, int index, void **data, unsigned int *length, char *valuestr, int valuestrlen);
    //return (freq[channel]/16384)*100;
    return 50;
    ERRCHECK(result);
}

float SoundManager::getAudibility()
{
    float audibility;
    FMOD_Channel_GetVolume(channel, &audibility);
    return audibility;
}

void SoundManager::setReverbEnabled(bool enabled)
{
    if (enabled)
    {
        result = FMOD_System_SetReverbProperties(system, 0, &currentReverbPreset);
        ERRCHECK(result);
    }
    else
    {
        result = FMOD_System_SetReverbProperties(system, 0, nullptr);
        ERRCHECK(result);
    }
}

void SoundManager::setReverbPreset(QString preset)
{
    FMOD_REVERB_PROPERTIES prop;
    if (preset == "Generic")
    {
        prop = FMOD_PRESET_GENERIC;
    }
    else if (preset == "Padded cell")
    {
        prop = FMOD_PRESET_PADDEDCELL;
    }
    else if (preset == "Room")
    {
        prop = FMOD_PRESET_ROOM;
    }

    else if (preset == "Bathroom")
    {
        prop = FMOD_PRESET_BATHROOM;
    }
    else if (preset == "Living room")
    {
        prop = FMOD_PRESET_LIVINGROOM;
    }
    else if (preset == "Stone room")
    {
        prop = FMOD_PRESET_STONEROOM;
    }
    else if (preset == "Auditorium")
    {
        prop = FMOD_PRESET_AUDITORIUM;
    }
    else if (preset == "Concert hall")
    {
        prop = FMOD_PRESET_CONCERTHALL;
    }
    else if (preset == "Cave")
    {
        prop = FMOD_PRESET_CAVE;
    }
    else if (preset == "Arena")
    {
        prop = FMOD_PRESET_ARENA;
    }
    else if (preset == "Hangar")
    {
        prop = FMOD_PRESET_HANGAR;
    }
    else if (preset == "Carpeted hallway")
    {
        prop = FMOD_PRESET_CARPETTEDHALLWAY;
    }
    else if (preset == "Hallway")
    {
        prop = FMOD_PRESET_HALLWAY;
    }
    else if (preset == "Stone corridor")
    {
        prop = FMOD_PRESET_STONECORRIDOR;
    }
    else if (preset == "Alley")
    {
        prop = FMOD_PRESET_ALLEY;
    }
    else if (preset == "Forest")
    {
        prop = FMOD_PRESET_FOREST;
    }
    else if (preset == "City")
    {
        prop = FMOD_PRESET_CITY;
    }
    else if (preset == "Mountains")
    {
        prop = FMOD_PRESET_MOUNTAINS;
    }
    else if (preset == "Quarry")
    {
        prop = FMOD_PRESET_QUARRY;
    }
    else if (preset == "Plain")
    {
        prop = FMOD_PRESET_PLAIN;
    }
    else if (preset == "Parking lot")
    {
        prop = FMOD_PRESET_PARKINGLOT;
    }
    else if (preset == "Sewer pipe")
    {
        prop = FMOD_PRESET_SEWERPIPE;
    }
    else if (preset == "Underwater")
    {
        prop = FMOD_PRESET_UNDERWATER;
    }

    currentReverbPreset = prop;
    //DebugWindow::instance()->addText("Soundmanager, reverb preset: "+ preset);
}

void SoundManager::setNormalizeFadeTime(int param)
{
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_FADETIME, param);
}

void SoundManager::setNormalizeThreshold(int param)
{
    float paramF = param / 100.0;
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_THRESHOLD, paramF);
}

void SoundManager::setNormalizeMaxAmp(int param)
{
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_MAXAMP, param);
}

void SoundManager::setNormalizeEnabled(bool enabled)
{
    FMOD_DSP_SetBypass(dspNormalizer, !enabled);
    //DebugWindow::instance()->addText("setNormalizeEnabled " + QString::number(enabled));
}

FMOD_RESULT SoundManager::getTag(const char* name, int index, FMOD_TAG* tag)
{
    return FMOD_Sound_GetTag(sound, name, index, tag);
}

int SoundManager::getNumTags()
{
    int numTags;
    FMOD_Sound_GetNumTags(sound, &numTags, nullptr);
    return numTags;
}

void SoundManager::loadPlugin(string filename, int prority)
{
    string pluginsDir = QString(libPath + PLUGINS_DIR + "/").toStdString();
    const char* pluginPath = (pluginsDir += filename).c_str();

    result = FMOD_System_LoadPlugin(system, pluginPath, nullptr, prority);

    if (result != FMOD_OK)
    {
        //DebugWindow::instance()->addText(QString(filename.c_str()));
    }
    ERRCHECK(result, pluginPath);

    //DebugWindow::instance()->addText("GetNumPlugins " + QString::number(numplugins));
}

void SoundManager::ERRCHECK(FMOD_RESULT result)
{
    ERRCHECK(result, "");
}

void SoundManager::ERRCHECK(FMOD_RESULT result, QString extra)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
        if (extra == "")
        {
            printf("\n");
        }
        if (extra != "")
        {
            printf(" - %s\n", extra.toStdString().c_str());
        }
        flush(cout);
    }
}

void SoundManager::Stop()
{
    FMOD_Channel_Stop(channel);
    //    FMOD_OUTPUTTYPE outputtype = static_cast<FMOD_OUTPUTTYPE>(currentDevice);
    //    if(outputtype==FMOD_OUTPUTTYPE_WAVWRITER)
    //    {
    //        setOutput(0);
    //    }
}

bool SoundManager::IsPlaying()
{
    if (!channel) return false;
    FMOD_BOOL playing;
    FMOD_Channel_IsPlaying(channel, &playing);
    return playing;
}

bool SoundManager::isWavWriterDeviceSelected() {
    return static_cast<FMOD_OUTPUTTYPE>(currentDevice) == FMOD_OUTPUTTYPE_WAVWRITER;
}

void SoundManager::SetPosition(unsigned int positon, FMOD_TIMEUNIT timeunit)
{
    FMOD_Channel_SetPosition(channel, positon, timeunit);
}

void SoundManager::SetVolume(float vol)
{
    FMOD_Channel_SetVolume(channel, vol);
}

float SoundManager::GetNominalFrequency()
{
    return m_nominalFrequency;
}

void SoundManager::SetFrequencyByMultiplier(float percent)
{
    FMOD_Channel_SetFrequency(channel, percent * m_nominalFrequency);
}

void SoundManager::SetMute(bool mute)
{
    FMOD_Channel_SetMute(channel, mute);
}

unsigned int SoundManager::GetPosition(FMOD_TIMEUNIT timeunit)
{
    unsigned int currentMs;
    FMOD_Channel_GetPosition(channel, &currentMs, timeunit);
    return currentMs;
}

unsigned int SoundManager::GetLength(FMOD_TIMEUNIT timeunit)
{
    unsigned int song_length_ms;
    //DebugWindow::instance()->addText("SoundManager: GetLength, Timeunit: " + QString::number(timeunit));
    FMOD_Sound_GetLength(sound, &song_length_ms, timeunit);
    return song_length_ms;
}

void SoundManager::Pause(bool pause)
{
    FMOD_Channel_SetPaused(channel, pause);
}

bool SoundManager::GetPaused()
{
    FMOD_BOOL pause;
    FMOD_Channel_GetPaused(channel, &pause);
    return pause;
}

void SoundManager::PlayAudio(bool startPaused)
{
    m_mutedChannelsMask = 0;
    m_mutedChannelsMaskString = "";
    FMOD_System_PlaySound(system, sound, channelgroup, startPaused, &channel);
    FMOD_Channel_GetFrequency(channel, &m_nominalFrequency);
    //DebugWindow::instance()->addText("SoundManager: PlaySound");
}

void SoundManager::Release()
{
    FMOD_Sound_Release(sound);
}

void SoundManager::ShutDown()
{
    FMOD_Sound_Release(sound);
    FMOD_System_Release(system);
}

void SoundManager::MuteChannels(unsigned int mask, QString maskStr)
{
    m_Info1->mutedChannelsMask = maskStr.toStdString();

    FMOD_Channel_SetPosition(channel, mask, FMOD_TIMEUNIT_MUTE_VOICE);

    m_mutedChannelsMask = mask;
    m_mutedChannelsMaskString = maskStr;
}

bool SoundManager::isChannelMuted(unsigned int channel)
{
    bool muted = false;
    if (m_mutedChannelsMaskString != nullptr &&
        m_mutedChannelsMaskString.at(channel) == '0')
    {
        muted = true;
    }
    return muted;
}

bool SoundManager::LoadSound(QString filename, bool isPlayModeRepeatSongEnabled)
{
    Stop();
    Release();
    m_mutedChannelsMask = 0;
    m_mutedChannelsMaskString = "";
    m_Info1 = new Info();
    m_Info1->clearMemory();
    m_Info1->clear();
    m_Info1->tempPath = QDir::tempPath().toStdString();
    m_Info1->dataPath = dataPath.toStdString();
    m_Info1->libPath = libPath.toStdString();
    m_Info1->userPath = userPath.toStdString();
    m_Info1->filename = filename.toStdString();
    m_Info1->isPlayModeRepeatSongEnabled = isPlayModeRepeatSongEnabled;
    FMOD_CREATESOUNDEXINFO extrainfo1;
    memset(&extrainfo1, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    extrainfo1.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    string dls = dataPath.toStdString() + PLUGINS_FMOD_DIR + "/gm.dls";
    extrainfo1.dlsname = dls.c_str();
    extrainfo1.userdata = m_Info1;

    cout << "m_Info1->filename: " << m_Info1->filename << "\n";
    //DebugWindow::instance()->addText("Loading " + filename + " for playing");
    cout << "FMOD_System_CreateSound\n";
    flush(cout);

    loadPluginChain();

    result = FMOD_System_CreateSound(system, filename.toStdString().c_str(),
                                     FMOD_ACCURATETIME | FMOD_CREATESTREAM | FMOD_LOOP_OFF | FMOD_MPEGSEARCH,
                                     &extrainfo1, &sound);

    if (m_Info1->pluginName.empty()) {
        m_Info1->pluginName = PLUGIN_fmod_NAME;
    }

    cout << "FMOD_System_CreateSound done\n";
    cout << "plugin: " << m_Info1->pluginName << "\n";
    flush(cout);

    FMOD_SOUND_TYPE type;
    FMOD_SOUND_FORMAT format;
    int channels;
    int bits;
    FMOD_Sound_GetFormat(sound, &type, &format, &channels, &bits);
    m_Info1->numChannelsStream = channels;
    if (result == FMOD_OK)
    {
        if (m_Info1->plugin == 0) //FMOD
        {
            m_Info1->fileformat = getFMODSoundFormat(sound);
        }
        return true;
    }
    else
    {
        ERRCHECK(result);
        delete m_Info1;
        m_Info1 = nullptr;
        return false;
    }
}

const char* SoundManager::getFMODSoundFormat(FMOD_SOUND* sound)
{
    FMOD_SOUND_TYPE type;
    const char* format;
    FMOD_Sound_GetFormat(sound, &type, nullptr, nullptr, nullptr);
    if (type != FMOD_SOUND_TYPE_UNKNOWN)
    {
        switch (type)
        {
        case FMOD_SOUND_TYPE_AIFF:
            format = "AIFF";
            break;
        case FMOD_SOUND_TYPE_ASF:
            format = "WMA/ASF/WMV";
            break;
        case FMOD_SOUND_TYPE_DLS:
            format = "FMOD Downloadable Sound Bank";
            break;
        case FMOD_SOUND_TYPE_FLAC:
            format = "FLAC";
            break;
        case FMOD_SOUND_TYPE_FSB:
            format = "FMOD Sample Bank";
            break;
        case FMOD_SOUND_TYPE_IT:
            format = "Impulse Tracker";
            break;
        case FMOD_SOUND_TYPE_MIDI:
            format = "MIDI";
            break;
        case FMOD_SOUND_TYPE_MOD:
            format = "Protracker/Fasttracker";
            break;
        case FMOD_SOUND_TYPE_MPEG:
            format = "MP2/MP3 MPEG";
            break;
        case FMOD_SOUND_TYPE_OGGVORBIS:
            format = "Ogg vorbis";
            break;
        case FMOD_SOUND_TYPE_PLAYLIST:
            format = "ASX/PLS/M3U/WAX playlist";
            break;
        case FMOD_SOUND_TYPE_RAW:
            format = "Raw PCM data";
            break;
        case FMOD_SOUND_TYPE_S3M:
            format = "ScreamTracker 3";
            break;
        case FMOD_SOUND_TYPE_USER:
            format = "User created sound";
            break;
        case FMOD_SOUND_TYPE_WAV:
            format = "Microsoft WAV";
            break;
        case FMOD_SOUND_TYPE_XM:
            format = "FastTracker 2 XM";
            break;
        case FMOD_SOUND_TYPE_XMA:
            format = "Xbox360 XMA";
            break;
        default:
            format = "Unknown format";
        }
    }
    else
    {
        format = 0; //3rd party format
    }
    return format;
}
