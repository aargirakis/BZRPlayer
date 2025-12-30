#include <fstream>
#include <QDir>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "mainwindow.h"
#include "plugins.h"
#include "soundmanager.h"

void SoundManager::Init(int outputDeviceProvided, const QString &filePathProvided) {
    result = FMOD_System_Create(&system, FMOD_VERSION);
    checkFmodError(result);

    // TODO
    result = FMOD_Debug_Initialize(FMOD_DEBUG_LEVEL_LOG, FMOD_DEBUG_MODE_FILE, nullptr, "fmodlog.txt");
    checkFmodError(result);

    unsigned int version;
    result = FMOD_System_GetVersion(system, &version, nullptr);
    checkFmodError(result);

    if (version < FMOD_VERSION) {
        // TODO
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version,
               FMOD_VERSION);
    }

    currentDevice = outputDeviceProvided;

    // TODO
    printf("Setting output to: %i\n", currentDevice);

    const auto outputType = static_cast<FMOD_OUTPUTTYPE>(outputDeviceProvided);
    result = FMOD_System_SetOutput(system, outputType);
    checkFmodError(result);

    if (outputType != FMOD_OUTPUTTYPE_WAVWRITER) {
        printf("FMOD_System_Init: %i\n", currentDevice);
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, nullptr);
        checkFmodError(result);
    } else {
        QString outputFilePath = filePathProvided;

        if (const QDir pathDir(userPath + "/recordings"); !pathDir.exists()) {
            QDir().mkdir(userPath + "/recordings");
        }

        outputFilePath = userPath + "/recordings/" + filePathProvided;
        cout << "output file path: " << outputFilePath.toStdString().c_str() << "\n";

        const QDateTime date = QDateTime::currentDateTime();
        const QString formattedTime = date.toString("yyyy.MM.dd - hh.mm.ss.ms");
        outputFilePath = outputFilePath + " " + formattedTime + ".wav";

        printf("FMOD_System_Init: WAVWRITER %i\n", currentDevice);

        cout << "output file path complete: " << outputFilePath.toStdString().c_str() << "\n";
        result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, outputFilePath.toStdString().data());
        checkFmodError(result);
        printf("WAVWRITER is set\n");
    }

    FMOD_System_CreateChannelGroup(system, "", &channelGroup);
    checkFmodError(result);

    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_NORMALIZE, &dspNormalizer);
    checkFmodError(result);

    result = FMOD_System_CreateDSPByType(system, FMOD_DSP_TYPE_FFT, &dspFft);
    checkFmodError(result);

    FMOD_DSP_SetActive(dspFft, true);
    checkFmodError(result);

    result = FMOD_DSP_SetParameterInt(dspFft, FMOD_DSP_FFT_WINDOW_HANNING, 16 * 2);
    checkFmodError(result);

    FMOD_ChannelGroup_AddDSP(channelGroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspFft);
    checkFmodError(result);

    FMOD_ChannelGroup_AddDSP(channelGroup, FMOD_CHANNELCONTROL_DSP_HEAD, dspNormalizer);
    checkFmodError(result);

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

    const auto fileBuffer = info->fileBuffer;
    const auto filesize = info->filesize;
    const auto filename = info->filename;

    if (PLUGIN_libsidplayfp_LIB != "" && isFormatSidOrMusStr(fileBuffer, filesize, filename, info->isSid)) {
        loadPlugin(PLUGIN_libsidplayfp_LIB, 0);
    }

    // libxmp seems to play Farandole Composer module really better
    if (PLUGIN_libopenmpt_LIB != "" && !isFormatFarandole(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_libopenmpt_LIB, 0);
    }

    if (PLUGIN_highly_experimental_LIB != "" && isFormatPsf(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_highly_experimental_LIB, 0);
    }

    if (PLUGIN_highly_theoretical_LIB != "" && isFormatPsf(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_highly_theoretical_LIB, 0);
    }

    if (PLUGIN_lazyusf2_LIB != "" && isFormatPsf(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_lazyusf2_LIB, 0);
    }

    if (PLUGIN_highly_quixotic_LIB != "" && isFormatPsf(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_highly_quixotic_LIB, 0);
    }

    if (PLUGIN_vio2sf_LIB != "" && isFormatPsf(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_vio2sf_LIB, 0);
    }

    if (PLUGIN_protrekkr_LIB != "" && isFormatProtrekkr(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_protrekkr_LIB, 0);
    }

    if (PLUGIN_hivelytracker_LIB != "" && isFormatAhxOrHvl(fileBuffer, filesize, info->isAhx)) {
        loadPlugin(PLUGIN_hivelytracker_LIB, 0);
    }

    if (PLUGIN_libstsound_LIB != "") {
        loadPlugin(PLUGIN_libstsound_LIB, 0);
    }

    if (PLUGIN_flod_LIB != "" && isFormatBPSoundMon1(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_flod_LIB, 0);
    }

    if (PLUGIN_sndh_player_LIB != "" && isFormatSndh(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_sndh_player_LIB, 0);
    }

    if (PLUGIN_furnace_LIB != "" && isFormatFurOrDfmOrZlib(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_furnace_LIB, 0);
    }

    // 16mb max allowed (emulated chipmem in uade is currently 8mb)
    if (PLUGIN_uade_LIB != "" && filesize <= 1024 * 1024 * 16 && !isFormatRiff(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_uade_LIB, 0);
    }

    //loadPlugin("plugin_quartet.dll", 0);

    if (PLUGIN_adplug_LIB != "") {
        loadPlugin(PLUGIN_adplug_LIB, 0);
    }

    if (PLUGIN_vgmstream_LIB != "") {
        loadPlugin(PLUGIN_vgmstream_LIB, 0);
    }

    if (PLUGIN_klystron_LIB != "" && isFormatKlystron(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_klystron_LIB, 0);
    }

    if (PLUGIN_asap_LIB != "") {
        loadPlugin(PLUGIN_asap_LIB, 0);
    }

    if (PLUGIN_libkss_LIB != "") {
        loadPlugin(PLUGIN_libkss_LIB, 0);
    }

    if (PLUGIN_organya_decoder_LIB != "" && isFormatOrganya1(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_organya_decoder_LIB, 0);
    }

    if (PLUGIN_sunvox_lib_LIB != "" && isFormatSunVox(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_sunvox_lib_LIB, 0);
    }

    if (PLUGIN_sc68_LIB != "" && isFormatSc68(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_sc68_LIB, 0);
    }

    if (PLUGIN_kdm_LIB != "") {
        loadPlugin(PLUGIN_kdm_LIB, 0);
    }

    if (PLUGIN_libpac_LIB != "" && isFormatPac(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_libpac_LIB, 0);
    }

    if (PLUGIN_libxmp_LIB != "" && !isFormatRiff(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_libxmp_LIB, 0);
    }

    // 100kb max allowed (biggest mdx on modland is 85kb)
    if (PLUGIN_mdxmini_LIB != "" && filesize <= 1024 * 100) {
        loadPlugin(PLUGIN_mdxmini_LIB, 0);
    }

    if (PLUGIN_libvgm_LIB != "") {
        loadPlugin(PLUGIN_libvgm_LIB, 0);
    }

    if (PLUGIN_game_music_emu_LIB != "") {
        loadPlugin(PLUGIN_game_music_emu_LIB, 0);
    }

    if (PLUGIN_audiodecoder_wsr_LIB != "" && isFormatWsr(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_audiodecoder_wsr_LIB, 0);
    }

    if (PLUGIN_v2m_player_LIB != "" && isFormatV2M(filesize, filename)) {
        loadPlugin(PLUGIN_v2m_player_LIB, 0);
    }

    if (PLUGIN_jaytrax_LIB != "" && isFormatJaytrax(fileBuffer, filesize)) {
        loadPlugin(PLUGIN_jaytrax_LIB, 0);
    }

    if (PLUGIN_audiofile_LIB != "") {
        loadPlugin(PLUGIN_audiofile_LIB, 0);
    }

    if (PLUGIN_zxtune_LIB != "") {
        loadPlugin(PLUGIN_zxtune_LIB, 0);
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

void SoundManager::loadPlugin(const string &pluginFilename, const int priority) {
    const string pluginPath = info->pluginsDir + pluginFilename;

    result = FMOD_System_LoadPlugin(system, pluginPath.c_str(), nullptr, priority);

    if (result != FMOD_OK) {
        //DebugWindow::instance()->addText(QString(pluginFilename.c_str()));
    }

    checkFmodError(result, pluginPath.c_str());

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

    if (info != nullptr) {
        unmapFile(info->fileBuffer, info->filesize, info->filePath);
    }
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

bool SoundManager::loadSound(const QString &filePath, Info *infoProvided) {
    stop();
    release();

    info = infoProvided;

    const auto filePathStr = filePath.toStdString();
    const bool isLocalFilePath = !filePathStr.starts_with("http");

    if (isLocalFilePath) {
        const auto [fileMapped, filesize] = mapFile(filePath);

        if (fileMapped == nullptr || filesize == NULL) return false;

        info->fileBuffer = fileMapped;
        info->filesize = filesize;
        info->isLocalFilePath = true;
    }

    info->tempPath = QDir::tempPath().toStdString();
    info->dataPath = dataPath.toStdString();
    info->libPath = libPath.toStdString();
    info->userPath = userPath.toStdString();
    info->filePath = filePathStr;

    const QFileInfo fileinfo(filePath);
    info->filename = fileinfo.fileName().toStdString();
    info->fileDir = fileinfo.path().toStdString();
    info->fileLastModified = fileinfo.lastModified().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    info->fileCreatedAt = fileinfo.birthTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();

    mutedChannelsMask = 0;
    mutedChannelsMaskString = "";

    FMOD_CREATESOUNDEXINFO extraInfo = {};
    extraInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    extraInfo.userdata = info;

    cout << "Loading " << filePathStr << " (subsong " << info->currentSubsong + 1 << ")" << endl;

    constexpr FMOD_MODE fmodModeNetwork = FMOD_ACCURATETIME | FMOD_CREATESTREAM;
    constexpr FMOD_MODE fmodModeLocal = fmodModeNetwork | FMOD_OPENMEMORY_POINT;
    constexpr FMOD_MODE fmodModeLocalPlugins = fmodModeLocal | FMOD_LOOP_OFF;

    FMOD_MODE fmodModeCurrent;

    const char *pathOrBuffer;

    // use fmod for network streams playback
    if (!isLocalFilePath) {
        fmodModeCurrent = fmodModeNetwork;
        pathOrBuffer = filePathStr.c_str();
    } else {
        pathOrBuffer = reinterpret_cast<const char *>(info->fileBuffer);
        extraInfo.length = static_cast<unsigned int>(info->filesize);

        // use fmod for midi playback
        if (isFormatMidi(info->fileBuffer, info->filesize)) {
            static const char *fmodDlsPath = strdup((dataPath + FMOD_DLS_PATH).toStdString().c_str());
            extraInfo.dlsname = fmodDlsPath;

            if (info->isPlayModeRepeatSongEnabled && info->isFmodSeamlessLoopEnabled) {
                info->isSeamlessLoopActive = true;
                fmodModeCurrent = fmodModeLocal | FMOD_LOOP_NORMAL;
            } else {
                fmodModeCurrent = fmodModeLocal | FMOD_LOOP_OFF;
            }
        } else {
            fmodModeCurrent = fmodModeLocalPlugins;
            info->pluginsDir = info->libPath + PLUGINS_DIR + "/";
            loadPluginChain();
        }
    }

    result = FMOD_System_CreateSound(system, pathOrBuffer, fmodModeCurrent, &extraInfo, &sound);

    if (result != FMOD_OK) {
        checkFmodError(result);
        unmapFile(info->fileBuffer, info->filesize, info->filePath);
        delete info;
        info = nullptr;
        return false;
    }

    cout << "FMOD_System_CreateSound done\n";

    FMOD_SOUND_TYPE type;
    int channels;
    FMOD_Sound_GetFormat(sound, &type, nullptr, &channels, nullptr);

    info->numChannelsStream = channels;

    if (info->plugin != PLUGIN_adplug &&
        info->plugin != PLUGIN_furnace &&
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
        }
    }

    cout << "selected plugin: " << info->pluginName << endl;

    return true;
}

pair<uint8_t *, size_t> SoundManager::mapFile(const QString &fileToMap) {
    const auto fileToMapStr = fileToMap.toStdString();
    uint8_t *fileMapped;
    size_t filesize;
#ifdef WIN32
    const auto hFile = CreateFileW(fileToMap.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ,
                                   nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Error reading file " << fileToMapStr << ": error " << GetLastError() << endl;
        return {nullptr, NULL};
    }

    LARGE_INTEGER size;

    if (!GetFileSizeEx(hFile, &size)) {
        cerr << "Error getting size of file " << fileToMapStr << ": error " << GetLastError() << endl;
        CloseHandle(hFile);
        return {nullptr, NULL};
    }

    filesize = static_cast<unsigned long>(size.QuadPart);
#else
    const int fd = open(fileToMapStr.c_str(), O_RDONLY);

    if (fd < 0) {
        cerr << "Error reading file " << fileToMapStr << ": " << strerror(errno) << endl;
        return {nullptr, NULL};
    }

    struct stat st = {};

    if (const int status = fstat(fd, &st); status < 0) {
        cerr << "Stat failed for file " << fileToMapStr << ": " << strerror(errno) << endl;
        close(fd);
        return {nullptr, NULL};
    }

    filesize = st.st_size;
#endif
    if (filesize <= 0) {
        cerr << "Invalid size for file " << fileToMapStr << endl;
        return {nullptr, NULL};
    }

#ifdef WIN32
    const auto hMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0,
                                            0, nullptr);

    auto err = GetLastError();

    CloseHandle(hFile);

    if (hMapping == nullptr) {
        cerr << "Error creating file mapping object for " << fileToMapStr << ": error " << err << endl;
        return {nullptr, NULL};
    }

    fileMapped = static_cast<uint8_t *>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, filesize));

    err = GetLastError();

    CloseHandle(hMapping);

    if (fileMapped == nullptr) {
        cerr << "Error mapping file " << fileToMapStr << " to memory: error " << err << endl;
        return {nullptr, NULL};
    }
