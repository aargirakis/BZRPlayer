#ifndef AMIGAPLAYER_H
#define AMIGAPLAYER_H
#include <string>
#include "Amiga.h"

using namespace std;

class BaseSample;
class BaseRow;

class AmigaPlayer
{
public:
    AmigaPlayer(Amiga* amiga);
    virtual ~AmigaPlayer();
    void setFrequency(int freq);
    int getFrequency();
    Amiga* amiga;
    string m_title;
    string format;
    int loopSong;

    virtual void setNTSC(bool value);
    void stereoSeparation(double value);
    void setVolume(double value);
    void mute(int index);
    int play();
    void pause();
    void stop();
    void mixer(void* _stream, unsigned long int length);
    int getChannels();
    int getVersion();
    double getVolume();
    virtual void process();
    virtual void initialize();
    virtual void reset();
    virtual int load(void* data, unsigned int length);
    virtual int load(void* data, unsigned int length, const char* filename);
    virtual unsigned int getCurrentRow();
    virtual unsigned int getCurrentPattern();
    virtual void getModRows(vector<BaseRow*>&);
    virtual vector<BaseSample*> getSamples();
    virtual bool getTitle(string& title);
    virtual unsigned char getSubsongsCount();
    virtual void setVersion(int version);

protected:
    //    Sound* sound;
    //    SoundChannel* soundChan;
    int soundPos;
    int ntsc;
    int speed;
    int tick;
    int sampleRate;
    unsigned short tempo;
    vector<BaseSample*> samples;
    double m_volume;
    bool m_ntsc;
    int m_channels;
    int m_flags;
    int m_mute;
    int m_version;
    int m_variant;
    int m_songNumber;
    int m_totalSongs;
};

#endif // AMIGAPLAYER_H
