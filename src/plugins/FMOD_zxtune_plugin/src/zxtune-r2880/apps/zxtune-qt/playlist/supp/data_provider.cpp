/**
* 
* @file
*
* @brief Playlist data provider implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "data_provider.h"
#include "ui/format.h"
#include "ui/utils.h"
#include "playlist/parameters.h"
#include <apps/base/playitem.h>
//common includes
#include <error_tools.h>
#include <progress_callback.h>
//library includes
#include <core/module_attrs.h>
#include <core/module_detect.h>
#include <core/module_open.h>
#include <core/plugin.h>
#include <core/plugin_attrs.h>
#include <debug/log.h>
#include <io/api.h>
#include <parameters/merged_accessor.h>
#include <parameters/template.h>
#include <parameters/tracking.h>
#include <sound/sound_parameters.h>
#include <strings/strings_format.h>
#include <strings/template.h>
//std includes
#include <deque>
//boost includes
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/mutex.hpp>
//text includes
#include "text/text.h"

#define FILE_TAG 0C9BBC6E

namespace
{
  const Debug::Stream Dbg("Playlist::DataProvider");
}

namespace
{
  class DataProvider
  {
  public:
    typedef boost::shared_ptr<const DataProvider> Ptr;

    virtual ~DataProvider() {}

    virtual Binary::Container::Ptr GetData(const String& dataPath) const = 0;
  };

  class SimpleDataProvider : public DataProvider
  {
  public:
    explicit SimpleDataProvider(Parameters::Accessor::Ptr ioParams)
      : Params(ioParams)
    {
    }

    virtual Binary::Container::Ptr GetData(const String& dataPath) const
    {
      return IO::OpenData(dataPath, *Params, Log::ProgressCallback::Stub());
    }
  private:
    const Parameters::Accessor::Ptr Params;
  };

  DataProvider::Ptr CreateSimpleDataProvider(Parameters::Accessor::Ptr ioParams)
  {
    return boost::make_shared<SimpleDataProvider>(ioParams);
  }

  template<class T>
  struct ObjectTraits;

  template<>
  struct ObjectTraits<Binary::Container::Ptr>
  {
    typedef std::size_t WeightType;

    static WeightType Weight(Binary::Container::Ptr obj)
    {
      return obj->Size();
    }
  };

  template<class T, class W = typename ObjectTraits<T>::WeightType>
  class ObjectsCache
  {
    struct Item
    {
      String Id;
      T Value;
      W Weight;

      Item()
        : Id()
        , Value()
        , Weight()
      {
      }

      Item(const String& id, T val)
        : Id(id)
        , Value(val)
        , Weight(ObjectTraits<T>::Weight(val))
      {
      }
    };

    typedef std::deque<Item> ItemsList;
  public:
    ObjectsCache()
      : TotalWeight()
    {
    }

    T Find(const String& id)
    {
      if (Item* res = FindItem(id))
      {
        return res->Value;
      }
      return T();
    }

    void Add(const String& id, T val)
    {
      if (Item* res = FindItem(id))
      {
        const W weight = ObjectTraits<T>::Weight(val);
        TotalWeight = TotalWeight + weight - res->Weight;
        res->Value = val;
        res->Weight = weight;
      }
      else
      {
        const Item item(id, val);
        Items.push_front(item);
        TotalWeight += item.Weight;
      }
    }

    void Del(const String& id)
    {
      const typename ItemsList::iterator it = std::find_if(Items.begin(), Items.end(),
        boost::bind(&Item::Id, _1) == id);
      if (it != Items.end())
      {
        TotalWeight -= ObjectTraits<T>::Weight(it->Value);
        Items.erase(it);
      }
    }

    void Fit(std::size_t maxCount, W maxWeight)
    {
      while (Items.size() > maxCount ||
             TotalWeight > maxWeight)
      {
        const Item entry = Items.back();
        Items.pop_back();
        TotalWeight -= entry.Weight;
      }
    }

    void Clear()
    {
      ItemsList().swap(Items);
      TotalWeight = 0;
    }

    std::size_t GetItemsCount() const
    {
      return Items.size();
    }

    W GetItemsWeight() const
    {
      return TotalWeight;
    }
  private:
    Item* FindItem(const String& id)
    {
      const typename ItemsList::iterator it = std::find_if(Items.begin(), Items.end(),
        boost::bind(&Item::Id, _1) == id);
      if (it != Items.end())
      {
        if (Items.size() > 1)
        {
          std::iter_swap(it, Items.begin());
        }
        return &Items.front();
      }
      return 0;
    }
  private:
    ItemsList Items;
    W TotalWeight;
  };

  class CacheParameters
  {
  public:
    explicit CacheParameters(Parameters::Accessor::Ptr params)
      : Params(params)
    {
    }

    std::size_t MemoryLimit() const
    {
      Parameters::IntType res = Parameters::ZXTuneQT::Playlist::Cache::MEMORY_LIMIT_MB_DEFAULT;
      Params->FindValue(Parameters::ZXTuneQT::Playlist::Cache::MEMORY_LIMIT_MB, res);
      return static_cast<std::size_t>(res * 1048576);
    }

    std::size_t FilesLimit() const
    {
      Parameters::IntType res = Parameters::ZXTuneQT::Playlist::Cache::FILES_LIMIT_DEFAULT;
      Params->FindValue(Parameters::ZXTuneQT::Playlist::Cache::FILES_LIMIT, res);
      return static_cast<std::size_t>(res);
    }
  private:
    const Parameters::Accessor::Ptr Params;
  };

  //cached data provider
  class CachedDataProvider : public DataProvider
  {
  public:
    typedef boost::shared_ptr<CachedDataProvider> Ptr;

    explicit CachedDataProvider(Parameters::Accessor::Ptr ioParams)
      : Params(ioParams)
      , Delegate(CreateSimpleDataProvider(ioParams))
    {
    }

    virtual Binary::Container::Ptr GetData(const String& dataPath) const
    {
      const boost::mutex::scoped_lock lock(Mutex);
      const std::size_t filesLimit = Params.FilesLimit();
      const std::size_t memLimit = Params.MemoryLimit();
      if (filesLimit != 0 && memLimit != 0)
      {
        return GetCachedData(dataPath, filesLimit, memLimit);
      }
      else
      {
        Cache.Clear();
        return Delegate->GetData(dataPath);
      }
    }

    void FlushCachedData(const String& dataPath)
    {
      const boost::mutex::scoped_lock lock(Mutex);
      if (Cache.GetItemsCount())
      {
        Cache.Del(dataPath);
        ReportCache();
      }
    }
  private:
    Binary::Container::Ptr GetCachedData(const String& dataPath, std::size_t filesLimit, std::size_t memLimit) const
    {
      if (const Binary::Container::Ptr cached = Cache.Find(dataPath))
      {
        return cached;
      }
      const Binary::Container::Ptr data = Delegate->GetData(dataPath);
      Cache.Add(dataPath, data);
      Cache.Fit(filesLimit, memLimit);
      ReportCache();
      return data;
    }
    
    void ReportCache() const
    {
      Dbg("Cache(%1%): %2% files, %3% bytes", this, Cache.GetItemsCount(), Cache.GetItemsWeight());
    }
  private:
    const CacheParameters Params;
    const DataProvider::Ptr Delegate;
    mutable boost::mutex Mutex;
    mutable ObjectsCache<Binary::Container::Ptr> Cache;
  };

  class DataSource
  {
  public:
    typedef boost::shared_ptr<const DataSource> Ptr;

    DataSource(CachedDataProvider::Ptr provider, IO::Identifier::Ptr id)
      : Provider(provider)
      , DataId(id)
    {
    }

    ~DataSource()
    {
      Provider->FlushCachedData(DataId->Path());
    }

    Binary::Container::Ptr GetData() const
    {
      return Provider->GetData(DataId->Path());
    }

    IO::Identifier::Ptr GetDataIdentifier() const
    {
      return DataId;
    }
  private:
    const CachedDataProvider::Ptr Provider;
    const IO::Identifier::Ptr DataId;
  };

  class ModuleSource
  {
  public:
    ModuleSource(Parameters::Accessor::Ptr coreParams, DataSource::Ptr source, IO::Identifier::Ptr moduleId)
      : CoreParams(coreParams)
      , Source(source)
      , ModuleId(moduleId)
    {
    }

    Module::Holder::Ptr GetModule(Parameters::Accessor::Ptr adjustedParams) const
    {
      try
      {
        const Binary::Container::Ptr data = Source->GetData();
        const ZXTune::DataLocation::Ptr location = ZXTune::OpenLocation(CoreParams, data, ModuleId->Subpath());
        const Module::Holder::Ptr module = Module::Open(location);
        const Parameters::Accessor::Ptr pathParams = CreatePathProperties(ModuleId);
        const Parameters::Accessor::Ptr moduleParams = Parameters::CreateMergedAccessor(pathParams, adjustedParams);
        return Module::CreateMixedPropertiesHolder(module, moduleParams);
      }
      catch (const Error&)
      {
        return Module::Holder::Ptr();
      }
    }

    String GetFullPath() const
    {
      return ModuleId->Full();
    }
  private:
    const Parameters::Accessor::Ptr CoreParams;
    const DataSource::Ptr Source;
    const IO::Identifier::Ptr ModuleId;
  };

  String GetStringProperty(const Parameters::Accessor& props, const Parameters::NameType& propName)
  {
    Parameters::StringType val;
    if (props.FindValue(propName, val))
    {
      return val;
    }
    return String();
  }

  Parameters::IntType GetIntProperty(const Parameters::Accessor& props, const Parameters::NameType& propName, Parameters::IntType defVal = 0)
  {
    Parameters::IntType val = defVal;
    props.FindValue(propName, val);
    return val;
  }

  class DynamicAttributesProvider
  {
  public:
    typedef boost::shared_ptr<const DynamicAttributesProvider> Ptr;

    DynamicAttributesProvider()
      : DisplayNameTemplate(Strings::Template::Create(Text::MODULE_PLAYLIST_FORMAT))
      , DummyDisplayName(DisplayNameTemplate->Instantiate(Strings::SkipFieldsSource()))
    {
    }

    String GetDisplayName(const Parameters::Accessor& properties) const
    {
      const Parameters::FieldsSourceAdapter<Strings::SkipFieldsSource> adapter(properties);
      String result = DisplayNameTemplate->Instantiate(adapter);
      if (result == DummyDisplayName)
      {
        if (!properties.FindValue(Module::ATTR_FULLPATH, result))
        {
          result.clear();
        }
      }
      return result;
    }
  private:
    const Strings::Template::Ptr DisplayNameTemplate;
    const String DummyDisplayName;
  };

  class DataImpl : public Playlist::Item::Data
                 , private Parameters::Modifier
  {
  public:
    DataImpl(DynamicAttributesProvider::Ptr attributes,
        const ModuleSource& source,
        Parameters::Container::Ptr adjustedParams,
        uint_t frames, const Parameters::Accessor& moduleProps,
        uint_t caps)
      : Caps(caps)
      , Attributes(attributes)
      , Source(source)
      , AdjustedParams(adjustedParams)
      , Type(GetStringProperty(moduleProps, Module::ATTR_TYPE))
      , Checksum(static_cast<uint32_t>(GetIntProperty(moduleProps, Module::ATTR_CRC)))
      , CoreChecksum(static_cast<uint32_t>(GetIntProperty(moduleProps, Module::ATTR_FIXEDCRC)))
      , Size(static_cast<std::size_t>(GetIntProperty(moduleProps, Module::ATTR_SIZE)))
    {
      Duration.SetCount(frames);
      LoadProperties(moduleProps);
    }

    virtual Module::Holder::Ptr GetModule() const
    {
      try
      {
        State = Error();
        return Source.GetModule(AdjustedParams);
      }
      catch (const Error& e)
      {
        State = e;
      }
      return Module::Holder::Ptr();
    }

    virtual Parameters::Container::Ptr GetAdjustedParameters() const
    {
      const Parameters::Modifier& cb = *this;
      return Parameters::CreatePostChangePropertyTrackedContainer(AdjustedParams, const_cast<Parameters::Modifier&>(cb));
    }

    virtual Playlist::Item::Capabilities GetCapabilities() const
    {
      return Caps;
    }

    //playlist-related properties
    virtual Error GetState() const
    {
      return State;
    }

    virtual String GetFullPath() const
    {
      return Source.GetFullPath();
    }

    virtual String GetType() const
    {
      return Type;
    }

    virtual String GetDisplayName() const
    {
      return DisplayName;
    }

    virtual Time::MillisecondsDuration GetDuration() const
    {
      return Duration;
    }

    virtual String GetAuthor() const
    {
      return Author;
    }

    virtual String GetTitle() const
    {
      return Title;
    }

    virtual uint32_t GetChecksum() const
    {
      return Checksum;
    }

    virtual uint32_t GetCoreChecksum() const
    {
      return CoreChecksum;
    }

    virtual std::size_t GetSize() const
    {
      return Size;
    }
  private:
    Parameters::Accessor::Ptr GetModuleProperties() const
    {
      if (const Module::Holder::Ptr holder = GetModule())
      {
        return holder->GetModuleProperties();
      }
      return Parameters::Accessor::Ptr();
    }
  private:
    virtual void SetValue(const Parameters::NameType& /*name*/, Parameters::IntType /*val*/)
    {
      OnPropertyChanged();
    }

    virtual void SetValue(const Parameters::NameType& /*name*/, const Parameters::StringType& /*val*/)
    {
      OnPropertyChanged();
    }

    virtual void SetValue(const Parameters::NameType& /*name*/, const Parameters::DataType& /*val*/)
    {
      OnPropertyChanged();
    }

    virtual void RemoveValue(const Parameters::NameType& /*name*/)
    {
      OnPropertyChanged();
    }

    void OnPropertyChanged()
    {
      if (const Parameters::Accessor::Ptr properties = GetModuleProperties())
      {
        LoadProperties(*properties);
      }
      else
      {
        DisplayName.clear();
        Author.clear();
        Title.clear();
        Duration.SetCount(0);
      }
    }

    void LoadProperties(const Parameters::Accessor& props)
    {
      DisplayName = Attributes->GetDisplayName(props);
      Author = GetStringProperty(props, Module::ATTR_AUTHOR);
      Title = GetStringProperty(props, Module::ATTR_TITLE);
      const Time::Microseconds period(GetIntProperty(props, Parameters::ZXTune::Sound::FRAMEDURATION, Parameters::ZXTune::Sound::FRAMEDURATION_DEFAULT));
      Duration.SetPeriod(period);
    }
  private:
    const Playlist::Item::Capabilities Caps;
    const DynamicAttributesProvider::Ptr Attributes;
    const ModuleSource Source;
    const Parameters::Container::Ptr AdjustedParams;
    const String Type;
    const uint32_t Checksum;
    const uint32_t CoreChecksum;
    const std::size_t Size;
    String DisplayName;
    String Author;
    String Title;
    Time::MillisecondsDuration Duration;
    mutable Error State;
  };

  class DetectCallback : public Module::DetectCallback
  {
  public:
    DetectCallback(Playlist::Item::DetectParameters& delegate,
                            DynamicAttributesProvider::Ptr attributes,
                            CachedDataProvider::Ptr provider, Parameters::Accessor::Ptr coreParams, IO::Identifier::Ptr dataId)
      : Delegate(delegate)
      , Attributes(attributes)
      , CoreParams(coreParams)
      , DataId(dataId)
      , Source(boost::make_shared<DataSource>(provider, dataId))
    {
    }

    virtual Parameters::Accessor::Ptr GetPluginsParameters() const
    {
      return CoreParams;
    }

    virtual void ProcessModule(ZXTune::DataLocation::Ptr location, ZXTune::Plugin::Ptr decoder, Module::Holder::Ptr holder) const
    {
      const String subPath = location->GetPath()->AsString();
      const Parameters::Container::Ptr adjustedParams = Delegate.CreateInitialAdjustedParameters();
      const Module::Information::Ptr info = holder->GetModuleInformation();
      const Parameters::Accessor::Ptr moduleProps = holder->GetModuleProperties();
      const IO::Identifier::Ptr moduleId = DataId->WithSubpath(subPath);
      const Parameters::Accessor::Ptr pathProps = CreatePathProperties(moduleId);
      const Parameters::Accessor::Ptr lookupModuleProps = Parameters::CreateMergedAccessor(pathProps, adjustedParams, moduleProps);
      const ModuleSource itemSource(CoreParams, Source, moduleId);
      const Playlist::Item::Data::Ptr playitem = boost::make_shared<DataImpl>(Attributes, itemSource, adjustedParams,
        info->FramesCount(), *lookupModuleProps, decoder->Capabilities());
      Delegate.ProcessItem(playitem);
    }

    virtual Log::ProgressCallback* GetProgress() const
    {
      return Delegate.GetProgress();
    }
  private:
    Playlist::Item::DetectParameters& Delegate;
    const DynamicAttributesProvider::Ptr Attributes;
    const Parameters::Accessor::Ptr CoreParams;
    const IO::Identifier::Ptr DataId;
    const DataSource::Ptr Source;
  };

  class DataProviderImpl : public Playlist::Item::DataProvider
  {
  public:
    explicit DataProviderImpl(Parameters::Accessor::Ptr parameters)
      : Provider(new CachedDataProvider(parameters))
      , CoreParams(parameters)
      , Attributes(boost::make_shared<DynamicAttributesProvider>())
    {
    }

    virtual void DetectModules(const String& path, Playlist::Item::DetectParameters& detectParams) const
    {
      const IO::Identifier::Ptr id = IO::ResolveUri(path);

      const String subPath = id->Subpath();
      if (subPath.empty())
      {
        const Binary::Container::Ptr data = Provider->GetData(id->Path());
        const ZXTune::DataLocation::Ptr location = ZXTune::CreateLocation(data);
        const DetectCallback detectCallback(detectParams, Attributes, Provider, CoreParams, id);
        Module::Detect(location, detectCallback);
      }
      else
      {
        OpenModule(path, detectParams);
      }
    }

    virtual void OpenModule(const String& path, Playlist::Item::DetectParameters& detectParams) const
    {
      const IO::Identifier::Ptr id = IO::ResolveUri(path);

      const Binary::Container::Ptr data = Provider->GetData(id->Path());
      const DetectCallback detectCallback(detectParams, Attributes, Provider, CoreParams, id);

      const ZXTune::DataLocation::Ptr location = ZXTune::OpenLocation(CoreParams, data, id->Subpath());
      Module::Open(location, detectCallback);
    }
  private:
    const CachedDataProvider::Ptr Provider;
    const Parameters::Accessor::Ptr CoreParams;
    const DynamicAttributesProvider::Ptr Attributes;
  };
}

namespace Playlist
{
  namespace Item
  {
    DataProvider::Ptr DataProvider::Create(Parameters::Accessor::Ptr parameters)
    {
      return boost::make_shared<DataProviderImpl>(parameters);
    }
  }
}
