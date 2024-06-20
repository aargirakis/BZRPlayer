/**
*
* @file
*
* @brief  PSF chiptune factory implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "psf.h"
#include "psf_bios.h"
#include "psf_exe.h"
#include "psf_vfs.h"
#include "xsf.h"
#include "xsf_factory.h"
//common includes
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <binary/container_factories.h>
#include <binary/compression/zlib_container.h>
#include <debug/log.h>
#include <module/attributes.h>
#include <module/players/analyzer.h>
#include <module/players/fading.h>
#include <module/players/streaming.h>
#include <parameters/tracking_helper.h>
#include <sound/chunk_builder.h>
#include <sound/render_params.h>
#include <sound/resampler.h>
#include <sound/sound_parameters.h>
//3rdparty includes
#include <3rdparty/he/Core/bios.h>
#include <3rdparty/he/Core/iop.h>
#include <3rdparty/he/Core/psx.h>
#include <3rdparty/he/Core/r3000.h>
#include <3rdparty/he/Core/spu.h>
//text includes
#include <module/text/platforms.h>

namespace Module
{
namespace PSF
{
  const Debug::Stream Dbg("Module::PSF");
 
  class VfsIO
  {
  public:
    VfsIO() = default;
    explicit VfsIO(PsxVfs::Ptr vfs)
      : Vfs(std::move(vfs))
    {
    }
    
    sint32 Read(const char* path, sint32 offset, char* buffer, sint32 length)
    {
      if (PreloadFile(path))
      {
        if (length == 0)
        {
          const auto result = GetSize();
          Dbg("Size()=%1%", result);
          return result;
        }
        else
        {
          const auto result = Read(offset, buffer, length);
          Dbg("Read(%2%@%1%)=%3%", offset, length, result);
          return result;
        }
      }
      return -1;
    }
  private:
    bool PreloadFile(const char* path) const
    {
      if (CachedName != path)
      {
        if (!Vfs->Find(path, CachedData))
        {
          Dbg("Not found '%1%'", path);
          return false;
        }
        Dbg("Open '%1%'", path);
        CachedName = path;
      }
      return true;
    }
    
    sint32 GetSize() const
    {
      return CachedData ? CachedData->Size() : 0;
    }
    
    sint32 Read(sint32 offset, char* buffer, sint32 length) const
    {
      if (CachedData)
      {
        if (const auto part = CachedData->GetSubcontainer(offset, length))
        {
          std::memcpy(buffer, part->Start(), part->Size());
          return part->Size();
        }
      }
      return 0;
    }
  private:
    PsxVfs::Ptr Vfs;
    mutable String CachedName;
    mutable Binary::Container::Ptr CachedData;
  };
  
  struct ModuleData
  {
    using Ptr = std::shared_ptr<const ModuleData>;
    using RWPtr = std::shared_ptr<ModuleData>;
    
    ModuleData() = default;
    ModuleData(const ModuleData&) = delete;
    
    uint_t Version = 0;
    PsxExe::Ptr Exe;
    PsxVfs::Ptr Vfs;
    XSF::MetaInformation::Ptr Meta;
    
    uint_t GetRefreshRate() const
    {
      if (Meta && Meta->RefreshRate)
      {
        return Meta->RefreshRate;
      }
      else if (Exe && Exe->RefreshRate)
      {
        return Exe->RefreshRate;
      }
      else
      {
        return 60;//NTSC by default
      }
    }
  };
  
  class HELibrary
  {
  private:
    HELibrary()
    {
      const auto& bios = GetSCPH10000HeBios();
      ::bios_set_embedded_image(bios.Start(), bios.Size());
      Require(0 == ::psx_init());
    }
    
  public:
    std::unique_ptr<uint8_t[]> CreatePSX(int version) const
    {
      std::unique_ptr<uint8_t[]> res(new uint8_t[::psx_get_state_size(version)]);
      ::psx_clear_state(res.get(), version);
      return res;
    }
    
    static const HELibrary& Instance()
    {
      static const HELibrary instance;
      return instance;
    }
  };
 
  struct SpuTrait
  {
    uint_t Base;
    uint_t PitchReg;
    uint_t VolReg;
  };
  
  const SpuTrait SPU1 = {0x1f801c00, 0x4, 0xc};//mirrored at {0x1f900000, 0x4, 0xa} for PS2
  const SpuTrait SPU2 = {0x1f900400, 0x4, 0xa};
  
  class PSXEngine : public Module::Analyzer
  {
  public:
    using Ptr = std::shared_ptr<PSXEngine>;
  
    void Initialize(const ModuleData& data)
    {
      if (data.Exe)
      {
        Emu = HELibrary::Instance().CreatePSX(1);
        SetupExe(*data.Exe);
        SoundFrequency = 44100;
        Spus.assign({SPU1});
      }
      else if (data.Vfs)
      {
        Emu = HELibrary::Instance().CreatePSX(2);
        SetupIo(data.Vfs);
        SoundFrequency = 48000;
        Spus.assign({SPU1, SPU2});
      }
      ::psx_set_refresh(Emu.get(), data.GetRefreshRate());
    }
    
    uint_t GetSoundFrequency() const
    {
      return SoundFrequency;
    }
    
    Sound::Chunk Render(uint_t samples)
    {
      Sound::Chunk result(samples);
      for (uint32_t doneSamples = 0; doneSamples < samples; )
      {
        uint32_t toRender = samples - doneSamples;
        const auto res = ::psx_execute(Emu.get(), 0x7fffffff, safe_ptr_cast<short int*>(&result[doneSamples]), &toRender, 0);
        Require(res >= 0);
        Require(toRender != 0);
        doneSamples += toRender;
      }
      return result;
    }
    
    void Skip(uint_t samples)
    {
      for (uint32_t skippedSamples = 0; skippedSamples < samples; )
      {
        uint32_t toSkip = samples - skippedSamples;
        const auto res = ::psx_execute(Emu.get(), 0x7fffffff, nullptr, &toSkip, 0);
        Require(res >= 0);
        Require(toSkip != 0);
        skippedSamples += toSkip;
      }
    }

    std::vector<ChannelState> GetState() const override
    {
      //http://problemkaputt.de/psx-spx.htm#soundprocessingunitspu
      const uint_t SPU_VOICES_COUNT = 24;
      std::vector<ChannelState> result;
      result.reserve(SPU_VOICES_COUNT * Spus.size());
      const auto iop = ::psx_get_iop_state(Emu.get());
      const auto spu = ::iop_get_spu_state(iop);
      for (const auto& trait : Spus)
      {
        for (uint_t voice = 0; voice < SPU_VOICES_COUNT; ++voice)
        {
          const auto voiceBase = trait.Base + (voice << 4);
          if (const auto pitch = static_cast<int16_t>(::spu_lh(spu, voiceBase + trait.PitchReg)))
          {
            const auto envVol = static_cast<int16_t>(::spu_lh(spu, voiceBase + trait.VolReg));
            if (envVol > 327)
            {
              ChannelState state;
              //0x1000 is for 44100Hz, assume it's C-8
              state.Band = pitch * 96 / 0x1000;
              state.Level = uint_t(envVol) * 100 / 32768;
              result.push_back(state);
            }
          }
        }
      }
      return result;
    }
  private:
    void SetupExe(const PsxExe& exe)
    {
      SetRAM(exe.RAM);
      SetRegisters(exe.PC, exe.SP);
    }
    
    void SetRAM(const MemoryRegion& mem)
    {
      const auto iop = ::psx_get_iop_state(Emu.get());
      ::iop_upload_to_ram(iop, mem.Start, mem.Data.data(), mem.Data.size());
    }
    
    void SetRegisters(uint32_t pc, uint32_t sp)
    {
      const auto iop = ::psx_get_iop_state(Emu.get());
      const auto cpu = ::iop_get_r3000_state(iop);
      ::r3000_setreg(cpu, R3000_REG_PC, pc);
      ::r3000_setreg(cpu, R3000_REG_GEN + 29, sp);
    }
    
    void SetupIo(PsxVfs::Ptr vfs)
    {
      Io = VfsIO(vfs);
      ::psx_set_readfile(Emu.get(), &ReadCallback, &Io);
    }
    
    static sint32 ReadCallback(void* context, const char* path, sint32 offset, char* buffer, sint32 length)
    {
      const auto io = static_cast<VfsIO*>(context);
      return io->Read(path, offset, buffer, length);
    }
  private:
    uint_t SoundFrequency = 0;
    std::vector<SpuTrait> Spus;
    std::unique_ptr<uint8_t[]> Emu;
    VfsIO Io;
  };
  
  class Renderer : public Module::Renderer
  {
  public:
    Renderer(ModuleData::Ptr data, Information::Ptr info, Sound::Receiver::Ptr target, Parameters::Accessor::Ptr params)
      : Data(std::move(data))
      , Iterator(Module::CreateStreamStateIterator(info))
      , State(Iterator->GetStateObserver())
      , Engine(MakePtr<PSXEngine>())
      , SoundParams(Sound::RenderParameters::Create(params))
      , Target(Module::CreateFadingReceiver(std::move(params), std::move(info), State, std::move(target)))
      , Looped()
    {
      Engine->Initialize(*Data);
      SamplesPerFrame = Engine->GetSoundFrequency() / Data->GetRefreshRate();
      ApplyParameters();
    }

    TrackState::Ptr GetTrackState() const override
    {
      return State;
    }

    Module::Analyzer::Ptr GetAnalyzer() const override
    {
      return Engine;
    }

    bool RenderFrame() override
    {
      try
      {
        ApplyParameters();

        Resampler->ApplyData(Engine->Render(SamplesPerFrame));
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
      SoundParams.Reset();
      Iterator->Reset();
      Engine->Initialize(*Data);
    }

    void SetPosition(uint_t frame) override
    {
      SeekTune(frame);
      Module::SeekIterator(*Iterator, frame);
    }
  private:
    void ApplyParameters()
    {
      if (SoundParams.IsChanged())
      {
        Looped = SoundParams->Looped();
        Resampler = Sound::CreateResampler(Engine->GetSoundFrequency(), SoundParams->SoundFreq(), Target);
      }
    }

    void SeekTune(uint_t frame)
    {
      uint_t current = State->Frame();
      if (frame < current)
      {
        Engine->Initialize(*Data);
        current = 0;
      }
      if (const uint_t delta = frame - current)
      {
        Engine->Skip(delta * SamplesPerFrame);
      }
    }
  private:
    const ModuleData::Ptr Data;
    const StateIterator::Ptr Iterator;
    const TrackState::Ptr State;
    const PSXEngine::Ptr Engine;
    uint_t SamplesPerFrame;
    Parameters::TrackingHelper<Sound::RenderParameters> SoundParams;
    const Sound::Receiver::Ptr Target;
    Sound::Receiver::Ptr Resampler;
    bool Looped;
  };

  class Holder : public Module::Holder
  {
  public:
    Holder(ModuleData::Ptr tune, Information::Ptr info, Parameters::Accessor::Ptr props)
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
      return MakePtr<Renderer>(Tune, Info, std::move(target), std::move(params));
    }
    
    static Ptr Create(ModuleData::Ptr tune, Parameters::Container::Ptr properties)
    {
      const auto period = Time::GetPeriodForFrequency<Time::Milliseconds>(tune->GetRefreshRate());
      const decltype(period) duration = tune->Meta->Duration;
      const uint_t frames = duration.Get() / period.Get();
      Information::Ptr info = CreateStreamInfo(frames);
      if (tune->Meta)
      {
        tune->Meta->Dump(*properties);
      }
      properties->SetValue(ATTR_PLATFORM, tune->Version == 1 ? Platforms::PLAYSTATION : Platforms::PLAYSTATION_2);
      properties->SetValue(Parameters::ZXTune::Sound::FRAMEDURATION, Time::Stamp<uint64_t, Parameters::ZXTune::Sound::FRAMEDURATION_PRECISION>(period).Get());
      return MakePtr<Holder>(std::move(tune), std::move(info), std::move(properties));
    }
  private:
    const ModuleData::Ptr Tune;
    const Information::Ptr Info;
    const Parameters::Accessor::Ptr Properties;
  };
  
  class ModuleDataBuilder
  {
  public:
    void AddExe(Binary::Data::Ptr packedSection)
    {
      Require(!Vfs);
      if (!Exe)
      {
        Exe = MakeRWPtr<PsxExe>();
      }
      const auto unpackedSection = Binary::Compression::Zlib::CreateDeferredDecompressContainer(std::move(packedSection));
      PsxExe::Parse(*unpackedSection, *Exe);
    }
    
    void AddVfs(const Binary::Container& reservedSection)
    {
      Require(!Exe);
      if (!Vfs)
      {
        Vfs = MakeRWPtr<PsxVfs>();
      }
      PsxVfs::Parse(reservedSection, *Vfs);
    }
    
    void AddMeta(const XSF::MetaInformation& meta)
    {
      if (!Meta)
      {
        Meta = MakeRWPtr<XSF::MetaInformation>(meta);
      }
      else
      {
        Meta->Merge(meta);
      }
    }
    
    ModuleData::Ptr CaptureResult(uint_t version)
    {
      auto res = MakeRWPtr<ModuleData>();
      res->Version = version;
      res->Exe = std::move(Exe);
      res->Vfs = std::move(Vfs);
      res->Meta = std::move(Meta);
      return res;
    }
  private:
    PsxExe::RWPtr Exe;
    PsxVfs::RWPtr Vfs;
    XSF::MetaInformation::RWPtr Meta;
  };
  
  class Factory : public XSF::Factory
  {
  public:
    Holder::Ptr CreateSinglefileModule(const XSF::File& file, Parameters::Container::Ptr properties) const override
    {
      ModuleDataBuilder builder;
      if (file.PackedProgramSection)
      {
        builder.AddExe(file.PackedProgramSection);
      }
      if (file.ReservedSection)
      {
        const auto clonedSection = Binary::CreateContainer(file.ReservedSection->Start(), file.ReservedSection->Size());
        builder.AddVfs(*clonedSection);
      }
      if (file.Meta)
      {
        builder.AddMeta(*file.Meta);
      }
      return Holder::Create(builder.CaptureResult(file.Version), std::move(properties));
    }
    
    Holder::Ptr CreateMultifileModule(const XSF::File& file, const std::map<String, XSF::File>& additionalFiles, Parameters::Container::Ptr properties) const override
    {
      ModuleDataBuilder builder;
      if (file.PackedProgramSection)
      {
        MergeExe(file, additionalFiles, builder);
      }
      if (file.ReservedSection)
      {
        MergeVfs(file, additionalFiles, builder);
      }
      MergeMeta(file, additionalFiles, builder);
      return Holder::Create(builder.CaptureResult(file.Version), std::move(properties));
    }
  private:
    /* https://bitbucket.org/zxtune/zxtune/wiki/MiniPSF
    
    The proper way to load a minipsf is as follows:
    - Load the executable data from the minipsf - this becomes the current executable.
    - Check for the presence of a "_lib" tag. If present:
      - RECURSIVELY load the executable data from the given library file. (Make sure to limit recursion to avoid crashing - I usually limit it to 10 levels)
      - Make the _lib executable the current one.
      - If applicable, we will use the initial program counter/stack pointer from the _lib executable.
      - Superimpose the originally loaded minipsf executable on top of the current executable. If applicable, use the start address and size to determine where to .
    - Check for the presence of "_libN" tags for N=2 and up (use "_lib%d")
      - RECURSIVELY load and superimpose all these EXEs on top of the current EXE. Do not modify the current program counter or stack pointer.
      - Start at N=2. Stop at the first tag name that doesn't exist.
    - (done)    
    */
    static void MergeExe(const XSF::File& data, const std::map<String, XSF::File>& additionalFiles, ModuleDataBuilder& dst)
    {
      auto it = data.Dependencies.begin();
      const auto lim = data.Dependencies.end();
      if (it != lim)
      {
        MergeExe(additionalFiles.at(*it), additionalFiles, dst);
      }
      dst.AddExe(data.PackedProgramSection);
      if (it != lim)
      {
        for (++it; it != lim; ++it)
        {
          MergeExe(additionalFiles.at(*it), additionalFiles, dst);
        }
      }
    }

    /* https://bitbucket.org/zxtune/zxtune/wiki/MiniPSF2
    
    The proper way to load a MiniPSF2 is as follows:
    - First, recursively load the virtual filesystems from each PSF2 file named by a library tag.
      - The first tag is "_lib"
      - The remaining tags are "_libN" for N>=2 (use "_lib%d")
      - Stop at the first tag name that doesn't exist.
    - Then, load the virtual filesystem from the current PSF2 file.

    If there are conflicting or redundant filenames, they should be overwritten in memory in the order in which the filesystem data was parsed. Later takes priority.
    */
    static void MergeVfs(const XSF::File& data, const std::map<String, XSF::File>& additionalFiles, ModuleDataBuilder& dst)
    {
      for (const auto& dep : data.Dependencies)
      {
        MergeVfs(additionalFiles.at(dep), additionalFiles, dst);
      }
      dst.AddVfs(*data.ReservedSection);
    }
    
    static void MergeMeta(const XSF::File& data, const std::map<String, XSF::File>& additionalFiles, ModuleDataBuilder& dst)
    {
      for (const auto& dep : data.Dependencies)
      {
        MergeMeta(additionalFiles.at(dep), additionalFiles, dst);
      }
      if (data.Meta)
      {
        dst.AddMeta(*data.Meta);
      }
    }
  };
  
  Module::Factory::Ptr CreateFactory()
  {
    return XSF::CreateFactory(MakePtr<Factory>());
  }
}
}
