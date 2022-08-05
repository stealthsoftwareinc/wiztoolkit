/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace utils {

// I'm pretty sure this is correct.
const unsigned int NUMERIC_VALS[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 9, 0, 0, 0, 0, 0, 0,
  0, 10, 11, 12, 13, 14, 15, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 10, 11, 12, 13, 14, 15, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};

} } // namespace wtk::utils
