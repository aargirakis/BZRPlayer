#ifndef S2PLAYER_H
#define S2PLAYER_H
#include <vector>
#include "AmigaPlayer.h"

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
    std::vector<BaseSample*> getSamples();

private:
    std::vector<BaseStep*> tracks;
    std::vector<BaseRow*> patterns;
    std::vector<S2Instrument*> instruments;
    std::vector<S2Sample*> samples;
    std::vector<signed char> arpeggios;
    std::vector<signed char> vibratos;
    std::vector<unsigned char> waves;
    int length;
    int speedDef;
    std::vector<S2Voice*> voices;
    int trackPos;
    int patternPos;
    int patternLen;
    std::vector<unsigned char> arpeggioFx;
    int arpeggioPos;


    static const int PERIODS[73];
};

#endif // S2PLAYER_H
