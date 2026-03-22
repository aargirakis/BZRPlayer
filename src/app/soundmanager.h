#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <QString>
#include "info.h"
#include "fmod_errors.h"

using namespace std;

class SoundManager
{
public:
    static SoundManager& getInstance()
    {
        static SoundManager instance;
        return instance;
    }

    Info* info;
    void Init(int outputDeviceProvided, const QString &outputFilenameProvided);
    void loadPluginChain();
    void loadPlugin(const string_view &filename, int priority);
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
    bool loadSound(const QString &filename, Info* infoProvided);
    static const char* getFmodSoundTypeName(FMOD_SOUND_TYPE type);
    void setReverbEnabled(bool);
    void setReverbPreset(const QString& preset);
    void setNormalizeEnabled(bool) const;
    void setNormalizeFadeTime(int) const;
    void setNormalizeMaxAmp(int) const;
    void setNormalizeThreshold(int) const;
    void setFrequencyByMultiplier(float) const;
    float getNominalFrequency() const;
    int getSoundData(unsigned int channelProvided);
    int getNumTags() const;
    FMOD_RESULT getTag(const char* name, int index, FMOD_TAG* tag) const;

private:
    SoundManager()
    {
    }

    FMOD_SYSTEM* system;
    FMOD_SOUND* sound;
    FMOD_CHANNEL* channel = nullptr;
    FMOD_RESULT result;
    FMOD_CHANNELGROUP* channelGroup;
    FMOD_DSP* dspNormalizer;
    FMOD_DSP* dspReverb;
    FMOD_DSP* dspFft;
    FMOD_REVERB_PROPERTIES currentReverbPreset;
    int currentDevice;
    float nominalFrequency;
    unsigned int mutedChannelsMask;
    QString mutedChannelsMaskString;
};

#endif // SOUNDMANAGER_H
