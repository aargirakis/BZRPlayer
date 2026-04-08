#include <QDir>
#include "mainwindow.h"
#include "plugins.h"
#include "soundmanager.h"

void SoundManager::Init(int outputDeviceProvided, const QString &outputFilenameProvided) {
    result = FMOD_System_Create(&system, FMOD_VERSION);
    checkFmodError(result);

    result = FMOD_Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_FILE, nullptr, "fmodlog.txt");
    checkFmodError(result);

    unsigned int version;
    result = FMOD_System_GetVersion(system, &version, nullptr);
    checkFmodError(result);

    if (version < FMOD_VERSION) {
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version,
               FMOD_VERSION);
    }

    currentDevice = outputDeviceProvided;

    printf("Setting ouput to: %i\n", currentDevice);

    const auto outputType = static_cast<FMOD_OUTPUTTYPE>(outputDeviceProvided);
    result = FMOD_System_SetOutput(system, outputType);
    checkFmodError(result);

    if (outputType != FMOD_OUTPUTTYPE_WAVWRITER) {
        printf("FMOD_System_Init: %i\n", currentDevice);
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, nullptr);
        checkFmodError(result);
    } else {
        QString outputPath = outputFilenameProvided;

        if (const QDir pathDir(userPath + "/recordings"); !pathDir.exists()) {
            QDir().mkdir(userPath + "/recordings");
        }

        outputPath = userPath + "/recordings/" + outputFilenameProvided;
        cout << "filename: " << outputPath.toStdString().c_str() << "\n";

        const QDateTime date = QDateTime::currentDateTime();
        const QString formattedTime = date.toString("yyyy.MM.dd - hh.mm.ss.ms");
        const QString outputPathFile = outputPath + " " + formattedTime + ".wav";

        printf("FMOD_System_Init: WAVWRITER %i\n", currentDevice);

        cout << "filename complete: " << outputPathFile.toStdString().c_str() << "\n";
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, outputPathFile.toStdString().data());
        checkFmodError(result);
        printf("WAVWRITER is set\n");
    }

    cout << "FMOD_System_CreateChannelGroup(system,"",&channelGroup)\n";
    FMOD_System_CreateChannelGroup(system, "", &channelGroup);
    checkFmodError(result);

    cout << "FMOD_System_CreateDSPByType(system,FMOD_DSP_TYPE_NORMALIZE, &dspNormalizer)\n";
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_NORMALIZE, &dspNormalizer);
    checkFmodError(result);

    cout << "FMOD_System_CreateDSPByType(system,FMOD.DSP_TYPE.FFT, &dspFFT)\n";
    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FFT, &dspFft);
    checkFmodError(result);
    cout << "FMOD_DSP_SetActive(dspFFT,true)\n";
    FMOD_DSP_SetActive(dspFft, true);
    checkFmodError(result);
    cout << "FMOD_DSP_SetParameterInt(dspFFT,FMOD_DSP_FFT_WINDOW_HANNING,1024*2\n";
    result = FMOD_DSP_SetParameterInt(dspFft, FMOD_DSP_FFT_WINDOW_HANNING, 16 * 2);
    checkFmodError(result);
    cout << "FMOD_ChannelGroup_AddDSP(channelGroup,FMOD_CHANNELCONTROL_DSP_HEAD,dspFFT)\n";
    FMOD_ChannelGroup_AddDSP(channelGroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspFft);
    checkFmodError(result);

    cout << "FMOD_ChannelGroup_AddDSP(channelGroup,FMOD_CHANNELCONTROL_DSP_HEAD,dspNormalizer))\n";
    FMOD_ChannelGroup_AddDSP(channelGroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspNormalizer);
    checkFmodError(result);

    cout << "FMOD_DSP_SetActive(dspNormalizer,true)\n";
    FMOD_DSP_SetActive(dspNormalizer, true);
    checkFmodError(result);
}

