/**
* 
* @file
*
* @brief  TurboSound containers support
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "core/plugins/enumerator.h"
#include "core/plugins/player_plugins_registrator.h"
#include "core/plugins/players/plugin.h"
#include "core/src/callback.h"
//common includes
#include <error.h>
#include <make_ptr.h>
//library includes
#include <core/module_open.h>
#include <core/plugin_attrs.h>
#include <debug/log.h>
#include <formats/chiptune/aym/turbosound.h>
#include <module/players/properties_helper.h>
#include <module/players/tracking.h>
#include <module/players/aym/aym_base.h>
#include <module/players/aym/turbosound.h>

namespace Module
{
namespace TS
{
  const Debug::Stream Dbg("Core::TSSupp");

  class DataBuilder : public Formats::Chiptune::TurboSound::Builder
  {
  public:
    DataBuilder(const Parameters::Accessor& params, const Binary::Container& data)
      : Params(params)
      , Data(data)
    {
    }

    void SetFirstSubmoduleLocation(std::size_t offset, std::size_t size) override
    {
      if (!(First = LoadChiptune(offset, size)))
      {
        Dbg("Failed to create first module");
      }
    }

    void SetSecondSubmoduleLocation(std::size_t offset, std::size_t size) override
    {
      if (!(Second = LoadChiptune(offset, size)))
      {
        Dbg("Failed to create second module");
      }
    }

    bool HasResult() const
    {
      return First && Second;
    }

    AYM::Chiptune::Ptr GetFirst() const
    {
      return First;
    }

    AYM::Chiptune::Ptr GetSecond() const
    {
      return Second;
    }
  private:
    AYM::Chiptune::Ptr LoadChiptune(std::size_t offset, std::size_t size) const
    {
      const Binary::Container::Ptr content = Data.GetSubcontainer(offset, size);
      if (const AYM::Holder::Ptr holder = std::dynamic_pointer_cast<const AYM::Holder>(Module::Open(Params, *content)))
      {
        return holder->GetChiptune();
      }
      else
      {
        return AYM::Chiptune::Ptr();
      }

    }
  private:
    const Parameters::Accessor& Params;
    const Binary::Container& Data;
    AYM::Chiptune::Ptr First;
    AYM::Chiptune::Ptr Second;
  };

  class Factory : public Module::Factory
  {
  public:
    explicit Factory(Formats::Chiptune::TurboSound::Decoder::Ptr decoder)
      : Decoder(std::move(decoder))
    {
    }

    Module::Holder::Ptr CreateModule(const Parameters::Accessor& params, const Binary::Container& data, Parameters::Container::Ptr properties) const override
    {
      try
      {
        DataBuilder dataBuilder(params, data);
        if (const Formats::Chiptune::Container::Ptr container = Decoder->Parse(data, dataBuilder))
        {
          if (dataBuilder.HasResult())
          {
            PropertiesHelper props(*properties);
            props.SetSource(*container);
            const TurboSound::Chiptune::Ptr chiptune = TurboSound::CreateChiptune(properties,
              dataBuilder.GetFirst(), dataBuilder.GetSecond());
            return TurboSound::CreateHolder(chiptune);
          }
        }
      }
      catch (const Error&)
      {
      }
      return Module::Holder::Ptr();
    }
  private:
    const Formats::Chiptune::TurboSound::Decoder::Ptr Decoder;
  };
}
}

namespace ZXTune
{
  void RegisterTSSupport(PlayerPluginsRegistrator& registrator)
  {
    //plugin attributes
    const Char ID[] = {'T', 'S', 0};
    const uint_t CAPS = Capabilities::Module::Type::MULTI | Capabilities::Module::Device::TURBOSOUND;

    const Formats::Chiptune::TurboSound::Decoder::Ptr decoder = Formats::Chiptune::TurboSound::CreateDecoder();
    const Module::Factory::Ptr factory = MakePtr<Module::TS::Factory>(decoder);
    const PlayerPlugin::Ptr plugin = CreatePlayerPlugin(ID, CAPS, decoder, factory);
    registrator.RegisterPlugin(plugin);
  }
}
