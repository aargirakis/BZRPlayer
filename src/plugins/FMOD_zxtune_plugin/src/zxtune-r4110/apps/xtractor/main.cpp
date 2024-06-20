/**
* 
* @file
*
* @brief XTractor tool main file
*
* @author vitamin.caig@gmail.com
*
**/


//common includes
#include <progress_callback.h>
#include <make_ptr.h>
//library includes
#include <analysis/path.h>
#include <analysis/result.h>
#include <analysis/scanner.h>
#include <async/data_receiver.h>
#include <binary/format_factories.h>
#include <debug/log.h>
#include <formats/archived/decoders.h>
#include <formats/chiptune/decoders.h>
#include <formats/image/decoders.h>
#include <formats/packed/decoders.h>
#include <io/api.h>
#include <io/providers_parameters.h>
#include <io/impl/boost_filesystem_path.h>
#include <parameters/container.h>
#include <platform/application.h>
#include <platform/version/api.h>
#include <strings/array.h>
#include <strings/fields.h>
#include <strings/format.h>
#include <strings/template.h>
//std includes
#include <iostream>
#include <locale>
#include <map>
#include <numeric>
#include <set>
//boost includes
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/join.hpp>
//text includes
#include "text/text.h"

namespace
{
  const Debug::Stream Dbg("XTractor");
}

namespace Analysis
{
  class Node
  {
  public:
    typedef std::shared_ptr<const Node> Ptr;
    virtual ~Node() = default;

    //! Name to distinguish. Can be empty
    virtual String Name() const = 0;
    //! Data associated with. Cannot be empty
    virtual Binary::Container::Ptr Data() const = 0;
    //! Parent node. Ptr() if root node
    virtual Ptr Parent() const = 0;
  };

  Node::Ptr CreateRootNode(Binary::Container::Ptr data, const String& name);
  Node::Ptr CreateSubnode(Node::Ptr parent, Binary::Container::Ptr data, const String& name);
  Node::Ptr CreateSubnode(Node::Ptr parent, Binary::Container::Ptr data, std::size_t offset);
}

namespace
{
  class RootNode : public Analysis::Node
  {
  public:
    RootNode(Binary::Container::Ptr data, String name)
      : DataVal(std::move(data))
      , NameVal(std::move(name))
    {
    }

    String Name() const override
    {
      return NameVal;
    }

    Binary::Container::Ptr Data() const override
    {
      return DataVal;
    }

    Analysis::Node::Ptr Parent() const override
    {
      return Analysis::Node::Ptr();
    }
  private:
    const Binary::Container::Ptr DataVal;
    const String NameVal;
  };

  class SubNode : public Analysis::Node
  {
  public:
    SubNode(Analysis::Node::Ptr parent, Binary::Container::Ptr data, String name)
      : ParentVal(std::move(parent))
      , DataVal(std::move(data))
      , NameVal(std::move(name))
    {
    }

    String Name() const override
    {
      return NameVal;
    }

    Binary::Container::Ptr Data() const override
    {
      return DataVal;
    }

    Analysis::Node::Ptr Parent() const override
    {
      return ParentVal;
    }
  private:
    const Analysis::Node::Ptr ParentVal;
    const Binary::Container::Ptr DataVal;
    const String NameVal;
  };
}

namespace Analysis
{
  //since data is required, place it first
  Node::Ptr CreateRootNode(Binary::Container::Ptr data, const String& name)
  {
    return MakePtr<RootNode>(std::move(data), name);
  }

  Node::Ptr CreateSubnode(Node::Ptr parent, Binary::Container::Ptr data, const String& name)
  {
    return MakePtr<SubNode>(std::move(parent), std::move(data), name);
  }

  Node::Ptr CreateSubnode(Node::Ptr parent, Binary::Container::Ptr data, std::size_t offset)
  {
    return MakePtr<SubNode>(std::move(parent), std::move(data), Strings::Format("+%1%", offset));
  }

  Node::Ptr CreateSubnode(Node::Ptr parent, Binary::Container::Ptr data, const String& name, std::size_t offset)
  {
    auto intermediate = CreateSubnode(parent, data, offset);
    return CreateSubnode(std::move(intermediate), std::move(data), name);
  }
}

namespace Analysis
{
  typedef DataReceiver<Node::Ptr> NodeReceiver;
  typedef DataTransmitter<Node::Ptr> NodeTransmitter;
  typedef DataTransceiver<Node::Ptr> NodeTransceiver;
}

