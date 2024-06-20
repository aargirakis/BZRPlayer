#include "PTVoice.h"
#include "BaseSample.h"

PTVoice::PTVoice(int index)
{
    this->index = index;
      enabled      = 0;
      loopCtr      = 0;
      loopPos      = 0;
      step         = 0;
      period       = 0;
      effect       = 0;
      param        = 0;
      volume       = 0;
      pointer      = 0;
      length       = 0;
      loopPtr      = 0;
      repeat       = 0;
      finetune     = 0;
      offset       = 0;
      portaDir     = 0;
      portaPeriod  = 0;
      portaSpeed   = 0;
      glissando    = 0;
      tremoloParam = 0;
      tremoloPos   = 0;
      tremoloWave  = 0;
      vibratoParam = 0;
      vibratoPos   = 0;
      vibratoWave  = 0;
      funkPos      = 0;
      funkSpeed    = 0;
      funkWave     = 0;
	  next = 0;
}
void PTVoice::initialize()
{
      channel      = 0;
      sample       = 0;
      enabled      = 0;
      loopCtr      = 0;
      loopPos      = 0;
      step         = 0;
      period       = 0;
      effect       = 0;
      param        = 0;
      volume       = 0;
      pointer      = 0;
      length       = 0;
      loopPtr      = 0;
      repeat       = 0;
      finetune     = 0;
      offset       = 0;
      portaDir     = 0;
      portaPeriod  = 0;
      portaSpeed   = 0;
      glissando    = 0;
      tremoloParam = 0;
      tremoloPos   = 0;
      tremoloWave  = 0;
      vibratoParam = 0;
      vibratoPos   = 0;
      vibratoWave  = 0;
      funkPos      = 0;
      funkSpeed    = 0;
      funkWave     = 0;
}
