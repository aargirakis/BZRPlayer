/**
* 
* @file
*
* @brief  YM/VTX support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "ym.h"
#include "formats/chiptune/chiptune_container.h"
//common includes
#include <byteorder.h>
#include <contract.h>
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <binary/input_stream.h>
#include <binary/typed_container.h>
#include <debug/log.h>
#include <formats/packed/lha_supp.h>
#include <math/numeric.h>
#include <strings/optimize.h>
//std includes
#include <array>
#include <cstring>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace YM
  {
    const Debug::Stream Dbg("Formats::Chiptune::YM");

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    typedef std::array<uint8_t, 14> RegistersDump;
    typedef std::array<uint8_t, 4> IdentifierType;

    const uint_t CLOCKRATE_MIN = 100000;//100kHz
    const uint_t CLOCKRATE_MAX = 10000000;//10MHz

    const uint_t INTFREQ_MIN = 25;
    const uint_t INTFREQ_DEFAULT = 50;
    const uint_t INTFREQ_MAX = 100;
    
    const uint_t DURATION_MIN = 1;
    const uint_t DURATION_MAX = 30 * 60;

    const std::size_t MAX_STRING_SIZE = 254;

    namespace Ver2
    {
      const uint8_t ID[] = {'Y', 'M', '2', '!'};

      PACK_PRE struct RawHeader
      {
        IdentifierType Signature;
        RegistersDump Row;
      } PACK_POST;
      
      const std::size_t MIN_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MIN;
      const std::size_t MAX_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MAX;

      bool CheckMinSize(std::size_t size)
      {
        return size >= MIN_SIZE;
      }

      bool FastCheck(const void* data, std::size_t size)
      {
        return CheckMinSize(size) && 0 == std::memcmp(data, ID, sizeof(ID));
      }
    }

    namespace Ver3
    {
      const uint8_t ID[] = {'Y', 'M', '3', '!'};

      PACK_PRE struct RawHeader
      {
        IdentifierType Signature;
        RegistersDump Row;
      } PACK_POST;

      const std::size_t MIN_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MIN;
      const std::size_t MAX_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MAX;

      bool CheckMinSize(std::size_t size)
      {
        return size >= MIN_SIZE;
      }

      bool FastCheck(const void* data, std::size_t size)
      {
        return CheckMinSize(size) && 0 == std::memcmp(data, ID, sizeof(ID));
      }
    }

    namespace Ver3b
    {
      const uint8_t ID[] = {'Y', 'M', '3', 'b'};

      PACK_PRE struct RawHeader
      {
        IdentifierType Signature;
        RegistersDump Row;
      } PACK_POST;

      const std::size_t MIN_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MIN + sizeof(uint32_t);
      const std::size_t MAX_SIZE = sizeof(IdentifierType) + sizeof(RegistersDump) * INTFREQ_DEFAULT * DURATION_MAX + sizeof(uint32_t);

      bool CheckMinSize(std::size_t size)
      {
        return size >= MIN_SIZE;
      }

      bool FastCheck(const void* data, std::size_t size)
      {
        return CheckMinSize(size) && 0 == std::memcmp(data, ID, sizeof(ID));
      }
    }

    namespace Ver5
    {
      const uint8_t ID[] = {'Y', 'M', '5', '!', 'L', 'e','O', 'n', 'A', 'r', 'D', '!'};

      typedef std::array<uint8_t, 16> RegistersDump;
      typedef std::array<uint8_t, 4> Footer;

      PACK_PRE struct RawHeader
      {
        uint8_t Signature[12];
        uint32_t Frames;
        uint32_t Attrs;
        uint16_t SamplesCount;
        uint32_t Clockrate;
        uint16_t IntFreq;
        uint32_t Loop;
        uint16_t ExtraSize;

        bool Interleaved() const
        {
          return 0 != (fromLE(Attrs) & 0x01000000);
        }

        bool DrumsSigned() const
        {
          return 0 != (fromLE(Attrs) & 0x02000000);
        }

        bool Drums4Bit() const
        {
          return 0 != (fromLE(Attrs) & 0x04000000);
        }
      } PACK_POST;

      const std::size_t MIN_SIZE = sizeof(RawHeader) + sizeof(RegistersDump) * INTFREQ_MIN * DURATION_MIN;
      const std::size_t MAX_SIZE = sizeof(RawHeader) + sizeof(RegistersDump) * INTFREQ_MAX * DURATION_MAX;

      bool CheckMinSize(std::size_t size)
      {
        return size >= MIN_SIZE;
      }

      bool FastCheck(const void* data, std::size_t size)
      {
        return CheckMinSize(size) && 0 == std::memcmp(data, ID, sizeof(ID));
      }
    }

    namespace Ver6
    {
      const uint8_t ID[] = {'Y', 'M', '6', '!', 'L', 'e','O', 'n', 'A', 'r', 'D', '!'};

      bool FastCheck(const void* data, std::size_t size)
      {
        return Ver5::CheckMinSize(size) && 0 == std::memcmp(data, ID, sizeof(ID));
      }
    }

    namespace Compressed
    {
      PACK_PRE struct RawHeader
      {
        uint8_t HeaderSize;
        uint8_t HeaderSum;
        char Method[5];
        uint32_t PackedSize;
        uint32_t OriginalSize;
        uint32_t Time;
        uint8_t Attribute;
        uint8_t Level;
        uint8_t NameLen;

        std::size_t GetDataOffset() const
        {
          return offsetof(RawHeader, Method) + HeaderSize;
        }
      } PACK_POST;

      const std::size_t FOOTER_SIZE = 1;

      bool FastCheck(const void* data, std::size_t size)
      {
        if (size < sizeof(RawHeader))
        {
          return 0;
        }
        const RawHeader& hdr = *static_cast<const RawHeader*>(data);
        const std::size_t hdrLen = hdr.GetDataOffset();
        if (hdrLen + fromLE(hdr.PackedSize) + FOOTER_SIZE > size)
        {
          return false;
        }
        const std::size_t origSize = fromLE(hdr.OriginalSize);
        return Math::InRange(origSize, Ver2::MIN_SIZE, Ver2::MAX_SIZE)
            || Math::InRange(origSize, Ver3::MIN_SIZE, Ver3::MAX_SIZE)
            || Math::InRange(origSize, Ver3b::MIN_SIZE, Ver3b::MAX_SIZE)
            || Math::InRange(origSize, Ver5::MIN_SIZE, Ver5::MAX_SIZE)
        ;
      }
    }
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    class StubBuilder : public Builder
    {
    public:
      void SetVersion(const String& /*version*/) override {}
      void SetChipType(bool /*ym*/) override {}
      void SetStereoMode(uint_t /*mode*/) override {}
      void SetLoop(uint_t /*loop*/) override {}
      void SetDigitalSample(uint_t /*idx*/, const Dump& /*data*/) override {}
      void SetClockrate(uint64_t /*freq*/) override {}
      void SetIntFreq(uint_t /*freq*/) override {}
      void SetTitle(const String& /*title*/) override {}
      void SetAuthor(const String& /*author*/) override {}
      void SetComment(const String& /*comment*/) override {}
      void SetYear(uint_t /*year*/) override {}
      void SetProgram(const String& /*program*/) override {}
      void SetEditor(const String& /*editor*/) override {}

      void AddData(const Dump& /*registers*/) override {}
    };

    bool FastCheck(const Binary::Container& rawData)
    {
      const void* const data = rawData.Start();
      const std::size_t size = rawData.Size();
      return Ver2::FastCheck(data, size)
          || Ver3::FastCheck(data, size)
          || Ver3b::FastCheck(data, size)
          || Ver5::FastCheck(data, size)
          || Ver6::FastCheck(data, size)
      ;
    }

    void ParseTransponedMatrix(const uint8_t* data, std::size_t size, std::size_t rows, std::size_t columns, Builder& target)
    {
      Require(rows != 0);
      for (std::size_t row = 0; row != rows; ++row)
      {
        Dump registers(columns);
        for (std::size_t col = 0, cursor = row; col != columns && cursor < size; ++col, cursor += rows)
        {
          registers[col] = data[cursor];
        }
        target.AddData(registers);
      }
    }

    void ParseMatrix(const uint8_t* data, std::size_t size, std::size_t rows, std::size_t columns, Builder& target)
    {
      Require(rows != 0);
      const uint8_t* cursor = data, *limit = data + size;
      for (std::size_t row = 0; row != rows; ++row)
      {
        const uint8_t* const nextCursor = cursor + columns;
        if (nextCursor <= limit)
        {
          const Dump registers(cursor, nextCursor);
          target.AddData(registers);
        }
        else
        {
          Dump registers = cursor < limit
              ? Dump(cursor, limit)
              : Dump();
          registers.resize(columns);
          target.AddData(registers);
        }
        cursor = nextCursor;
      }
    }
    
    Formats::Chiptune::Container::Ptr ParseUnpacked(const Binary::Container& rawData, Builder& target)
    {
      const void* const data = rawData.Start();
      const std::size_t size = rawData.Size();
      try
      {
        if (Ver2::FastCheck(data, size)
         || Ver3::FastCheck(data, size)
         || Ver3b::FastCheck(data, size))
        {
          Binary::InputStream stream(rawData);
          const IdentifierType& type = stream.ReadField<IdentifierType>();
          target.SetVersion(String(type.begin(), type.end()));

          const std::size_t dumpOffset = sizeof(IdentifierType);
          const std::size_t dumpSize = size - dumpOffset;
          const std::size_t columns = sizeof(RegistersDump);
          const std::size_t lines = dumpSize / columns;
          const std::size_t matrixSize = lines * columns;
          const auto src = stream.ReadRawData(matrixSize);
          ParseTransponedMatrix(src, matrixSize, lines, columns, target);
          if (Ver3b::FastCheck(data, size))
          {
            const uint_t loop = fromBE(stream.ReadField<uint32_t>());
            target.SetLoop(loop);
          }
          const auto subData = stream.GetReadData();
          return CreateCalculatingCrcContainer(subData, dumpOffset, matrixSize);
        }
        else if (Ver5::FastCheck(data, size)
              || Ver6::FastCheck(data, size))
        {
          Binary::InputStream stream(rawData);
          const Ver5::RawHeader& header = stream.ReadField<Ver5::RawHeader>();
          if (0 != header.SamplesCount)
          {
            Dbg("Digital samples are not supported");
            return Formats::Chiptune::Container::Ptr();
          }
          target.SetVersion(String(header.Signature, header.Signature + sizeof(IdentifierType)));
          target.SetClockrate(fromBE(header.Clockrate));
          target.SetIntFreq(fromBE(header.IntFreq));
          target.SetLoop(fromBE(header.Loop));
          target.SetTitle(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
          target.SetAuthor(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
          target.SetComment(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));

          const std::size_t dumpOffset = stream.GetPosition();
          const std::size_t dumpSize = size - sizeof(Ver5::Footer) - dumpOffset;
          const std::size_t lines = fromBE(header.Frames);
          Dbg("ymver5: dump started at %1%, size %2%, vtbls %3%", dumpOffset, dumpSize, lines);
          const std::size_t columns = sizeof(Ver5::RegistersDump);
          const std::size_t matrixSize = dumpSize;
          const std::size_t availLines = dumpSize / columns;
          if (availLines != lines)
          {
            Dbg("available only %1% lines", availLines);
          }
          const auto src = stream.ReadRawData(matrixSize);
          if (header.Interleaved())
          {
            ParseTransponedMatrix(src, matrixSize, lines, columns, target);
          }
          else
          {
            ParseMatrix(src, matrixSize, lines, columns, target);
          }
          const auto subData = stream.GetReadData();
          return CreateCalculatingCrcContainer(subData, dumpOffset, matrixSize);
        }
      }
      catch (const std::exception&)
      {
        Dbg("Failed to parse");
      }
      return Formats::Chiptune::Container::Ptr();
    }

    const std::string FORMAT(
      "'Y'M"
      "'2-'6"
      "'!|'b"
    );
      
    Formats::Chiptune::Container::Ptr ParsePacked(const Binary::Container& rawData, Builder& target)
    {
      const void* const data = rawData.Start();
      const std::size_t size = rawData.Size();
      if (!Compressed::FastCheck(data, size))
      {
        return Formats::Chiptune::Container::Ptr();
      }
      const Compressed::RawHeader& hdr = *static_cast<const Compressed::RawHeader*>(data);
      const std::size_t packedOffset = hdr.GetDataOffset();
      const std::size_t packedSize = fromLE(hdr.PackedSize);
      const Binary::Container::Ptr packed = rawData.GetSubcontainer(packedOffset, packedSize);
      const std::size_t unpackedSize = fromLE(hdr.OriginalSize);
      if (const Formats::Packed::Container::Ptr unpacked = Formats::Packed::Lha::DecodeRawData(*packed, FromCharArray(hdr.Method), unpackedSize))
      {
        if (ParseUnpacked(*unpacked, target))
        {
          const Binary::Container::Ptr subData = rawData.GetSubcontainer(0, packedOffset + packedSize + Compressed::FOOTER_SIZE);
          return CreateCalculatingCrcContainer(subData, packedOffset, packedSize);
        }
      }
      return Formats::Chiptune::Container::Ptr();
    }

    const std::string PACKED_FORMAT(
      "16-ff"      //header size
      "?"          //header sum
      "'-'l'h'5'-" //method
      "????"       //packed size
      "????"       //original size
      "????"       //time+date
      "%00x00xxx"  //attribute
      "00"         //level
    );

    class YMDecoder : public Formats::Chiptune::YM::Decoder
    {
    public:
      YMDecoder()
        //disable seeking due to slight format  
        : Format(Binary::CreateMatchOnlyFormat(FORMAT))
      {
      }

      String GetDescription() const override
      {
        return Text::YM_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        if (!Format->Match(rawData))
        {
          return Formats::Chiptune::Container::Ptr();
        }
        Builder& stub = GetStubBuilder();
        return ParseUnpacked(rawData, stub);
      }

      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& data, Builder& target) const override
      {
        return ParseUnpacked(data, target);
      }
    private:
      const Binary::Format::Ptr Format;
    };

    class PackedDecoder : public Formats::Chiptune::YM::Decoder
    {
    public:
      PackedDecoder()
        : Format(Binary::CreateFormat(PACKED_FORMAT))
      {
      }

      String GetDescription() const override
      {
        return Text::YM_PACKED_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        if (!Format->Match(rawData))
        {
          return Formats::Chiptune::Container::Ptr();
        }
        Builder& stub = GetStubBuilder();
        return ParsePacked(rawData, stub);
      }

      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& data, Builder& target) const override
      {
        return ParsePacked(data, target);
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }

  namespace VTX
  {
    using namespace YM;

    const uint_t CHIP_AY = 0x7961;
    const uint_t CHIP_YM = 0x6d79;
    const uint_t CHIP_AY_OLD = 0x5941;
    const uint_t CHIP_YM_OLD = 0x4d59;

    const uint_t LAYOUT_MASK = 7;
    const uint_t LAYOUT_MIN = 0;
    const uint_t LAYOUT_MAX = 6;

    const uint_t UNPACKED_MIN = sizeof(RegistersDump) * INTFREQ_MIN * 1;//1 sec
    const uint_t UNPACKED_MAX = sizeof(RegistersDump) * INTFREQ_MAX * 30 * 60;//30 min

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct RawBasicHeader
    {
      uint16_t ChipType;
      uint8_t LayoutMode;
      uint16_t Loop;
      uint32_t Clockrate;
      uint8_t IntFreq;
    } PACK_POST;

    PACK_PRE struct RawNewHeader : RawBasicHeader
    {
      uint16_t Year;
      uint32_t UnpackedSize;
    } PACK_POST;

    PACK_PRE struct RawOldHeader : RawBasicHeader
    {
      uint32_t UnpackedSize;
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(RawBasicHeader) == 10, "Invalid layout");
    static_assert(sizeof(RawNewHeader) == 16, "Invalid layout");
    static_assert(sizeof(RawOldHeader) == 14, "Invalid layout");

    template<class HeaderType>
    bool FastCheck(const HeaderType& hdr)
    {
      if (!Math::InRange<uint_t>(hdr.LayoutMode & LAYOUT_MASK, LAYOUT_MIN, LAYOUT_MAX))
      {
        return false;
      }
      if (!Math::InRange<uint_t>(hdr.IntFreq, INTFREQ_MIN, INTFREQ_MAX))
      {
        return false;
      }
      if (!Math::InRange<uint_t>(fromLE(hdr.Clockrate), CLOCKRATE_MIN, CLOCKRATE_MAX))
      {
        return false;
      }
      if (!Math::InRange<uint_t>(fromLE(hdr.UnpackedSize), UNPACKED_MIN, UNPACKED_MAX))
      {
        return false;
      }
      return true;
    }

    bool FastCheck(const Binary::Container& rawData)
    {
      const Binary::TypedContainer typedData(rawData);
      if (const RawBasicHeader* basic = typedData.GetField<RawBasicHeader>(0))
      {
        const uint16_t type = fromLE(basic->ChipType);
        if (type == CHIP_AY || type == CHIP_YM)
        {
          if (const RawNewHeader* hdr = typedData.GetField<RawNewHeader>(0))
          {
            return FastCheck(*hdr);
          }
        }
        else if (type == CHIP_AY_OLD || type == CHIP_YM_OLD)
        {
          if (const RawOldHeader* hdr = typedData.GetField<RawOldHeader>(0))
          {
            return FastCheck(*hdr);
          }
        }
      }
      return false;
    }

    Formats::Chiptune::Container::Ptr ParseVTX(const Binary::Container& rawData, Builder& target)
    {
      if (!FastCheck(rawData))
      {
        return Formats::Chiptune::Container::Ptr();
      }
      try
      {
        Binary::InputStream stream(rawData);
        const RawBasicHeader& hdr = stream.ReadField<VTX::RawBasicHeader>();
        const uint_t chipType = fromLE(hdr.ChipType);
        const bool ym = chipType == CHIP_YM || chipType == CHIP_YM_OLD;
        const bool newVersion = chipType == CHIP_YM || chipType == CHIP_AY;
        target.SetChipType(ym);
        target.SetStereoMode(hdr.LayoutMode);
        target.SetLoop(fromLE(hdr.Loop));
        target.SetClockrate(fromLE(hdr.Clockrate));
        target.SetIntFreq(hdr.IntFreq);
        if (newVersion)
        {
          target.SetYear(fromLE(stream.ReadField<uint16_t>()));
        }
        const uint_t unpackedSize = fromLE(stream.ReadField<uint32_t>());
        target.SetTitle(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
        target.SetAuthor(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
        if (newVersion)
        {
          target.SetProgram(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
          target.SetEditor(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
          target.SetComment(Strings::OptimizeAscii(stream.ReadCString(MAX_STRING_SIZE)));
        }

        const std::size_t packedOffset = stream.GetPosition();
        Dbg("Packed data at %1%", packedOffset);
        const Binary::Container::Ptr packed = stream.ReadRestData();
        if (const Packed::Container::Ptr unpacked = Packed::Lha::DecodeRawData(*packed, "-lh5-", unpackedSize))
        {
          const std::size_t doneSize = unpacked->Size();
          const std::size_t columns = sizeof(RegistersDump);
          Require(0 == (unpackedSize % columns));
          const std::size_t lines = doneSize / columns;
          ParseTransponedMatrix(static_cast<const uint8_t*>(unpacked->Start()), doneSize, lines, columns, target);
          const std::size_t packedSize = unpacked->PackedSize();
          const Binary::Container::Ptr subData = rawData.GetSubcontainer(0, packedOffset + packedSize);
          return CreateCalculatingCrcContainer(subData, packedOffset, packedSize);
        }
        Dbg("Failed to decode ym data");
      }
      catch (const std::exception&)
      {
        Dbg("Failed to parse");
      }
      return Formats::Chiptune::Container::Ptr();
    }

    const std::string FORMAT(
      "('a|'A|'y|'Y)('y|'Y|'m|'M)" //type
      "00-06"          //layout
      "??"             //loop
      "??01-9800"      //clockrate
      "19-64"          //intfreq, 25..100Hz
    );

    class Decoder : public Formats::Chiptune::YM::Decoder
    {
    public:
      Decoder()
        : Format(Binary::CreateFormat(FORMAT))
      {
      }

      String GetDescription() const override
      {
        return Text::VTX_DECODER_DESCRIPTION;
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
        return ParseVTX(rawData, stub);
      }

      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& data, Builder& target) const override
      {
        return ParseVTX(data, target);
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }//namespace VTX

  namespace YM
  {
    Builder& GetStubBuilder()
    {
      static StubBuilder stub;
      return stub;
    }

    Decoder::Ptr CreatePackedYMDecoder()
    {
      return MakePtr<YMDecoder>();
    }

    Decoder::Ptr CreateYMDecoder()
    {
      return MakePtr<PackedDecoder>();
    }

    Decoder::Ptr CreateVTXDecoder()
    {
      return MakePtr<VTX::Decoder>();
    }
  }

  Formats::Chiptune::Decoder::Ptr CreatePackedYMDecoder()
  {
    return YM::CreatePackedYMDecoder();
  }

  Formats::Chiptune::Decoder::Ptr CreateYMDecoder()
  {
    return YM::CreateYMDecoder();
  }

  Formats::Chiptune::Decoder::Ptr CreateVTXDecoder()
  {
    return YM::CreateVTXDecoder();
  }
}//namespace Chiptune
}//namespace Formats
