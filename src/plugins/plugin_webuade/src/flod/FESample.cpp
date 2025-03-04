#include "FESample.h"

FESample::FESample()
{
    arpeggio = std::vector<signed char>(16);
    type = 0;
    synchro = 0;
    envelopeVol = 0;
    attackSpeed = 0;
    attackVol = 0;
    decaySpeed = 0;
    decayVol = 0;
    sustainTime = 0;
    releaseSpeed = 0;
    releaseVol = 0;
    arpeggioLimit = 0;
    arpeggioSpeed = 0;
    vibratoDelay = 0;
    vibratoDepth = 0;
    vibratoSpeed = 0;
    pulseCounter = 0;
    pulseDelay = 0;
    pulsePosL = 0;
    pulsePosH = 0;
    pulseSpeed = 0;
    pulseRateNeg = 0;
    pulseRatePos = 0;
    blendCounter = 0;
    blendDelay = 0;
    blendRate = 0;
}
