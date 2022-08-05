/**
 * Copyright 2021, Stealth Software Technologies, Inc.
 */

#include <set>

#include <gtest/gtest.h>

#include <wtk/utils/is_prime.h>

TEST(is_prime, is_prime_64)
{
  // all primes between 0, 256
  std::set<uint64_t> primes = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
    37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103,
    107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
    181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251 };

  for(size_t i = 0; i < 256; i++)
  {
    if(primes.find(i) != primes.end())
    {
      EXPECT_TRUE(wtk::utils::is_prime(i));
    }
    else
    {
      EXPECT_FALSE(wtk::utils::is_prime(i));
    }
  }
}