#else
    fileMapped = static_cast<uint8_t *>(mmap(nullptr, filesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE,
                                             fd, 0));

    const auto errStr = strerror(errno);

    close(fd);

    if (fileMapped == MAP_FAILED) {
        cerr << "Error mapping file " << fileToMapStr << " to memory: " << errStr << endl;
        return {nullptr, NULL};
    }
#endif

    return {fileMapped, filesize};
}

void SoundManager::unmapFile(uint8_t *fileMapped, const size_t filesize, const string_view &filePath) {
    if (fileMapped != nullptr) {
#ifdef WIN32
        if (!UnmapViewOfFile(fileMapped)) {
            cerr << "Error unmapping file " << filePath << " from memory: error " << GetLastError() << endl;
        }
#else
        if (const int rc = munmap(fileMapped, filesize); rc < 0) {
            cerr << "Error unmapping file " << filePath << " from memory: " << strerror(errno) << endl;
        }
#endif
    }
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

bool SoundManager::isFormatMidi(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magicHeaderChunk[] = "MThd";
    constexpr char magicTrackChunk[] = "MTrk";
    constexpr uint8_t magicTrackChunkOffset1 = 0x04;
    constexpr uint8_t magicLength = 0x04;
    constexpr uint8_t formatOffset = 0x08;

    if (filesize < magicLength) return false;

    if (memcmp(fileBuffer, magicHeaderChunk, magicLength) != 0) {
        return false;
    }

    uint16_t format;

    if (filesize < formatOffset + sizeof(format)) return false;

    memcpy(&format, &fileBuffer[formatOffset], sizeof(format));

    if (byteswap(format) > 0x02) return false;

    uint32_t magicTrackChunkOffset2;

    memcpy(&magicTrackChunkOffset2, &fileBuffer[magicTrackChunkOffset1], sizeof(magicTrackChunkOffset2));
    magicTrackChunkOffset2 = formatOffset + byteswap(magicTrackChunkOffset2);

    if (filesize < magicTrackChunkOffset2 + magicLength) return false;

    return memcmp(&fileBuffer[magicTrackChunkOffset2], magicTrackChunk, magicLength) == 0;
}

bool SoundManager::isFormatSidOrMusStr(const uint8_t *fileBuffer, const size_t filesize, const string_view &filename,
                                       bool &isSid) {
    // 96kb max allowed (biggest sid in HVSC is 63kb while biggest mus/str in CGSC is 29kb)
    if (filesize > 1024 * 96) return false;

    constexpr char magic1[] = "PSID";
    constexpr char magic2[] = "RSID";
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicLength) return false;

    if (memcmp(fileBuffer, magic1, magicLength) == 0 || memcmp(fileBuffer, magic2, magicLength) == 0) {
        isSid = true;
        return true;
    }

    if (const string ext = filesystem::path(filename).extension().string();
        strcasecmp(ext.c_str(), ".MUS") == 0 || strcasecmp(ext.c_str(), ".STR") == 0) {
        return true;
    }

    return false;
}

