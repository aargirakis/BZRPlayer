/**
 *
 * @file
 *
 * @brief  OGG vorbis parser interface
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#pragma once

// local includes
#include "formats/chiptune/builder_meta.h"
// library includes
#include <formats/chiptune.h>

namespace Formats
{
  namespace Chiptune
  {
    namespace OggVorbis
    {
      // Use simplified parsing due to thirdparty library used
      class Builder
      {
      public:
        virtual ~Builder() = default;

        virtual MetaBuilder& GetMetaBuilder() = 0;

        virtual void SetStreamId(uint32_t id) = 0;
        virtual void AddUnknownPacket(Binary::View data) = 0;
        virtual void SetProperties(uint_t channels, uint_t frequency, uint_t blockSizeLo, uint_t blockSizeHi) = 0;
        // Full setup block, including header
        virtual void SetSetup(Binary::View data) = 0;
        virtual void AddFrame(std::size_t offset, uint_t framesCount, Binary::View data) = 0;
      };

      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& data, Builder& target);
      Builder& GetStubBuilder();

      class DumpBuilder : public Builder
      {
      public:
        using Ptr = std::shared_ptr<DumpBuilder>;

        virtual Binary::Container::Ptr GetDump() = 0;
      };

      DumpBuilder::Ptr CreateDumpBuilder(std::size_t sizeHint);
    }  // namespace OggVorbis

    Decoder::Ptr CreateOGGDecoder();
  }  // namespace Chiptune
}  // namespace Formats
