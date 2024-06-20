/**
 *
 * @file
 *
 * @brief Exception helpers
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#pragma once

// common includes
#include <error.h>
// platform includes
#include <jni.h>
// std includes
#include <type_traits>

namespace Jni
{
  class Exception
  {
  public:
    explicit Exception(const char* type)
      : Exception(type, "")
    {}

    Exception(const char* type, const char* msg)
      : Type(type)
      , Message(msg)
    {}

    const char* GetType() const
    {
      return Type;
    }

    const char* GetMessage() const
    {
      return Message;
    }

  protected:
    const char* const Type;
    const char* Message;
  };

  class NullPointerException : public Exception
  {
  public:
    NullPointerException()
      : Exception("java/lang/NullPointerException")
    {}
  };

  class IllegalArgumentException : public Exception
  {
  public:
    explicit IllegalArgumentException(const char* msg)
      : Exception("java/lang/IllegalArgumentException", msg)
    {}
  };

  inline void CheckArgument(bool condition, const char* msg = "")
  {
    if (!condition)
    {
      throw IllegalArgumentException(msg);
    }
  }

  class ResolvingException : public Exception
  {
  public:
    explicit ResolvingException(String msg)
      : Exception("app/zxtune/core/ResolvingException")
      , MessageStorage(std::move(msg))
    {
      Message = MessageStorage.c_str();
    }

  private:
    const String MessageStorage;
  };

  inline void Throw(JNIEnv* env, const char* clsName, const char* msg)
  {
    const auto cls = env->FindClass(clsName);
    env->ThrowNew(cls, msg);
  }

  inline void Throw(JNIEnv* env, const char* msg)
  {
    Throw(env, "app/zxtune/core/jni/JniRuntimeException", msg);
  }

  inline void Throw(JNIEnv* env, const Error& err)
  {
    Throw(env, err.GetText().c_str());
  }

  inline void Throw(JNIEnv* env, const std::exception& err)
  {
    Throw(env, err.what());
  }

  inline void Throw(JNIEnv* env, jthrowable thr)
  {
    env->ExceptionClear();
    env->Throw(thr);
  }

  inline void Throw(JNIEnv* env, const Exception& ex)
  {
    Throw(env, ex.GetType(), ex.GetMessage());
  }

  inline void ThrowIfError(JNIEnv* env)
  {
    if (const jthrowable e = env->ExceptionOccurred())
    {
      env->ExceptionDescribe();
      throw e;
    }
  }

  template<class T>
  struct DefaultReturn
  {
    static T Get()
    {
      return T();
    }
  };

  template<>
  struct DefaultReturn<void>
  {
    static void Get() {}
  };

  template<class Func>
  typename std::result_of<Func()>::type Call(JNIEnv* env, Func f)
  {
    try
    {
      return f();
    }
    catch (jthrowable e)
    {
      Throw(env, e);
    }
    catch (const Exception& e)
    {
      Throw(env, e);
    }
    catch (const Error& e)
    {
      Throw(env, e);
    }
    catch (const std::exception& e)
    {
      Throw(env, e);
    }
    catch (...)
    {
      Throw(env, "Unknown error");
    }
    return DefaultReturn<typename std::result_of<Func()>::type>::Get();
  }
}  // namespace Jni