bool SoundManager::isFormatFarandole(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "FAR\xFE";
    constexpr uint8_t magicOffset = 0x00;
    constexpr uint8_t magicLength = 0x04;
    constexpr char crLfEof[] = "\x0D\x0A\x1A";
    constexpr uint8_t crLfEofOffset = 0x2C;
    constexpr uint8_t crLfEofLength = 0x03;

    if (filesize < crLfEofOffset + crLfEofLength) return false;

    return memcmp(&fileBuffer[magicOffset], magic, magicLength) == 0 &&
           memcmp(&fileBuffer[crLfEofOffset], crLfEof, crLfEofLength) == 0;
}

bool SoundManager::isFormatRiff(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "RIFF";
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatProtrekkr(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic1[] = "PROTREK";
    constexpr char magic2[] = "TWNNSNG";
    constexpr uint8_t magicLength = 0x07;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic1, magicLength) == 0 || memcmp(fileBuffer, magic2, magicLength) == 0;
}

bool SoundManager::isFormatAhxOrHvl(const uint8_t *fileBuffer, const size_t filesize, bool &isAhx) {
    constexpr char magic1[] = "THX";
    constexpr char magic2[] = "HVL";
    constexpr uint8_t magicLength = 0x03;

    if (filesize < magicLength) return false;

    if (memcmp(fileBuffer, magic1, magicLength) == 0) {
        isAhx = true;
        return true;
    }
    if (memcmp(fileBuffer, magic2, magicLength) == 0) {
        return true;
    }

    return false;
}

