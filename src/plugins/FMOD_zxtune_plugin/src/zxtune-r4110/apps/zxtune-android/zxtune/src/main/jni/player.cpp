/**
* 
* @file
*
* @brief Player access implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "debug.h"
#include "exception.h"
#include "global_options.h"
#include "module.h"
#include "player.h"
#include "properties.h"
#include "zxtune.h"
//common includes
#include "contract.h"
#include <make_ptr.h>
//library includes
#include <parameters/merged_accessor.h>
#include <sound/mixer_factory.h>
#include <sound/silence.h>
#include <sound/sound_parameters.h>
//std includes
#include <ctime>
#include <deque>

namespace
{
  static_assert(Sound::Sample::CHANNELS == 2, "Incompatible sound channels count");
  static_assert(Sound::Sample::BITS == 16, "Incompatible sound sample bits count");
  static_assert(Sound::Sample::MID == 0, "Incompatible sound sample type");

  class BufferTarget : public Sound::Receiver
  {
  public:
    typedef std::shared_ptr<BufferTarget> Ptr;

    void ApplyData(Sound::Chunk data) override
    {
      TotalSamples += data.size();
      Buffers.emplace_back(std::move(data));
    }

    void Flush() override
    {
    }

    std::size_t GetSamples(std::size_t count, int16_t* target)
    {
      if (Buffers.empty())
      {
        return 0;
      }
      Buff& cur = Buffers.front();
      const std::size_t samples = count / Sound::Sample::CHANNELS;
      const std::size_t copied = cur.Get(samples, target);
      if (0 == cur.Avail)
      {
        Buffers.pop_front();
      }
      return copied * Sound::Sample::CHANNELS;
    }
    
    uint64_t GetTotalSamplesDone() const
    {
      return TotalSamples;
    }
  private:
    struct Buff
    {
      explicit Buff(Sound::Chunk data)
        : Data(std::move(data))
        , Avail(Data.size())
      {
      }
      
      std::size_t Get(std::size_t count, void* target)
      {
        const std::size_t toCopy = std::min(count, Avail);
        std::memcpy(target, &Data.back() + 1 - Avail, toCopy * sizeof(Data.front()));
        Avail -= toCopy;
        return toCopy;
      }
    
      Sound::Chunk Data;
      std::size_t Avail;
    };
    std::deque<Buff> Buffers;
    uint64_t TotalSamples = 0;
  };
  
  class RenderingPerformanceAccountant
  {
  public:
    void StartAccounting()
    {
      LastStart = std::clock();
    }

    void StopAccounting()
    {
      Clocks += std::clock() - LastStart;
      ++Frames;
    }
    
    uint_t Measure(uint64_t totalSamples, uint_t sampleRate) const
    {
      if (const uint64_t totalClocks = Clocks + Frames / 2) //compensate measuring error
      {
        // 100 * (totalSamples / sampleRate) / (totalClocks / CLOCKS_PER_SEC)
        return (totalSamples * CLOCKS_PER_SEC * 100) / (totalClocks * sampleRate);
      }
      else
      {
        return 0;
      }
    }
  private:
    std::clock_t LastStart = 0;
    std::clock_t Clocks = 0;
    uint_t Frames = 0;
  };

  class PlayerControl : public Player::Control
  {
  public:
    PlayerControl(Parameters::Accessor::Ptr props, Parameters::Modifier::Ptr params, Module::Renderer::Ptr render, BufferTarget::Ptr buffer)
      : Props(std::move(props))
      , Params(std::move(params))
      , Renderer(std::move(render))
      , Buffer(std::move(buffer))
      , TrackState(Renderer->GetTrackState())
      , Analyser(Renderer->GetAnalyzer())
    {
    }
    
    Parameters::Accessor::Ptr GetProperties() const override
    {
      return Props;
    }
    
    Parameters::Modifier::Ptr GetParameters() const override
    {
      return Params;
    }
    
    uint_t GetPosition() const override
    {
      return TrackState->Frame();
    }

    uint_t Analyze(uint_t maxEntries, uint32_t* bands, uint32_t* levels) const override
    {
      const auto& result = Analyser->GetState();
      uint_t doneEntries = 0;
      for (auto it = result.begin(), lim = result.end(); it != lim && doneEntries != maxEntries; ++it, ++doneEntries)
      {
        bands[doneEntries] = it->Band;
        levels[doneEntries] = it->Level;
      }
      return doneEntries;
    }

    bool Render(uint_t samples, int16_t* buffer) override
    {
      bool hasMoreFrames = true;
      while (hasMoreFrames)
      {
        if (const std::size_t got = Buffer->GetSamples(samples, buffer))
        {
          buffer += got;
          samples -= got;
          if (!samples)
          {
            break;
          }
        }
        RenderingPerformance.StartAccounting();
        hasMoreFrames = Renderer->RenderFrame();
        RenderingPerformance.StopAccounting();
      }
      std::fill_n(buffer, samples, 0);
      return samples == 0;
    }

    void Seek(uint_t frame) override
    {
      Renderer->SetPosition(frame);
    }

    uint_t GetPlaybackPerformance() const override
    {
      Parameters::IntType sampleRate = Parameters::ZXTune::Sound::FREQUENCY_DEFAULT;
      Props->FindValue(Parameters::ZXTune::Sound::FREQUENCY, sampleRate);
      return RenderingPerformance.Measure(Buffer->GetTotalSamplesDone(), sampleRate);
    }
  private:
    const Parameters::Accessor::Ptr Props;
    const Parameters::Modifier::Ptr Params;
    const Module::Renderer::Ptr Renderer;
    const BufferTarget::Ptr Buffer;
    const Module::TrackState::Ptr TrackState;
    const Module::Analyzer::Ptr Analyser;
    RenderingPerformanceAccountant RenderingPerformance;
  };

  Player::Control::Ptr CreateControl(Module::Holder::Ptr module)
  {
    auto globalParameters = Parameters::GlobalOptions();
    auto localParameters = Parameters::Container::Create();
    auto internalProperties = module->GetModuleProperties();
    auto properties = Parameters::CreateMergedAccessor(localParameters, std::move(internalProperties), std::move(globalParameters));
    auto buffer = MakePtr<BufferTarget>();
    auto pipeline = Sound::CreateSilenceDetector(properties, buffer);
    auto renderer = module->CreateRenderer(properties, std::move(pipeline));
    return MakePtr<PlayerControl>(std::move(properties), std::move(localParameters), std::move(renderer), std::move(buffer));
  }

  template<class StorageType, class ResultType>
  class AutoArray
  {
  public:
    AutoArray(JNIEnv* env, StorageType storage)
      : Env(env)
      , Storage(storage)
      , Length(Env->GetArrayLength(Storage))
      , Content(static_cast<ResultType*>(Env->GetPrimitiveArrayCritical(Storage, 0)))
    {
    }

    ~AutoArray()
    {
      if (Content)
      {
        Env->ReleasePrimitiveArrayCritical(Storage, Content, 0);
      }
    }

    operator bool () const
    {
      return Length != 0 && Content != 0;
    }

    ResultType* Data() const
    {
      return Length ? Content : 0;
    }

    std::size_t Size() const
    {
      return Length;
    }
  private:
    JNIEnv* const Env;
    const StorageType Storage;
    const jsize Length;
    ResultType* const Content;
  };
}

namespace Player
{
  Player::Storage::HandleType Create(Module::Holder::Ptr module)
  {
    auto ctrl = CreateControl(module);
    Dbg("Player::Create(module=%p)=%p", module.get(), ctrl.get());
    return Player::Storage::Instance().Add(std::move(ctrl));
  }
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1Close
  (JNIEnv* /*env*/, jclass /*self*/, jint handle)
{
  if (Player::Storage::Instance().Fetch(handle))
  {
    Dbg("Player::Close(handle=%1%)", handle);
  }
}

