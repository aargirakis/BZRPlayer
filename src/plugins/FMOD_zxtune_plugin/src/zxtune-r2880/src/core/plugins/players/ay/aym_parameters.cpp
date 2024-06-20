/**
* 
* @file
*
* @brief  AYM parameters helpers implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "aym_parameters.h"
#include "freq_tables_internal.h"
//common includes
#include <error_tools.h>
//library includes
#include <core/core_parameters.h>
#include <devices/aym/chip.h>
#include <l10n/api.h>
#include <math/numeric.h>
#include <sound/matrix_mixer.h>
#include <sound/render_params.h>
#include <sound/sample.h>
//std includes
#include <cstring>
#include <numeric>
//boost includes
#include <boost/make_shared.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
//text includes
#include <core/text/core.h>

#define FILE_TAG 6972CAAF

namespace
{
  const L10n::TranslateFunctor translate = L10n::TranslateFunctor("core");
}

namespace Module
{
namespace AYM
{
  //duty-cycle related parameter: accumulate letters to bitmask functor
  inline uint_t LetterToMask(uint_t val, const Char letter)
  {
    static const Char LETTERS[] = {'A', 'B', 'C', 'N', 'E'};
    static const uint_t MASKS[] =
    {
      Devices::AYM::CHANNEL_MASK_A,
      Devices::AYM::CHANNEL_MASK_B,
      Devices::AYM::CHANNEL_MASK_C,
      Devices::AYM::CHANNEL_MASK_N,
      Devices::AYM::CHANNEL_MASK_E
    };
  BOOST_STATIC_ASSERT(sizeof(LETTERS) / sizeof(*LETTERS) == sizeof(MASKS) / sizeof(*MASKS));
    const std::ptrdiff_t pos = std::find(LETTERS, boost::end(LETTERS), letter) - LETTERS;
    if (pos == boost::size(LETTERS))
    {
      throw MakeFormattedError(THIS_LINE,
        translate("Invalid duty cycle mask item: '%1%'."), String(1, letter));
    }
    return val | MASKS[pos];
  }

  uint_t String2Mask(const String& str)
  {
    return std::accumulate(str.begin(), str.end(), uint_t(0), LetterToMask);
  }

  Devices::AYM::LayoutType String2Layout(const String& str)
  {
    if (str == Text::MODULE_LAYOUT_ABC)
    {
      return Devices::AYM::LAYOUT_ABC;
    }
    else if (str == Text::MODULE_LAYOUT_ACB)
    {
      return Devices::AYM::LAYOUT_ACB;
    }
    else if (str == Text::MODULE_LAYOUT_BAC)
    {
      return Devices::AYM::LAYOUT_BAC;
    }
    else if (str == Text::MODULE_LAYOUT_BCA)
    {
      return Devices::AYM::LAYOUT_BCA;
    }
    else if (str == Text::MODULE_LAYOUT_CBA)
    {
      return Devices::AYM::LAYOUT_CBA;
    }
    else if (str == Text::MODULE_LAYOUT_CAB)
    {
      return Devices::AYM::LAYOUT_CAB;
    }
    else if (str == Text::MODULE_LAYOUT_MONO)
    {
      return Devices::AYM::LAYOUT_MONO;
    }
    else
    {
      throw MakeFormattedError(THIS_LINE,
        translate("Invalid layout value (%1%)."), str);
    }
  }

  class ChipParametersImpl : public Devices::AYM::ChipParameters
  {
  public:
    explicit ChipParametersImpl(Parameters::Accessor::Ptr params)
      : Params(params)
      , SoundParams(Sound::RenderParameters::Create(params))
    {
    }

    virtual uint_t Version() const
    {
      return Params->Version();
    }

    virtual uint64_t ClockFreq() const
    {
      Parameters::IntType val = Parameters::ZXTune::Core::AYM::CLOCKRATE_DEFAULT;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::CLOCKRATE, val) &&
          !Math::InRange(val, Parameters::ZXTune::Core::AYM::CLOCKRATE_MIN, Parameters::ZXTune::Core::AYM::CLOCKRATE_MAX))
      {
        throw MakeFormattedError(THIS_LINE,
          translate("Invalid clock frequency (%1%)."), val);
      }
      return val;
    }

    virtual uint_t SoundFreq() const
    {
      return SoundParams->SoundFreq();
    }

    virtual Devices::AYM::ChipType Type() const
    {
      Parameters::IntType intVal = 0;
      Params->FindValue(Parameters::ZXTune::Core::AYM::TYPE, intVal);
      return static_cast<Devices::AYM::ChipType>(intVal);
    }

    virtual Devices::AYM::InterpolationType Interpolation() const
    {
      Parameters::IntType intVal = 0;
      Params->FindValue(Parameters::ZXTune::Core::AYM::INTERPOLATION, intVal);
      return static_cast<Devices::AYM::InterpolationType>(intVal);
    }

    virtual uint_t DutyCycleValue() const
    {
      Parameters::IntType intVal = 50;
      const bool found = Params->FindValue(Parameters::ZXTune::Core::AYM::DUTY_CYCLE, intVal);
      //duty cycle in percents should be in range 1..99 inc
      if (found && (intVal < 1 || intVal > 99))
      {
        throw MakeFormattedError(THIS_LINE,
          translate("Invalid duty cycle value (%1%)."), intVal);
      }
      return static_cast<uint_t>(intVal);
    }

    virtual uint_t DutyCycleMask() const
    {
      Parameters::IntType intVal = 0;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::DUTY_CYCLE_MASK, intVal))
      {
        return static_cast<uint_t>(intVal);
      }
      Parameters::StringType strVal;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::DUTY_CYCLE_MASK, strVal))
      {
        return String2Mask(strVal);
      }
      return 0;
    }

    virtual Devices::AYM::LayoutType Layout() const
    {
      Parameters::IntType intVal = Devices::AYM::LAYOUT_ABC;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::LAYOUT, intVal))
      {
        if (intVal < static_cast<int_t>(Devices::AYM::LAYOUT_ABC) ||
            intVal >= static_cast<int_t>(Devices::AYM::LAYOUT_LAST))
        {
          throw MakeFormattedError(THIS_LINE,
            translate("Invalid layout value (%1%)."), intVal);
        }
        return static_cast<Devices::AYM::LayoutType>(intVal);
      }
      Parameters::StringType strVal;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::LAYOUT, strVal))
      {
        return String2Layout(strVal);
      }
      return Devices::AYM::LAYOUT_ABC;
    }
  private:
    const Parameters::Accessor::Ptr Params;
    const Sound::RenderParameters::Ptr SoundParams;
  };

  class TrackParametersImpl : public TrackParameters
  {
  public:
    explicit TrackParametersImpl(Parameters::Accessor::Ptr params)
      : Params(params)
    {
    }

    virtual uint_t Version() const
    {
      return Params->Version();
    }

    virtual void FreqTable(FrequencyTable& table) const
    {
      Parameters::StringType newName;
      if (Params->FindValue(Parameters::ZXTune::Core::AYM::TABLE, newName))
      {
        GetFreqTable(newName, table);
      }
      else
      {
        Parameters::DataType newData;
        if (Params->FindValue(Parameters::ZXTune::Core::AYM::TABLE, newData))
        {
          // as dump
          if (newData.size() != table.size() * sizeof(table.front()))
          {
            throw MakeFormattedError(THIS_LINE,
              translate("Invalid frequency table size (%1%)."), newData.size());
          }
          std::memcpy(&table.front(), &newData.front(), newData.size());
        }
      }
    }
  private:
    const Parameters::Accessor::Ptr Params;
    const Sound::RenderParameters::Ptr Delegate;
  };

  Devices::AYM::ChipParameters::Ptr CreateChipParameters(Parameters::Accessor::Ptr params)
  {
    return boost::make_shared<ChipParametersImpl>(params);
  }

  TrackParameters::Ptr TrackParameters::Create(Parameters::Accessor::Ptr params)
  {
    return boost::make_shared<TrackParametersImpl>(params);
  }
}
}
