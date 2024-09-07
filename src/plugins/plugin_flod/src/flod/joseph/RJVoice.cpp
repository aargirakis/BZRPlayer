#include "RJVoice.h"

RJVoice::RJVoice(int index)
{
    this->index = index;
    next = 0;
}
void RJVoice::initialize()
{
      channel      = 0;
      sample       = 0;
      active       = 0;
      enabled      = 0;
      trackPos     = 0;
      patternPos   = 0;
      speed1       = 6;
      speed2       = 0;
      tick1        = 1;
      tick2        = 1;
      note         = 0;
      period       = 0;
      periodMod    = 0;
      periodPos    = 0;
      volume       = 0;
      volumePos    = 0;
      volumeScale  = 0;
      portaCounter = 0;
      portaPeriod  = 0;
      portaStep    = 0;
      envelPos     = 0;
      envelStep    = 0;
      envelScale   = 0;
      envelStart   = 0;
      envelEnd1    = 0;
      envelEnd2    = 0;
      envelVolume  = 0;
}
