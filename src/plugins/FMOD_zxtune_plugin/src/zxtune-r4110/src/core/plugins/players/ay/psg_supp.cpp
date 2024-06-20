/**
* 
* @file
*
* @brief  PSG support plugin
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "aym_plugin.h"
#include "core/plugins/player_plugins_registrator.h"
//library includes
#include <formats/chiptune/aym/psg.h>
#include <module/players/aym/psg.h>

namespace ZXTune
{
  void RegisterPSGSupport(PlayerPluginsRegistrator& registrator)
  {
    //plugin attributes
    const Char ID[] = {'P', 'S', 'G', 0};

    const Formats::Chiptune::Decoder::Ptr decoder = Formats::Chiptune::CreatePSGDecoder();
    const Module::AYM::Factory::Ptr factory = Module::PSG::CreateFactory();
    const PlayerPlugin::Ptr plugin = CreateStreamPlayerPlugin(ID, decoder, factory);;
    registrator.RegisterPlugin(plugin);
  }
}
