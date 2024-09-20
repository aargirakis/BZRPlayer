#ifndef MAIN_H
#define MAIN_H

#include <unordered_map>
#include <iostream>


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

static inline unsigned get_le32(void const* p)
    {
        return (unsigned)((unsigned char const*)p)[3] << 24 |
            (unsigned)((unsigned char const*)p)[2] << 16 |
            (unsigned)((unsigned char const*)p)[1] << 8 |
            (unsigned)((unsigned char const*)p)[0];
    }

    static inline void set_le32(void* p, unsigned n)
    {
        ((unsigned char*)p)[0] = (unsigned char)n;
        ((unsigned char*)p)[1] = (unsigned char)(n >> 8);
        ((unsigned char*)p)[2] = (unsigned char)(n >> 16);
        ((unsigned char*)p)[3] = (unsigned char)(n >> 24);
    }



#endif // MAIN_H