bool SoundManager::isFormatBPSoundMon1(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "BPSM";
    constexpr uint8_t magicOffset = 0x1A;
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicOffset + magicLength) return false;

    return memcmp(&fileBuffer[magicOffset], magic, magicLength) == 0;
}

bool SoundManager::isFormatFurOrDfmOrZlib(const uint8_t *fileBuffer, const size_t filesize) {
    // furnace, deflemask & zlib (with no/fast/default/best compression)
    constexpr char magicFur[] = "-Furnace module-";
    constexpr char magicDfm[] = ".DelekDefleMask.";
    constexpr uint8_t magicFurAndDfmLength = 0x10;
    constexpr char magicZlibNo[] = "\x78\x01";
    constexpr uint8_t magicZlibNoLength = 0x02;
    constexpr char magicZlibFast[] = "\x78\x5E";
    constexpr uint8_t magicZlibFastLength = 0x02;
    constexpr char magicZlibDefault[] = "\x78\x9C";
    constexpr uint8_t magicZlibDefaultLength = 0x02;
    constexpr char magicZlibBest[] = "\x78\xDA";
    constexpr uint8_t magicZlibBestLength = 0x02;

    if (filesize < magicFurAndDfmLength) return false;

    return memcmp(fileBuffer, magicFur, magicFurAndDfmLength) == 0 ||
           memcmp(fileBuffer, magicDfm, magicFurAndDfmLength) == 0 ||
           memcmp(fileBuffer, magicZlibNo, magicZlibNoLength) == 0 ||
           memcmp(fileBuffer, magicZlibFast, magicZlibFastLength) == 0 ||
           memcmp(fileBuffer, magicZlibDefault, magicZlibDefaultLength) == 0 ||
           memcmp(fileBuffer, magicZlibBest, magicZlibBestLength) == 0;
}

