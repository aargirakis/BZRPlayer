#ifndef FESample_H
#define FESample_H
#include "BaseSample.h"
#include <vector>

class FESample : public BaseSample
{
    friend class FEPlayer;

public:
    FESample();

private:
    signed char type;
    int synchro;
    int envelopeVol;
    int attackSpeed;
    int attackVol;
    int decaySpeed;
    int decayVol;
    int sustainTime;
    int releaseSpeed;
    int releaseVol;
    std::vector<signed char> arpeggio;
    int arpeggioLimit;
    int arpeggioSpeed;
    int vibratoDelay;
    int vibratoDepth;
    int vibratoSpeed;
    int pulseCounter;
    int pulseDelay;
    int pulsePosL;
    int pulsePosH;
    int pulseSpeed;
    signed char pulseRateNeg;
    int pulseRatePos;
    int blendCounter;
    int blendDelay;
    int blendRate;
};

#endif // FESample_H
