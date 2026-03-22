#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <unordered_map>

using namespace std;

inline bool keyExists(const unordered_map<string, string> &m, const string &key) {
    // key is not present
    if (!m.contains(key))
        return false;

    return true;
}

template<typename T1, typename T2>
constexpr auto Min(T1 &&a, T2 &&b) {
    return forward<T1>(a) < forward<T2>(b) ? forward<T1>(a) : forward<T2>(b);
}

template<typename T1, typename T2, typename... Ts>
constexpr auto Min(T1 &&a, T2 &&b, Ts &&... others) {
    return Min(Min(forward<T1>(a), forward<T2>(b)), forward<Ts>(others)...);
}

template<typename T1, typename T2>
constexpr auto Max(T1 &&a, T2 &&b) {
    return forward<T1>(a) > forward<T2>(b) ? forward<T1>(a) : forward<T2>(b);
}

template<typename T1, typename T2, typename... Ts>
constexpr auto Max(T1 &&a, T2 &&b, Ts &&... others) {
    return Max(Max(forward<T1>(a), forward<T2>(b)), forward<Ts>(others)...);
}

template<typename T1, typename T2, typename T3>
constexpr auto Clamp(T1 &&a, T2 &&min, T3 &&max) {
    return Max(forward<T2>(min), Min(forward<T3>(max), forward<T1>(a)));
}

static unsigned get_le32(void const *p) {
    return static_cast<unsigned>(static_cast<unsigned char const *>(p)[3]) << 24 |
           static_cast<unsigned>(static_cast<unsigned char const *>(p)[2]) << 16 |
           static_cast<unsigned>(static_cast<unsigned char const *>(p)[1]) << 8 |
           static_cast<unsigned>(static_cast<unsigned char const *>(p)[0]);
}

static void set_le32(void *p, unsigned n) {
    static_cast<unsigned char *>(p)[0] = static_cast<unsigned char>(n);
    static_cast<unsigned char *>(p)[1] = static_cast<unsigned char>(n >> 8);
    static_cast<unsigned char *>(p)[2] = static_cast<unsigned char>(n >> 16);
    static_cast<unsigned char *>(p)[3] = static_cast<unsigned char>(n >> 24);
}

#endif // MAIN_H