void SoundManager::loadPluginChain() {
    /* https://web.archive.org/web/20250320051703/https://qa.fmod.com/t/set-priority-for-internal-file-codecs/18597:
     * Registering a codec with a priority of 0 will give it the highest priority.
     * If another codec already has a priority of 0 then your newly registered codec will still have a higher priority,
     * and both codecs will have a higher priority than a codec with a priority of 1.
     * You cannot change the priority order of FMOD internal codecs, but here is the priority of each codec as of 2.02.06:
     *
     * FMOD_SOUND_TYPE_FSB,         250
     * FMOD_SOUND_TYPE_WAV,         600
     * FMOD_SOUND_TYPE_OGGVORBIS,   800
     * FMOD_SOUND_TYPE_AIFF,        1000
     * FMOD_SOUND_TYPE_FLAC,        1100
     * FMOD_SOUND_TYPE_MOD,         1200
     * FMOD_SOUND_TYPE_S3M,         1300
     * FMOD_SOUND_TYPE_XM,          1400
     * FMOD_SOUND_TYPE_IT,          1500
     * FMOD_SOUND_TYPE_MIDI,        1600
     * FMOD_SOUND_TYPE_DLS,         1700
     * FMOD_SOUND_TYPE_ASF,         1900
     * FMOD_SOUND_TYPE_AUDIOQUEUE,  2200
     * FMOD_SOUND_TYPE_MEDIACODEC,  2250
     * FMOD_SOUND_TYPE_MPEG,        2400
     * FMOD_SOUND_TYPE_PLAYLIST,    2450
     * FMOD_SOUND_TYPE_RAW,         2500
     * FMOD_SOUND_TYPE_USER         2600
     */

    if (PLUGIN_libsidplayfp_LIB != "") {
        loadPlugin(PLUGIN_libsidplayfp_LIB, 0);
    }

    if (PLUGIN_libopenmpt_LIB != "") {
        loadPlugin(PLUGIN_libopenmpt_LIB, 0);
    }

    if (PLUGIN_highly_experimental_LIB != "") {
        loadPlugin(PLUGIN_highly_experimental_LIB, 1);
    }

    if (PLUGIN_highly_theoretical_LIB != "") {
        loadPlugin(PLUGIN_highly_theoretical_LIB, 1);
    }

    if (PLUGIN_lazyusf2_LIB != "") {
        loadPlugin(PLUGIN_lazyusf2_LIB, 1);
    }

    if (PLUGIN_highly_quixotic_LIB != "") {
        loadPlugin(PLUGIN_highly_quixotic_LIB, 1);
    }

    if (PLUGIN_vio2sf_LIB != "") {
        loadPlugin(PLUGIN_vio2sf_LIB, 1);
    }

    if (PLUGIN_protrekkr_LIB != "") {
        loadPlugin(PLUGIN_protrekkr_LIB, 1);
    }

    if (PLUGIN_hivelytracker_LIB != "") {
        loadPlugin(PLUGIN_hivelytracker_LIB, 1);
    }

    if (PLUGIN_libstsound_LIB != "") {
        loadPlugin(PLUGIN_libstsound_LIB, 1);
    }

    if (PLUGIN_flod_LIB != "") {
        loadPlugin(PLUGIN_flod_LIB, 1);
    }

    if (PLUGIN_sndh_player_LIB != "") {
        loadPlugin(PLUGIN_sndh_player_LIB, 1);
    }

    if (PLUGIN_furnace_LIB != "") {
        loadPlugin(PLUGIN_furnace_LIB, 1);
    }

    if (PLUGIN_uade_LIB != "") {
        loadPlugin(PLUGIN_uade_LIB, 1);
    }

    //loadPlugin("plugin_quartet.dll",1);

    if (PLUGIN_adplug_LIB != "") {
        loadPlugin(PLUGIN_adplug_LIB, 599);
    }

    if (PLUGIN_vgmstream_LIB != "") {
        loadPlugin(PLUGIN_vgmstream_LIB, 599);
    }

    if (PLUGIN_klystron_LIB != "") {
        loadPlugin(PLUGIN_klystron_LIB, 1701);
    }

    if (PLUGIN_asap_LIB != "") {
        loadPlugin(PLUGIN_asap_LIB, 1701);
    }

    if (PLUGIN_libkss_LIB != "") {
        loadPlugin(PLUGIN_libkss_LIB, 1701);
    }

    if (PLUGIN_organya_decoder_LIB != "") {
        loadPlugin(PLUGIN_organya_decoder_LIB, 1701);
    }

    if (PLUGIN_sunvox_lib_LIB != "") {
        loadPlugin(PLUGIN_sunvox_lib_LIB, 1701);
    }

    if (PLUGIN_sc68_LIB != "") {
        loadPlugin(PLUGIN_sc68_LIB, 1701);
    }

    if (PLUGIN_kdm_LIB != "") {
        loadPlugin(PLUGIN_kdm_LIB, 1701);
    }

    if (PLUGIN_libpac_LIB != "") {
        loadPlugin(PLUGIN_libpac_LIB, 1701);
    }

    if (PLUGIN_libxmp_LIB != "") {
        loadPlugin(PLUGIN_libxmp_LIB, 1701);
    }

    if (PLUGIN_mdxmini_LIB != "") {
        loadPlugin(PLUGIN_mdxmini_LIB, 1701);
    }

    if (PLUGIN_libvgm_LIB != "") {
        loadPlugin(PLUGIN_libvgm_LIB, 1701);
    }

    if (PLUGIN_game_music_emu_LIB != "") {
        loadPlugin(PLUGIN_game_music_emu_LIB, 1701);
    }

    if (PLUGIN_audiodecoder_wsr_LIB != "") {
        loadPlugin(PLUGIN_audiodecoder_wsr_LIB, 1701);
    }

    if (PLUGIN_v2m_player_LIB != "") {
        loadPlugin(PLUGIN_v2m_player_LIB, 1701);
    }

    if (PLUGIN_jaytrax_LIB != "") {
        loadPlugin(PLUGIN_jaytrax_LIB, 1701);
    }

    if (PLUGIN_audiofile_LIB != "") {
        loadPlugin(PLUGIN_audiofile_LIB, 1701);
    }

    if (PLUGIN_zxtune_LIB != "") {
        loadPlugin(PLUGIN_zxtune_LIB, 1701);
    }
}