namespace Formats
{
  namespace Archived
  { 
    void FillScanner(Analysis::Scanner& scanner)
    {
      scanner.AddDecoder(CreateZipDecoder());
      scanner.AddDecoder(CreateRarDecoder());
      scanner.AddDecoder(CreateZXZipDecoder());
      scanner.AddDecoder(CreateSCLDecoder());
      scanner.AddDecoder(CreateTRDDecoder());
      scanner.AddDecoder(CreateHripDecoder());
      scanner.AddDecoder(CreateLhaDecoder());
      scanner.AddDecoder(CreateZXStateDecoder());
      scanner.AddDecoder(CreateUMXDecoder());
      scanner.AddDecoder(Create7zipDecoder());
    }
  }

  namespace Packed
  {
    void FillScanner(Analysis::Scanner& scanner)
    {
      scanner.AddDecoder(CreateCodeCruncher3Decoder());
      scanner.AddDecoder(CreateCompressorCode4Decoder());
      scanner.AddDecoder(CreateCompressorCode4PlusDecoder());
      scanner.AddDecoder(CreateDataSquieezerDecoder());
      scanner.AddDecoder(CreateESVCruncherDecoder());
      scanner.AddDecoder(CreateHrumDecoder());
      scanner.AddDecoder(CreateHrust1Decoder());
      scanner.AddDecoder(CreateHrust21Decoder());
      scanner.AddDecoder(CreateHrust23Decoder());
      scanner.AddDecoder(CreateLZSDecoder());
      scanner.AddDecoder(CreateMSPackDecoder());
      scanner.AddDecoder(CreatePowerfullCodeDecreaser61Decoder());
      scanner.AddDecoder(CreatePowerfullCodeDecreaser61iDecoder());
      scanner.AddDecoder(CreatePowerfullCodeDecreaser62Decoder());
      scanner.AddDecoder(CreateTRUSHDecoder());
      scanner.AddDecoder(CreateGamePackerDecoder());
      scanner.AddDecoder(CreateGamePackerPlusDecoder());
      scanner.AddDecoder(CreateTurboLZDecoder());
      scanner.AddDecoder(CreateTurboLZProtectedDecoder());
      scanner.AddDecoder(CreateCharPresDecoder());
      scanner.AddDecoder(CreatePack2Decoder());
      scanner.AddDecoder(CreateLZH1Decoder());
      scanner.AddDecoder(CreateLZH2Decoder());
      scanner.AddDecoder(CreateFullDiskImageDecoder());
      scanner.AddDecoder(CreateHobetaDecoder());
      scanner.AddDecoder(CreateSna128Decoder());
      scanner.AddDecoder(CreateTeleDiskImageDecoder());
      scanner.AddDecoder(CreateZ80V145Decoder());
      scanner.AddDecoder(CreateZ80V20Decoder());
      scanner.AddDecoder(CreateZ80V30Decoder());
      scanner.AddDecoder(CreateMegaLZDecoder());
      scanner.AddDecoder(CreateDSKDecoder());
      scanner.AddDecoder(CreateGzipDecoder());
      //players
      scanner.AddDecoder(CreateCompiledASC0Decoder());
      scanner.AddDecoder(CreateCompiledASC1Decoder());
      scanner.AddDecoder(CreateCompiledASC2Decoder());
      scanner.AddDecoder(CreateCompiledST3Decoder());
      scanner.AddDecoder(CreateCompiledSTP1Decoder());
      scanner.AddDecoder(CreateCompiledSTP2Decoder());
      scanner.AddDecoder(CreateCompiledPT24Decoder());
      scanner.AddDecoder(CreateCompiledPTU13Decoder());
    }
  }

  namespace Image
  { 
    void FillScanner(Analysis::Scanner& scanner)
    {
      scanner.AddDecoder(CreateLaserCompact52Decoder());
      scanner.AddDecoder(CreateASCScreenCrusherDecoder());
      scanner.AddDecoder(CreateLaserCompact40Decoder());
    }
  }

