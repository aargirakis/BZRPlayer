#include "JBVoice.h"

JBVoice::JBVoice(int index)
{
    this->index = index;
    next = 0;
    prev = 0;
}

void JBVoice::initialize()
{
    channel = 0;
    track = 0;
    trackLen = 0;
    trackLoop = 0;
    trackPos = 0;
    patternPos = 0;
    loopCounter = 0;
    loopPos = 0;
    flags = 0;
    state = 0;
    delay = 0;
    counter = 1;
    note = 0;
    sample1 = 0;
    sample2 = 0;
    volume = 0;
    volumeMod = 255;
    volCounter = 0;
    volPointer = 0;
    volPos = 0;
    periodMod = 0;
    slidePointer = 0;
    slidePos = 0;
    slideStep = 0;
    slideLimit = 0;
    slideValue = 0;
    portaCounter = 0;
    portaStep = 0;
    portaPeriod = 0;
}
