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

//common includes
#include <error.h>
//platform includes
#include <jni.h>
//std includes
#include <type_traits>

namespace Jni
{
  class Exception
  {
  public:
    explicit Exception(const char* type)
      : Exception(type, "")
    {
    }
    
    Exception(const char* type, const char* msg)
      : Type(type)
      , Message(msg)
    {
    }
    
    const char* GetType() const
    {
      return Type;
    }
    
    const char* GetMessage() const
    {
      return Message;
    }
  private:
    const char* const Type;
    const char* const Message;
  };
  
  class NullPointerException : public Exception
  {
  public:
    NullPointerException()
      : Exception("java/lang/NullPointerException")
    {
    }
  };

  inline void Throw(JNIEnv* env, const char* clsName, const char* msg)
  {
    const auto cls = env->FindClass(clsName);
    env->ThrowNew(cls, msg);
  }
  
  inline void Throw(JNIEnv* env, const char* msg)
  {
    Throw(env, "java/lang/Exception", msg);
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
    static void Get()
    {
    }
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
}
