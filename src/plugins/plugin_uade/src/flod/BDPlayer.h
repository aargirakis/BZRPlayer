#ifndef BDPLAYER_H
#define BDPLAYER_H
#include <vector>
#include "AmigaPlayer.h"

class BaseSample;
class BDVoice;
class BDSample;

class BDPlayer : public AmigaPlayer
{
public:
    BDPlayer(Amiga* amiga);
    ~BDPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();

private:
    void setSample(BDVoice* voice, int counter);
    void fx(BDVoice* voice);
    std::vector<int> songs;
    std::vector<int> banks;
    int patterns;
    std::vector<BDVoice*> voices;
    std::vector<BDSample*> samples;

    int commands;
    int periods;
    int fadeStep;
    int complete;

    unsigned int position;
    unsigned char* stream;
    unsigned long int length;

    unsigned char getSubsongsCount();
    void selectSong(unsigned char);
};

#endif // BDPLAYER_H
