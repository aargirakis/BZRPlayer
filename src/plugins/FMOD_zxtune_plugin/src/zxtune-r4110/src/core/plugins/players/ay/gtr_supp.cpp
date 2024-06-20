/**
* 
* @file
*
* @brief  GlobalTracker support plugin
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "aym_plugin.h"
#include "core/plugins/player_plugins_registrator.h"
//library includes
#include <formats/chiptune/aym/globaltracker.h>
#include <module/players/aym/globaltracker.h>

namespace ZXTune
{
  void RegisterGTRSupport(PlayerPluginsRegistrator& registrator)
  {
    //plugin attributes
    const Char ID[] = {'G', 'T', 'R', 0};

    const Formats::Chiptune::Decoder::Ptr decoder = Formats::Chiptune::CreateGlobalTrackerDecoder();
    const Module::AYM::Factory::Ptr factory = Module::GlobalTracker::CreateFactory();
    const PlayerPlugin::Ptr plugin = CreateTrackPlayerPlugin(ID, decoder, factory);
    registrator.RegisterPlugin(plugin);
  }
}
