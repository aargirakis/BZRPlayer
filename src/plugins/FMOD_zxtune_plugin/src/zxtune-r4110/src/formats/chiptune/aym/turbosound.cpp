/**
* 
* @file
*
* @brief  TurboSound container support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "turbosound.h"
#include "formats/chiptune/chiptune_container.h"
//common includes
#include <byteorder.h>
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <binary/typed_container.h>
#include <math/numeric.h>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace TurboSound
  {
    const std::size_t MIN_SIZE = 256;
    const std::size_t MAX_MODULE_SIZE = 16384;
    const std::size_t MAX_SIZE = MAX_MODULE_SIZE * 2;

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct Footer
    {
      uint8_t ID1[4];//'PT3!' or other type
      uint16_t Size1;
      uint8_t ID2[4];//same
      uint16_t Size2;
      uint8_t ID3[4];//'02TS'
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(Footer) == 16, "Invalid layout");

    const std::string FOOTER_FORMAT(
      "%0xxxxxxx%0xxxxxxx%0xxxxxxx21"  // uint8_t ID1[4];//'PT3!' or other type
      "?%00xxxxxx"                     // uint16_t Size1;
      "%0xxxxxxx%0xxxxxxx%0xxxxxxx21"  // uint8_t ID2[4];//same
      "?%00xxxxxx"                     // uint16_t Size2;
      "'0'2'T'S"                       // uint8_t ID3[4];//'02TS'
    );

    class StubBuilder : public Builder
    {
    public:
      void SetFirstSubmoduleLocation(std::size_t /*offset*/, std::size_t /*size*/) override {}
      void SetSecondSubmoduleLocation(std::size_t /*offset*/, std::size_t /*size*/) override {}
    };

    class ModuleTraits
    {
    public:
      ModuleTraits(const Binary::Data& data, std::size_t footerOffset)
        : FooterOffset(footerOffset)
        , Foot(footerOffset != data.Size() ? safe_ptr_cast<const Footer*>(static_cast<const uint8_t*>(data.Start()) + footerOffset) : nullptr)
        , FirstSize(Foot ? fromLE(Foot->Size1) : 0)
        , SecondSize(Foot ? fromLE(Foot->Size2) : 0)
      {
      }

      bool Matched() const
      {
        return Foot != nullptr && FooterOffset == FirstSize + SecondSize && Math::InRange(FooterOffset, MIN_SIZE, MAX_SIZE);
      }

      std::size_t NextOffset() const
      {
        if (Foot == nullptr)
        {
          return FooterOffset;
        }
        const std::size_t totalSize = FirstSize + SecondSize;
        if (totalSize < FooterOffset)
        {
          return FooterOffset - totalSize;
        }
        else
        {
          return FooterOffset + sizeof(*Foot);
        }
      }

      std::size_t GetFirstModuleSize() const
      {
        return FirstSize;
      }

      std::size_t GetSecondModuleSize() const
      {
        return SecondSize;
      }

      std::size_t GetTotalSize() const
      {
        return FooterOffset + sizeof(*Foot);
      }
    private:
      const std::size_t FooterOffset;
      const Footer* const Foot;
      const std::size_t FirstSize;
      const std::size_t SecondSize;
    };

    class FooterFormat : public Binary::Format
    {
    public:
      typedef std::shared_ptr<const FooterFormat> Ptr;

      FooterFormat()
        : Delegate(Binary::CreateFormat(FOOTER_FORMAT))
      {
      }

      bool Match(const Binary::Data& data) const override
      {
        const ModuleTraits traits = GetTraits(data);
        return traits.Matched();
      }

      std::size_t NextMatchOffset(const Binary::Data& data) const override
      {
        const ModuleTraits traits = GetTraits(data);
        return traits.NextOffset();
      }

      ModuleTraits GetTraits(const Binary::Data& data) const
      {
        return ModuleTraits(data, Delegate->NextMatchOffset(data));
      }
    private:
      const Binary::Format::Ptr Delegate;
    };

    class DecoderImpl : public Decoder
    {
    public:
      DecoderImpl()
        : Format(MakePtr<FooterFormat>())
      {
      }

      String GetDescription() const override
      {
        return Text::TURBOSOUND_DECODER_DESCRIPTION;
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
        Builder& stub = GetStubBuilder();
        return Parse(rawData, stub);
      }

      Formats::Chiptune::Container::Ptr Parse(const Binary::Container& rawData, Builder& target) const override
      {
        const ModuleTraits& traits = Format->GetTraits(rawData);

        if (!traits.Matched())
        {
          return Formats::Chiptune::Container::Ptr();
        }

        target.SetFirstSubmoduleLocation(0, traits.GetFirstModuleSize());
        target.SetSecondSubmoduleLocation(traits.GetFirstModuleSize(), traits.GetSecondModuleSize());

        const std::size_t usedSize = traits.GetTotalSize();
        const Binary::Container::Ptr subData = rawData.GetSubcontainer(0, usedSize);
        //use whole container as a fixed data
        return CreateCalculatingCrcContainer(subData, 0, usedSize);
      }
    private:
      const FooterFormat::Ptr Format;
    };

    Builder& GetStubBuilder()
    {
      static StubBuilder stub;
      return stub;
    }

    Decoder::Ptr CreateDecoder()
    {
      return MakePtr<DecoderImpl>();
    }
  }//namespace TurboSound

  Decoder::Ptr CreateTurboSoundDecoder()
  {
    return TurboSound::CreateDecoder();
  }
}//namespace Chiptune
}//namespace Formats
