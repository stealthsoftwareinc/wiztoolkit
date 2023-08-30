/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_CHARMAP_H_
#define WTK_UTILS_CHARMAP_H_

#include <cstddef>
#include <unordered_map>

namespace wtk {
namespace utils {

struct CharHasher
{
  size_t operator() (char const* const& val) const;
};

struct CharComparer
{
  bool operator() (char const* const& left, char const* const& right) const;
};

/**
 * It should go without saying, but make sure that keys are null-terminated
 * and have lifetime longer than the map. E.g. it should be safe to use the
 * pool-allocated strings coming off the IR2 parsers.
 */
template<typename T>
using CharMap = std::unordered_map<char const*, T, CharHasher, CharComparer>; 

} } // namespace wtk::utils

#endif//WTK_UTILS_CHARMAP_H_