  namespace Chiptune
  {
    void FillScanner(Analysis::Scanner& scanner)
    {
      //try TurboSound first
      scanner.AddDecoder(CreateTurboSoundDecoder());
      scanner.AddDecoder(CreatePSGDecoder());
      scanner.AddDecoder(CreateDigitalStudioDecoder());
      scanner.AddDecoder(CreateSoundTrackerDecoder());
      scanner.AddDecoder(CreateSoundTrackerCompiledDecoder());
      scanner.AddDecoder(CreateSoundTracker3Decoder());
      scanner.AddDecoder(CreateSoundTrackerProCompiledDecoder());
      scanner.AddDecoder(CreateASCSoundMaster0xDecoder());
      scanner.AddDecoder(CreateASCSoundMaster1xDecoder());
      scanner.AddDecoder(CreateProTracker2Decoder());
      scanner.AddDecoder(CreateProTracker3Decoder());
      scanner.AddDecoder(CreateProSoundMakerCompiledDecoder());
      scanner.AddDecoder(CreateGlobalTrackerDecoder());
      scanner.AddDecoder(CreateProTracker1Decoder());
      scanner.AddDecoder(CreateVTXDecoder());
      scanner.AddDecoder(CreateYMDecoder());
      scanner.AddDecoder(CreateTFDDecoder());
      scanner.AddDecoder(CreateTFCDecoder());
      scanner.AddDecoder(CreateVortexTracker2Decoder());
      scanner.AddDecoder(CreateChipTrackerDecoder());
      scanner.AddDecoder(CreateSampleTrackerDecoder());
      scanner.AddDecoder(CreateProDigiTrackerDecoder());
      scanner.AddDecoder(CreateSQTrackerDecoder());
      scanner.AddDecoder(CreateProSoundCreatorDecoder());
      scanner.AddDecoder(CreateFastTrackerDecoder());
      scanner.AddDecoder(CreateETrackerDecoder());
      scanner.AddDecoder(CreateSQDigitalTrackerDecoder());
      scanner.AddDecoder(CreateTFMMusicMaker05Decoder());
      scanner.AddDecoder(CreateTFMMusicMaker13Decoder());
      scanner.AddDecoder(CreateDigitalMusicMakerDecoder());
      scanner.AddDecoder(CreateExtremeTracker1Decoder());
      scanner.AddDecoder(CreateAYCDecoder());
      scanner.AddDecoder(CreateSPCDecoder());
      scanner.AddDecoder(CreateMultiTrackContainerDecoder());
      scanner.AddDecoder(CreateAYEMULDecoder());
      scanner.AddDecoder(CreateAbyssHighestExperienceDecoder());
    }
  }
}

namespace Parsing
{
  class Result
  {
  public:
    typedef std::unique_ptr<const Result> Ptr;
    virtual ~Result() = default;

    virtual String Name() const = 0;
    virtual Binary::Container::Ptr Data() const = 0;
  };

  Result::Ptr CreateResult(const String& name, Binary::Container::Ptr data);

  typedef DataReceiver<Result::Ptr> Target;

  typedef DataTransmitter<Result::Ptr> Source;
  typedef DataTransceiver<Result::Ptr> Pipe;
}

namespace
{
  class StaticResult : public Parsing::Result
  {
  public:
    StaticResult(String name, Binary::Container::Ptr data)
      : NameVal(std::move(name))
      , DataVal(std::move(data))
    {
    }

    String Name() const override
    {
      return NameVal;
    }

    Binary::Container::Ptr Data() const override
    {
      return DataVal;
    }
  private:
    const String NameVal;
    const Binary::Container::Ptr DataVal;
  };
}

namespace Parsing
{
  Result::Ptr CreateResult(const String& name, Binary::Container::Ptr data)
  {
    return MakePtr<StaticResult>(name, data);
  }
}

namespace
{
  class SaveTarget : public Parsing::Target
  {
  public:
    SaveTarget()
      : Params(Parameters::Container::Create())
    {
      Params->SetValue(Parameters::ZXTune::IO::Providers::File::OVERWRITE_EXISTING, 2);
    }

    void ApplyData(Parsing::Result::Ptr result) override
    {
      try
      {
        const String filePath = result->Name();
        const Binary::OutputStream::Ptr target = IO::CreateStream(filePath, *Params, Log::ProgressCallback::Stub());
        const Binary::Container::Ptr data = result->Data();
        target->ApplyData(*data);
      }
      catch (const Error& e)
      {
        std::cout << e.ToString();
      }
    }

    void Flush() override
    {
    }
  private:
    const Parameters::Container::Ptr Params;
  };

  class StatisticTarget : public Parsing::Target
  {
  public:
    StatisticTarget()
      : Total(0)
      , TotalSize(0)
    {
    }

    void ApplyData(Parsing::Result::Ptr data) override
    {
      ++Total;
      TotalSize += data->Data()->Size();
    }

    void Flush() override
    {
      std::cout << Strings::Format(Text::STATISTIC_OUTPUT, Total, TotalSize) << std::endl;
    }
  private:
    std::size_t Total;
    uint64_t TotalSize;
  };
}

namespace Parsing
{
  Parsing::Target::Ptr CreateSaveTarget()
  {
    return MakePtr<SaveTarget>();
  }

  Parsing::Target::Ptr CreateStatisticTarget()
  {
    return MakePtr<StatisticTarget>();
  }
}

namespace
{
  class SizeFilter : public Analysis::NodeReceiver
  {
  public:
    SizeFilter(std::size_t minSize, Analysis::NodeReceiver::Ptr target)
      : MinSize(minSize)
      , Target(std::move(target))
    {
    }

