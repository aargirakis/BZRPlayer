#include "HMVoice.h"
#include "AmigaRow.h"
#include "HMSample.h"

HMVoice::HMVoice(int index)
{
    this->index = index;
      enabled      = 0;
      period       = 0;
      effect       = 0;
      param        = 0;
      volume1      = 0;
      volume2      = 0;
      handler      = 0;
      portaDir     = 0;
      portaPeriod  = 0;
      portaSpeed   = 0;
      vibratoPos   = 0;
      vibratoSpeed = 0;
      wavePos      = 0;
	  next = 0;
}
void HMVoice::initialize()
{
      enabled      = 0;
      period       = 0;
      effect       = 0;
      param        = 0;
      volume1      = 0;
      volume2      = 0;
      handler      = 0;
      portaDir     = 0;
      portaPeriod  = 0;
      portaSpeed   = 0;
      vibratoPos   = 0;
      vibratoSpeed = 0;
      wavePos      = 0;
}
