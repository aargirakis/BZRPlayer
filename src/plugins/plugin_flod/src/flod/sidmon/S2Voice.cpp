#include "S2Voice.h"
#include "BaseRow.h"
#include "BaseStep.h"

S2Voice::S2Voice(int index)
{
    this->index = index;
    next           = 0;
}
void S2Voice::initialize()
{
    channel        = 0;
    step           = 0;
    row            = 0;
    instr          = 0;
    sample         = 0;
    enabled        = 0;
    pattern        = 0;
    instrument     = 0;
    note           = 0;
    period         = 0;
    volume         = 0;
    original       = 0;
    adsrPos        = 0;
    sustainCtr     = 0;
    pitchBend      = 0;
    pitchBendCtr   = 0;
    noteSlideTo    = 0;
    noteSlideSpeed = 0;
    waveCtr        = 0;
    wavePos        = 0;
    arpeggioCtr    = 0;
    arpeggioPos    = 0;
    vibratoCtr     = 0;
    vibratoPos     = 0;
    speed          = 0;
}