bool SoundManager::isFormatPsf(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "PSF";
    constexpr uint8_t magicLength = 0x03;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatKlystron(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "cyd!song";
    constexpr uint8_t magicLength = 0x08;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatOrganya1(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "Org-02";
    constexpr uint8_t magicLength = 0x06;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatSunVox(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "SVOX";
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatSc68(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic[] = "SC68";
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicLength) return false;

    return memcmp(fileBuffer, magic, magicLength) == 0;
}

bool SoundManager::isFormatSndh(const uint8_t *fileBuffer, const size_t filesize) {
    // 2mb max allowed (biggest sndh on modland is 1383730 bytes)
    if (filesize > 1024 * 2048) return false;

    constexpr char magic[] = "SNDH";
    constexpr uint8_t magicOffset = 0x0C;
    constexpr char magicPacked1[] = "ICE!";
    constexpr char magicPacked2[] = "Ice!";
    constexpr uint8_t magicPackedOffset = 0x00;
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magicOffset + magicLength) return false;

    return memcmp(&fileBuffer[magicOffset], magic, magicLength) == 0 ||
           memcmp(&fileBuffer[magicPackedOffset], magicPacked1, magicLength) == 0 ||
           memcmp(&fileBuffer[magicPackedOffset], magicPacked2, magicLength) == 0;
}

bool SoundManager::isFormatPac(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr char magic1[] = "PACG";
    constexpr uint8_t magic1Offset = 0x00;
    constexpr char magic2[] = "PAIN";
    constexpr uint8_t magic2Offset = 0x08;
    constexpr uint8_t magicLength = 0x04;

    if (filesize < magic2Offset + magicLength) return false;

    return memcmp(&fileBuffer[magic1Offset], magic1, magicLength) == 0 &&
           memcmp(&fileBuffer[magic2Offset], magic2, magicLength) == 0;
}

bool SoundManager::isFormatWsr(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr size_t magic1OffsetFromEnd = 0x20;

    if (filesize < magic1OffsetFromEnd) return false;

    constexpr char magic1[] = "WSRF";
    const size_t magic1Offset = filesize - magic1OffsetFromEnd;
    constexpr uint8_t magic1Length = 0x04;
    constexpr char magic2[] = "\xEA";
    const size_t magic2Offset = filesize - 0x10;
    constexpr uint8_t magic2Length = 0x01;

    return memcmp(&fileBuffer[magic1Offset], magic1, magic1Length) == 0 &&
           memcmp(&fileBuffer[magic2Offset], magic2, magic2Length) == 0;
}

bool SoundManager::isFormatV2M(const size_t filesize, const string_view &filename) {
    if (filesize < 480) return false;

    if (const string ext = filesystem::path(filename).extension().string();
        strcasecmp(ext.c_str(), ".V2") == 0 || strcasecmp(ext.c_str(), ".V2M") == 0) {
        return true;
    }

    return false;
}

bool SoundManager::isFormatJaytrax(const uint8_t *fileBuffer, const size_t filesize) {
    constexpr uint32_t verMugician1 = 0xD80;
    constexpr uint32_t verMugician2 = 0xD81;
    constexpr uint8_t verLength = 0x04;

    if (filesize < verLength) return false;

    uint32_t value;
    memcpy(&value, fileBuffer, verLength);

    return value == verMugician1 || value == verMugician2;
}