    void ApplyData(Analysis::Node::Ptr result) override
    {
      if (result->Data()->Size() >= MinSize)
      {
        Target->ApplyData(std::move(result));
      }
    }

    void Flush() override
    {
      Target->Flush();
    }
  private:
    const std::size_t MinSize;
    const Analysis::NodeReceiver::Ptr Target;
  };

  class EmptyDataFilter : public Analysis::NodeReceiver
  {
  public:
    explicit EmptyDataFilter(Analysis::NodeReceiver::Ptr target)
      : Target(std::move(target))
    {
    }

    void ApplyData(Analysis::Node::Ptr result) override
    {
      const Binary::Container::Ptr data = result->Data();
      const uint8_t* const begin = static_cast<const uint8_t*>(data->Start());
      const uint8_t* const end = begin + data->Size();
      if (end != std::find_if(begin, end, std::bind1st(std::not_equal_to<uint8_t>(), *begin)))
      {
        Target->ApplyData(std::move(result));
      }
    }

    void Flush() override
    {
      Target->Flush();
    }
  private:
    const Analysis::NodeReceiver::Ptr Target;
  };

  class MatchedDataFilter : public Analysis::NodeReceiver
  {
  public:
    MatchedDataFilter(const std::string& format, Analysis::NodeReceiver::Ptr target)
      : Format(Binary::CreateFormat(format))
      , Target(std::move(target))
    {
    }

    void ApplyData(Analysis::Node::Ptr result) override
    {
      const Binary::Container::Ptr data = result->Data();
      const std::size_t size = data->Size();
      if (Format->Match(*data) || size != Format->NextMatchOffset(*data))
      {
        Target->ApplyData(std::move(result));
      }
    }

    void Flush() override
    {
      Target->Flush();
    }
  private:
    const Binary::Format::Ptr Format;
    const Analysis::NodeReceiver::Ptr Target;
  };
}

namespace Analysis
{
  NodeReceiver::Ptr CreateSizeFilter(std::size_t minSize, NodeReceiver::Ptr target)
  {
    return MakePtr<SizeFilter>(minSize, target);
  }

  NodeReceiver::Ptr CreateEmptyDataFilter(NodeReceiver::Ptr target)
  {
    return MakePtr<EmptyDataFilter>(target);
  }

  NodeReceiver::Ptr CreateMatchFilter(const std::string& filter, NodeReceiver::Ptr target)
  {
    return MakePtr<MatchedDataFilter>(filter, target);
  }
}

namespace
{
  class NestedScannerTarget : public Analysis::Scanner::Target
  {
  public:
    NestedScannerTarget(Analysis::Node::Ptr root, Analysis::NodeReceiver& toScan, Analysis::NodeReceiver& toStore)
      : Root(std::move(root))
      , ToScan(toScan)
      , ToStore(toStore)
    {
    }

    void Apply(const Formats::Archived::Decoder& decoder, std::size_t offset, Formats::Archived::Container::Ptr data) override
    {
      const String name = decoder.GetDescription();
      Dbg("Found %1% in %2% bytes at %3%", name, data->Size(), offset);
      auto archNode = Analysis::CreateSubnode(Root, data, name, offset);
      const ScanFiles walker(ToScan, std::move(archNode));
      data->ExploreFiles(walker);
    }

    void Apply(const Formats::Packed::Decoder& decoder, std::size_t offset, Formats::Packed::Container::Ptr data) override
    {
      const String name = decoder.GetDescription();
      Dbg("Found %1% in %2% bytes at %3%", name, data->PackedSize(), offset);
      auto packNode = Analysis::CreateSubnode(Root, std::move(data), name, offset);
      ToScan.ApplyData(std::move(packNode));
    }

    void Apply(const Formats::Image::Decoder& decoder, std::size_t offset, Formats::Image::Container::Ptr data) override
    {
      const String name = decoder.GetDescription();
      Dbg("Found %1% in %2% bytes at %3%", name, data->OriginalSize(), offset);
      auto imageNode = Analysis::CreateSubnode(Root, std::move(data), Strings::Format("+%1%.image", offset));
      ToStore.ApplyData(std::move(imageNode));
    }

    void Apply(const Formats::Chiptune::Decoder& decoder, std::size_t offset, Formats::Chiptune::Container::Ptr data) override
    {
      const String name = decoder.GetDescription();
      Dbg("Found %1% in %2% bytes at %3%", name, data->Size(), offset);
      auto chiptuneNode = Analysis::CreateSubnode(Root, std::move(data), Strings::Format("+%1%.chiptune", offset));
      ToStore.ApplyData(std::move(chiptuneNode));
    }

