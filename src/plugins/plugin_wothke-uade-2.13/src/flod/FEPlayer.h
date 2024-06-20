#ifndef FEPlayer_H
#define FEPlayer_H
#include <vector>
#include "AmigaPlayer.h"

class FESong;
class FEVoice;
class FESample;

class FEPlayer : public AmigaPlayer
{
public:

    FEPlayer(Amiga* amiga);
    ~FEPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();
private:

    unsigned int position;
    std::vector<FESong*> songs;
    std::vector<FESample*>samples;
    signed char *patterns;
    FESong* song;
    std::vector<FEVoice*> voices;
    int complete;
    int sampleFlag;




    static const int PERIODS[72];


};

#endif // FEPlayer_H
