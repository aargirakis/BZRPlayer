/**
*
* @file
*
* @brief  Xsf-based files common code implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "xsf.h"
#include "xsf_factory.h"
//common includes
#include <contract.h>
#include <error_tools.h>
#include <make_ptr.h>
//library includes
#include <debug/log.h>
#include <module/additional_files.h>
#include <module/players/duration.h>
#include <module/players/properties_helper.h>

#define FILE_TAG 153C53E0

namespace Module
{
namespace XSF
{
  const Debug::Stream Dbg("Module::xSF");

  class MultiFileHolder : public Module::Holder
                        , public Module::AdditionalFiles
  {
  public:
    MultiFileHolder(XSF::Factory::Ptr factory, File head, Parameters::Container::Ptr properties)
      : HolderFactory(std::move(factory))
      , Properties(std::move(properties))
      , Head(std::move(head))
    {
      LoadDependenciesFrom(Head);
      Head.CloneData();
    }
    
    Module::Information::Ptr GetModuleInformation() const override
    {
      return GetDelegate().GetModuleInformation();
    }

    Parameters::Accessor::Ptr GetModuleProperties() const override
    {
      return GetDelegate().GetModuleProperties();
    }

    Renderer::Ptr CreateRenderer(Parameters::Accessor::Ptr params, Sound::Receiver::Ptr target) const override
    {
      return GetDelegate().CreateRenderer(std::move(params), std::move(target));
    }
    
    Strings::Array Enumerate() const override
    {
      Strings::Array result;
      for (const auto& dep : Files)
      {
        if (0 == dep.second.Version)
        {
          result.push_back(dep.first);
        }
      }
      return result;
    }
    
    void Resolve(const String& name, Binary::Container::Ptr data) override
    {
      Dbg("Resolving dependency '%1%'", name);
      auto& file = Files.at(name);
      Require(0 == file.Version);
      if (Parse(name, *data, file))
      {
        LoadDependenciesFrom(file);
        file.CloneData();
      }
    }
  private:
    void LoadDependenciesFrom(const File& file)
    {
      Require(Head.Version == file.Version);
      for (const auto& dep : file.Dependencies)
      {
        Require(!dep.empty());
        Require(Files.emplace(dep, File()).second);
        Dbg("Found unresolved dependency '%1%'", dep);
      }
    }
    
    const Module::Holder& GetDelegate() const
    {
      if (!Delegate)
      {
        Require(!Files.empty());
        FillStrings();
        Delegate = HolderFactory->CreateMultifileModule(Head, Files, std::move(Properties));
        Files.clear();
        Head = File();
      }
      return *Delegate;
    }
    
    void FillStrings() const
    {
      Strings::Array linear;
      linear.reserve(Files.size());
      for (const auto& dep : Files)
      {
        if (0 != dep.second.Version)
        {
          linear.push_back(dep.first);
        }
        else
        {
          Dbg("Unresolved '%1%'", dep.first);
          throw MakeFormattedError(THIS_LINE, "Unresolved dependency '%1%'", dep.first);
        }
      }
      PropertiesHelper(*Properties).SetStrings(linear);
    }
  private:
    const XSF::Factory::Ptr HolderFactory;
    mutable Parameters::Container::Ptr Properties;
    mutable File Head;
    mutable std::map<String, File> Files;
    
    mutable Holder::Ptr Delegate;
  };
  
  void FillDuration(const Parameters::Accessor& params, File& file)
  {
    const bool hasMeta = !!file.Meta;
    const bool hasDuration = hasMeta && file.Meta->Duration.Get();
    if (!hasDuration)
    {
      const MetaInformation::RWPtr newMeta = MakeRWPtr<MetaInformation>();
      newMeta->Duration = GetDuration(params);
      if (hasMeta)
      {
        newMeta->Merge(*file.Meta);
      }
      file.Meta = newMeta;
    }
  }
  
  class GenericFactory : public Module::Factory
  {
  public:
    explicit GenericFactory(XSF::Factory::Ptr delegate)
      : Delegate(std::move(delegate))
    {
    }

    Holder::Ptr CreateModule(const Parameters::Accessor& params, const Binary::Container& rawData, Parameters::Container::Ptr properties) const override
    {
      try
      {
        Dbg("Try to parse");
        File file;
        if (const auto source = Parse(rawData, file))
        {
          PropertiesHelper props(*properties);
          props.SetSource(*source);

          FillDuration(params, file);
          if (file.Dependencies.empty())
          {
            Dbg("Singlefile");
            return Delegate->CreateSinglefileModule(file, std::move(properties));
          }
          else
          {
            Dbg("Multifile");
            return MakePtr<MultiFileHolder>(Delegate, std::move(file), std::move(properties));
          }
        }
      }
      catch (const std::exception&)
      {
        Dbg("Failed to parse");
      }
      return Module::Holder::Ptr();
    }
  private:
    const XSF::Factory::Ptr Delegate;
  };

  Module::Factory::Ptr CreateFactory(XSF::Factory::Ptr delegate)
  {
    return MakePtr<GenericFactory>(std::move(delegate));
  }
}
}
