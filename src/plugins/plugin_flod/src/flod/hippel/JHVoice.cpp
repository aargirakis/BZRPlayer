#include "JHVoice.h"

JHVoice::JHVoice(int index)
{
    this->index = index;
    next = 0;
}
void JHVoice::initialize()
{
      channel     = 0;
      enabled     = 0;
      cosoCtr = 0;
      cosoSpeed   = 0;
      trackPtr    = 0;
      trackPos    = 12;
      trackTrans = 0;
      patternPtr  = 0;
      patternPos  = 0;
      freqsPtr   = 0;
      freqsPos   = 0;
      volsPtr   = 0;
      volsPos   = 0;
      sample      = -1;
      loopPtr     = 0;
      repeat      = 0;
      tick        = 0;
      note        = 0;
      transpose   = 0;
      info        = 0;
      infoPrev    = 0;
      volume      = 0;
      volCtr  = 1;
      volSpeed    = 1;
      volSustain  = 0;
      volTrans   = 0;
      volFade     = 100;
      portaDelta  = 0;
      vibrato     = 0;
      vibDelay    = 0;
      vibDelta    = 0;
      vibDepth    = 0;
      vibSpeed    = 0;
      slide       = 0;
      sldActive   = 0;
      sldDone     = 0;
      sldCtr  = 0;
      sldSpeed    = 0;
      sldDelta    = 0;
      sldPointer  = 0;
      sldLen      = 0;
      sldEnd      = 0;
      sldLoopPtr  = 0;
}
