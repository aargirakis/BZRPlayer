/**
* 
* @file
*
* @brief Asynchronous job interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//std includes
#include <memory>

namespace Async
{
  class Job
  {
  public:
    typedef std::shared_ptr<Job> Ptr;
    virtual ~Job() = default;

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;

    virtual bool IsActive() const = 0;
    virtual bool IsPaused() const = 0;
  };
}