    void Apply(std::size_t offset, Binary::Container::Ptr data) override
    {
      Dbg("Unresolved %1% bytes at %2%", data->Size(), offset);
      auto rawNode = Analysis::CreateSubnode(Root, std::move(data), offset);
      ToStore.ApplyData(std::move(rawNode));
    }
  private:
    class ScanFiles : public Formats::Archived::Container::Walker
    {
    public:
      ScanFiles(Analysis::NodeReceiver& toScan, Analysis::Node::Ptr node)
        : ToScan(toScan)
        , ArchiveNode(std::move(node))
      {
      }

      void OnFile(const Formats::Archived::File& file) const override
      {
        if (const Binary::Container::Ptr data = file.GetData())
        {
          const String name = file.GetName();
          Dbg("Processing %1%", name);
          auto fileNode = Analysis::CreateSubnode(ArchiveNode, data, name);
          ToScan.ApplyData(std::move(fileNode));
        }
      }
    private:
      Analysis::NodeReceiver& ToScan;
      const Analysis::Node::Ptr ArchiveNode;
    };
  private:
    const Analysis::Node::Ptr Root;
    Analysis::NodeReceiver& ToScan;
    Analysis::NodeReceiver& ToStore;
  };

  class AnalysisTarget : public Analysis::NodeTransceiver
  {
  public:
    AnalysisTarget()
      : Scanner(Analysis::CreateScanner())
    {
      Formats::Archived::FillScanner(*Scanner);
      Formats::Packed::FillScanner(*Scanner);
      Formats::Image::FillScanner(*Scanner);
      Formats::Chiptune::FillScanner(*Scanner);
    }

    void ApplyData(Analysis::Node::Ptr node) override
    {
      Dbg("Analyze %1%", node->Name());
      NestedScannerTarget target(node, *this, *Target);
      try
      {
        Scanner->Scan(node->Data(), target);
      }
      catch (const std::exception& e)
      {
        std::cout << "Failed to process " << node->Name() << "(" << e.what() << ")\n";
      }
    }

    void Flush() override
    {
      Target->Flush();
    }

    void SetTarget(Analysis::NodeReceiver::Ptr target) override
    {
      Target = target;
    }
  private:
    const Analysis::Scanner::RWPtr Scanner;
    Analysis::NodeReceiver::Ptr Target;
  };
}

namespace
{
  typedef DataReceiver<String> StringsReceiver;

  typedef DataTransceiver<String, Analysis::Node::Ptr> OpenPoint;

  class OpenPointImpl : public OpenPoint
  {
  public:
    OpenPointImpl()
      : Analyse(Analysis::NodeReceiver::CreateStub())
      , Params(Parameters::Container::Create())
    {
    }

    void ApplyData(String filename) override
    {
      try
      {
        Dbg("Opening '%1%'", filename);
        const Binary::Container::Ptr data = IO::OpenData(filename, *Params, Log::ProgressCallback::Stub());
        auto root = Analysis::CreateRootNode(data, filename);
        Analyse->ApplyData(std::move(root));
      }
      catch (const Error& e)
      {
        std::cout << e.ToString();
      }
    }

    void Flush() override
    {
      Analyse->Flush();
    }

    void SetTarget(Analysis::NodeReceiver::Ptr analyse) override
    {
      assert(analyse);
      Analyse = analyse;
    }
  private:
    Analysis::NodeReceiver::Ptr Analyse;
    const Parameters::Accessor::Ptr Params;
  };

  class PathTemplate : public Strings::FieldsSource
  {
  public:
    explicit PathTemplate(Analysis::Node::Ptr node)
      : Node(std::move(node))
    {
    }

    String GetFieldValue(const String& fieldName) const override
    {
      static const Char SUBPATH_DELIMITER[] = {'/', 0};
      static const Char FLATPATH_DELIMITER[] = {'_', 0};

      if (fieldName == Text::TEMPLATE_FIELD_FILENAME)
      {
        const IO::Identifier& id = GetRootIdentifier();
        return id.Filename();
      }
      else if (fieldName == Text::TEMPLATE_FIELD_PATH)
      {
        const IO::Identifier& id = GetRootIdentifier();
        return id.Path();
      }
      else if (fieldName == Text::TEMPLATE_FIELD_FLATPATH)
      {
        //TODO: use IO::FilenameTemplate
        const IO::Identifier& id = GetRootIdentifier();
        const boost::filesystem::path path(id.Path());
        const boost::filesystem::path root(path.root_directory());
        Strings::Array components;
        for (boost::filesystem::path::const_iterator it = path.begin(), lim = path.end(); it != lim; ++it)
        {
          if (*it != root)
          {
            components.push_back(IO::Details::ToString(*it));
          }
        }
        return boost::algorithm::join(components, FLATPATH_DELIMITER);
      }
      else if (fieldName == Text::TEMPLATE_FIELD_SUBPATH)
      {
        const Strings::Array& subPath = GetSubpath();
        return boost::algorithm::join(subPath, SUBPATH_DELIMITER);
      }
      else if (fieldName == Text::TEMPLATE_FIELD_FLATSUBPATH)
      {
        const Strings::Array& subPath = GetSubpath();
        return boost::algorithm::join(subPath, FLATPATH_DELIMITER);
      }
      else
      {
        return String();
      }
    }
  private:
    const IO::Identifier& GetRootIdentifier() const
    {
      if (!RootIdentifier)
      {
        FillCache();
      }
      return *RootIdentifier;
    }

