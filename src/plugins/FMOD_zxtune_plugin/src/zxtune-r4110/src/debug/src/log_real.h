/**
* 
* @file
*
* @brief  Debug logging implementation
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <types.h>
//std includes
#include <cassert>
#include <string>
//library includes
#include <strings/format.h>

namespace Debug
{
  //! @brief Unconditionally outputs debug message
  void Message(const std::string& module, const std::string& msg);

  //! @brief Checks if debug logs are enabled for module
  bool IsEnabledFor(const std::string& module);

  /*
     @brief Per-module debug stream
     @code
       const Debug::Stream Dbg(THIS_MODULE);
       ...
       Dbg("message %1%", parameter);
     @endcode
  */
  class Stream
  {
  public:
    explicit Stream(const char* module)
      : Module(module)
      , Enabled(IsEnabledFor(Module))
    {
    }

    //! @brief Conditionally outputs debug message from specified module
    void operator ()(const char* msg) const
    {
      assert(msg);
      if (Enabled)
      {
        Message(Module, msg);
      }
    }

    //! @brief Conditionally outputs formatted debug message from specified module
    template<class... P>
    void operator ()(const char* msg, P&&... p) const
    {
      assert(msg);
      if (Enabled)
      {
        Message(Module, Strings::Format(msg, p...));
      }
    }
  private:
    const std::string Module;
    const bool Enabled;
  };
}
