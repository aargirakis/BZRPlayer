/**
* 
* @file
*
* @brief Module access interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "storage.h"
//library includes
#include <binary/binary_container.h>
#include <core/module_holder.h>

namespace Module
{
  typedef ObjectsStorage<Module::Holder::Ptr> Storage;

  Storage::HandleType Create(Binary::Container::Ptr data);
}