    const Strings::Array& GetSubpath() const
    {
      if (!Subpath.get())
      {
        FillCache();
      }
      return *Subpath;
    }

    void FillCache() const
    {
      assert(!RootIdentifier && !Subpath.get());
      Strings::Array subpath;
      for (Analysis::Node::Ptr node = Node; node;)
      {
        if (const Analysis::Node::Ptr prevNode = node->Parent())
        {
          subpath.push_back(node->Name());
          node = prevNode;
        }
        else
        {
          const String fileName = node->Name();
          RootIdentifier = IO::ResolveUri(fileName);
          Subpath.reset(new Strings::Array(subpath.rbegin(), subpath.rend()));
          break;
        }
      }
    }

  private:
    const Analysis::Node::Ptr Node;
    mutable IO::Identifier::Ptr RootIdentifier;
    mutable std::unique_ptr<Strings::Array> Subpath;
  };

  class TargetNamePoint : public Analysis::NodeReceiver
  {
  public:
    TargetNamePoint(const String& nameTemplate, Parsing::Target::Ptr target)
      : Template(Strings::Template::Create(nameTemplate))
      , Target(std::move(target))
    {
    }

    void ApplyData(Analysis::Node::Ptr node) override
    {
      const PathTemplate fields(node);
      const String filename = Template->Instantiate(fields);
      auto result = Parsing::CreateResult(filename, node->Data());
      Target->ApplyData(std::move(result));
    }

    void Flush() override
    {
      Target->Flush();
    }
  private:
    const Strings::Template::Ptr Template;
    const Parsing::Target::Ptr Target;
  };

  class Valve : public Analysis::NodeTransceiver
  {
  public:
    explicit Valve(Analysis::NodeReceiver::Ptr target = Analysis::NodeReceiver::CreateStub())
      : Target(std::move(target))
    {
    }

    ~Valve() override
    {
      //break possible cycles
      Target = Analysis::NodeReceiver::CreateStub();
    }

    void ApplyData(Analysis::Node::Ptr data) override
    {
      Target->ApplyData(std::move(data));
    }

    void Flush() override
    {
      Target->Flush();
    }

    void SetTarget(Analysis::NodeReceiver::Ptr target) override
    {
      assert(target);
      Target = target;
    }
  private:
    Analysis::NodeReceiver::Ptr Target;
  };

  class ResolveDirsPoint : public DataTransceiver<String>
  {
  public:
    ResolveDirsPoint()
      : Target(StringsReceiver::CreateStub())
    {
    }

    void ApplyData(String filename) override
    {
      const boost::filesystem::path path(filename);
      if (boost::filesystem::is_directory(path))
      {
        ApplyRecursive(path);
      }
      else
      {
        Target->ApplyData(filename);
      }
    }

    void Flush() override
    {
      Target->Flush();
    }

    void SetTarget(StringsReceiver::Ptr target) override
    {
      Target = target;
    }
  private:
    void ApplyRecursive(const boost::filesystem::path& path) const
    {
      for (boost::filesystem::recursive_directory_iterator iter(path/*, boost::filesystem::symlink_option::recurse*/), lim = boost::filesystem::recursive_directory_iterator();
           iter != lim; ++iter)
      {
        const boost::filesystem::path subpath = iter->path();
        if (!boost::filesystem::is_directory(iter->status()))
        {
          const String subPathString = subpath.string();
          Target->ApplyData(subPathString);
        }
      }
    }
  private:
    StringsReceiver::Ptr Target;
  };

  template<class Object>
  typename DataReceiver<Object>::Ptr AsyncWrap(std::size_t threads, std::size_t queueSize, typename DataReceiver<Object>::Ptr target)
  {
    return Async::DataReceiver<Object>::Create(threads, queueSize, target);
  }

  class TargetOptions
  {
  public:
    virtual ~TargetOptions() = default;

    virtual String TargetNameTemplate() const = 0;
    virtual bool IgnoreEmptyData() const = 0;
    virtual std::size_t MinDataSize() const = 0;
    virtual std::string FormatFilter() const = 0;
    virtual std::size_t SaveThreadsCount() const = 0;
    virtual std::size_t SaveDataQueueSize() const = 0;
    virtual bool StatisticOutput() const = 0;
  };

