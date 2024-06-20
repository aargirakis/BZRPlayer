/**
* 
* @file
*
* @brief  SSF parser implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "segasaturnsoundformat.h"
//common includes
#include <make_ptr.h>
//library includes
#include <binary/format_factories.h>
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
namespace Chiptune
{
  namespace SegaSaturnSoundFormat
  {
    const std::string FORMAT(
      "'P'S'F"
      "11"
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
        return Text::SEGASATURNSOUNDFORMAT_DECODER_DESCRIPTION;
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

  Decoder::Ptr CreateSSFDecoder()
  {
    return MakePtr<SegaSaturnSoundFormat::Decoder>();
  }
}
}
