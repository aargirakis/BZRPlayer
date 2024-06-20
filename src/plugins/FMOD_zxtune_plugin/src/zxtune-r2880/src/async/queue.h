/**
* 
* @file
*
* @brief Typed thread-safe queue interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//boost includes
#include <boost/shared_ptr.hpp>

namespace Async
{
  template<class T>
  class Queue
  {
  public:
    typedef boost::shared_ptr<Queue<T> > Ptr;

    virtual ~Queue() {}

    //! @brief Enqueue object
    virtual void Add(T val) = 0;
    //! @brief Get last available object, block until arrives or break
    //! @return true if object is acquired, else otherwise
    //! @invariant In case of false return res will keep intact
    virtual bool Get(T& res) = 0;
    //! @brief Clear queue and release all the waiters
    virtual void Reset() = 0;
    //! @brief Wait until queue become empty
    virtual void Flush() = 0;
  };
}
