/**
* 
* @file
*
* @brief  Beeper support
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <make_ptr.h>
//local includes
#include <devices/beeper.h>
#include <devices/details/renderers.h>
#include <parameters/tracking_helper.h>
//std includes
#include <utility>

namespace Devices
{
namespace Beeper
{
  class BeeperPSG
  {
  public:
    void SetNewData(bool level)
    {
      Levels = level ? Sound::Sample(Sound::Sample::MAX / 2, Sound::Sample::MAX / 2) : Sound::Sample();
    }
    
    void Tick(uint_t /*ticks*/)
    {
    }

    Sound::Sample GetLevels() const
    {
      return Levels;
    }
    
    void Reset()
    {
      Levels = Sound::Sample();
    }
  private:
    Sound::Sample Levels;
  };

  class ChipImpl : public Chip
  {
  public:
    ChipImpl(ChipParameters::Ptr params, Sound::Receiver::Ptr target)
      : Params(std::move(params))
      , Target(std::move(target))
      , Renderer(Clock, PSG)
      , ClockFreq()
      , SoundFreq()
    {
    }
    
    void RenderData(const std::vector<DataChunk>& src) override
    {
      if (src.empty())
      {
        return;
      }
      const Stamp end = src.back().TimeStamp;
      if (Clock.HasSamplesBefore(end))
      {
        SynchronizeParameters();
        const uint_t samples = Clock.SamplesTill(end);
        Sound::ChunkBuilder builder;
        builder.Reserve(samples);
        for (const auto& chunk : src)
        {
          Renderer.Render(chunk.TimeStamp, builder);
          PSG.SetNewData(chunk.Level);
        }
        Target->ApplyData(builder.CaptureResult());
        Target->Flush();
      }
      else
      {
        for (const auto& chunk : src)
        {
          PSG.SetNewData(chunk.Level);
        }
      }
    }

    void Reset() override
    {
      Params.Reset();
      PSG.Reset();
      Clock.Reset();
      ClockFreq = 0;
      SoundFreq = 0;
    }
  private:
    void SynchronizeParameters()
    {
      if (Params.IsChanged())
      {
        const uint64_t clkFreq = Params->ClockFreq();
        const uint_t sndFreq = Params->SoundFreq();
        if (clkFreq != ClockFreq || sndFreq != SoundFreq)
        {
          Clock.SetFrequency(clkFreq, sndFreq);
          Renderer.SetClockFrequency(clkFreq);
        }
      }
    }
  private:
    Parameters::TrackingHelper<ChipParameters> Params;
    const Sound::Receiver::Ptr Target;
    BeeperPSG PSG;
    Details::ClockSource<Stamp> Clock;
    Details::HQRenderer<Stamp, BeeperPSG> Renderer;
    uint64_t ClockFreq;
    uint_t SoundFreq;
  };

  Chip::Ptr CreateChip(ChipParameters::Ptr params, Sound::Receiver::Ptr target)
  {
    return MakePtr<ChipImpl>(params, target);
  }
}
}
