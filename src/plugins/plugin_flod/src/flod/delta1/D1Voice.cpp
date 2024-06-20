#include "D1Voice.h"
#include "BaseRow.h"

D1Voice::D1Voice(int index)
{
    this->index=index;
    next = 0;
}
void D1Voice::initialize()
{
    channel = 0;
    sample        = 0;
    trackPos      = 0;
    patternPos    = 0;
    status        = 0;
    speed         = 1;
    step          = 0;
    row           = 0;
    note          = 0;
    period        = 0;
    arpeggioPos   = 0;
    pitchBend     = 0;
    tableCtr      = 0;
    tablePos      = 0;
    vibratoCtr    = 0;
    vibratoDir    = 0;
    vibratoPos    = 0;
    vibratoPeriod = 0;
    volume        = 0;
    attackCtr     = 0;
    decayCtr      = 0;
    releaseCtr    = 0;
    sustain       = 1;
}
