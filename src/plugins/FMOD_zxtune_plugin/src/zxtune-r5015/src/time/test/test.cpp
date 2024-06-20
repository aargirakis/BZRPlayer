/**
 *
 * @file
 *
 * @brief  Time test
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#include <time/duration.h>
#include <time/instant.h>
#include <time/serialize.h>

#include <iostream>

namespace
{
  void Test(const std::string& msg, bool val)
  {
    std::cout << (val ? "Passed" : "Failed") << " test for " << msg << std::endl;
    if (!val)
      throw 1;
  }

  template<class T>
  void Test(const std::string& msg, T result, T reference)
  {
    if (result == reference)
    {
      std::cout << "Passed test for " << msg << std::endl;
    }
    else
    {
      std::cout << "Failed test for " << msg << " (got: " << result << " expected: " << reference << ")" << std::endl;
      throw 1;
    }
  }
}  // namespace

int main()
{
  try
  {
    std::cout << "---- Test for time instant ----" << std::endl;
    {
      Time::AtNanosecond ns(10000000ull);
      {
        const Time::AtNanosecond ons(ns);
        Test<uint64_t>("Ns => Ns", ons.Get(), ns.Get());
        // check out type matching
        const Time::AtMicrosecond ous = ns.CastTo<Time::Microsecond>();
        Test<uint64_t>("Ns => Us", ous.Get(), ns.Get() / 1000);
        const auto oms = ns.CastTo<Time::Millisecond>();
        Test<uint64_t>("Ns => Ms", oms.Get(), ns.Get() / 1000000);
      }
      Time::AtMicrosecond us(2000000);
      {
        const Time::AtNanosecond ons(us);
        Test<uint64_t>("Us => Ns", ons.Get(), uint64_t(us.Get()) * 1000);
        const Time::AtMicrosecond ous(us);
        Test<uint64_t>("Us => Us", ous.Get(), us.Get());
        const auto oms = us.CastTo<Time::Millisecond>();
        Test<uint64_t>("Us => Ms", oms.Get(), us.Get() / 1000);
      }
      Time::AtMillisecond ms(3000000);
      {
        const Time::AtNanosecond ons(ms);
        Test<uint64_t>("Ms => Ns", ons.Get(), uint64_t(ms.Get()) * 1000000);
        const Time::AtMicrosecond ous(ms);
        Test<uint64_t>("Ms => Us", ous.Get(), ms.Get() * 1000);
        const Time::AtMillisecond oms(ms);
        Test<uint64_t>("Ms => Ms", oms.Get(), ms.Get());
      }
    }
    std::cout << "---- Test for time duration ----" << std::endl;
    {
      const Time::Seconds s(123);
      const Time::Milliseconds ms(234567);
      const Time::Microseconds us(345678900);
      // 123s = 2m03s
      Test<String>("Seconds ToString()", Time::ToString(s), "2:03.00");
      // 234567ms = 234.567s = 3m54.56s
      Test<String>("Milliseconds ToString()", Time::ToString(ms), "3:54.56");
      // 345678900us = 345678ms = 345.67s = 5m45.67s
      Test<String>("Microseconds ToString()", Time::ToString(us), "5:45.67");
      Test("s < ms", s < ms);
      Test("s < us", s < us);
      Test("ms < us", ms < us);
      // 234.567s + 123s = 5m57.56s
      {
        const auto ms_s = ms + s;
        static_assert(std::is_same<decltype(ms_s), decltype(ms)>::value, "Wrong common type");
        Test<String>("ms+s ToString()", Time::ToString(ms_s), "5:57.56");
      }
      // 345.6789s + 123s = 7m48.67
      {
        const auto us_s = us + s;
        static_assert(std::is_same<decltype(us_s), decltype(us)>::value, "Wrong common type");
        Test<String>("us+s ToString()", Time::ToString(us_s), "7:48.67");
      }
      // 345.6789s + 234.567s = 9:40.24
      {
        const auto us_ms = us + ms;
        static_assert(std::is_same<decltype(us_ms), decltype(us)>::value, "Wrong common type");
        Test<String>("us+ms ToString()", Time::ToString(us_ms), "9:40.24");
      }
      {
        const auto period = Time::Microseconds::FromFrequency(50);
        Test<uint64_t>("P(50Hz)", period.Get(), 20000);
        Test<uint32_t>("F(25ms)", Time::Milliseconds(25).ToFrequency(), 40);
        Test<double>("3s / 2000ms", Time::Seconds(3).Divide<float>(Time::Milliseconds(2000)), 1.5f);
        const auto done = Time::Microseconds::FromRatio(4762800, 44100);
        const auto total = Time::Milliseconds(180000);
        Test<uint64_t>("D(~4.7M@44100)", done.Get(), 108000000);
        Test<float>("3m / ~4.7M@44100", done.Divide<float>(total), 0.6f);
        Test<uint32_t>("3m / ~4.7M@44100, %", (done * 100).Divide<uint32_t>(total), 60);
        Test<uint32_t>("D(4.6M@44.1kHz)", Time::Milliseconds::FromRatio(4685324, 44100).Get(), 106243);
      }
    }
  }
  catch (int code)
  {
    return code;
  }
}
