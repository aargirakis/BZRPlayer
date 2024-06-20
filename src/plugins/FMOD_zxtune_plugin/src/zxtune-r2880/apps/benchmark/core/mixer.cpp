/**
* 
* @file
*
* @brief  Mixer test implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "mixer.h"
#include "timer.h"
//library includes
#include <sound/matrix_mixer.h>

namespace Benchmark
{
  namespace Mixer
  {
    template<unsigned Channels>
    double Test(const Time::Milliseconds& duration, uint_t soundFreq)
    {
      const typename Sound::FixedChannelsMixer<Channels>::Ptr mixer = Sound::FixedChannelsMatrixMixer<Channels>::Create();

      typename Sound::MultichannelSample<Channels>::Type input;
      const Timer timer;
      const uint_t totalFrames = uint64_t(duration.Get()) * soundFreq / duration.PER_SECOND;
      for (uint_t frame = 0; frame != totalFrames; ++frame)
      {
        mixer->ApplyData(input);
      }
      const Time::Nanoseconds elapsed = timer.Elapsed<Time::Nanoseconds>();
      const Time::Nanoseconds emulated(duration);
      return double(emulated.Get()) / elapsed.Get();
    }

    double Test(uint_t channels, const Time::Milliseconds& duration, uint_t soundFreq)
    {
      switch (channels)
      {
      case 1:
        return Test<1>(duration, soundFreq);
      case 2:
        return Test<2>(duration, soundFreq);
      case 3:
        return Test<3>(duration, soundFreq);
      case 4:
        return Test<4>(duration, soundFreq);
      default:
        return 0;
      }
    }
  }
}
