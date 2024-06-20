#include <tools.h>
#include <formatter.h>
#include <core/module_holder.h>
#include <core/plugin.h>
#include <devices/aym.h>
#include <sound/backend.h>
#include <sound/error_codes.h>
#include <sound/render_params.h>

#include <iostream>
#include <iomanip>

#include <boost/thread.hpp>

namespace
{
  using namespace ZXTune;
  using namespace ZXTune::Sound;

  void ErrOuter(unsigned level, Error::LocationRef loc, Error::CodeType code, const String& text)
  {
    const String txt = (Formatter("\t%1%\n\tCode: %2%\n\tAt: %3%\n") % text % Error::CodeToString(code) % Error::LocationToString(loc)).str();
    if (level)
    {
      std::cerr << "\t-------\n";
    }
    std::cerr << txt;
  }
  
  bool ShowIfError(const Error& e)
  {
    if (e)
    {
      e.WalkSuberrors(ErrOuter);
    }
    return e;
  }

  void TestError(const char* msg, const Error& e)
  {
    const bool res(ShowIfError(e));
    std::cout << msg << " error test: " << (res ? "passed" : "failed") << "\n";
  }
  
  void TestSuccess(const char* msg, const Error& e)
  {
    const bool res(ShowIfError(e));
    std::cout << msg << " test: " << (res ? "failed" : "passed") << "\n";
  }
  
  class DummyHolder;
  Module::Player::Ptr CreateDummyPlayer(const DummyHolder& holder);
  
  class DummyPlugin : public Plugin
  {
  public:
    virtual String Id() const
    {
      return "Dummy";
    }

    virtual String Description() const
    {
      return "DummyPlugin";
    }

    virtual String Version() const
    {
      return "0";
    }

    virtual uint_t Capabilities() const
    {
      return 0;
    }
  };

  class DummyHolder : public Module::Holder
  {
  public:
    DummyHolder() {}
    
    virtual Plugin::Ptr GetPlugin() const
    {
      static Plugin::Ptr plugin(new DummyPlugin());
      return plugin;
    }
    
    virtual void GetModuleInformation(Module::Information& info) const
    {
      info.PhysicalChannels = 3;
    }
    
    virtual Module::Player::Ptr CreatePlayer() const
    {
      return CreateDummyPlayer(*this);
    }

    virtual Error Convert(const Module::Conversion::Parameter& param, Dump& dst) const
    {
      return Error();
    }
  };

  
  class DummyPlayer : public Module::Player
  {
  public:
    explicit DummyPlayer(const DummyHolder& holder)
      : Holder(holder)
      , Chip(AYM::CreateChip())
      , Chunk()
      , Frames()
    {
      Chunk.Mask = AYM::DataChunk::MASK_ALL_REGISTERS ^ (1 << AYM::DataChunk::REG_TONEN);
      Chunk.Data[AYM::DataChunk::REG_MIXER] = ~7;
      Chunk.Data[AYM::DataChunk::REG_VOLA] = Chunk.Data[AYM::DataChunk::REG_VOLB] = Chunk.Data[AYM::DataChunk::REG_VOLC] = 15;
    }
     
    virtual const Module::Holder& GetModule() const
    {
      return Holder;
    }
    
    virtual Error GetPlaybackState(Module::State&, Module::Analyze::ChannelsState&) const
    {
      return Error();
    }
    
    virtual Error RenderFrame(const RenderParameters& params, PlaybackState& state, MultichannelReceiver& receiver)
    {
      const uint16_t toneA((Frames + 1) * 1);
      const uint16_t toneB((Frames + 1) * 2);
      const uint16_t toneC((Frames + 1) * 3);
      Chunk.Data[AYM::DataChunk::REG_TONEA_L] = toneA & 0xff;
      Chunk.Data[AYM::DataChunk::REG_TONEA_H] = toneA >> 8;
      Chunk.Data[AYM::DataChunk::REG_TONEB_L] = toneB & 0xff;
      Chunk.Data[AYM::DataChunk::REG_TONEB_H] = toneB >> 8;
      Chunk.Data[AYM::DataChunk::REG_TONEC_L] = toneC & 0xff;
      Chunk.Data[AYM::DataChunk::REG_TONEC_H] = toneC >> 8;
      Chunk.Tick += params.ClocksPerFrame();
      Chip->RenderData(params, Chunk, receiver);
      Chunk.Mask &= ~(AYM::DataChunk::REG_VOLA | AYM::DataChunk::REG_VOLB | AYM::DataChunk::REG_VOLC);
      if (++Frames > 500)
      {
        state = MODULE_STOPPED;
      }
      else
      {
        state = MODULE_PLAYING;
      }
      return Error();
    }
    