int SoundManager::getSoundData(const unsigned int channelProvided) {
    FMOD_DSP_PARAMETER_FFT *fft = nullptr;

    result = FMOD_DSP_GetParameterData(dspFft, FMOD_DSP_FFT_SPECTRUMDATA, (void **) &fft, nullptr, nullptr, 0);

    float val = 0;
    //for (int channelProvided = 0; channelProvided < fft->numchannels; channelProvided++)
    //{
    for (int bin = 0; bin < fft->length / 2; bin++) {
        val += fft->spectrum[channelProvided][bin];
    }
    //}
    // clipping is probably over 5 (guessing?)
    // so clamp values over 5 to 5
    if (val > 5) {
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
    //return (freq[channelProvided]/16384)*100;
    //return 50;
    //ERRCHECK(result);
}

void SoundManager::setReverbEnabled(const bool enabled) {
    result = FMOD_System_SetReverbProperties(system, 0, enabled ? &currentReverbPreset : nullptr);
    checkFmodError(result);
}

void SoundManager::setReverbPreset(const QString &preset) {
    FMOD_REVERB_PROPERTIES prop;

    if (preset == "Generic") {
        prop = FMOD_PRESET_GENERIC;
    } else if (preset == "Padded cell") {
        prop = FMOD_PRESET_PADDEDCELL;
    } else if (preset == "Room") {
        prop = FMOD_PRESET_ROOM;
    } else if (preset == "Bathroom") {
        prop = FMOD_PRESET_BATHROOM;
    } else if (preset == "Living room") {
        prop = FMOD_PRESET_LIVINGROOM;
    } else if (preset == "Stone room") {
        prop = FMOD_PRESET_STONEROOM;
    } else if (preset == "Auditorium") {
        prop = FMOD_PRESET_AUDITORIUM;
    } else if (preset == "Concert hall") {
        prop = FMOD_PRESET_CONCERTHALL;
    } else if (preset == "Cave") {
        prop = FMOD_PRESET_CAVE;
    } else if (preset == "Arena") {
        prop = FMOD_PRESET_ARENA;
    } else if (preset == "Hangar") {
        prop = FMOD_PRESET_HANGAR;
    } else if (preset == "Carpeted hallway") {
        prop = FMOD_PRESET_CARPETTEDHALLWAY;
    } else if (preset == "Hallway") {
        prop = FMOD_PRESET_HALLWAY;
    } else if (preset == "Stone corridor") {
        prop = FMOD_PRESET_STONECORRIDOR;
    } else if (preset == "Alley") {
        prop = FMOD_PRESET_ALLEY;
    } else if (preset == "Forest") {
        prop = FMOD_PRESET_FOREST;
    } else if (preset == "City") {
        prop = FMOD_PRESET_CITY;
    } else if (preset == "Mountains") {
        prop = FMOD_PRESET_MOUNTAINS;
    } else if (preset == "Quarry") {
        prop = FMOD_PRESET_QUARRY;
    } else if (preset == "Plain") {
        prop = FMOD_PRESET_PLAIN;
    } else if (preset == "Parking lot") {
        prop = FMOD_PRESET_PARKINGLOT;
    } else if (preset == "Sewer pipe") {
        prop = FMOD_PRESET_SEWERPIPE;
    } else if (preset == "Underwater") {
        prop = FMOD_PRESET_UNDERWATER;
    }

    currentReverbPreset = prop;
    //DebugWindow::instance()->addText("Soundmanager, reverb preset: "+ preset);
}

void SoundManager::setNormalizeFadeTime(const int param) const {
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_FADETIME, static_cast<float>(param));
}

