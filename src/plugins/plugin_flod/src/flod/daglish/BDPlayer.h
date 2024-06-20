#ifndef BDPLAYER_H
#define BDPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseSample;
class BDVoice;
class BDSample;

class BDPlayer : public AmigaPlayer
{
public:
    BDPlayer(Amiga* amiga);
    ~BDPlayer();
    int load(void* data, unsigned long int _length);
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
	unsigned char *stream;
    unsigned long int length;


    void process();
    void initialize();
    void printData();
    unsigned char getSubsongsCount();
    void selectSong(unsigned char);
    std::vector<BaseSample*> getSamples();

};

#endif // BDPLAYER_H
