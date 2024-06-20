/**
* 
* @file
*
* @brief  GSF related stuff. ROM
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "memory_region.h"
//std includes
#include <memory>

namespace Binary
{
  class Container;
}

namespace Module
{
  namespace GSF
  {
    struct GbaRom
    {
      using Ptr = std::unique_ptr<const GbaRom>;
      using RWPtr = std::unique_ptr<GbaRom>;
      
      GbaRom() = default;
      GbaRom(const GbaRom&) = delete;
      GbaRom& operator = (const GbaRom&) = delete;
      
      uint32_t EntryPoint = 0;
      MemoryRegion Content;
      
      static void Parse(const Binary::Container& data, GbaRom& rom);
    };
  }
}
