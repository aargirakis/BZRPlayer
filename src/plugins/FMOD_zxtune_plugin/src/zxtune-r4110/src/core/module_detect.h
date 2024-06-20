/**
*
* @file
*
* @brief  Modules detecting functionality
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//library includes
#include <binary/container.h>
#include <core/data_location.h>
#include <core/plugin.h>
#include <module/holder.h>
#include <parameters/accessor.h>

//forward declarations
namespace Log
{
  class ProgressCallback;
}

//! @brief Global library namespace
namespace Module
{
  class DetectCallback
  {
  public:
    virtual ~DetectCallback() = default;

    //! @brief Process module
    virtual void ProcessModule(ZXTune::DataLocation::Ptr location, ZXTune::Plugin::Ptr decoder, Module::Holder::Ptr holder) const = 0;
    //! @brief Logging callback
    virtual Log::ProgressCallback* GetProgress() const = 0;
  };

  //! @brief Recursively search all the modules inside location
  //! @param params Parameters for plugins
  //! @param data Data to scan
  //! @param callback Detect callback
  void Detect(const Parameters::Accessor& params, Binary::Container::Ptr data, const DetectCallback& callback);

  //! @brief Opens module directly from location
  //! @param params Parameters for plugins
  //! @param location Start data location
  //! @param callback Detect callback
  //! @throw Error if no module found
  void Open(const Parameters::Accessor& params, Binary::Container::Ptr data, const String& subpath, const DetectCallback& callback);
}
