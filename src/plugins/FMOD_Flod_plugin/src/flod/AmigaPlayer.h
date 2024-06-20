#ifndef AMIGAPLAYER_H
#define AMIGAPLAYER_H
#include <string>
#include "Amiga.h"
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
    std::string m_title;
    std::string format;
    int loopSong;

    virtual void setNTSC(bool value);
    void stereoSeparation(double value);
    void setVolume(double value);
	void mute(int index);
    int play();
    void pause();
    void stop();
    void mixer(void *_stream, unsigned long int length);
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
    virtual void getModRows(std::vector<BaseRow*>&);
    virtual std::vector<BaseSample*> getSamples();
    virtual bool getTitle(std::string& title);
    virtual unsigned char getSubsongsCount();
    virtual void selectSong(unsigned char);
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
    std::vector<BaseSample*> samples;
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
