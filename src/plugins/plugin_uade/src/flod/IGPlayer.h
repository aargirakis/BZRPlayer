#ifndef IGPLAYER_H
#define IGPLAYER_H
#include <vector>
#include "AmigaPlayer.h"

class BaseStep;
class BaseRow;
class BaseSample;
class IGVoice;
class IGBlock;

class IGPlayer : public AmigaPlayer
{
public:
    IGPlayer(Amiga* amiga);
    ~IGPlayer();
    int load(void* data, unsigned long int length, const char* filename);
    std::vector<BaseSample*> getSamples();

private:
    std::vector<int> comData;
    std::vector<int> perData;
    std::vector<int> volData;
    std::vector<IGVoice*> voices;
    std::vector<BaseSample*> samples;
    int irqtime;
    int complete;
    int speed;

    int tune(IGBlock* block, std::vector<int> data, int value);


    static const int PERIODS[102];
    static const int TICKS[12];
};

#endif // IGPLAYER_H
