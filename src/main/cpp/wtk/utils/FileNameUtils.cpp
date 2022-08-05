/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <string>

#include <wtk/utils/FileNameUtils.h>

namespace wtk {
namespace utils {

bool isRelation(std::string const& str)
{
  size_t last = str.size() - 1;

  if((ssize_t) last - 3 < 0) { return false; }

  return str[last - 3] == '.' && str[last - 2] == 'r'
    && str[last - 1] == 'e' && str[last] == 'l';
}

bool isInstance(std::string const& str)
{
  size_t last = str.size() - 1;

  if((ssize_t) last - 3 < 0) { return false; }

  return str[last - 3] == '.' && str[last - 2] == 'i'
    && str[last - 1] == 'n' && str[last] == 's';
}

bool isWitness(std::string const& str)
{
  size_t last = str.size() - 1;

  if((ssize_t) last - 3 < 0) { return false; }

  return str[last - 3] == '.' && str[last - 2] == 'w'
    && str[last - 1] == 'i' && str[last] == 't';
}

bool isBristol(std::string const& str)
{
  size_t last = str.size() - 1;
  if((ssize_t) last - 3 < 0) { return false; }

  return str[last - 3] == '.' && str[last - 2] == 'b'
    && str[last - 1] == 't' && str[last] == 'l';
}

} } // namespace wtk::utils
