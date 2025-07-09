#include "FEVoice.h"

FEVoice::FEVoice(int index)
{
    this->index = index;
    next = 0;
}

void FEVoice::initialize()
{
    channel = 0;
    sample = 0;
    trackPos = 0;
    patternPos = 0;
    tick = 1;
    busy = 1;
    note = 0;
    period = 0;
    volume = 0;
    envelopePos = 0;
    sustainTime = 0;
    arpeggioPos = 0;
    arpeggioSpeed = 0;
    portamento = 0;
    portaCounter = 0;
    portaDelay = 0;
    portaFlag = 0;
    portaLimit = 0;
    portaNote = 0;
    portaPeriod = 0;
    portaSpeed = 0;
    vibrato = 0;
    vibratoDelay = 0;
    vibratoDepth = 0;
    vibratoFlag = 0;
    vibratoSpeed = 0;
    pulseCounter = 0;
    pulseDelay = 0;
    pulseDir = 0;
    pulsePos = 0;
    pulseSpeed = 0;
    blendCounter = 0;
    blendDelay = 0;
    blendDir = 0;
    blendPos = 0;
}
