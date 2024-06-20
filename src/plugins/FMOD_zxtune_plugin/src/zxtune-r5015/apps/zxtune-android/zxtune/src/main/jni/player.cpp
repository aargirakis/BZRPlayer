/**
 *
 * @file
 *
 * @brief Player access implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "player.h"
#include "debug.h"
#include "exception.h"
#include "global_options.h"
#include "jni_player.h"
#include "module.h"
#include "properties.h"
// common includes
#include "contract.h"
#include <make_ptr.h>
#include <pointers.h>
// library includes
#include <module/players/pipeline.h>
#include <parameters/merged_accessor.h>
#include <parameters/tracking_helper.h>
#include <sound/impl/fft_analyzer.h>
#include <sound/mixer_factory.h>
#include <sound/render_params.h>
// std includes
#include <atomic>
#include <ctime>
#include <deque>

namespace
{
  class NativePlayerJni
  {
  public:
    static void Init(JNIEnv* env)
    {
      const auto tmpClass = env->FindClass("app/zxtune/core/jni/JniPlayer");
      Class = static_cast<jclass>(env->NewGlobalRef(tmpClass));
      Require(Class);
      Constructor = env->GetMethodID(Class, "<init>", "(I)V");
      Require(Constructor);
      Handle = env->GetFieldID(Class, "handle", "I");
      Require(Handle);
    }

    static void Cleanup(JNIEnv* env)
    {
      env->DeleteGlobalRef(Class);
      Class = nullptr;
      Constructor = 0;
      Handle = 0;
    }

    static Player::Storage::HandleType GetHandle(JNIEnv* env, jobject self)
    {
      return env->GetIntField(self, Handle);
    }

    static jobject Create(JNIEnv* env, Player::Storage::HandleType handle)
    {
      return env->NewObject(Class, Constructor, handle);
    }

  private:
    static jclass Class;
    static jmethodID Constructor;
    static jfieldID Handle;
  };

  jclass NativePlayerJni::Class;
  jmethodID NativePlayerJni::Constructor;
  jfieldID NativePlayerJni::Handle;

  static_assert(Sound::Sample::CHANNELS == 2, "Incompatible sound channels count");
  static_assert(Sound::Sample::BITS == 16, "Incompatible sound sample bits count");
  static_assert(Sound::Sample::MID == 0, "Incompatible sound sample type");

  class BufferTarget
  {
  public:
    void Add(Sound::Chunk data)
    {
      TotalSamples += data.size();
      Buffers.emplace_back(std::move(data));
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
      {}

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
      const uint_t MIN_DURATION_SEC = 1;
      const uint_t minSamples = sampleRate * MIN_DURATION_SEC;
      if (totalSamples >= minSamples)
      {
        if (const uint64_t totalClocks = Clocks + Frames / 2)  // compensate measuring error
        {
          // 100 * (totalSamples / sampleRate) / (totalClocks / CLOCKS_PER_SEC)
          return (totalSamples * CLOCKS_PER_SEC * 100) / (totalClocks * sampleRate);
        }
      }
      return 0;
    }

  private:
    std::clock_t LastStart = 0;
    std::clock_t Clocks = 0;
    uint_t Frames = 0;
  };

  class AnalyzerControl
  {
  public:
    AnalyzerControl()
      : Delegate(Sound::FFTAnalyzer::Create())
    {}

    void FrameStarted()
    {
      const std::scoped_lock guard(Lock);
      Current.Buffer.swap(Next.Buffer);
      Next.Parts = Current.Parts;
      Current.Parts = 0;
    }

    void FrameReady(uint_t count, void* buffer)
    {
      if (count && ++IdleFrames < 10)
      {
        const std::scoped_lock guard(Lock);
        const auto samples = count / Sound::Sample::CHANNELS;
        Next.Buffer.resize(samples);
        std::memcpy(Next.Buffer.data(), buffer, samples * sizeof(Sound::Sample));
      }
    }

    uint_t Analyze(uint_t maxEntries, uint8_t* levels) const
    {
      IdleFrames = 0;
      const std::scoped_lock guard(Lock);
      FeedPart();
      Delegate->GetSpectrum(safe_ptr_cast<Sound::Analyzer::LevelType*>(levels), maxEntries);
      return maxEntries;
    }

  private:
    struct State
    {
      Sound::Chunk Buffer;
      uint_t Parts = 0;
    };

    void FeedPart() const
    {
      const auto done = Current.Parts++;
      const auto total = std::max<uint_t>(Next.Parts, done + 1);
      if (const auto samples = Current.Buffer.size())
      {
        const auto start = samples * done / total;
        const auto end = samples * (done + 1) / total;
        Delegate->FeedSound(Current.Buffer.data() + start, end - start);
      }
      else
      {
        const Sound::Sample EMPTY[16];
        Delegate->FeedSound(EMPTY, 16);
      }
    }

  private:
    const Sound::FFTAnalyzer::Ptr Delegate;
    mutable std::mutex Lock;
    mutable State Current;
    State Next;
    mutable std::atomic<uint_t> IdleFrames = 0;
  };

  class PlayerControl : public Player::Control
  {
  public:
    PlayerControl(const Module::Holder& holder, uint_t samplerate, Parameters::Accessor::Ptr globalParams)
      : Duration(holder.GetModuleInformation()->Duration())
      , Samplerate(samplerate)
      , LocalParameters(Parameters::Container::Create())
      , Props(Parameters::CreateMergedAccessor(LocalParameters, std::move(globalParams)))
      , Renderer(Module::CreatePipelinedRenderer(holder, samplerate, MakeSingletonPointer(*Props)))
      , State(Renderer->GetState())
    {
      Require(Duration.Get() != 0);
    }

    Parameters::Modifier& GetParameters() const override
    {
      return *LocalParameters;
    }

    uint_t GetPosition() const override
    {
      return State->At().CastTo<Player::TimeBase>().Get();
    }

    uint_t Analyze(uint_t maxEntries, uint8_t* levels) const override
    {
      return Analyzer.Analyze(maxEntries, levels);
    }

    bool Render(uint_t samples, int16_t* buffer) override
    {
      Analyzer.FrameStarted();
      auto rest = samples;
      auto* target = buffer;
      for (;;)
      {
        if (const auto got = Buffer.GetSamples(rest, target))
        {
          target += got;
          rest -= got;
          if (!rest)
          {
            break;
          }
        }
        auto chunk = RenderNextFrame();
        if (chunk.empty())
        {
          break;
        }
        Buffer.Add(std::move(chunk));
      }
      std::fill_n(target, rest, 0);
      Analyzer.FrameReady(samples, buffer);
      return rest == 0;
    }

    void Seek(uint_t pos) override
    {
      Renderer->SetPosition(Time::Instant<Player::TimeBase>(pos));
    }

    uint_t GetPlaybackPerformance() const override
    {
      return RenderingPerformance.Measure(Buffer.GetTotalSamplesDone(), Samplerate);
    }

    // TODO: move to State
    uint_t GetPlaybackProgress() const override
    {
      const auto played = Time::Microseconds::FromRatio(Buffer.GetTotalSamplesDone(), Samplerate);
      return (played * 100).Divide<uint_t>(Duration);
    }

  private:
    Sound::Chunk RenderNextFrame()
    {
      ApplyParameters();
      RenderingPerformance.StartAccounting();
      auto chunk = Renderer->Render(Looped);
      RenderingPerformance.StopAccounting();
      return chunk;
    }

  private:
    void ApplyParameters()
    {
      if (Props.IsChanged())
      {
        Looped = Sound::GetLoopParameters(*Props);
      }
    }

  private:
    const Time::Milliseconds Duration;
    const uint_t Samplerate;
    const Parameters::Container::Ptr LocalParameters;
    Parameters::TrackingHelper<Parameters::Accessor> Props;
    const Module::Renderer::Ptr Renderer;
    const Module::State::Ptr State;
    BufferTarget Buffer;
    Sound::LoopParameters Looped;
    RenderingPerformanceAccountant RenderingPerformance;
    AnalyzerControl Analyzer;
  };

  Player::Control::Ptr CreateControl(const Module::Holder& module, uint_t samplerate)
  {
    return MakePtr<PlayerControl>(module, samplerate, MakeSingletonPointer(Parameters::GlobalOptions()));
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
    {}

    ~AutoArray()
    {
      if (Content)
      {
        Env->ReleasePrimitiveArrayCritical(Storage, Content, 0);
      }
    }

    operator bool() const
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
}  // namespace

namespace Player
{
  jobject Create(JNIEnv* env, const Module::Holder& module, uint_t samplerate)
  {
    auto ctrl = CreateControl(module, samplerate);
    Dbg("Player::Create(module=%p)=%p", &module, ctrl.get());
    const auto handle = Player::Storage::Instance().Add(std::move(ctrl));
    return NativePlayerJni::Create(env, handle);
  }

  void InitJni(JNIEnv* env)
  {
    NativePlayerJni::Init(env);
  }

  void CleanupJni(JNIEnv* env)
  {
    NativePlayerJni::Cleanup(env);
  }
}  // namespace Player

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniPlayer_close(JNIEnv* /*env*/, jclass /*self*/, jint handle)
{
  if (Player::Storage::Instance().Fetch(handle))
  {
    Dbg("Player::Close(handle=%1%)", handle);
  }
}

