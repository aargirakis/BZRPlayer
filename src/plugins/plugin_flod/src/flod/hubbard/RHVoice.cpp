#include "RHVoice.h"

RHVoice::RHVoice(int index)
{
    this->index = index;
    next = 0;
}
void RHVoice::initialize()
{
      channel    = 0;
      sample     = 0;
      trackPtr   = 0;
      trackPos   = 0;
      patternPos = 0;
      tick       = 1;
      busy       = 1;
      flags      = 0;
      note       = 0;
      period     = 0;
      volume     = 0;
      portaSpeed = 0;
      vibratoPtr = 0;
      vibratoPos = 0;
      synthPos   = 0;
}
