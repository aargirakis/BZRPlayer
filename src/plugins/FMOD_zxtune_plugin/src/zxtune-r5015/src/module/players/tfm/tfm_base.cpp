/**
 *
 * @file
 *
 * @brief  TFM-based chiptunes common functionality implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "module/players/tfm/tfm_base.h"
// common includes
#include <make_ptr.h>
// library includes
#include <sound/loop.h>
// std includes
#include <utility>

namespace Module
{
  class TFMRenderer : public Renderer
  {
  public:
    TFMRenderer(Time::Microseconds frameDuration, TFM::DataIterator::Ptr iterator, Devices::TFM::Chip::Ptr device)
      : Iterator(std::move(iterator))
      , Device(std::move(device))
      , FrameDuration(frameDuration)
    {}

    State::Ptr GetState() const override
    {
      return Iterator->GetStateObserver();
    }

    Sound::Chunk Render(const Sound::LoopParameters& looped) override
    {
      if (!Iterator->IsValid())
      {
        return {};
      }
      TransferChunk();
      Iterator->NextFrame(looped);
      LastChunk.TimeStamp += FrameDuration;
      return Device->RenderTill(LastChunk.TimeStamp);
    }

    void Reset() override
    {
      Iterator->Reset();
      Device->Reset();
      LastChunk.TimeStamp = {};
    }

    void SetPosition(Time::AtMillisecond request) override
    {
      const auto state = GetState();
      if (request < state->At())
      {
        Iterator->Reset();
        Device->Reset();
        LastChunk.TimeStamp = {};
      }
      while (state->At() < request && Iterator->IsValid())
      {
        TransferChunk();
        Iterator->NextFrame({});
      }
    }

  private:
    void TransferChunk()
    {
      Iterator->GetData(LastChunk.Data);
      Device->RenderData(LastChunk);
    }

  private:
    const TFM::DataIterator::Ptr Iterator;
    const Devices::TFM::Chip::Ptr Device;
    const Time::Duration<Devices::TFM::TimeUnit> FrameDuration;
    Devices::TFM::DataChunk LastChunk;
  };
}  // namespace Module

namespace Module
{
  namespace TFM
  {
    Renderer::Ptr CreateRenderer(Time::Microseconds frameDuration, DataIterator::Ptr iterator,
                                 Devices::TFM::Chip::Ptr device)
    {
      return MakePtr<TFMRenderer>(frameDuration, std::move(iterator), std::move(device));
    }
  }  // namespace TFM
}  // namespace Module