void SoundManager::setNormalizeThreshold(const int param) const {
    const float paramF = param / 100.0;
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_THRESHOLD, paramF);
}

void SoundManager::setNormalizeMaxAmp(const int param) const {
    FMOD_DSP_SetParameterFloat(dspNormalizer, FMOD_DSP_NORMALIZE_MAXAMP, static_cast<float>(param));
}

void SoundManager::setNormalizeEnabled(const bool enabled) const {
    FMOD_DSP *dsp = nullptr;
    FMOD_ChannelGroup_GetDSP(channelGroup, 0, &dsp);

    if (dsp) {
        FMOD_DSP_SetBypass(dspNormalizer, !enabled);
        //DebugWindow::instance()->addText("setNormalizeEnabled " + QString::number(enabled));
    }
}

FMOD_RESULT SoundManager::getTag(const char *name, const int index, FMOD_TAG *tag) const {
    return FMOD_Sound_GetTag(sound, name, index, tag);
}

int SoundManager::getNumTags() const {
    int numTags;
    FMOD_Sound_GetNumTags(sound, &numTags, nullptr);
    return numTags;
}

void SoundManager::loadPlugin(const string_view &filename, const int priority) {
    string pluginsDir = QString(libPath + PLUGINS_DIR + "/").toStdString();
    const char *pluginPath = (pluginsDir += filename).c_str();

    result = FMOD_System_LoadPlugin(system, pluginPath, nullptr, priority);

    if (result != FMOD_OK) {
        //DebugWindow::instance()->addText(QString(filename.c_str()));
    }

    checkFmodError(result, pluginPath);

    //DebugWindow::instance()->addText("GetNumPlugins " + QString::number(numplugins));
}

void SoundManager::checkFmodError(const FMOD_RESULT result) {
    checkFmodError(result, "");
}

void SoundManager::checkFmodError(const FMOD_RESULT result, const QString &msg) {
    if (result != FMOD_OK) {
        printf("FMOD error! (%d) %s", result, FMOD_ErrorString(result));

        if (msg == "") {
            printf("\n");
        }
        if (msg != "") {
            printf(" - %s\n", msg.toStdString().c_str());
        }

        flush(cout);
    }
}

void SoundManager::stop() const {
    FMOD_Channel_Stop(channel);
    //    FMOD_OUTPUTTYPE outputType = static_cast<FMOD_OUTPUTTYPE>(currentDevice);
    //    if(outputType==FMOD_OUTPUTTYPE_WAVWRITER)
    //    {
    //        setOutput(0);
    //    }
}

