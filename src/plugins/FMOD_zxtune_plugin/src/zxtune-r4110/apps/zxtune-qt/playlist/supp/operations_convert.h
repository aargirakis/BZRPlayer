/**
* 
* @file
*
* @brief Convert operation factory
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "conversion.h"
#include "operations.h"
//library includes
#include <sound/service.h>

namespace Playlist
{
  namespace Item
  {
    //export
    class ConversionResultNotification : public Playlist::TextNotification
    {
    public:
      typedef std::shared_ptr<ConversionResultNotification> Ptr;

      virtual void AddSucceed() = 0;
      virtual void AddFailedToOpen(const String& path) = 0;
      virtual void AddFailedToConvert(const String& path, const Error& err) = 0;
    };

    TextResultOperation::Ptr CreateSoundFormatConvertOperation(Playlist::Model::IndexSet::Ptr items,
      const String& type, Sound::Service::Ptr service, ConversionResultNotification::Ptr result);

    TextResultOperation::Ptr CreateExportOperation(const String& nameTemplate,
      Parameters::Accessor::Ptr params, ConversionResultNotification::Ptr result);
    TextResultOperation::Ptr CreateExportOperation(Playlist::Model::IndexSet::Ptr items,
      const String& nameTemplate, Parameters::Accessor::Ptr params, ConversionResultNotification::Ptr result);

    //dispatcher over factories described above
    TextResultOperation::Ptr CreateConvertOperation(Playlist::Model::IndexSet::Ptr items, const Conversion::Options& opts, ConversionResultNotification::Ptr result);
    TextResultOperation::Ptr CreateConvertOperation(const Conversion::Options& opts, ConversionResultNotification::Ptr result);
  }
}
