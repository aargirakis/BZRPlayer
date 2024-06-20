/**
* 
* @file
*
* @brief  GBS support implementation
*
* @author vitamin.caig@gmail.com
*
**/

//common includes
#include <byteorder.h>
#include <contract.h>
#include <crc_.h>
#include <make_ptr.h>
#include <pointers.h>
//library includes
#include <binary/container_factories.h>
#include <binary/format_factories.h>
#include <formats/multitrack.h>
#include <math/numeric.h>
//std includes
#include <array>
#include <cstring>
#include <utility>

namespace Formats
{
namespace Multitrack
{
  namespace GBS
  {
    typedef std::array<uint8_t, 3> SignatureType;
    
    typedef std::array<char, 32> StringType;

    const SignatureType SIGNATURE = {{'G', 'B', 'S'}};

#ifdef USE_PRAGMA_PACK
#pragma pack(push,1)
#endif
    PACK_PRE struct RawHeader
    {
      SignatureType Signature;
      uint8_t Version;
      uint8_t SongsCount;
      uint8_t StartSong; //1-based
      uint16_t LoadAddr;
      uint16_t InitAddr;
      uint16_t PlayAddr;
      uint16_t StackPointer;
      uint8_t TimerModulo;
      uint8_t TimerControl;
      StringType Title;
      StringType Artist;
      StringType Copyright;
    } PACK_POST;
#ifdef USE_PRAGMA_PACK
#pragma pack(pop)
#endif

    static_assert(sizeof(RawHeader) == 112, "Invalid layout");
    
    const std::size_t MAX_SIZE = 1048576;

    const std::string FORMAT =
      "'G'B'S"
      "01"    //version
      "01-ff" //1 song minimum
      "01-ff" //first song
      //do not pay attention to addresses
     ;
     
    const std::size_t MIN_SIZE = 256;

    const RawHeader* GetHeader(const Binary::Data& rawData)
    {
      if (rawData.Size() < MIN_SIZE)
      {
        return nullptr;
      }
      const RawHeader* hdr = safe_ptr_cast<const RawHeader*>(rawData.Start());
      if (hdr->Signature != SIGNATURE)
      {
        return nullptr;
      }
      if (!hdr->SongsCount || !hdr->StartSong)
      {
        return nullptr;
      }
      return hdr;
    }
    
    class Container : public Formats::Multitrack::Container
    {
    public:
      Container(const RawHeader* hdr, Binary::Container::Ptr data)
        : Hdr(hdr)
        , Delegate(std::move(data))
      {
      }
      
      //Binary::Container
      const void* Start() const override
      {
        return Delegate->Start();
      }

      std::size_t Size() const override
      {
        return Delegate->Size();
      }

      Binary::Container::Ptr GetSubcontainer(std::size_t offset, std::size_t size) const override
      {
        return Delegate->GetSubcontainer(offset, size);
      }
      
      //Formats::Multitrack::Container
      uint_t FixedChecksum() const override
      {
        //just skip text fields
        const uint8_t* const data = static_cast<const uint8_t*>(Delegate->Start());
        const uint32_t part1 = Crc32(data, offsetof(RawHeader, Title));
        const uint32_t part2 = Crc32(data + sizeof(RawHeader), Delegate->Size() - sizeof(RawHeader), part1);
        return part2;
      }

      uint_t TracksCount() const override
      {
        return Hdr->SongsCount;
      }

      uint_t StartTrackIndex() const override
      {
        return Hdr->StartSong - 1;
      }
      
      Container::Ptr WithStartTrackIndex(uint_t idx) const override
      {
        std::unique_ptr<Dump> content(new Dump(Delegate->Size()));
        std::memcpy(&content->front(), Delegate->Start(), content->size());
        RawHeader* const hdr = safe_ptr_cast<RawHeader*>(&content->front());
        Require(idx < hdr->SongsCount);
        hdr->StartSong = idx + 1;
        return MakePtr<Container>(hdr, Binary::CreateContainer(std::move(content)));
      }
    private:
      const RawHeader* const Hdr;
      const Binary::Container::Ptr Delegate;
    };

    class Decoder : public Formats::Multitrack::Decoder
    {
    public:
      //Use match only due to lack of end detection
      Decoder()
        : Format(Binary::CreateMatchOnlyFormat(FORMAT, MIN_SIZE))
      {
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Multitrack::Container::Ptr Decode(const Binary::Container& rawData) const override
      {
        if (const RawHeader* hdr = GetHeader(rawData))
        {
          const Binary::Container::Ptr used = rawData.GetSubcontainer(0, std::min(rawData.Size(), MAX_SIZE));
          return MakePtr<Container>(hdr, used);
        }
        else
        {
          return Formats::Multitrack::Container::Ptr();
        }
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }//namespace GBS

  Decoder::Ptr CreateGBSDecoder()
  {
    return MakePtr<GBS::Decoder>();
  }
}//namespace Multitrack
}//namespace Formats
