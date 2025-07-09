#include "MGVoice.h"

MGVoice::MGVoice(int index)
{
    this->index = index;
    next = 0;
}

void MGVoice::initialize()
{
    sample = 0;
    step = 0;
    note = 0;
    period = 0;
    val1 = 0;
    val2 = 0;
    fperiod = 0;
    arpStep = 0;
    fxCtr = 0;
    pitch = 0;
    pitchCtr = 0;
    pitchStep = 0;
    portamento = 0;
    volume = 0;
    volCtr = 0;
    volStep = 0;
    mixMute = 1;
    mixPtr = 0;
    mixEnd = 0;
    mixSpeed = 0;
    mixStep = 0;
    mixVolume = 0;
}
