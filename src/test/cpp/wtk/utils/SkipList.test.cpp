/**
 * Copyright 2021, Stealth Software Technologies, Inc.
 */

#include <chrono>
#include <random>
#include <set>

#include <gtest/gtest.h>

#include <wtk/utils/SkipList.h>

TEST(SkipList, test_singles)
{
  auto rand = std::bind(
      std::uniform_int_distribution<uint64_t>(0,8192),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));
  (void) rand(); // it always seems to generate 0 first, regardless of seed.

  std::set<uint64_t> expect;
  wtk::utils::SkipList<uint64_t> actual;

  for(size_t i = 0; i < 500; i++)
  {
    // printf("List: ");
    // actual.print();

    uint64_t newNum = rand();

    if(expect.find(newNum) == expect.end()) // shouldn't have it yet.
    {
      // printf("adding %lu\n", newNum);
      EXPECT_FALSE(actual.has(newNum));
      EXPECT_TRUE(actual.insert(newNum));
      expect.insert(newNum);
    }
    else // should already have it.
    {
      // printf("checking for %lu\n", newNum);
      EXPECT_TRUE(actual.has(newNum));
      EXPECT_FALSE(actual.insert(newNum));
    }

    EXPECT_TRUE(actual.integrityCheck());
  }

  // printf("List: ");
  // actual.print();
}

bool hasRange(std::set<uint64_t> const& s, uint64_t start, uint64_t end)
{
  for(uint64_t i = start; i <= end; i++)
  {
    if(s.find(i) != s.end())
    {
      return true;
    }
  }

  return false;
}

void insert(std::set<uint64_t> & s, uint64_t start, uint64_t end)
{
  for(uint64_t i = start; i <= end; i++)
  {
    s.insert(i);
  }
}

TEST(SkipList, test_ranges)
{
  auto rand_start = std::bind(
      std::uniform_int_distribution<uint64_t>(0, 256),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));
  // it always seems to generate 0 first, regardless of seed.
  (void) rand_start();

  // curve between about 0 and 10
  auto rand_length = std::bind(
      std::negative_binomial_distribution<uint64_t>(6, 0.625),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));
  (void) rand_length();

  std::set<uint64_t> expect;
  wtk::utils::SkipList<uint64_t> actual;

  for(size_t i = 0; i < 20000; i++)
  {

    uint64_t range_start = rand_start();
    uint64_t range_end = range_start + rand_length();

    if(range_start == range_end && i % 3 == 0)
    {
      // if the range is a single number, sometimes insert it as a single.
      if(hasRange(expect, range_start, range_end))
      {
        //printf("checking for single %lu\n", range_start);
        EXPECT_TRUE(actual.has(range_start));
        EXPECT_FALSE(actual.insert(range_start));
      }
      else
      {
        // printf("adding single %lu\n", range_start);
        EXPECT_FALSE(actual.has(range_start));
        EXPECT_TRUE(actual.insert(range_start));
        expect.insert(range_start);
        // printf("List: ");
        // actual.print();
      }
    }
    else
    {
      if(hasRange(expect, range_start, range_end))
      {
        //printf("checking for range %lu, %lu\n", range_start, range_end);
        EXPECT_TRUE(actual.has(range_start, range_end));
        EXPECT_FALSE(actual.insert(range_start, range_end));
      }
      else
      {
        // printf("adding range %lu, %lu\n", range_start, range_end);
        EXPECT_FALSE(actual.has(range_start, range_end));
        EXPECT_TRUE(actual.insert(range_start, range_end));
        insert(expect, range_start, range_end);
        // printf("List: ");
        // actual.print();
      }
    }

    EXPECT_TRUE(actual.integrityCheck());
  }

  // printf("List: ");
  // actual.print();
}
