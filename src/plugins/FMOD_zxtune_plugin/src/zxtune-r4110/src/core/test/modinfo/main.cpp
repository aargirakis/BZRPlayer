/**
* 
* @file
*
* @brief  ModInfo utility
*
* @author vitamin.caig@gmail.com
*
**/

#include <error_tools.h>
#include <progress_callback.h>
#include <binary/container_factories.h>
#include <core/module_open.h>
#include <io/api.h>
#include <iostream>
#include <parameters/container.h>
#include <parameters/template.h>

namespace
{
  Module::Holder::Ptr OpenModuleByPath(const String& fullPath)
  {
    const Parameters::Container::Ptr emptyParams = Parameters::Container::Create();
    const String filename = fullPath;//TODO: split if required
    const Binary::Container::Ptr data = IO::OpenData(filename, *emptyParams, Log::ProgressCallback::Stub());
    return Module::Open(*emptyParams, *data);
  }

  void ShowModuleInfo(const Module::Information& info)
  {
    std::cout <<
      "Positions: " << info.PositionsCount() << " (" << info.LoopPosition() << ')' << std::endl <<
      "Patterns: " << info.PatternsCount() << std::endl <<
      "Frames: " << info.FramesCount() << " (" << info.LoopFrame() << ')' << std::endl <<
      "Channels: " << info.ChannelsCount() << std::endl <<
      "Initial tempo: " << info.Tempo() << std::endl;
  }
  
  class PrintValuesVisitor : public Parameters::Visitor
  {
  public:
    void SetValue(const Parameters::NameType& name, Parameters::IntType val) override
    {
      Write(name, Parameters::ConvertToString(val));
    }

    virtual void SetValue(const Parameters::NameType& name, const Parameters::StringType& val) override
    {
      Write(name, Parameters::ConvertToString(val));
    }
    
    virtual void SetValue(const Parameters::NameType& name, const Parameters::DataType& val) override
    {
      Write(name, Parameters::ConvertToString(val));
    }
  private:
    static void Write(const Parameters::NameType& name, const String& value)
    {
      std::cout << name.Name() << ": " << value << std::endl;
    }
  };

  void ShowProperties(const Parameters::Accessor& props)
  {
    static PrintValuesVisitor PRINTER;
    props.Process(PRINTER);
  }

  void ShowModuleProperties(const Module::Holder& module)
  {
    ShowProperties(*module.GetModuleProperties());
    ShowModuleInfo(*module.GetModuleInformation());
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    return 0;
  }
  try
  {
    const Module::Holder::Ptr module = OpenModuleByPath(argv[1]);
    ShowModuleProperties(*module);
    return 0;
  }
  catch (const Error& e)
  {
    std::cout << e.ToString() << std::endl;
    return -1;
  }
}