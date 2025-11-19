#ifndef D2PLAYER_H
#define D2PLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

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
    vector<BaseStep*> tracks;
    vector<BaseRow*> patterns;
    vector<D2Sample*> samples;
    vector<int> data;
    vector<signed char> arpeggios;
    vector<D2Voice*> voices;
    unsigned int noise;
    static const int PERIODS[85];
    void process();
    void initialize();
    void printData();
    vector<BaseSample*> getSamples();
};

#endif // D2PLAYER_H
