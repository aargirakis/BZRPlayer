/**
*
* @file
*
* @brief  Modules holder interface definition
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <binary/data.h>
#include <module/information.h>
#include <module/renderer.h>
#include <parameters/accessor.h>
#include <sound/receiver.h>

namespace Module
{
  //! @brief %Module holder interface
  class Holder
  {
  public:
    //! @brief Pointer type
    typedef std::shared_ptr<const Holder> Ptr;

    virtual ~Holder() = default;

    //! @brief Retrieving info about loaded module
    virtual Information::Ptr GetModuleInformation() const = 0;

    //! @brief Retrieving properties of loaded module
    virtual Parameters::Accessor::Ptr GetModuleProperties() const = 0;

    //! @brief Creating new renderer instance
    //! @return New player
    virtual Renderer::Ptr CreateRenderer(Parameters::Accessor::Ptr params, Sound::Receiver::Ptr target) const = 0;
  };

  Holder::Ptr CreateMixedPropertiesHolder(Holder::Ptr delegate, Parameters::Accessor::Ptr props);
}
