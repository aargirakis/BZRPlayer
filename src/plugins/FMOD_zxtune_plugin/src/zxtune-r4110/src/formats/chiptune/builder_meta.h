/**
* 
* @file
*
* @brief  Metadata builder interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <types.h>
//library includes
#include <strings/array.h>

namespace Formats
{
  namespace Chiptune
  {
    class MetaBuilder
    {
    public:
      virtual ~MetaBuilder() = default;

      virtual void SetProgram(const String& program) = 0;
      virtual void SetTitle(const String& title) = 0;
      virtual void SetAuthor(const String& author) = 0;
      virtual void SetStrings(const Strings::Array& strings) = 0;
    };

    MetaBuilder& GetStubMetaBuilder();
  }
}
