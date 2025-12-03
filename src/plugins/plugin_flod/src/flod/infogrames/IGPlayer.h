#ifndef IGPLAYER_H
#define IGPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class BaseStep;
class BaseRow;
class BaseSample;
class IGVoice;
class IGBlock;

class IGPlayer : public AmigaPlayer {
public:
    IGPlayer(Amiga *amiga);

    ~IGPlayer();

    int load(void *data, unsigned long int length, const char *filename);

private:
    vector<int> comData;
    vector<int> perData;
    vector<int> volData;
    vector<IGVoice *> voices;
    vector<BaseSample *> samples;
    int irqtime;
    int complete;

    int tune(IGBlock *block, vector<int> data, int value);


    static const int PERIODS[102];
    static const int TICKS[12];

    void process();

    void initialize();

    void printData();

    vector<BaseSample *> getSamples();
};

#endif // IGPLAYER_H