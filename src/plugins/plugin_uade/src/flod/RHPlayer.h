#ifndef RHPLAYER_H
#define RHPLAYER_H
#include <vector>
#include "AmigaPlayer.h"

class RHSong;
class RHVoice;
class RHSample;

class RHPlayer : public AmigaPlayer
{
public:
    RHPlayer(Amiga* amiga);
    ~RHPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();

private:
    unsigned char* stream;
    std::vector<RHSong*> songs;
    std::vector<RHSample*> samples;
    RHSong* song;
    int periods;
    int vibrato;
    std::vector<RHVoice*> voices;
    void* data;
    int complete;
    int variant;
    unsigned int position;
};

#endif // RHPLAYER_H
