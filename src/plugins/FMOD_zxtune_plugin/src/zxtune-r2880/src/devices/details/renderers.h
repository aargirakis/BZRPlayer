/**
* 
* @file
*
* @brief  PSG-based renderers implementation
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <devices/details/clock_source.h>
#include <sound/chunk_builder.h>
#include <sound/lpfilter.h>

namespace Devices
{
namespace Details
{
  template<class StampType>
  class Renderer
  {
  public:
    virtual ~Renderer() {}

    virtual void Render(StampType tillTime, uint_t samples, Sound::ChunkBuilder& target) = 0;
    virtual void Render(StampType tillTime, Sound::ChunkBuilder& target) = 0;
  };

  /*
  Time (1 uS)
  ||||||||||||||||||||||||||||||||||||||||||||||

  PSG (1773400 / 8 Hz, ~5uS)

  |    |    |    |    |    |    |    |    |    |    |    |    |    |

  Sound (44100 Hz, Clock.SamplePeriod = ~22uS)
  |                     |                     |

                    ->|-|<-- Clock.TicksDelta
                      Clock.NextSampleTime -->|

  */

  template<class StampType, class PSGType>
  class BaseRenderer : public Renderer<StampType>
  {
    typedef typename ClockSource<StampType>::FastStamp FastStamp;
  public:
    template<class ParameterType>
    BaseRenderer(ClockSource<StampType>& clock, ParameterType& psg)
      : Clock(clock)
      , PSG(psg)
    {
    }

    virtual void Render(StampType tillTime, uint_t samples, Sound::ChunkBuilder& target)
    {
      FinishPreviousSample(target);
      RenderMultipleSamples(samples - 1, target);
      StartNextSample(FastStamp(tillTime.Get()));
    }

    virtual void Render(StampType tillTime, Sound::ChunkBuilder& target)
    {
      const FastStamp end(tillTime.Get());
      if (Clock.HasSamplesBefore(end))
      {
        FinishPreviousSample(target);
        while (Clock.HasSamplesBefore(end))
        {
          RenderSingleSample(target);
        }
      }
      StartNextSample(end);
    }
  private:
    void FinishPreviousSample(Sound::ChunkBuilder& target)
    {
      if (const uint_t ticksPassed = Clock.AdvanceTimeToNextSample())
      {
        PSG.Tick(ticksPassed);
      }
      target.Add(PSG.GetLevels());
      Clock.UpdateNextSampleTime();
    }

    void RenderMultipleSamples(uint_t samples, Sound::ChunkBuilder& target)
    {
      for (uint_t count = samples; count != 0; --count)
      {
        const uint_t ticksPassed = Clock.AllocateSample();
        PSG.Tick(ticksPassed);
        target.Add(PSG.GetLevels());
      }
      Clock.CommitSamples(samples);
    }

    void RenderSingleSample(Sound::ChunkBuilder& target)
    {
      const uint_t ticksPassed = Clock.AdvanceSample();
      PSG.Tick(ticksPassed);
      target.Add(PSG.GetLevels());
    }

    void StartNextSample(FastStamp till)
    {
      if (const uint_t ticksPassed = Clock.AdvanceTime(till))
      {
        PSG.Tick(ticksPassed);
      }
    }
  protected:
    ClockSource<StampType>& Clock;
    PSGType PSG;
  };

  /*
    Simple decimation algorithm without any filtering
  */
  template<class PSGType>
  class LQWrapper
  {
  public:
    explicit LQWrapper(PSGType& delegate)
      : Delegate(delegate)
    {
    }

    void Tick(uint_t ticks)
    {
      Delegate.Tick(ticks);
    }

    Sound::Sample GetLevels() const
    {
      return Delegate.GetLevels();
    }
  private:
    PSGType& Delegate;
  };

  /*
    Simple decimation with post simple FIR filter (0.5, 0.5)
  */
  template<class PSGType>
  class MQWrapper
  { 
  public:
    explicit MQWrapper(PSGType& delegate)
      : Delegate(delegate)
    {
    }

    void Tick(uint_t ticks)
    {
      Delegate.Tick(ticks);
    }

    Sound::Sample GetLevels() const
    {
      const Sound::Sample curLevel = Delegate.GetLevels();
      return Interpolate(curLevel);
    }
  private:
    Sound::Sample Interpolate(Sound::Sample newLevel) const
    {
      const Sound::Sample out = Average(PrevLevel, newLevel);
      PrevLevel = newLevel;
      return out;
    }

    static Sound::Sample::Type Average(Sound::Sample::WideType first, Sound::Sample::WideType second)
    {
      return static_cast<Sound::Sample::Type>((first + second) / 2);
    }

    static Sound::Sample Average(Sound::Sample first, Sound::Sample second)
    {
      return Sound::Sample(Average(first.Left(), second.Left()), Average(first.Right(), second.Right()));
    }
  private:
    PSGType& Delegate;
    mutable Sound::Sample PrevLevel;
  };

  /*
    Decimation is performed after 2-order IIR LPF
    Cutoff freq of LPF should be less than Nyquist frequency of target signal
  */
  template<class PSGType>
  class HQWrapper
  {
  public:
    explicit HQWrapper(PSGType& delegate)
      : Delegate(delegate)
    {
    }

    void SetFrequency(uint64_t clockFreq, uint_t soundFreq)
    {
      Filter.SetParameters(clockFreq, soundFreq / 4);
    }

    void Tick(uint_t ticksPassed)
    {
      while (ticksPassed--)
      {
        Filter.Feed(Delegate.GetLevels());
        Delegate.Tick(1);
      }
    }

    Sound::Sample GetLevels() const
    {
      return Filter.Get();
    }
  private:
    PSGType& Delegate;
    Sound::LPFilter Filter;
  };   

  template<class StampType, class PSGType>
  class LQRenderer : public BaseRenderer<StampType, LQWrapper<PSGType> >
  {
    typedef BaseRenderer<StampType, LQWrapper<PSGType> > Parent;
  public:
    LQRenderer(ClockSource<StampType>& clock, PSGType& psg)
      : Parent(clock, psg)
    {
    }
  };

  template<class StampType, class PSGType>
  class MQRenderer : public BaseRenderer<StampType, MQWrapper<PSGType> >
  {
    typedef BaseRenderer<StampType, MQWrapper<PSGType> > Parent;
  public:
    MQRenderer(ClockSource<StampType>& clock, PSGType& psg)
      : Parent(clock, psg)
    {
    }
  private:
  };

  template<class StampType, class PSGType>
  class HQRenderer : public BaseRenderer<StampType, HQWrapper<PSGType> >
  {
    typedef BaseRenderer<StampType, HQWrapper<PSGType> > Parent;
  public:
    HQRenderer(ClockSource<StampType>& clock, PSGType& psg)
      : Parent(clock, psg)
    {
    }

    void SetFrequency(uint64_t clockFreq, uint_t soundFreq)
    {
      Parent::PSG.SetFrequency(clockFreq, soundFreq);
    }
  };
}
}