bool SoundManager::isPlaying() const {
    if (!channel) return false;

    FMOD_BOOL playing;
    FMOD_Channel_IsPlaying(channel, &playing);
    return playing;
}

bool SoundManager::isWavWriterDeviceSelected() const {
    return static_cast<FMOD_OUTPUTTYPE>(currentDevice) == FMOD_OUTPUTTYPE_WAVWRITER;
}

void SoundManager::setPosition(const unsigned int positon, const FMOD_TIMEUNIT timeUnit) const {
    FMOD_Channel_SetPosition(channel, positon, timeUnit);
}

void SoundManager::setVolume(const float volume) const {
    FMOD_Channel_SetVolume(channel, volume);
}

float SoundManager::getNominalFrequency() const {
    return nominalFrequency;
}

void SoundManager::setFrequencyByMultiplier(const float percent) const {
    FMOD_Channel_SetFrequency(channel, percent * nominalFrequency);
}

void SoundManager::setMute(const bool mute) const {
    FMOD_Channel_SetMute(channel, mute);
}

unsigned int SoundManager::getPosition(const FMOD_TIMEUNIT timeUnit) const {
    unsigned int currentMs;
    FMOD_Channel_GetPosition(channel, &currentMs, timeUnit);
    return currentMs;
}

unsigned int SoundManager::getLength(const FMOD_TIMEUNIT timeUnit) const {
    unsigned int songLengthMs;
    //DebugWindow::instance()->addText("SoundManager: getLength, Timeunit: " + QString::number(timeUnit));
    FMOD_Sound_GetLength(sound, &songLengthMs, timeUnit);
    return songLengthMs;
}

void SoundManager::pause(const bool pause) const {
    FMOD_Channel_SetPaused(channel, pause);
}

bool SoundManager::isPaused() const {
    FMOD_BOOL pause;
    FMOD_Channel_GetPaused(channel, &pause);
    return pause;
}

void SoundManager::playAudio(const bool startPaused) {
    mutedChannelsMask = 0;
    mutedChannelsMaskString = "";
    FMOD_System_PlaySound(system, sound, channelGroup, startPaused, &channel);
    FMOD_Channel_GetFrequency(channel, &nominalFrequency);
    //DebugWindow::instance()->addText("SoundManager: PlaySound");
}

void SoundManager::release() const {
    FMOD_Sound_Release(sound);
}

void SoundManager::shutdown() const {
    FMOD_Sound_Release(sound);
    FMOD_System_Release(system);
}

void SoundManager::muteChannels(const unsigned int mask, const QString &maskStr) {
    info->mutedChannelsMask = maskStr.toStdString();

    FMOD_Channel_SetPosition(channel, mask, FMOD_TIMEUNIT_MUTE_VOICE);

    mutedChannelsMask = mask;
    mutedChannelsMaskString = maskStr;
}

bool SoundManager::isChannelMuted(const unsigned int channelProvided) const {
    bool muted = false;

    if (mutedChannelsMaskString != nullptr && mutedChannelsMaskString.at(channelProvided) == '0') {
        muted = true;
    }

    return muted;
}

