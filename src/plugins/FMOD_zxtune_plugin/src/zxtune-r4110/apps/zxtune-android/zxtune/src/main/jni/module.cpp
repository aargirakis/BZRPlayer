/**
* 
* @file
*
* @brief Module access implementation
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
#include <contract.h>
//library includes
#include <binary/container_factories.h>
#include <core/module_open.h>
#include <core/module_detect.h>
#include <module/additional_files.h>

namespace
{
  Module::Storage::HandleType CreateModule(Binary::Container::Ptr data, const String& subpath)
  {
    const Parameters::Accessor::Ptr options = Parameters::GlobalOptions();
    auto module = subpath.empty()
      ? Module::Open(*options, *data)
      : Module::Open(*options, data, subpath);
    Dbg("Module::Create(data=%p, subpath=%s)=%p", data.get(), subpath, module.get());
    return Module::Storage::Instance().Add(std::move(module));
  }

  class DetectCallback : public Module::DetectCallback
  {
  public:
    explicit DetectCallback(JNIEnv* env, jobject delegate)
      : Env(env)
      , Delegate(delegate)
      , CallbackClass()
      , OnModuleMethod()
    {
    }

    void ProcessModule(ZXTune::DataLocation::Ptr location, ZXTune::Plugin::Ptr /*decoder*/,
      Module::Holder::Ptr holder) const override
    {
      const jmethodID methodId = GetMethodId();
      const Jni::TempJString subpath(Env, location->GetPath()->AsString());
      const int handle = Module::Storage::Instance().Add(std::move(holder));
      Env->CallNonvirtualVoidMethod(Delegate, CallbackClass, methodId, subpath.Get(), handle);
      Jni::ThrowIfError(Env);
    }

    Log::ProgressCallback* GetProgress() const override
    {
      return nullptr;
    }
  private:
    jmethodID GetMethodId() const
    {
      if (!OnModuleMethod)
      {
        CallbackClass = Env->GetObjectClass(Delegate);
        OnModuleMethod = Env->GetMethodID(CallbackClass, "onModule", "(Ljava/lang/String;I)V");
      }
      return OnModuleMethod;
    }
  private:
    JNIEnv* const Env;
    const jobject Delegate;
    mutable jclass CallbackClass;
    mutable jmethodID OnModuleMethod;
  };

  void DetectModules(Binary::Container::Ptr data, Module::DetectCallback& cb)
  {
    Module::Detect(*Parameters::GlobalOptions(), std::move(data), cb);
  }

  Binary::Container::Ptr CreateContainer(JNIEnv* env, jobject buffer)
  {
    const auto addr = env->GetDirectBufferAddress(buffer);
    const auto capacity = env->GetDirectBufferCapacity(buffer);
    Require(capacity && addr);
    return Binary::CreateNonCopyContainer(addr, capacity);
  }
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1Create
  (JNIEnv* env, jclass /*self*/, jobject buffer, jstring subpath)
{
  return Jni::Call(env, [=] ()
  {
    return CreateModule(CreateContainer(env, buffer), Jni::MakeString(env, subpath));
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Module_1Close
  (JNIEnv* /*env*/, jclass /*self*/, jint handle)
{
  if (Module::Storage::Instance().Fetch(handle))
  {
    Dbg("Module::Close(handle=%1%)", handle);
  }
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Module_1Detect
  (JNIEnv* env, jclass /*self*/, jobject buffer, jobject cb)
{
  return Jni::Call(env, [=] ()
  {
    DetectCallback callbackAdapter(env, cb);
    DetectModules(CreateContainer(env, buffer), callbackAdapter);
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1GetDuration
  (JNIEnv* env, jclass /*self*/, jint moduleHandle)
{
  return Jni::Call(env, [=] ()
  {
    return Module::Storage::Instance().Get(moduleHandle)->GetModuleInformation()->FramesCount();
  });
}

JNIEXPORT jlong JNICALL Java_app_zxtune_ZXTune_Module_1GetProperty__ILjava_lang_String_2J
  (JNIEnv* env, jclass /*self*/, jint moduleHandle, jstring propName, jlong defVal)
{
  return Jni::Call(env, [=] ()
  {
    const auto& module = Module::Storage::Instance().Get(moduleHandle);
    const auto& params = module->GetModuleProperties();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  });
}

JNIEXPORT jstring JNICALL Java_app_zxtune_ZXTune_Module_1GetProperty__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv* env, jclass /*self*/, jint moduleHandle, jstring propName, jstring defVal)
{
  return Jni::Call(env, [=] ()
  {
    const auto& module = Module::Storage::Instance().Get(moduleHandle);
    const auto& params = module->GetModuleProperties();
    const Jni::PropertiesReadHelper props(env, *params);
    return props.Get(propName, defVal);
  });
}

JNIEXPORT jint JNICALL Java_app_zxtune_ZXTune_Module_1CreatePlayer
  (JNIEnv* env, jclass /*self*/, jint moduleHandle)
{
  return Jni::Call(env, [=] ()
  {
    Dbg("Module::CreatePlayer(handle=%x)", moduleHandle);
    const auto& module = Module::Storage::Instance().Get(moduleHandle);
    return Player::Create(module);
  });
}


JNIEXPORT jobjectArray JNICALL Java_app_zxtune_ZXTune_Module_1GetAdditionalFiles
  (JNIEnv* env, jclass /*self*/, jint moduleHandle)
{
  return Jni::Call(env, [=] ()
  {
    const auto& module = Module::Storage::Instance().Get(moduleHandle);
    if (const auto files = dynamic_cast<const Module::AdditionalFiles*>(module.get()))
    {
      const auto& filenames = files->Enumerate();
      if (const auto count = filenames.size())
      {
        const auto result = env->NewObjectArray(count, env->FindClass("java/lang/String"), nullptr);
        for (std::size_t i = 0; i < count; ++i)
        {
          env->SetObjectArrayElement(result, i, Jni::MakeJstring(env, filenames[i]));
        }
        return result;
      }
    }
    return jobjectArray(nullptr);
  });
}

JNIEXPORT void JNICALL Java_app_zxtune_ZXTune_Module_1ResolveAdditionalFile
  (JNIEnv* env, jclass /*self*/, jint moduleHandle, jstring fileName, jobject data)
{
  return Jni::Call(env, [=] ()
  {
    const auto& module = Module::Storage::Instance().Get(moduleHandle);
    auto& files = const_cast<Module::AdditionalFiles&>(dynamic_cast<const Module::AdditionalFiles&>(*module));
    files.Resolve(Jni::MakeString(env, fileName), CreateContainer(env, data));
  });
}
