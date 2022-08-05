/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_IS_PRIME_H_
#define WTK_UTILS_IS_PRIME_H_

#include <array>

namespace wtk {
namespace utils {

template<typename Number_T>
bool is_prime(Number_T candidate);

} } // namespace wtk::utils

#include <wtk/utils/is_prime.t.h>

#endif // WTK_UTILS_IS_PRIME_H_