    virtual Error Reset()
    {
      Chip->Reset();
      return Error();
    }
    
    virtual Error SetPosition(uint_t frame)
    {
      return Error();
    }
    
    virtual Error SetParameters(const Parameters::Map& params)
    {
      return Error();
    }
    
  private:
    const DummyHolder& Holder;
    const AYM::Chip::Ptr Chip;
    AYM::DataChunk Chunk;
    unsigned Frames;
  };
  
  Module::Player::Ptr CreateDummyPlayer(const DummyHolder& holder)
  {
    return Module::Player::Ptr(new DummyPlayer(holder));
  }
  
  void TestErrors(Backend& backend)
  {
    std::cout << "---- Test for error situations ---" << std::endl;

    TestError("Play", backend.Play());
    TestError("Pause", backend.Pause());
    TestError("Stop", backend.Stop());
    TestError("SetPosition", backend.SetPosition(0));
  }

  void TestVolume(Backend& backend)
  {
    std::cout << "Check for volume support: ";
    if (const VolumeControl::Ptr volCtrl = backend.GetVolumeControl())
    {
      std::cout << " checking for get: ";
      MultiGain volume;
      ThrowIfError(volCtrl->GetVolume(volume));
      std::cout << "{" << volume[0] << "," << volume[1] << "}\n";
      MultiGain newVol = { {1.0, 1.0} };
      std::cout << " checking for set: ";
      ThrowIfError(volCtrl->SetVolume(newVol));
      std::cout << "passed\n";
      //return previous
      ThrowIfError(volCtrl->SetVolume(volume));
    }
    else
    {
      std::cout << "unsupported\n";
    }
  }

  void TestBackend(const BackendCreator& creator)
  {
    std::cout << "Backend:\n"
    " Id: " << creator.Id() << "\n"
    " Version: " << creator.Version() << "\n"
    " Description: " << creator.Description() << "\n";

    Parameters::Map params;
    Backend::Ptr backend;
    ThrowIfError(creator.CreateBackend(params, backend));

    TestSuccess("Empty player set", backend->SetModule(Module::Holder::Ptr()));
    TestErrors(*backend);
    TestVolume(*backend);

    TestSuccess("Player set", backend->SetModule(Module::Holder::Ptr(new DummyHolder())));
    /*
    TestSuccess("Play", backend->Play());
    TestSuccess("Pause", backend->Pause());
    TestSuccess("Resume", backend->Play());
    TestSuccess("Stop", backend->Stop());
    */
    std::cout << "Playing [";
    ThrowIfError(backend->Play());
#if 0
//polling mode
    Backend::State state;
    for (;;)
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      std::cout << '.' << std::flush;
      ThrowIfError(backend->GetCurrentState(state));
      if (state == Backend::STOPPED)
      {
        break;
      }
    }
#else
    {
      SignalsCollector::Ptr signals = backend->CreateSignalsCollector(Backend::MODULE_STOP | Backend::MODULE_FINISH);
      uint_t signal = 0;
      while (!signals->WaitForSignals(signal, 1000))
      {
        std::cout << '.' << std::flush;
      }
    }
#endif
    std::cout << "] stopped" << std::endl;
  }
}

int main()
{
  using namespace ZXTune;
  using namespace ZXTune::Sound;
  
  for (BackendCreator::Iterator::Ptr iterator = EnumerateBackends(); iterator->IsValid(); iterator->Next())
  {
    try
    {
      const BackendCreator::Ptr creator = iterator->Get();
      TestBackend(*creator);
    }
    catch (const Error& e)
    {
      std::cout << " Failed\n";
      e.WalkSuberrors(ErrOuter);
    }
  }
}
