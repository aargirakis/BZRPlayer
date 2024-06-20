/**
* 
* @file
*
* @brief  DAC-based chiptune interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <devices/dac.h>
#include <parameters/accessor.h>
#include <module/information.h>
#include <module/players/iterator.h>

namespace Module
{
  namespace DAC
  {
    class DataIterator : public StateIterator
    {
    public:
      typedef std::shared_ptr<DataIterator> Ptr;

      virtual void GetData(Devices::DAC::Channels& data) const = 0;
    };

    class Chiptune
    {
    public:
      typedef std::shared_ptr<const Chiptune> Ptr;
      virtual ~Chiptune() = default;

      virtual Information::Ptr GetInformation() const = 0;
      virtual Parameters::Accessor::Ptr GetProperties() const = 0;
      virtual DataIterator::Ptr CreateDataIterator() const = 0;
      virtual void GetSamples(Devices::DAC::Chip::Ptr chip) const = 0;
    };
  }
}