  class AnalysisOptions
  {
  public:
    virtual ~AnalysisOptions() = default;

    virtual std::size_t AnalysisThreads() const = 0;
    virtual std::size_t AnalysisDataQueueSize() const = 0;
  };

  Analysis::NodeReceiver::Ptr CreateTarget(const TargetOptions& opts)
  {
    const Parsing::Target::Ptr save = opts.StatisticOutput()
      ? Parsing::CreateStatisticTarget()
      : Parsing::CreateSaveTarget();
    const Analysis::NodeReceiver::Ptr makeName = MakePtr<TargetNamePoint>(opts.TargetNameTemplate(), save);
    const Analysis::NodeReceiver::Ptr storeAll = makeName;
    const Analysis::NodeReceiver::Ptr storeNoEmpty = opts.IgnoreEmptyData()
      ? Analysis::CreateEmptyDataFilter(storeAll)
      : storeAll;
    const std::size_t minSize = opts.MinDataSize();
    const Analysis::NodeReceiver::Ptr storeEnoughSize = minSize
      ? Analysis::CreateSizeFilter(minSize, storeNoEmpty)
      : storeNoEmpty;
    const std::string filter = opts.FormatFilter();
    const Analysis::NodeReceiver::Ptr storeMatchedFilter = !filter.empty()
      ? Analysis::CreateMatchFilter(filter, storeEnoughSize)
      : storeEnoughSize;
    const Analysis::NodeReceiver::Ptr result = storeMatchedFilter;
    return AsyncWrap<Analysis::Node::Ptr>(opts.SaveThreadsCount(), opts.SaveDataQueueSize(), result);
  }

  template<class InType, class OutType = InType>
  class TransceivePipe : public DataTransceiver<InType, OutType>
  {
  public:
    TransceivePipe(typename DataReceiver<InType>::Ptr input, typename DataTransmitter<OutType>::Ptr output)
      : Input(std::move(input))
      , Output(std::move(output))
    {
    }

    void ApplyData(InType data) override
    {
      Input->ApplyData(std::move(data));
    }

    void Flush() override
    {
      Input->Flush();
    }

    void SetTarget(typename DataReceiver<OutType>::Ptr target) override
    {
      Output->SetTarget(target);
    }
  private:
    const typename DataReceiver<InType>::Ptr Input;
    const typename DataTransmitter<OutType>::Ptr Output;
  };

  Analysis::NodeTransceiver::Ptr CreateAnalyser(const AnalysisOptions& opts)
  {
    const Analysis::NodeTransceiver::Ptr analyser = MakePtr<AnalysisTarget>();
    const Analysis::NodeReceiver::Ptr input = AsyncWrap<Analysis::Node::Ptr>(opts.AnalysisThreads(), opts.AnalysisDataQueueSize(), analyser);
    return MakePtr<TransceivePipe<Analysis::Node::Ptr> >(input, analyser);
  }

  OpenPoint::Ptr CreateSource()
  {
    const OpenPoint::Ptr open = MakePtr<OpenPointImpl>();
    const ResolveDirsPoint::Ptr resolve = MakePtr<ResolveDirsPoint>();
    resolve->SetTarget(open);
    return MakePtr<TransceivePipe<String, Analysis::Node::Ptr> >(resolve, open);
  }