JNIEXPORT jboolean JNICALL Java_app_zxtune_core_jni_JniPlayer_render(JNIEnv* env, jobject self, jshortArray buffer)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    typedef AutoArray<jshortArray, int16_t> ArrayType;
    ArrayType buf(env, buffer);
    Jni::CheckArgument(buf, "Empty render buffer");
    if (const auto player = Player::Storage::Instance().Find(playerHandle))
    {
      return player->Render(buf.Size(), buf.Data());
    }
    else
    {
      return false;
    }
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_core_jni_JniPlayer_analyze(JNIEnv* env, jobject self, jbyteArray levels)
{
  return Jni::Call(env, [=]() {
    // Should be before AutoArray calls - else causes 'using JNI after critical get' error
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    const auto player = Player::Storage::Instance().Find(playerHandle);
    typedef AutoArray<jbyteArray, uint8_t> ArrayType;
    ArrayType rawLevels(env, levels);
    if (rawLevels && player)
    {
      return player->Analyze(rawLevels.Size(), rawLevels.Data());
    }
    else
    {
      return uint_t(0);
    }
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_core_jni_JniPlayer_getPositionMs(JNIEnv* env, jobject self)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Find(playerHandle))
    {
      return player->GetPosition();
    }
    else
    {
      return uint_t(0);
    }
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniPlayer_setPositionMs(JNIEnv* env, jobject self, jint position)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Find(playerHandle))
    {
      player->Seek(position);
    }
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_core_jni_JniPlayer_getPerformance(JNIEnv* env, jobject self)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Find(playerHandle))
    {
      return player->GetPlaybackPerformance();
    }
    else
    {
      return uint_t(0);
    }
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_core_jni_JniPlayer_getProgress(JNIEnv* env, jobject self)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Find(playerHandle))
    {
      return player->GetPlaybackProgress();
    }
    else
    {
      return uint_t(0);
    }
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniPlayer_setProperty__Ljava_lang_String_2J(JNIEnv* env, jobject self,
                                                                                            jstring propName,
                                                                                            jlong value)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Get(playerHandle))
    {
      auto& params = player->GetParameters();
      Jni::PropertiesWriteHelper helper(env, params);
      helper.Set(propName, value);
    }
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniPlayer_setProperty__Ljava_lang_String_2Ljava_lang_String_2(
    JNIEnv* env, jobject self, jstring propName, jstring value)
{
  return Jni::Call(env, [=]() {
    const auto playerHandle = NativePlayerJni::GetHandle(env, self);
    if (const auto player = Player::Storage::Instance().Get(playerHandle))
    {
      auto& params = player->GetParameters();
      Jni::PropertiesWriteHelper helper(env, params);
      helper.Set(propName, value);
    }
  });
}