JNIEXPORT jboolean JNICALL Java_app_zxtune_ZXTune_Player_1Render
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jshortArray buffer)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    typedef AutoArray<jshortArray, int16_t> ArrayType;
    ArrayType buf(env, buffer);
    Require(buf);
    return player->Render(buf.Size(), buf.Data());
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1Analyze
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jintArray bands, jintArray levels)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    typedef AutoArray<jintArray, uint32_t> ArrayType;
    ArrayType rawBands(env, bands);
    ArrayType rawLevels(env, levels);
    if (rawBands && rawLevels)
    {
      return player->Analyze(std::min(rawBands.Size(), rawLevels.Size()), rawBands.Data(), rawLevels.Data());
    }
    else
    {
      return uint_t(0);
    }
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1GetPosition
  (JNIEnv* env, jclass /*self*/, jint playerHandle)
{
  return Jni::Call(env, [=] ()
  {
    return Player::Storage::Instance().Get(playerHandle)->GetPosition();
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetPosition
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jint position)
{
  return Jni::Call(env, [=] ()
  {
    Player::Storage::Instance().Get(playerHandle)->Seek(position);
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Player_1GetPlaybackPerformance
  (JNIEnv* env, jclass /*self*/, jint playerHandle)
{
  return Jni::Call(env, [=] ()
  {
    return Player::Storage::Instance().Get(playerHandle)->GetPlaybackPerformance();
  });
}

JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong defVal)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    const auto& props= player->GetProperties();
    const Jni::PropertiesReadHelper helper(env, *props);
    return helper.Get(propName, defVal);
  });
}

JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Player_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring defVal)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    const auto& props = player->GetProperties();
    const Jni::PropertiesReadHelper helper(env, *props);
    return helper.Get(propName, defVal);
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jlong value)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    const auto& params = player->GetParameters();
    Jni::PropertiesWriteHelper helper(env, *params);
    helper.Set(propName, value);
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Player_1SetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint playerHandle, jstring propName, jstring value)
{
  return Jni::Call(env, [=] ()
  {
    const auto& player = Player::Storage::Instance().Get(playerHandle);
    const auto& params = player->GetParameters();
    Jni::PropertiesWriteHelper helper(env, *params);
    helper.Set(propName, value);
  });
}
