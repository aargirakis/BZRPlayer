#include "D2Voice.h"
#include "BaseRow.h"
#include "D2Sample.h"

D2Voice::D2Voice(int index)
{
    this->index = index;
    next = 0;
}
void D2Voice::initialize()
{
    channel        = 0;
    sample         = 0;
    trackPtr       = 0;
    trackPos       = 0;
    trackLen       = 0;
    patternPos     = 0;
    restart        = 0;
    step           = 0;
    row            = 0;
    note           = 0;
    period         = 0;
    finalPeriod    = 0;
    arpeggioPtr    = 0;
    arpeggioPos    = 0;
    pitchBend      = 0;
    portamento     = 0;
    tableCtr       = 0;
    tablePos       = 0;
    vibratoCtr     = 0;
    vibratoDir     = 0;
    vibratoPos     = 0;
    vibratoPeriod  = 0;
    vibratoSustain = 0;
    volume         = 0;
    volumeMax      = 63;
    volumePos      = 0;
    volumeSustain  = 0;
}
