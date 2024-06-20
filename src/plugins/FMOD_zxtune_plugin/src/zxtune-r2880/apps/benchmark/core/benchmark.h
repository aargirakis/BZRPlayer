/**
* 
* @file
*
* @brief  Public interface of benchmark libray.
*
* @author vitamin.caig@gmail.com
*
* @note   This is non-production library and application is used to research, so it looks much more complex than required.
*
**/

#pragma once

//std includes
#include <string>

namespace Benchmark
{
  class PerformanceTest
  {
  public:
    virtual ~PerformanceTest() {}

    virtual std::string Category() const = 0;
    virtual std::string Name() const = 0;
    //! @return Performance index
    virtual double Execute() const = 0;
  };

  class TestsVisitor
  {
  public:
    virtual ~TestsVisitor() {}

    virtual void OnPerformanceTest(const PerformanceTest& test) = 0;
  };

  void ForAllTests(TestsVisitor& visitor);
}
