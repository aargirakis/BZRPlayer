/**
* 
* @file
*
* @brief  GSF parser implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "gameboyadvancesoundformat.h"
//common includes
#include <byteorder.h>
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
#include <binary/input_stream.h>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace GameBoyAdvanceSoundFormat
  {
/*
Offset         Size    Description
0x0000000      4       GSF_Entry_Point
0x0000004      4       GSF_Offset
0x0000008      4       Size of Rom.
0x000000C      XX      The Rom data itself.
*/    
    void ParseRom(const Binary::Container& data, Builder& target)
    {
      Binary::InputStream stream(data);
      target.SetEntryPoint(fromLE(stream.ReadField<uint32_t>()));
      const auto addr = fromLE(stream.ReadField<uint32_t>());
      const std::size_t size = fromLE(stream.ReadField<uint32_t>());
      const std::size_t avail = stream.GetRestSize();
      target.SetRom(addr, *stream.ReadData(std::min(size, avail)));
    }

    const std::string FORMAT(
      "'P'S'F"
      "22"
    );
    
    class Decoder : public Formats::Chiptune::Decoder
    {
    public:
      Decoder()
        : Format(Binary::CreateMatchOnlyFormat(FORMAT))
      {
      }

      String GetDescription() const override
      {
        return Text::GAMEBOYADVANCESOUNDFORMAT_DECODER_DESCRIPTION;
      }

      Binary::Format::Ptr GetFormat() const override
      {
        return Format;
      }

      bool Check(const Binary::Container& rawData) const override
      {
        return Format->Match(rawData);
      }

      Formats::Chiptune::Container::Ptr Decode(const Binary::Container& /*rawData*/) const override
      {
        return Formats::Chiptune::Container::Ptr();//TODO
      }
    private:
      const Binary::Format::Ptr Format;
    };
  }

  Decoder::Ptr CreateGSFDecoder()
  {
    return MakePtr<GameBoyAdvanceSoundFormat::Decoder>();
  }
}
}
