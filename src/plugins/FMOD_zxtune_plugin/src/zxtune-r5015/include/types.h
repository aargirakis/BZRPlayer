/**
 *
 * @file
 *
 * @brief  Basic types definitions
 *
 * @author vitamin.caig@gmail.com
 *
 **/

#pragma once

// common includes
#include <char_type.h>
#include <string_view.h>
// std includes
#include <cstdint>
#include <cstring>
#include <string>

//@{
//! @brief Integer types
using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

/// Unsigned integer type
typedef unsigned int uint_t;
/// Signed integer type
typedef signed int int_t;
//@}

/// String-related types

//! @brief %String type
typedef std::basic_string<Char> String;

typedef basic_string_view<Char> StringView;

// assertions
static_assert(sizeof(uint_t) >= sizeof(uint32_t), "Invalid uint_t type");
static_assert(sizeof(int_t) >= sizeof(int32_t), "Invalid int_t type");
static_assert(sizeof(uint8_t) == 1, "Invalid uint8_t type");
static_assert(sizeof(int8_t) == 1, "Invalid int8_t type");
static_assert(sizeof(uint16_t) == 2, "Invalid uint16_t type");
static_assert(sizeof(int16_t) == 2, "Invalid int16_t type");
static_assert(sizeof(uint32_t) == 4, "Invalid uint32_t type");
static_assert(sizeof(int32_t) == 4, "Invalid int32_t type");
static_assert(sizeof(uint64_t) == 8, "Invalid uint64_t type");
static_assert(sizeof(int64_t) == 8, "Invalid int64_t type");