bool SoundManager::loadSound(const QString &filename, Info *infoProvided) {
    stop();
    release();

    mutedChannelsMask = 0;
    mutedChannelsMaskString = "";

    info = infoProvided;
    info->tempPath = QDir::tempPath().toStdString();
    info->dataPath = dataPath.toStdString();
    info->libPath = libPath.toStdString();
    info->userPath = userPath.toStdString();
    info->filename = filename.toStdString();

    FMOD_CREATESOUNDEXINFO extraInfo = {};

    extraInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    const string dls = dataPath.toStdString() + PLUGINS_FMOD_DIR + "/gm.dls";
    extraInfo.dlsname = dls.c_str();
    extraInfo.userdata = info;

    cout << "Loading " << info->filename << " (subsong " << info->currentSubsong + 1 << ")" << endl;

    loadPluginChain();

    constexpr FMOD_MODE fmodMode = FMOD_ACCURATETIME | FMOD_CREATESTREAM;

    result = FMOD_System_CreateSound(system, filename.toStdString().c_str(), fmodMode | FMOD_LOOP_OFF, &extraInfo,
                                     &sound);

    if (result != FMOD_OK) {
        checkFmodError(result);
        delete info;
        info = nullptr;
        return false;
    }

    FMOD_SOUND_TYPE type;
    int channels;
    FMOD_Sound_GetFormat(sound, &type, nullptr, &channels, nullptr);

    info->numChannelsStream = channels;

    if (info->plugin != PLUGIN_adplug &&
        info->plugin != PLUGIN_hivelytracker &&
        info->plugin != PLUGIN_libopenmpt &&
        info->plugin != PLUGIN_libsidplayfp &&
        info->plugin != PLUGIN_libvgm &&
        info->plugin != PLUGIN_libxmp &&
        info->plugin != PLUGIN_sndh_player &&
        info->plugin != PLUGIN_uade &&
        info->plugin != PLUGIN_vgmstream
    ) {
        info->isContinuousPlaybackActive = false;

        if (info->plugin == PLUGIN_fmod) {
            info->pluginName = PLUGIN_fmod_NAME;
            info->fileFormat = getFmodSoundTypeName(type);

            if (info->isPlayModeRepeatSongEnabled && info->isFmodSeamlessLoopEnabled) {
                info->isSeamlessLoopActive = true;
                FMOD_Sound_SetMode(sound, fmodMode | FMOD_LOOP_NORMAL);
            }
        }
    }

    cout << "FMOD_System_CreateSound done\n";
    cout << "plugin: " << info->pluginName << "\n";
    flush(cout);

    return true;
}

const char *SoundManager::getFmodSoundTypeName(const FMOD_SOUND_TYPE type) {
    if (type == FMOD_SOUND_TYPE_UNKNOWN) {
        return nullptr; // 3rd party format
    }

    switch (type) {
        case FMOD_SOUND_TYPE_AIFF:
            return "AIFF";
        case FMOD_SOUND_TYPE_ASF:
            return "ASF/WMA/WMV";
        case FMOD_SOUND_TYPE_AT9:
            return "Sony ATRAC9 (FSB)";
        case FMOD_SOUND_TYPE_AUDIOQUEUE:
            return "Apple Audio Queue";
        case FMOD_SOUND_TYPE_DLS:
            return "Downloadable Sound Bank";
        case FMOD_SOUND_TYPE_FADPCM:
            return "FADPCM (FSB)";
        case FMOD_SOUND_TYPE_FLAC:
            return "FLAC";
        case FMOD_SOUND_TYPE_FSB:
            return "FMOD Sample Bank";
        case FMOD_SOUND_TYPE_IT:
            return "Impulse Tracker";
        case FMOD_SOUND_TYPE_MEDIA_FOUNDATION:
            return "MMF (FSB)";
        case FMOD_SOUND_TYPE_MEDIACODEC:
            return "Google Media Codec";
        case FMOD_SOUND_TYPE_MIDI:
            return "MIDI";
        case FMOD_SOUND_TYPE_MOD:
            return "Protracker/Fasttracker";
        case FMOD_SOUND_TYPE_MPEG:
            return "MP2/MP3/RIFF";
        case FMOD_SOUND_TYPE_OGGVORBIS:
            return "Ogg vorbis";
        case FMOD_SOUND_TYPE_OPUS:
            return "Opus (FSB)";
        case FMOD_SOUND_TYPE_PLAYLIST:
            return "ASX/PLS/M3U/WAX playlist";
        case FMOD_SOUND_TYPE_RAW:
            return "Raw PCM data";
        case FMOD_SOUND_TYPE_S3M:
            return "ScreamTracker 3";
        case FMOD_SOUND_TYPE_USER:
            return "User created sound";
        case FMOD_SOUND_TYPE_VORBIS:
            return "Vorbis (FSB)";
        case FMOD_SOUND_TYPE_WAV:
            return "Microsoft WAV";
        case FMOD_SOUND_TYPE_XM:
            return "FastTracker 2 XM";
        case FMOD_SOUND_TYPE_XMA:
            return "Xbox Media Audio (FSB)";
        default:
            return "Unknown format";
    }
}