  class Options : public AnalysisOptions
                , public TargetOptions
  {
  public:
    Options()
      : AnalysisThreadsValue(1)
      , AnalysisDataQueueSizeValue(10)
      , TargetNameTemplateValue(Text::DEFAULT_TARGET_NAME_TEMPLATE)
      , IgnoreEmptyDataValue(false)
      , MinDataSizeValue(0)
      , FormatFilterValue()
      , SaveThreadsCountValue(1)
      , SaveDataQueueSizeValue(500)
      , StatisticOutputValue(false)
      //cmdline
      , OptionsDescription(Text::TARGET_SECTION)
    {
      using namespace boost::program_options;
      OptionsDescription.add_options()
        (Text::ANALYSIS_THREADS_KEY, value<std::size_t>(&AnalysisThreadsValue), Text::ANALYSIS_THREADS_DESC)
        (Text::ANALYSIS_QUEUE_SIZE_KEY, value<std::size_t>(&AnalysisDataQueueSizeValue), Text::ANALYSIS_QUEUE_SIZE_DESC)
        (Text::TARGET_NAME_TEMPLATE_KEY, value<String>(&TargetNameTemplateValue), Text::TARGET_NAME_TEMPLATE_DESC)
        (Text::IGNORE_EMPTY_KEY, bool_switch(&IgnoreEmptyDataValue), Text::IGNORE_EMPTY_DESC)
        (Text::MINIMAL_SIZE_KEY, value<std::size_t>(&MinDataSizeValue), Text::MINIMAL_SIZE_DESC)
        (Text::FORMAT_FILTER_KEY, value<std::string>(&FormatFilterValue), Text::FORMAT_FILTER_DESC)
        (Text::SAVE_THREADS_KEY, value<std::size_t>(&SaveThreadsCountValue), Text::SAVE_THREADS_DESC)
        (Text::SAVE_QUEUE_SIZE_KEY, value<std::size_t>(&SaveDataQueueSizeValue), Text::SAVE_QUEUE_SIZE_DESC)
        (Text::OUTPUT_STATISTIC_KEY, bool_switch(&StatisticOutputValue), Text::OUTPUT_STATISTIC_DESC)
       ;
    }

    std::size_t AnalysisThreads() const override
    {
      return AnalysisThreadsValue;
    }

    std::size_t AnalysisDataQueueSize() const override
    {
      return AnalysisDataQueueSizeValue;
    }

    String TargetNameTemplate() const override
    {
      return TargetNameTemplateValue;
    }

    bool IgnoreEmptyData() const override
    {
      return IgnoreEmptyDataValue;
    }

    std::size_t MinDataSize() const override
    {
      return MinDataSizeValue;
    }

    std::string FormatFilter() const override
    {
      return FormatFilterValue;
    }

    std::size_t SaveThreadsCount() const override
    {
      return SaveThreadsCountValue;
    }

    std::size_t SaveDataQueueSize() const override
    {
      return SaveDataQueueSizeValue;
    }

    bool StatisticOutput() const override
    {
      return StatisticOutputValue;
    }

    const boost::program_options::options_description& GetOptionsDescription() const
    {
      return OptionsDescription;
    }
  private:
    std::size_t AnalysisThreadsValue;
    std::size_t AnalysisDataQueueSizeValue;
    String TargetNameTemplateValue;
    bool IgnoreEmptyDataValue;
    std::size_t MinDataSizeValue;
    std::string FormatFilterValue;
    std::size_t SaveThreadsCountValue;
    std::size_t SaveDataQueueSizeValue;
    bool StatisticOutputValue;
    boost::program_options::options_description OptionsDescription;
  };
}

class MainApplication : public Platform::Application
{
public:
  MainApplication()
  {
  }
  
  int Run(Strings::Array args) override
  {
    Strings::Array paths;
    if (!ParseCmdline(std::move(args), paths))
    {
      return 1;
    }

    /*

                                           analyseThreads                 saveThreads
                                             |                              |
    file/dir -> [resolve] -> name -> [open] -*-> data -> [convert] -> data -*-> [filter] -> [createDir] -> [save]
                                        |           |            |                             |       |
                                       factory      +<-converted-+                            factory directory

    */
    const Analysis::NodeReceiver::Ptr result = CreateTarget(Opts);
    const Analysis::NodeTransceiver::Ptr analyse = CreateAnalyser(Opts);
    const OpenPoint::Ptr input = CreateSource();

    input->SetTarget(analyse);
    analyse->SetTarget(result);

    std::for_each(paths.begin(), paths.end(), boost::bind(&StringsReceiver::ApplyData, input.get(), _1));
    input->Flush();
    return 0;
  }
private:
  bool ParseCmdline(Strings::Array args, Strings::Array& paths) const
  {
    using namespace boost::program_options;
    options_description options(Strings::Format(Text::USAGE_SECTION, args[0]));
    options.add_options()
      (Text::HELP_KEY, Text::HELP_DESC)
      (Text::VERSION_KEY, Text::VERSION_DESC)
    ;
    options.add(Opts.GetOptionsDescription());
    options.add_options()
      (Text::INPUT_KEY, value<Strings::Array>(&paths), Text::INPUT_DESC)
    ;
    positional_options_description inputPositional;
    inputPositional.add(Text::INPUT_KEY, -1);

    variables_map vars;
    //args should not contain program name
    args.erase(args.begin());
    store(command_line_parser(args).options(options).positional(inputPositional).run(), vars);
    notify(vars);

    if (vars.count(Text::VERSION_KEY))
    {
      std::cout << Platform::Version::GetProgramVersionString() << std::endl;
      return false;
    }
    else if (vars.count(Text::HELP_KEY) || paths.empty())
    {
      std::cout << options << std::endl;
      return false;
    }
    return true;
  }
private:
  const Options Opts;
};

namespace Platform
{
  std::unique_ptr<Application> Application::Create()
  {
    return std::unique_ptr<Application>(new MainApplication());
  }
}
