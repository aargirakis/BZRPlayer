#pragma once
#pragma warning(disable : 4200)
#pragma warning(disable : 26812)

#include <cstring>
#include <type_traits>
#include <utility>

namespace core
{
    /**
    * Memory
    */
    inline void* Alloc(size_t size, size_t alignment = 0)
    {
#ifdef WIN32
        return _aligned_malloc(size, alignment > 0 ? alignment : 16);
#else
        return aligned_alloc(alignment > 0 ? alignment : 16, size);
#endif
    }

    template <typename T>
    T* Alloc(size_t size, size_t alignment = alignof(T))
    {
        return static_cast<T*>(Alloc(size, alignment));
    }

    inline void* Calloc(size_t count, size_t size)
    {
        auto p = Alloc(size * count, 16);
        memset(p, 0, size * count);
        return p;
    }

    inline void *ReAlloc(void *ptr, size_t size, size_t alignment = 0) {
#ifdef WIN32
        return _aligned_realloc(ptr, size, alignment > 0 ? alignment : 16);
#else
        return realloc(ptr, size); //TODO do an aligned realloc
#endif
    }

    template <typename T>
    T* ReAlloc(void* ptr, size_t size)
    {
        return static_cast<T*>(ReAlloc(ptr, size, alignof(T)));
    }

    inline void Free(const void* ptr)
    {
#ifdef WIN32
        _aligned_free(const_cast<void*>(ptr));
#else
        free(const_cast<void*>(ptr));
#endif
    }

    template <typename T1, typename T2>
    constexpr T1 AlignUp(T1 size, T2 alignment)
    {
        return T1((size + alignment - 1) & ~(alignment - 1));
    }

    /**
    * Maths
    */

    template <typename T1, typename T2>
    constexpr auto Min(T1&& a, T2&& b)
    {
        return std::forward<T1>(a) < std::forward<T2>(b) ? std::forward<T1>(a) : std::forward<T2>(b);
    }

    template<typename T1, typename T2, typename... Ts>
    constexpr auto Min(T1&& a, T2&& b, Ts&&... others)
    {
        return Min(Min(std::forward<T1>(a), std::forward<T2>(b)), std::forward<Ts>(others)...);
    }

    template <typename T1, typename T2>
    constexpr auto Max(T1&& a, T2&& b)
    {
        return std::forward<T1>(a) > std::forward<T2>(b) ? std::forward<T1>(a) : std::forward<T2>(b);
    }

    template<typename T1, typename T2, typename... Ts>
    constexpr auto Max(T1&& a, T2&& b, Ts&&... others)
    {
        return Max(Max(std::forward<T1>(a), std::forward<T2>(b)), std::forward<Ts>(others)...);
    }

    template <typename T1, typename T2, typename T3>
    constexpr auto Clamp(T1&& a, T2&& min, T3&& max)
    {
        assert(min <= max);
        return Max(std::forward<T2>(min), Min(std::forward<T3>(max), std::forward<T1>(a)));
    }

    template <typename T1>
    constexpr auto Saturate(T1&& a)
    {
        return Max(static_cast<std::remove_reference_t<T1>>(0), Min(static_cast<std::remove_reference_t<T1>>(1), std::forward<T1>(a)));
    }
}
// namespace core
