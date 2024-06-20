/**
* 
* @file
*
* @brief  Game Music Emu-based formats support plugin
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "kss_supp.h"
#include "vgm_supp.h"
#include "core/plugins/player_plugins_registrator.h"
#include "core/plugins/players/plugin.h"
//common includes
#include <byteorder.h>
#include <contract.h>
#include <error.h>
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <binary/compression/zlib_stream.h>
#include <core/plugin_attrs.h>
#include <debug/log.h>
#include <devices/details/analysis_map.h>
#include <formats/chiptune/decoders.h>
#include <formats/multitrack/decoders.h>
#include <formats/chiptune/multitrack/decoders.h>
#include <formats/chiptune/multitrack/multitrack.h>
#include <math/numeric.h>
#include <module/attributes.h>
#include <module/players/duration.h>
#include <module/players/properties_helper.h>
#include <module/players/streaming.h>
#include <parameters/tracking_helper.h>
#include <sound/chunk_builder.h>
#include <sound/render_params.h>
#include <sound/sound_parameters.h>
#include <strings/optimize.h>
//std includes
#include <map>
//boost includes
#include <boost/algorithm/string/predicate.hpp>
//3rdparty
#include <3rdparty/gme/gme/Gbs_Emu.h>
#include <3rdparty/gme/gme/Hes_Emu.h>
#include <3rdparty/gme/gme/Nsf_Emu.h>
#include <3rdparty/gme/gme/Nsfe_Emu.h>
#include <3rdparty/gme/gme/Sap_Emu.h>
#include <3rdparty/gme/gme/Vgm_Emu.h>
#include <3rdparty/gme/gme/Gym_Emu.h>
#include <3rdparty/gme/gme/Kss_Emu.h>
//text includes
#include <module/text/platforms.h>

#define FILE_TAG 513E65A8

namespace Module
{
namespace GME
{
  const Debug::Stream Dbg("Core::GMESupp");

  typedef std::shared_ptr< ::Music_Emu> EmuPtr;

  typedef EmuPtr (*EmuCreator)();
  
  template<class EmuType>
  EmuPtr Create()
  {
    return EmuPtr(new EmuType());
  }
  
  class GME : public Module::Analyzer
  {
  public:
    typedef std::shared_ptr<GME> Ptr;
    
    GME(EmuCreator create, Dump data, uint_t track)
      : CreateEmu(create)
      , Data(std::move(data))
      , Track(track)
      , SoundFreq(0)
    {
      const uint_t FAKE_SOUND_FREQUENCY = 30000;
      Load(FAKE_SOUND_FREQUENCY);
    }
    
    uint_t GetTotalTracks() const
    {
      return Emu->track_count();
    }
    
    void GetInfo(::track_info_t& info) const
    {
      CheckError(Emu->track_info(&info, Track));
    }
    
    void Reset()
    {
      CheckError(Emu->start_track(Track));
    }
    
    void Render(uint_t samples, Sound::ChunkBuilder& target)
    {
      static_assert(Sound::Sample::CHANNELS == 2, "Incompatible sound channels count");
      static_assert(Sound::Sample::BITS == 16, "Incompatible sound bits count");
      ::Music_Emu::sample_t* const buffer = safe_ptr_cast< ::Music_Emu::sample_t*>(target.Allocate(samples));
      CheckError(Emu->play(samples * Sound::Sample::CHANNELS, buffer));
    }
    
    void Skip(uint_t samples)
    {
      CheckError(Emu->skip(samples));
    }
    
    void SetSoundFreq(uint_t soundFreq)
    {
      if (soundFreq != SoundFreq)
      {
        Reload(soundFreq);
      }
    }
    
    std::vector<ChannelState> GetState() const override
    {
      std::vector<voice_status_t> voices(Emu->voice_count());
      const int actual = Emu->voices_status(&voices[0], voices.size());
      std::vector<ChannelState> result(actual);
      for (int chan = 0; chan < actual; ++chan)
      {
        const voice_status_t& in = voices[chan];
        Devices::Details::AnalysisMap& analysis = GetAnalysisFor(in.frequency);
        ChannelState& state = result[chan];
        state.Level = in.level * 100 / voice_max_level;
        state.Band = analysis.GetBandByPeriod(in.divider);
      }
      //required by compiler
      return std::move(result);
    }
  private:
    inline static void CheckError(::blargg_err_t err)
    {
      if (err)
      {
        throw std::runtime_error(err);
      }
    }

    void Load(uint_t soundFreq)
    {
      auto emu = CreateEmu();
      CheckError(emu->set_sample_rate(soundFreq));
      CheckError(emu->load_mem(&Data.front(), Data.size()));
      Emu.swap(emu);
      SoundFreq = soundFreq;
    }

    void Reload(uint_t soundFreq)
    {
      const auto oldPos = Emu ? Emu->tell() : 0;
      Load(soundFreq);
      Reset();
      if (oldPos)
      {
        CheckError(Emu->seek(oldPos));
      }
    }
    
    Devices::Details::AnalysisMap& GetAnalysisFor(int freq) const
    {
      Devices::Details::AnalysisMap& result = Analysis[freq];
      result.SetClockRate(freq);
      return result;
    }
  private:
    const EmuCreator CreateEmu;
    const Dump Data;
    const uint_t Track;
    uint_t SoundFreq;
    EmuPtr Emu;
    mutable std::map<int, Devices::Details::AnalysisMap> Analysis;
  };
  
  class Renderer : public Module::Renderer
  {
  public:
    Renderer(GME::Ptr tune, StateIterator::Ptr iterator, Sound::Receiver::Ptr target, Parameters::Accessor::Ptr params)
      : Tune(std::move(tune))
      , Iterator(std::move(iterator))
      , State(Iterator->GetStateObserver())
      , SoundParams(Sound::RenderParameters::Create(std::move(params)))
      , Target(std::move(target))
      , Looped()
    {
      ApplyParameters();
    }

    TrackState::Ptr GetTrackState() const override
    {
      return State;
    }

    Module::Analyzer::Ptr GetAnalyzer() const override
    {
      return Tune;
    }

    bool RenderFrame() override
    {
      try
      {
        ApplyParameters();

        Sound::ChunkBuilder builder;
        const uint_t samplesPerFrame = SoundParams->SamplesPerFrame();
        builder.Reserve(samplesPerFrame);
        Tune->Render(samplesPerFrame, builder);
        Target->ApplyData(builder.CaptureResult());
        Iterator->NextFrame(Looped);
        return Iterator->IsValid();
      }
      catch (const std::exception&)
      {
        return false;
      }
    }

    void Reset() override
    {
      try
      {
        SoundParams.Reset();
        Iterator->Reset();
        Tune->Reset();
      }
      catch (const std::exception& e)
      {
        Dbg(e.what());
      }
    }

    void SetPosition(uint_t frame) override
    {
      try
      {
        SeekTune(frame);
      }
      catch (const std::exception& e)
      {
        Dbg(e.what());
      }
      Module::SeekIterator(*Iterator, frame);
    }
  private:
    void ApplyParameters()
    {
      if (SoundParams.IsChanged())
      {
        Looped = SoundParams->Looped();
        Tune->SetSoundFreq(SoundParams->SoundFreq());
      }
    }

    void SeekTune(uint_t frame)
    {
      uint_t current = State->Frame();
      if (frame < current)
      {
        Tune->Reset();
        current = 0;
      }
      if (const uint_t delta = frame - current)
      {
        Tune->Skip(delta * SoundParams->SamplesPerFrame());
      }
    }
  private:
    const GME::Ptr Tune;
    const StateIterator::Ptr Iterator;
    const TrackState::Ptr State;
    Parameters::TrackingHelper<Sound::RenderParameters> SoundParams;
    const Sound::Receiver::Ptr Target;
    bool Looped;
  };
  
  class Holder : public Module::Holder
  {
  public:
    Holder(GME::Ptr tune, Information::Ptr info, Parameters::Accessor::Ptr props)
      : Tune(std::move(tune))
      , Info(std::move(info))
      , Properties(std::move(props))
    {
    }

    Module::Information::Ptr GetModuleInformation() const override
    {
      return Info;
    }

    Parameters::Accessor::Ptr GetModuleProperties() const override
    {
      return Properties;
    }

    Renderer::Ptr CreateRenderer(Parameters::Accessor::Ptr params, Sound::Receiver::Ptr target) const override
    {
      try
      {
        return MakePtr<Renderer>(Tune, Module::CreateStreamStateIterator(Info), target, params);
      }
      catch (const std::exception& e)
      {
        throw Error(THIS_LINE, e.what());
      }
    }
  private:
    const GME::Ptr Tune;
    const Information::Ptr Info;
    const Parameters::Accessor::Ptr Properties;
  };
  
  //TODO: rework, extract GYM parsing code to Formats library
  Dump DefaultDataCreator(const Binary::Data& data)
  {
    return Dump(static_cast<const uint8_t*>(data.Start()), static_cast<const uint8_t*>(data.Start()) + data.Size());
  }
  
  using PlatformDetector = String (*)(const Dump&);
  
  struct PluginDescription
  {
    const Char* const Id;
    const uint_t ChiptuneCaps;
    const EmuCreator CreateEmu;
    const decltype(&DefaultDataCreator) CreateData;
    const PlatformDetector DetectPlatform;
  };
  
  namespace GYM
  {
    Dump CreateData(const Binary::Data& data)
    {
      Binary::DataInputStream input(data.Start(), data.Size());
      Binary::DataBuilder output(data.Size());
      const std::size_t packedSizeOffset = 424;
      output.Add(input.ReadRawData(packedSizeOffset), packedSizeOffset);
      if (const auto packedSize = fromLE(input.ReadField<uint32_t>()))
      {
        output.Add(uint32_t(0));
        Binary::Compression::Zlib::Decompress(input, output);
      }
      else
      {
        output.Add(packedSize);
        const auto rest = input.GetRestSize();
        output.Add(input.ReadRawData(rest), rest);
      }
      Dump result;
      output.CaptureResult(result);
      return result;
    }
  }
 
  const Time::Milliseconds PERIOD(20);
  
  Time::Milliseconds GetProperties(const GME& gme, PropertiesHelper& props)
  {
    ::track_info_t info;
    gme.GetInfo(info);
    
    const auto system = Strings::OptimizeAscii(info.system);
    const auto song = Strings::OptimizeAscii(info.song);
    const auto game = Strings::OptimizeAscii(info.game);
    const auto author = Strings::OptimizeAscii(info.author);
    const auto comment = Strings::OptimizeAscii(info.comment);
    const auto copyright = Strings::OptimizeAscii(info.copyright);
    const auto dumper = Strings::OptimizeAscii(info.dumper);
    
    props.SetComputer(system);
    props.SetTitle(game);
    props.SetTitle(song);
    props.SetProgram(game);
    props.SetAuthor(dumper);
    props.SetAuthor(author);
    props.SetComment(copyright);
    props.SetComment(comment);
    props.SetFramesFrequency(Time::GetFrequencyForPeriod(PERIOD));

    if (info.length > 0)
    {
      return Time::Milliseconds(info.length);
    }
    else if (info.loop_length > 0)
    {
      return Time::Milliseconds(info.intro_length + info.loop_length);
    }
    else
    {
      return Time::Milliseconds();
    }
  }
  
  class MultitrackFactory : public Module::Factory
  {
  public:
    MultitrackFactory(PluginDescription desc, Formats::Multitrack::Decoder::Ptr decoder)
      : Desc(std::move(desc))
      , Decoder(std::move(decoder))
    {
    }
    
    Module::Holder::Ptr CreateModule(const Parameters::Accessor& params, const Binary::Container& rawData, Parameters::Container::Ptr properties) const override
    {
      try
      {
        if (const Formats::Multitrack::Container::Ptr container = Decoder->Decode(rawData))
        {
          if (container->TracksCount() > 1)
          {
            Require(HasContainer(Desc.Id, properties));
          }

          PropertiesHelper props(*properties);
          auto data = Desc.CreateData(*container);
          props.SetPlatform(Desc.DetectPlatform(data));
          const GME::Ptr tune = MakePtr<GME>(Desc.CreateEmu, std::move(data), container->StartTrackIndex());
          const Time::Milliseconds storedDuration = GetProperties(*tune, props);
          const Time::Milliseconds duration = storedDuration == Time::Milliseconds() ? Time::Milliseconds(GetDuration(params)) : storedDuration;
          const Information::Ptr info = CreateStreamInfo(duration.Get() / PERIOD.Get());
        
          props.SetSource(*Formats::Chiptune::CreateMultitrackChiptuneContainer(container));
        
          return MakePtr<Holder>(tune, info, properties);
        }
      }
      catch (const std::exception& e)
      {
        Dbg("Failed to create %1%: %2%", Desc.Id, e.what());
      }
      return Module::Holder::Ptr();
    }
  private:
    static bool HasContainer(const String& type, Parameters::Accessor::Ptr params)
    {
      Parameters::StringType container;
      Require(params->FindValue(Module::ATTR_CONTAINER, container));
      return container == type || boost::algorithm::ends_with(container, ">" + type);
    }
  private:
    const PluginDescription Desc;
    const Formats::Multitrack::Decoder::Ptr Decoder;
  };
  
  class SingletrackFactory : public Module::Factory
  {
  public:
    SingletrackFactory(PluginDescription desc, Formats::Chiptune::Decoder::Ptr decoder)
      : Desc(std::move(desc))
      , Decoder(std::move(decoder))
    {
    }
    
    Module::Holder::Ptr CreateModule(const Parameters::Accessor& params, const Binary::Container& rawData, Parameters::Container::Ptr properties) const override
    {
      try
      {
        if (const Formats::Chiptune::Container::Ptr container = Decoder->Decode(rawData))
        {
          PropertiesHelper props(*properties);
          auto data = Desc.CreateData(*container);
          props.SetPlatform(Desc.DetectPlatform(data));
          const GME::Ptr tune = MakePtr<GME>(Desc.CreateEmu, std::move(data), 0);
          const Time::Milliseconds storedDuration = GetProperties(*tune, props);
          const Time::Milliseconds duration = storedDuration == Time::Milliseconds() ? Time::Milliseconds(GetDuration(params)) : storedDuration;
          const Information::Ptr info = CreateStreamInfo(duration.Get() / PERIOD.Get());
        
          props.SetSource(*container);
        
          return MakePtr<Holder>(tune, info, properties);
        }
      }
      catch (const std::exception& e)
      {
        Dbg("Failed to create %1%: %2%", Desc.Id, e.what());
      }
      return Module::Holder::Ptr();
    }
  private:
    const PluginDescription Desc;
    const Formats::Chiptune::Decoder::Ptr Decoder;
  };

  struct MultitrackPluginDescription
  {
    typedef Formats::Multitrack::Decoder::Ptr (*MultitrackDecoderCreator)();
    typedef Formats::Chiptune::Decoder::Ptr (*ChiptuneDecoderCreator)(Formats::Multitrack::Decoder::Ptr);

    PluginDescription Desc;
    const MultitrackDecoderCreator CreateMultitrackDecoder;
    const ChiptuneDecoderCreator CreateChiptuneDecoder;
  };
  
  const MultitrackPluginDescription MULTITRACK_PLUGINS[] =
  {
    //nsf
    {
      {
        "NSF",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::RP2A0X,
        &Create< ::Nsf_Emu>,
        &DefaultDataCreator,
        [](const Dump&) -> String {return Platforms::NINTENDO_ENTERTAINMENT_SYSTEM;}
      },
      &Formats::Multitrack::CreateNSFDecoder,
      &Formats::Chiptune::CreateNSFDecoder,
    },
    //nsfe
    {
      {
        "NSFE",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::RP2A0X,
        &Create< ::Nsfe_Emu>,
        &DefaultDataCreator,
        [](const Dump&) -> String {return Platforms::NINTENDO_ENTERTAINMENT_SYSTEM;}
      },
      &Formats::Multitrack::CreateNSFEDecoder,
      &Formats::Chiptune::CreateNSFEDecoder,
    },
    //gbs
    {
      {
        "GBS",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::LR35902,
        &Create< ::Gbs_Emu>,
        &DefaultDataCreator,
        [](const Dump&) -> String {return Platforms::GAME_BOY;}
      },
      &Formats::Multitrack::CreateGBSDecoder,
      &Formats::Chiptune::CreateGBSDecoder,
    },
    //kssx
    {
      {
        "KSSX",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::MULTI,
        &Create< ::Kss_Emu>,
        &DefaultDataCreator,
        &KSS::DetectPlatform
      },
      &Formats::Multitrack::CreateKSSXDecoder,
      &Formats::Chiptune::CreateKSSXDecoder
    },
    //hes
    {
      {
        "HES",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::HUC6270,
        &Create< ::Hes_Emu>,
        &DefaultDataCreator,
        [](const Dump&) -> String {return Platforms::PC_ENGINE;}
      },
      &Formats::Multitrack::CreateHESDecoder,
      &Formats::Chiptune::CreateHESDecoder
    },
  };
  
  struct SingletrackPluginDescription
  {
    typedef Formats::Chiptune::Decoder::Ptr (*ChiptuneDecoderCreator)();

    PluginDescription Desc;
    const ChiptuneDecoderCreator CreateChiptuneDecoder;
  };

  const SingletrackPluginDescription SINGLETRACK_PLUGINS[] =
  {
    //vgm
    {
      {
        "VGM",
        ZXTune::Capabilities::Module::Type::STREAM | ZXTune::Capabilities::Module::Device::MULTI,
        &Create< ::Vgm_Emu>,
        &DefaultDataCreator,
        &VGM::DetectPlatform
      },
      &Formats::Chiptune::CreateVideoGameMusicDecoder
    },
    //gym
    {
      {
        "GYM",
        ZXTune::Capabilities::Module::Type::STREAM | ZXTune::Capabilities::Module::Device::MULTI,
        &Create< ::Gym_Emu>,
        &GYM::CreateData,
        [](const Dump&) -> String {return Platforms::SEGA_GENESIS;}
      },
      &Formats::Chiptune::CreateGYMDecoder
    },
    //kss
    {
      {
        "KSS",
        ZXTune::Capabilities::Module::Type::MEMORYDUMP | ZXTune::Capabilities::Module::Device::MULTI,
        &Create< ::Kss_Emu>,
        &DefaultDataCreator,
        &KSS::DetectPlatform
      },
      &Formats::Chiptune::CreateKSSDecoder
    }
  };
}
}

namespace ZXTune
{
  void RegisterMultitrackGMEPlugins(PlayerPluginsRegistrator& registrator)
  {
    for (const auto& desc : Module::GME::MULTITRACK_PLUGINS)
    {
      const Formats::Multitrack::Decoder::Ptr multi = desc.CreateMultitrackDecoder();
      const Formats::Chiptune::Decoder::Ptr decoder = desc.CreateChiptuneDecoder(multi);
      const Module::Factory::Ptr factory = MakePtr<Module::GME::MultitrackFactory>(desc.Desc, multi);
      const PlayerPlugin::Ptr plugin = CreatePlayerPlugin(desc.Desc.Id, desc.Desc.ChiptuneCaps, decoder, factory);
      registrator.RegisterPlugin(plugin);
    }
  }
  
  void RegisterSingletrackGMEPlugins(PlayerPluginsRegistrator& registrator)
  {
    for (const auto& desc : Module::GME::SINGLETRACK_PLUGINS)
    {
      const Formats::Chiptune::Decoder::Ptr decoder = desc.CreateChiptuneDecoder();
      const Module::Factory::Ptr factory = MakePtr<Module::GME::SingletrackFactory>(desc.Desc, decoder);
      const PlayerPlugin::Ptr plugin = CreatePlayerPlugin(desc.Desc.Id, desc.Desc.ChiptuneCaps, decoder, factory);
      registrator.RegisterPlugin(plugin);
    }
  }

  void RegisterGMEPlugins(PlayerPluginsRegistrator& registrator)
  {
    RegisterMultitrackGMEPlugins(registrator);
    RegisterSingletrackGMEPlugins(registrator);
  }
}
