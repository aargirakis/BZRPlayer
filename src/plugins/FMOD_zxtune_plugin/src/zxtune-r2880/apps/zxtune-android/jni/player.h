/**
* 
* @file
*
* @brief Player access interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "storage.h"
//library includes
#include <core/module_holder.h>
#include <parameters/parameters_container.h>

namespace Player
{
  class Control
  {
  public:
    typedef boost::shared_ptr<Control> Ptr;
    virtual ~Control() {}

    virtual uint_t GetPosition() const = 0;
    virtual uint_t Analyze(uint_t maxEntries, uint32_t* bands, uint32_t* levels) const = 0;
    virtual Parameters::Container::Ptr GetParameters() const = 0;
    
    virtual bool Render(uint_t samples, int16_t* buffer) = 0;
    virtual void Seek(uint_t frame) = 0;
  };

  typedef ObjectsStorage<Control::Ptr> Storage;

  Storage::HandleType Create(Module::Holder::Ptr module);
}
