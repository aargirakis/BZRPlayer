#ifndef S1SAMPLE_H
#define S1SAMPLE_H
#include "BaseSample.h"
#include <vector>

class S1Sample : public BaseSample
{
    friend class S1Player;

public:
    S1Sample();

private:
    int waveform;
    std::vector<int> arpeggio;
    int attackSpeed;
    int attackMax;
    int decaySpeed;
    int decayMin;
    int sustain;
    int releaseSpeed;
    int releaseMin;
    int phaseShift;
    int phaseSpeed;
    signed char pitchFall;
};

#endif // S2SAMPLE_H
