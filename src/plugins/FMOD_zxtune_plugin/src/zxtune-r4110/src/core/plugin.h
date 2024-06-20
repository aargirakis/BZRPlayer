/**
*
* @file
*
* @brief  Plugins related interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//common includes
#include <iterator.h>
#include <types.h>

namespace ZXTune
{
  //! @brief Basic plugin interface
  class Plugin
  {
  public:
    //! Pointer type
    typedef std::shared_ptr<const Plugin> Ptr;
    //! Iterator type
    typedef ObjectIterator<Plugin::Ptr> Iterator;

    //! Virtual destructor
    virtual ~Plugin() = default;

    //! Identification string
    virtual String Id() const = 0;
    //! Textual description
    virtual String Description() const = 0;
    //! Plugin capabilities @see plugin_attrs.h
    virtual uint_t Capabilities() const = 0;
  };

  Plugin::Iterator::Ptr EnumeratePlugins();
}
