/**
* 
* @file
*
* @brief Components dialog implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "componentsdialog.h"
#include "componentsdialog.ui.h"
#include "ui/utils.h"
#include "supp/options.h"
//library includes
#include <core/plugin.h>
#include <core/plugin_attrs.h>
#include <io/provider.h>
#include <sound/backend_attrs.h>
#include <sound/service.h>
#include <strings/format.h>
//std includes
#include <utility>
//qt includes
#include <QtGui/QApplication>
#include <QtGui/QDialog>
//text includes
#include "text/text.h"

namespace
{
  QString Translate(const char* msg)
  {
    return QApplication::translate("ComponentsDialog", msg, nullptr, QApplication::UnicodeUTF8);
  }

  template<class T>
  QTreeWidgetItem* CreateTreeWidgetItem(T* parent, const char* title)
  {
    return new QTreeWidgetItem(parent, QStringList(Translate(title)));
  }

  void AddCapability(uint_t caps, uint_t mask, QTreeWidgetItem& root, const char* notation)
  {
    if (mask == (caps & mask))
    {
      CreateTreeWidgetItem(&root, notation);
    }
  }

  class PluginsTreeHelper
  {
  public:
    explicit PluginsTreeHelper(QTreeWidget& widget)
      : Widget(widget)
      , Players(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "Player plugins")))
      , Containers(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "Container plugins")))
    {
      FillPlayersByDevices();
      FillContainersByTypes();
    }

    void AddPlugin(const ZXTune::Plugin& plugin)
    {
      using namespace ZXTune::Capabilities::Category;
      switch (plugin.Capabilities() & MASK)
      {
      case MODULE:
        AddPlayerPlugin(plugin);
        break;
      case CONTAINER:
        AddContainerPlugin(plugin);
        break;
      default:
        assert(!"Unknown plugin");
        break;
      }
    }
  private:
    void FillPlayersByDevices()
    {
      using namespace ZXTune::Capabilities::Module::Device;
      PlayersByDeviceType[AY38910] = PlayersByDeviceType[TURBOSOUND] = CreateTreeWidgetItem(Players, "AY-3-8910/YM2149F/Turbosound");
      PlayersByDeviceType[DAC] = CreateTreeWidgetItem(Players, "DAC");
      PlayersByDeviceType[YM2203] = PlayersByDeviceType[TURBOFM] = CreateTreeWidgetItem(Players, "YM2203/TurboFM");
      PlayersByDeviceType[SAA1099] = CreateTreeWidgetItem(Players, "SAA1099");
      PlayersByDeviceType[MOS6581] = CreateTreeWidgetItem(Players, "MOS6581/SID");
      PlayersByDeviceType[SPC700] = CreateTreeWidgetItem(Players, "SPC700");
      PlayersByDeviceType[RP2A0X] = CreateTreeWidgetItem(Players, "RP2A03/RP2A07");
      PlayersByDeviceType[LR35902] = CreateTreeWidgetItem(Players, "LR35902/PAPU");
      PlayersByDeviceType[CO12294] = CreateTreeWidgetItem(Players, "CO12294/POKEY");
      PlayersByDeviceType[HUC6270] = CreateTreeWidgetItem(Players, "HuC6270");
      PlayersByDeviceType[MULTI] = CreateTreeWidgetItem(Players, QT_TRANSLATE_NOOP("ComponentsDialog", "Multidevice"));
    }
    
    void FillContainersByTypes()
    {
      using namespace ZXTune::Capabilities::Container::Type;
      ContainersByType.resize(MASK);
      ContainersByType[ARCHIVE] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Archive"));
      ContainersByType[COMPRESSOR] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Compressor"));
      ContainersByType[SNAPSHOT] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Snapshot"));
      ContainersByType[DISKIMAGE] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Disk image"));
      ContainersByType[DECOMPILER] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Decompiler"));
      ContainersByType[MULTITRACK] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Multitrack"));
      ContainersByType[SCANER] = CreateTreeWidgetItem(Containers, QT_TRANSLATE_NOOP("ComponentsDialog", "Data scanner"));
    }
    
    void AddPlayerPlugin(const ZXTune::Plugin& plugin)
    {
      const uint_t deviceType = plugin.Capabilities() & ZXTune::Capabilities::Module::Device::MASK;
      for (std::map<uint_t, QTreeWidgetItem*>::const_iterator it = PlayersByDeviceType.begin(), lim = PlayersByDeviceType.end(); it != lim; ++it)
      {
        if (0 != (deviceType & it->first))
        {
          AddPlayerPluginItem(plugin, *it->second);
          return;
        }
      }
      assert(!"Unknown player plugin");
    }

    void AddPlayerPluginItem(const ZXTune::Plugin& plugin, QTreeWidgetItem& root)
    {
      using namespace ZXTune::Capabilities::Module;
      //root
      const uint_t caps = plugin.Capabilities();
      const String& title = Strings::Format("[%s] %s", plugin.Id(), plugin.Description());
      QTreeWidgetItem* const pluginItem = new QTreeWidgetItem(&root, QStringList(ToQString(title)));
      FillModuleType(caps & Type::MASK, *pluginItem);
      //conversion
      if (const uint_t convCaps = caps & Conversion::MASK)
      {
        QTreeWidgetItem* const conversionItem = CreateTreeWidgetItem(pluginItem, QT_TRANSLATE_NOOP("ComponentsDialog", "Conversion targets"));
        FillConversionCapabilities(convCaps, *conversionItem);
      }
      //traits
      if (const uint_t traits = caps & Traits::MASK)
      {
        FillModuleTraits(traits, *pluginItem);
      }
    }
    
    void FillModuleType(uint_t type, QTreeWidgetItem& root)
    {
      using namespace ZXTune::Capabilities::Module::Type;
      AddCapability(1 << type, 1 << TRACK, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Track structure"));
      AddCapability(1 << type, 1 << STREAM, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Stream structure"));
      AddCapability(1 << type, 1 << MEMORYDUMP, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Memory dump structure"));
      AddCapability(1 << type, 1 << MULTI, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Multistructure"));
    }

    void FillConversionCapabilities(uint_t caps, QTreeWidgetItem& root)
    {
      using namespace ZXTune::Capabilities::Module::Conversion;
      AddCapability(caps, OUT, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Streamed .out format"));
      AddCapability(caps, PSG, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Streamed .psg format"));
      AddCapability(caps, YM, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Streamed .ym format"));
      AddCapability(caps, ZX50, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Streamed .zx50 format"));
      AddCapability(caps, TXT, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Vortex .txt format"));
      AddCapability(caps, AYDUMP, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Raw aydump format"));
      AddCapability(caps, FYM, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Compressed .fym format"));
    }
    
    void FillModuleTraits(uint_t traits, QTreeWidgetItem& root)
    {
      using namespace ZXTune::Capabilities::Module::Traits;
      AddCapability(traits, MULTIFILE, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Multiple files layout"));
    }

    void AddContainerPlugin(const ZXTune::Plugin& plugin)
    {
      const uint_t containerType = plugin.Capabilities() & ZXTune::Capabilities::Container::Type::MASK;
      if (QTreeWidgetItem* item = ContainersByType[containerType])
      {
        AddContainerPluginItem(plugin, *item);
      }
      else
      {
        assert(!"Unknown container plugin");
      }
    }

    void AddContainerPluginItem(const ZXTune::Plugin& plugin, QTreeWidgetItem& root)
    {
      const uint_t caps = plugin.Capabilities();
      const String& description = plugin.Description();

      //root
      QTreeWidgetItem* const pluginItem = new QTreeWidgetItem(&root, QStringList(ToQString(description)));
      //capabilities
      FillContainerTraits(caps, *pluginItem);
    }

    void FillContainerTraits(uint_t caps, QTreeWidgetItem& root) const
    {
      using namespace ZXTune::Capabilities::Container::Traits;
      AddCapability(caps, DIRECTORIES, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Directories support"));
      AddCapability(caps, PLAIN, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Plain data structure format"));
      AddCapability(caps, ONCEAPPLIED, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Not recursive"));
    }
  private:
    QTreeWidget& Widget;
    //1st level
    QTreeWidgetItem* const Players;
    QTreeWidgetItem* const Containers;
    //2nd level of Players
    std::map<uint_t, QTreeWidgetItem*> PlayersByDeviceType;
    //2nd level of Containers
    std::vector<QTreeWidgetItem*> ContainersByType;
  };

  bool IsPlaybackBackend(const Sound::BackendInformation& backend)
  {
    return Sound::CAP_TYPE_SYSTEM == (backend.Capabilities() & Sound::CAP_TYPE_MASK);
  }

  bool IsFilesaveBackend(const Sound::BackendInformation& backend)
  {
    return Sound::CAP_TYPE_FILE == (backend.Capabilities() & Sound::CAP_TYPE_MASK);
  }

  bool IsHardwareBackend(const Sound::BackendInformation& backend)
  {
    return Sound::CAP_TYPE_HARDWARE == (backend.Capabilities() & Sound::CAP_TYPE_MASK);
  }

  template<class T>
  QTreeWidgetItem* CreateRootItem(T& root, const String& description, const Error& status)
  {
    QTreeWidgetItem* const item = new QTreeWidgetItem(&root, QStringList(ToQString(description)));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    if (status)
    {
      item->setCheckState(0, Qt::Unchecked);
      item->setToolTip(0, ToQString(status.ToString()));
    }
    else
    {
      item->setCheckState(0, Qt::Checked);
    }
    return item;
  }

  class BackendsTreeHelper
  {
  public:
    explicit BackendsTreeHelper(QTreeWidget& widget)
      : Widget(widget)
      , Playbacks(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "Playback backends")))
      , Filesaves(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "File backends")))
      , Hardwares(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "Hardware backends")))
      , Others(CreateTreeWidgetItem(&Widget, QT_TRANSLATE_NOOP("ComponentsDialog", "Other backends")))
    {
    }

    void AddBackend(const Sound::BackendInformation& backend)
    {
      if (IsPlaybackBackend(backend))
      {
        assert(!IsFilesaveBackend(backend));
        assert(!IsHardwareBackend(backend));
        AddBackend(*Playbacks, backend);
      }
      else if (IsFilesaveBackend(backend))
      {
        assert(!IsHardwareBackend(backend));
        AddBackend(*Filesaves, backend);
      }
      else if (IsHardwareBackend(backend))
      {
        AddBackend(*Hardwares, backend);
      }
      else
      {
        AddBackend(*Others, backend);
      }
    }
  private:
    void AddBackend(QTreeWidgetItem& root, const Sound::BackendInformation& backend)
    {
      //root
      QTreeWidgetItem* const backendItem = CreateRootItem(root, backend.Description(), backend.Status());
      //features
      if (uint_t features = backend.Capabilities() & Sound::CAP_FEAT_MASK)
      {
        QTreeWidgetItem* const featuresItem = CreateTreeWidgetItem(backendItem, QT_TRANSLATE_NOOP("ComponentsDialog", "Features"));
        FillBackendFeatures(features, *featuresItem);
      }
    }

    void FillBackendFeatures(uint_t feats, QTreeWidgetItem& root)
    {
      AddCapability(feats, Sound::CAP_FEAT_HWVOLUME, root, QT_TRANSLATE_NOOP("ComponentsDialog", "Hardware volume control"));
    }
  private:
    QTreeWidget& Widget;
    //1st level
    QTreeWidgetItem* const Playbacks;
    QTreeWidgetItem* const Filesaves;
    QTreeWidgetItem* const Hardwares;
    QTreeWidgetItem* const Others;
  };

  class ProvidersTreeHelper
  {
  public:
    explicit ProvidersTreeHelper(QTreeWidget& widget)
      : Widget(widget)
    {
    }

    void AddProvider(const IO::Provider& provider)
    {
      //root
      CreateRootItem(Widget, provider.Description(), provider.Status());
    }
  private:
    QTreeWidget& Widget;
  };

  class ComponentsDialog : public QDialog
                         , private Ui::ComponentsDialog
  {
  public:
    explicit ComponentsDialog(QWidget& parent)
      : QDialog(&parent)
    {
      setupUi(this);
      FillPluginsTree();
      FillBackendsTree();
      FillProvidersTree();
    }
  private:
    void FillPluginsTree()
    {
      PluginsTreeHelper tree(*pluginsTree);

      for (ZXTune::Plugin::Iterator::Ptr plugins = ZXTune::EnumeratePlugins(); plugins->IsValid(); plugins->Next())
      {
        const ZXTune::Plugin::Ptr plugin = plugins->Get();
        tree.AddPlugin(*plugin);
      }
    }

    void FillBackendsTree()
    {
      BackendsTreeHelper tree(*backendsTree);
      const Sound::Service::Ptr svc = Sound::CreateGlobalService(GlobalOptions::Instance().Get());
      for (Sound::BackendInformation::Iterator::Ptr backends = svc->EnumerateBackends(); backends->IsValid(); backends->Next())
      {
        const Sound::BackendInformation::Ptr backend = backends->Get();
        tree.AddBackend(*backend);
      }
    }

    void FillProvidersTree()
    {
      ProvidersTreeHelper tree(*providersTree);

      for (IO::Provider::Iterator::Ptr providers = IO::EnumerateProviders(); providers->IsValid(); providers->Next())
      {
        const IO::Provider::Ptr provider = providers->Get();
        tree.AddProvider(*provider);
      }
    }
  };
}

namespace UI
{
  void ShowComponentsInformation(QWidget& parent)
  {
    ComponentsDialog dialog(parent);
    dialog.exec();
  }
}
