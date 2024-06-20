#include "FXVoice.h"
#include <iostream>
#include "MyEndian.h"

FXVoice::FXVoice(int index)
{
    this->index = index;
    next = 0;
}

void FXVoice::initialize()
{
    channel     = 0;
    sample      = 0;
    period      = 0;
    effect      = 0;
    param       = 0;
    volume      = 0;
    last        = 0;
    slideCtr    = 0;
    slideDir    = 0;
    slideParam  = 0;
    slidePeriod = 0;
    slideSpeed  = 0;
    stepPeriod  = 0;
    stepSpeed   = 0;
    stepWanted  = 0;
}
