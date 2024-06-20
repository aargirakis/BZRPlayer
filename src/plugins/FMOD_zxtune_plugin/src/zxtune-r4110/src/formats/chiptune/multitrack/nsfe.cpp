/**
* 
* @file
*
* @brief  NSFE chiptunes support
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "multitrack.h"
//text includes
#include <formats/text/chiptune.h>

namespace Formats
{
  namespace Chiptune
  {
    Decoder::Ptr CreateNSFEDecoder(Formats::Multitrack::Decoder::Ptr decoder)
    {
      return CreateMultitrackChiptuneDecoder(Text::NSFE_DECODER_DESCRIPTION, decoder);
    }
  }
}
