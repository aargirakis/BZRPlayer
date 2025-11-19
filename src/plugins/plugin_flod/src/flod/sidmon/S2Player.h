#ifndef S2PLAYER_H
#define S2PLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class BaseStep;
class BaseRow;
class S2Sample;
class S2Voice;
class S2Instrument;

class S2Player : public AmigaPlayer
{
public:
    S2Player(Amiga* amiga);
    ~S2Player();
    int load(void* data, unsigned long int _length);

private:
    vector<BaseStep*> tracks;
    vector<BaseRow*> patterns;
    vector<S2Instrument*> instruments;
    vector<S2Sample*> samples;
    vector<signed char> arpeggios;
    vector<signed char> vibratos;
    vector<unsigned char> waves;
    int length;
    int speedDef;
    vector<S2Voice*> voices;
    int trackPos;
    int patternPos;
    int patternLen;
    vector<unsigned char> arpeggioFx;
    int arpeggioPos;


    static const int PERIODS[73];

    void process();
    void initialize();
    void printData();
    vector<BaseSample*> getSamples();
};

#endif // S2PLAYER_H
