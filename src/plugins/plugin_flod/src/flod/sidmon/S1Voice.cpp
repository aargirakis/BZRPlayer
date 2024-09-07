#include "S1Voice.h"

S1Voice::S1Voice(int index)
{
    this->index = index;
    next = 0;
}
void S1Voice::initialize()
{
    channel = 0;
    step           = 0;
    row            = 0;
    sample       =  0;
    samplePtr    = -1;
    sampleLen    =  0;
    note         =  0;
    noteTimer    =  0;
    period       =  0x9999;
    volume       =  0;
    bendTo       =  0;
    bendSpeed    =  0;
    arpeggioCtr  =  0;
    envelopeCtr  =  0;
    pitchCtr     =  0;
    pitchFallCtr =  0;
    sustainCtr   =  0;
    phaseTimer   =  0;
    phaseSpeed   =  0;
    wavePos      =  0;
    waveList     =  0;
    waveTimer    =  0;
    waitCtr      =  0;
}
