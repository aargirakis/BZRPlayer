#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QString>
#include "info.h"
#include "fmod_errors.h"

using namespace std;

class SoundManager {
public:
    static SoundManager &getInstance() {
        static SoundManager instance;
        return instance;
    }

    Info *info;

    void Init(int outputDeviceProvided, const QString &filePathProvided);

    void loadPluginChain();

    void loadPlugin(const string &pluginFilename, int priority);

    static void checkFmodError(FMOD_RESULT result);

    static void checkFmodError(FMOD_RESULT result, const QString &msg);

    void stop() const;

    bool isPlaying() const;

    bool isWavWriterDeviceSelected() const;

    void playAudio(bool startPaused);

    void release() const;

    void shutdown() const;

    void muteChannels(unsigned int mask, const QString &maskStr);

    bool isChannelMuted(unsigned int channelProvided) const;

    void pause(bool pause) const;

    bool isPaused() const;

    unsigned int getPosition(FMOD_TIMEUNIT) const;

    unsigned int getLength(FMOD_TIMEUNIT timeUnit) const;

    void setPosition(unsigned int positon, FMOD_TIMEUNIT timeUnit) const;

    void setVolume(float volume) const;

    void setMute(bool mute) const;

    bool loadSound(const QString &filePath, Info *infoProvided);

    static pair<uint8_t *, size_t> mapFile(const QString &fileToMap);

    static void unmapFile(uint8_t *fileMapped, size_t filesize, const string_view &filePath);

    static const char *getFmodSoundTypeName(FMOD_SOUND_TYPE type);

    static bool isFormatMidi(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatSidOrMusStr(const uint8_t *fileBuffer, size_t filesize, const string_view &filename,
                                    bool &isSid);

    static bool isFormatFarandole(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatRiff(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatProtrekkr(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatAhxOrHvl(const uint8_t *fileBuffer, size_t filesize, bool &isAhx);

    static bool isFormatBPSoundMon1(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatFurOrDfmOrZlib(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatPsf(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatKlystron(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatOrganya1(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatSunVox(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatSc68(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatSndh(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatPac(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatWsr(const uint8_t *fileBuffer, size_t filesize);

    static bool isFormatV2M(size_t filesize, const string_view &filename);

    static bool isFormatJaytrax(const uint8_t *fileBuffer, size_t filesize);

    void setReverbEnabled(bool);

    void setReverbPreset(const QString &preset);

    void setNormalizeEnabled(bool) const;

    void setNormalizeFadeTime(int) const;

    void setNormalizeMaxAmp(int) const;

    void setNormalizeThreshold(int) const;

    void setFrequencyByMultiplier(float) const;

    float getNominalFrequency() const;

    int getSoundData(unsigned int channelProvided);

    int getNumTags() const;

    FMOD_RESULT getTag(const char *name, int index, FMOD_TAG *tag) const;

private:
    SoundManager() {
    }

    FMOD_SYSTEM *system;
    FMOD_SOUND *sound;
    FMOD_CHANNEL *channel = nullptr;
    FMOD_RESULT result;
    FMOD_CHANNELGROUP *channelGroup;
    FMOD_DSP *dspNormalizer;
    FMOD_DSP *dspReverb;
    FMOD_DSP *dspFft;
    FMOD_REVERB_PROPERTIES currentReverbPreset;
    int currentDevice;
    float nominalFrequency;
    unsigned int mutedChannelsMask;
    QString mutedChannelsMaskString;
};

#endif // SOUNDMANAGER_H
