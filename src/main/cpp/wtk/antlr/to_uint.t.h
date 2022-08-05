/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_PARSE_NUMBER_H_
#define WTK_ANTLR_PARSE_NUMBER_H_

#include <cstddef>
#include <string>

#include <wtk/utils/NumUtils.h>
#include <wtk/utils/hints.h>

namespace wtk {
namespace antlr {

/**
 * Specialized number parser for strings which are guaranteed (presumably
 * by ANTLR to be in one of the binary, octal, decimal, or hexadecimal
 * forms allowed by the SIEVE IR.
 */
template<typename Number_T>
void num_to_uint(char const* start, char const* end, Number_T& num)
{
  if(end - start > 2 && *start == '0') // could have 0x/0b/0o prefix
  {
    if(start[1] == 'x' || start[1] == 'X')
    {
      wtk::utils::hex_to_uint(start + 2, end, num);
    }
    else if(start[1] == 'o')
    {
      wtk::utils::oct_to_uint(start + 2, end, num);
    }
    else if(start[1] == 'b' || start[1] == 'B')
    {
      wtk::utils::bin_to_uint(start + 2, end, num);
    }
    return;
  }

  wtk::utils::dec_to_uint(start, end, num);
}

template<typename Number_T>
void num_to_uint(std::string const& str, Number_T& num)
{
  return num_to_uint(str.c_str(), str.c_str() + str.size(), num);
}

template<typename Number_T>
void wire_to_uint(std::string const& str, Number_T& num)
{
  return num_to_uint(str.c_str() + 1, str.c_str() + str.size(), num);
}

} } // namespace wtk::antlr

#endif // WTK_ANTLR_PARSE_NUMBER_H_
