/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

/**
 * Template specializations of NumUtils for GMP++.
 */

#ifndef WTK_UTILS_NUM_UTILS_GMP_
#define WTK_UTILS_NUM_UTILS_GMP_

#include<wtk/utils/NumUtils.h>

#include <gmpxx.h>

namespace wtk {
namespace utils {

template<>
std::string dec<mpz_class>(mpz_class num)
{
  return num.get_str();
}

template<>
ALWAYS_INLINE inline size_t cast_size<mpz_class>(mpz_class const& n)
{
  return static_cast<size_t>(n.get_ui());
}

template<>
ALWAYS_INLINE inline wire_idx cast_wire<mpz_class>(mpz_class const& n)
{
  return static_cast<wire_idx>(n.get_ui());
}

template<>
ALWAYS_INLINE inline type_idx cast_type<mpz_class>(mpz_class const& n)
{
  return static_cast<type_idx>(n.get_ui());
}

} } // namespace wtk::utils

#endif//WTK_UTILS_NUM_UTILS_GMP_
