#include "AmigaPlayer.h"
#include "Amiga.h"
#include "AmigaChannel.h"
#include <iostream>
#include <vector>
AmigaPlayer::AmigaPlayer(Amiga* amiga)
{
    this->amiga = amiga;
    this->amiga->player = this;
    //todo do not hardcode sample rate
    sampleRate = 44100;
    setNTSC(0);
    loopSong = 0;
    soundPos = 0.0;
    tempo    = 125;
    format="";
	m_volume = 1.0;
    m_channels = 4;
    m_flags = 0;
    m_mute=0;
    m_version = 0;
    m_variant = 0;
    m_songNumber = 0;
    m_totalSongs = 0;
    speed = 0;
}

AmigaPlayer::~AmigaPlayer()
{
    delete amiga;
}
void AmigaPlayer::setFrequency(int freq)
{
    sampleRate = freq;
}

int AmigaPlayer::getFrequency()
{
    return sampleRate;
}
double AmigaPlayer::getVolume()
{
    return m_volume;
}
void AmigaPlayer::setNTSC(bool value)
{
	m_ntsc = value;
    if (value) {
        amiga->clock = (double)3579545 / sampleRate;
        amiga->samplesTick = 735;
    } else {
        amiga->clock = (double)3546895 / sampleRate;
        amiga->samplesTick = 882;
    }

    ntsc = value;
}
void AmigaPlayer::stereoSeparation(double value)
{
    AmigaChannel* chan = amiga->channels[0];
    if (value < 0.0) value = 0.0; else if (value > 1.0) value = 1.0;

    do
	{
        chan->level = value * chan->panning;
	}
	while (chan = chan->next);
}
void AmigaPlayer::setVolume(double value)
{
    if (value < 0.0) value = 0.0; else if (value > 1.0) value = 1.0;
	
    amiga->master = (value / m_channels) * 0.015625;
}
void AmigaPlayer::mute(int index)
{
      if (index >= 0 && index < m_channels) {
        amiga->channels[index]->mute ^= 1;
        m_flags ^= (1 << index);
      } else {
        m_mute ^= 1;

        for (int i = 0; i < m_channels; ++i) {
          amiga->channels[i]->mute = m_mute | (m_flags & (1 << i));
        }
      }
}
int AmigaPlayer::getChannels()
{
    return m_channels;
}

int AmigaPlayer::load(void* data, unsigned int length, const char* filename)
{
  amiga->reset();
  m_version = 0;
  m_variant = 0;

  //stream.endian = "bigEndian";
  //stream.position = 0;
  return 0;
}

int AmigaPlayer::load(void* data, unsigned int length)
{
  amiga->reset();
  m_version = 0;
  m_variant = 0;
  //stream.endian = "bigEndian";
  //stream.position = 0;
  return 0;
}
int AmigaPlayer::getVersion()
{
    return m_version;
}

int AmigaPlayer::play()
{
    //if (version == 0) return 0;
    if (soundPos == 0.0) initialize();
    return 1;
}

void AmigaPlayer::pause()
{
//  if (version == 0 || !soundChan) return;
//  soundPos = soundChan->position;
//  soundChan->stop();
//  sound->removeEventListener(SampleDataEvent.SAMPLE_DATA, amiga->mixer);
}
void AmigaPlayer::stop()
{
//    if (version == 0) return;
//    if (soundChan) {
//      soundChan.stop();
//      sound.removeEventListener(SampleDataEvent.SAMPLE_DATA, amiga.mixer);
//    }
    soundPos = 0.0;
    reset();
}
std::vector<BaseSample*> AmigaPlayer::getSamples()
{
    return std::vector<BaseSample*>(0);
}
void AmigaPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    //return std::vector<AmigaRow*>(0);
}
bool AmigaPlayer::getTitle(std::string& title)
{
    title ="";
    return false;
}
unsigned int AmigaPlayer::getCurrentRow()
{
    return 0;
}
unsigned int AmigaPlayer::getCurrentPattern()
{
    return 0;
}
unsigned char AmigaPlayer::getSubsongsCount()
{
    return 0;
}
void AmigaPlayer::selectSong(unsigned char subsong)
{

}
void AmigaPlayer::initialize()
{
    amiga->setup();
    amiga->initialize();
    tick = 0;
}
void AmigaPlayer::process()
{
}
void AmigaPlayer::reset()
{

}
void AmigaPlayer::mixer(void *_stream, unsigned long int length)
{
    amiga->mixer(_stream,length);
}
void AmigaPlayer::setVersion(int version)
{

}
