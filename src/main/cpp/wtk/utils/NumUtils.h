/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_NUM_UTILS_H_
#define WTK_UTILS_NUM_UTILS_H_

#include <cstddef>
#include <algorithm>
#include <string>

#include <wtk/utils/hints.h>

/* NTS: charge indirect for this, plan to move it. */

namespace wtk {
namespace utils {

template<typename Number_T>
std::string dec(Number_T num);

template<typename Number_T>
std::string hex(Number_T num);

// returns the shorter (in characters) of dec or hex.
// After 1mil it is better to use hex, because 7 dec characters
// becomes 5 hex characters, with an '0x' prefix.
template<typename Number_T>
std::string short_str(Number_T num);

/**
 * Converts a string known to be valid hexadecimal (excluding the 0x prefix)
 * into an integer.
 */
template<typename Number_T>
ALWAYS_INLINE static inline void hex_to_uint(
    char const* start, char const* end, Number_T& num);

/**
 * Converts a string known to be valid decimal into an integer.
 */
template<typename Number_T>
ALWAYS_INLINE static inline void dec_to_uint(
    char const* start, char const* end, Number_T& num);

/**
 * Convert a string known to be valid octal (excluding the 0o prefix)
 * into an integer.
 */
template<typename Number_T>
ALWAYS_INLINE static inline void oct_to_uint(
    char const* start, char const* end, Number_T& num);

/**
 * Convert a string known to be valid ascii binary (excluding the 0b prefix)
 * into an integer.
 */
template<typename Number_T>
ALWAYS_INLINE static inline void bin_to_uint(
    char const* start, char const* end, Number_T& num);

} } // namespace wtk::utils

#include <wtk/utils/NumUtils.t.h>

#endif // WTK_UTILS_NUM_UTILS_H_
