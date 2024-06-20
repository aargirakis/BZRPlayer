/**
* 
* @file
*
* @brief Playlist statistic operations interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "operations.h"

namespace Playlist
{
  namespace Item
  {
    class StatisticTextNotification : public Playlist::TextNotification
    {
    public:
      typedef std::shared_ptr<StatisticTextNotification> Ptr;

      virtual void AddInvalid() = 0;
      virtual void AddValid() = 0;
      virtual void AddType(const String& type) = 0;
      virtual void AddDuration(const Time::MillisecondsDuration& duration) = 0;
      virtual void AddSize(std::size_t size) = 0;
      virtual void AddPath(const String& path) = 0;
    };

    TextResultOperation::Ptr CreateCollectStatisticOperation(StatisticTextNotification::Ptr result);
    TextResultOperation::Ptr CreateCollectStatisticOperation(Playlist::Model::IndexSet::Ptr items, StatisticTextNotification::Ptr result);
  }
}
