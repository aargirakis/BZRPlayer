#ifndef D2PLAYER_H
#define D2PLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseStep;
class BaseRow;
class D2Sample;
class D2Voice;

class D2Player : public AmigaPlayer
{
public:
    D2Player(Amiga* amiga);
    ~D2Player();
    int load(void* data, unsigned long int length);

private:
    std::vector<BaseStep*> tracks;
    std::vector<BaseRow*> patterns;
    std::vector<D2Sample*> samples;
    std::vector<int> data;
    std::vector<signed char> arpeggios;
    std::vector<D2Voice*> voices;
    unsigned int noise;
    static const int PERIODS[85];
    void process();
    void initialize();
    void printData();
    std::vector<BaseSample*> getSamples();
};

#endif // D2PLAYER_H
