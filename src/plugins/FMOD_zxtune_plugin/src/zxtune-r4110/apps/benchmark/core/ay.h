/**
* 
* @file
*
* @brief  AY test interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <devices/aym/chip.h>
#include <time/stamp.h>

namespace Benchmark
{
  namespace AY
  {
    Devices::AYM::Chip::Ptr CreateDevice(uint64_t clockFreq, uint_t soundFreq, Devices::AYM::InterpolationType interpolate);
    double Test(Devices::AYM::Chip& dev, const Time::Milliseconds& duration, const Time::Microseconds& frameDuration);
  }
}
