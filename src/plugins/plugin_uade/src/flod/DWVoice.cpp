#include "DWVoice.h"

DWVoice::DWVoice(int index, int bitflag)
{
    this->index = index;
    this->bitFlag = bitflag;
    next = 0;
    channel = 0;
    sample = 0;
    trackPtr = 0;
    trackPos = 0;
    patternPos = 0;
    freqsPtr = 0;
    freqsPos = 0;
    volsPtr = 0;
    volsPos = 0;
    volSpeed = 0;
    volCtr = 0;
    halve = 0;
    speed = 0;
    tick = 0;
    busy = 0;
    flags = 0;
    note = 0;
    period = 0;
    transpose = 0;
    portaDelay = 0;
    portaDelta = 0;
    portaSpeed = 0;
    vibrato = 0;
    vibDelta = 0;
    vibSpeed = 0;
    vibDepth = 0;
}

void DWVoice::initialize()
{
    channel = 0;
    sample = 0;
    trackPtr = 0;
    trackPos = 0;
    patternPos = 0;
    freqsPtr = 0;
    freqsPos = 0;
    volsPtr = 0;
    volsPos = 0;
    volSpeed = 0;
    volCtr = 0;
    halve = 0;
    speed = 0;
    tick = 1;
    busy = -1;
    flags = 0;
    note = 0;
    period = 0;
    transpose = 0;
    portaDelay = 0;
    portaDelta = 0;
    portaSpeed = 0;
    vibrato = 0;
    vibDelta = 0;
    vibSpeed = 0;
    vibDepth = 0;
}
