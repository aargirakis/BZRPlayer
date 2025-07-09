#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include "fmod_errors.h"
#include "info.h"
#include <string>
#include <QString>

using namespace std;

class SoundManager
{
public:
    static SoundManager& getInstance()
    {
        static SoundManager instance;
        return instance;
    }

    Info* m_Info1;
    void Init(int device, QString outputFilename);
    void loadPlugin(string filename, int priority);
    void ERRCHECK(FMOD_RESULT result);
    void ERRCHECK(FMOD_RESULT result, QString string);
    void Stop();
    bool IsPlaying();

    void PlayAudio(bool startPaused);
    void Release();
    void ShutDown();
    void MuteChannels(unsigned int mask, QString maskStr);
    bool isChannelMuted(unsigned int channel);
    void Pause(bool pause);
    bool GetPaused();
    unsigned int GetPosition(FMOD_TIMEUNIT);
    unsigned int GetLength(FMOD_TIMEUNIT timeunit);
    void SetPosition(unsigned int positon, FMOD_TIMEUNIT timeunit);
    void SetVolume(float volume);
    void SetMute(bool mute);
    bool LoadSound(QString filename, bool isPlayModeRepeatSongEnabled);
    const char* getFMODSoundFormat(FMOD_SOUND* sound);
    void setReverbEnabled(bool);
    void setReverbPreset(QString preset);
    void setNormalizeEnabled(bool);
    void setNormalizeFadeTime(int);
    void setNormalizeMaxAmp(int);
    void setNormalizeThreshold(int);
    void SetFrequency(float);
    float GetFrequency();
    float getAudibility();
    int getSoundData(int channel);
    int getNumTags();
    FMOD_RESULT getTag(const char* name, int index, FMOD_TAG* tag);

private:
    SoundManager()
    {
    }

    FMOD_SYSTEM* system;
    FMOD_SOUND* soundPlay;
    FMOD_CHANNEL* channel = nullptr;
    FMOD_RESULT result;
    FMOD_CHANNELGROUP* channelgroup;
    FMOD_DSP* dspNormalizer;
    FMOD_DSP* dspReverb;
    FMOD_DSP* dspFFT;
    FMOD_REVERB_PROPERTIES currentReverbPreset;
    int currentDevice;
    float m_DefaultFrequency;
    unsigned int m_mutedChannelsMask;
    QString m_mutedChannelsMaskString;
};

#endif // SOUNDMANAGER_H
