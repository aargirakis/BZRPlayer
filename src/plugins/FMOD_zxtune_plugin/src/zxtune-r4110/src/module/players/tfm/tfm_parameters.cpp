/**
* 
* @file
*
* @brief  TFM parameters helpers implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "tfm_parameters.h"
//common includes
#include <make_ptr.h>
//library includes
#include <core/core_parameters.h>
#include <sound/render_params.h>
//std includes
#include <utility>

namespace Module
{
namespace TFM
{
  class ChipParameters : public Devices::TFM::ChipParameters
  {
  public:
    explicit ChipParameters(Parameters::Accessor::Ptr params)
      : Params(params)
      , SoundParams(Sound::RenderParameters::Create(params))
    {
    }

    uint_t Version() const override
    {
      return Params->Version();
    }

    uint64_t ClockFreq() const override
    {
      Parameters::IntType val = Parameters::ZXTune::Core::FM::CLOCKRATE_DEFAULT;
      Params->FindValue(Parameters::ZXTune::Core::FM::CLOCKRATE, val);
      return val;
    }

    uint_t SoundFreq() const override
    {
      return SoundParams->SoundFreq();
    }
  private:
    const Parameters::Accessor::Ptr Params;
    const Sound::RenderParameters::Ptr SoundParams;
  };

  Devices::TFM::ChipParameters::Ptr CreateChipParameters(Parameters::Accessor::Ptr params)
  {
    return MakePtr<ChipParameters>(params);
  }
}
}
