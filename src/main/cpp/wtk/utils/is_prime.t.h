/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

/**
 * This is a fairly naieve check.
 * Basically every prime > 30 must be 30n + {1, 7, 11, 13, 17, 19, 23, 29}
 * because all other numbers would be divisible by either 2, 3, or 5.
 */
template<typename Number_T>
bool is_prime(Number_T candidate)
{
  static Number_T const INCREMENT = 30;
  static std::array<Number_T, 3> const INCREMENT_PRIME_FACTORS { 2, 3, 5 };
  static std::array<Number_T, 7> const PRIMES_LT_INCREMENT {
    7, 11, 13, 17, 19, 23, 29
  };

  if(candidate == 1) { return false; }

  // eliminate numbers divisible by 2, 3, and 5
  for(Number_T pf : INCREMENT_PRIME_FACTORS)
  {
    if(candidate != pf && candidate % pf == 0) { return false; }
  }


  // preempt on numbers divisible by primes > 30
  for(Number_T plt : PRIMES_LT_INCREMENT)
  {
    if(candidate != plt && candidate % plt == 0) { return false; }
  }

  Number_T factor = 0;
  do
  {
    factor += INCREMENT;
    for(Number_T plt : PRIMES_LT_INCREMENT)
    {
      if(candidate == factor + plt) { return true; }
    }
    for(Number_T plt : PRIMES_LT_INCREMENT)
    {
      if(candidate % (factor + plt) == 0) { return false; }
    }
  } while (factor * factor <= candidate);

  return true;
}

} } // namespace wtk::utils
