/**
* 
* @file
*
* @brief  PSG support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "psg.h"
#include "formats/chiptune/chiptune_container.h"
//common includes
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <binary/typed_container.h>
//std includes
#include <cstring>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace PSG
  {
    enum
    {
      MARKER = 0x1a,

      INT_BEGIN = 0xff,
      INT_SKIP = 0xfe,
      MUS_END = 0xfd
    };

    const uint8_t SIGNATURE[] = {'P', 'S', 'G'};

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct Header
    {
      uint8_t Sign[3];
      uint8_t Marker;
      uint8_t Version;
      uint8_t Interrupt;
      uint8_t Padding[10];
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(Header) == 16, "Invalid layout");

    const std::size_t MIN_SIZE = sizeof(Header);

    class StubBuilder : public Builder
    {
    public:
      void AddChunks(std::size_t /*count*/) override {}
      void SetRegister(uint_t /*reg*/, uint_t /*val*/) override {}
    };

    bool FastCheck(const Binary::Container& rawData)
    {
      if (rawData.Size() <= sizeof(Header))
      {
        return false;
      }
      const Header* const header = static_cast<const Header*>(rawData.Start());
      return 0 == std::memcmp(header->Sign, SIGNATURE, sizeof(SIGNATURE)) &&
         MARKER == header->Marker;
    }

    const std::string FORMAT(
      "'P'S'G" // uint8_t Sign[3];
      "1a"     // uint8_t Marker;
    );

    class Decoder : public Formats::Chiptune::Decoder
    {
    public:
      Decoder()
        : Format(Binary::CreateFormat(FORMAT, MIN_SIZE))
      {
      }

      String GetDescription() const override
      {
        return Text::PSG_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return FastCheck(rawData);
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        Builder& stub = GetStubBuilder();
        return Parse(rawData, stub);
      }
    private:
      const Binary::Format::Ptr Format;
    };

    Formats::Chiptune::Container::Ptr Parse(const Binary::Container& rawData, Builder& target)
    {
      if (!FastCheck(rawData))
      {
        return Formats::Chiptune::Container::Ptr();
      }

      const Binary::TypedContainer& data(rawData);
      const Header& header = *data.GetField<Header>(0);
      //workaround for some emulators
      const std::size_t offset = (header.Version == INT_BEGIN) ? offsetof(Header, Version) : sizeof(header);
      std::size_t restSize = rawData.Size() - offset;
      const uint8_t* bdata = data.GetField<uint8_t>(offset);
      //detect as much chunks as possible, in despite of real format issues
      while (restSize)
      {
        const uint_t reg = *bdata;
        ++bdata;
        --restSize;
        if (INT_BEGIN == reg)
        {
          target.AddChunks(1);
        }
        else if (INT_SKIP == reg)
        {
          if (restSize < 1)
          {
            ++restSize;//put byte back
            break;
          }
          target.AddChunks(4 * *bdata);
          ++bdata;
          --restSize;
        }
        else if (MUS_END == reg)
        {
          break;
        }
        else if (reg <= 15) //register
        {
          if (restSize < 1)
          {
            ++restSize;//put byte back
            break;
          }
          target.SetRegister(reg, *bdata);
          ++bdata;
          --restSize;
        }
        else
        {
          ++restSize;//put byte back
          break;
        }
      }
      const std::size_t usedSize = rawData.Size() - restSize;
      const Binary::Container::Ptr subData = rawData.GetSubcontainer(0, usedSize);
      return CreateCalculatingCrcContainer(subData, offset, usedSize - offset);
    }

    Builder& GetStubBuilder()
    {
      static StubBuilder stub;
      return stub;
    }
  }//namespace PSG

  Decoder::Ptr CreatePSGDecoder()
  {
    return MakePtr<PSG::Decoder>();
  }
}//namespace Chiptune
}//namespace Formats
