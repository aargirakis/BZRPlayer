#ifndef MAIN_H
#define MAIN_H

#include <unordered_map>
#include <iostream>
#include "Array.h"


bool keyExists(std::unordered_map<std::string, std::string> m, std::string key)
{
    // Key is not present
    if (m.find(key) == m.end())
        return false;

    return true;
}

template <typename T1, typename T2>
  inline constexpr auto Min(T1&& a, T2&& b)
  {
      return std::forward<T1>(a) < std::forward<T2>(b) ? std::forward<T1>(a) : std::forward<T2>(b);
  }

  template<typename T1, typename T2, typename... Ts>
  inline constexpr auto Min(T1&& a, T2&& b, Ts&&... others)
  {
      return Min(Min(std::forward<T1>(a), std::forward<T2>(b)), std::forward<Ts>(others)...);
  }

  template <typename T1, typename T2>
  inline constexpr auto Max(T1&& a, T2&& b)
  {
      return std::forward<T1>(a) > std::forward<T2>(b) ? std::forward<T1>(a) : std::forward<T2>(b);
  }

  template<typename T1, typename T2, typename... Ts>
  inline constexpr auto Max(T1&& a, T2&& b, Ts&&... others)
  {
      return Max(Max(std::forward<T1>(a), std::forward<T2>(b)), std::forward<Ts>(others)...);
  }

  template <typename T1, typename T2, typename T3>
  inline constexpr auto Clamp(T1&& a, T2&& min, T3&& max)
  {
      return Max(std::forward<T2>(min), Min(std::forward<T3>(max), std::forward<T1>(a)));
  }

struct valid_range
{
    uint32_t start;
    uint32_t size;
};

#endif // MAIN_H
