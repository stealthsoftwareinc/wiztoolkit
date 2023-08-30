/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <cinttypes>
#include <cstring>

#include <wtk/utils/CharMap.h>

namespace wtk {
namespace utils {

size_t CharHasher::operator() (char const* const& val) const
{
  size_t hash = 5381;
  char const* place = val;

  while(*place != '\0')
  {
    hash = ((hash << 5) + hash) ^ (size_t) *place;
    place++;
  }

  return hash;
}

bool CharComparer::operator() (
    char const* const& left, char const* const& right) const
{
  return 0 == strcmp(left, right);
}

} } // namespace wtk::utils
