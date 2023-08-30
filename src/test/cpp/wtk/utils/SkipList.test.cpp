/**
 * Copyright 2021, Stealth Software Technologies, Inc.
 */

#include <chrono>
#include <random>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include <wtk/utils/SkipList.h>

TEST(SkipList, test_singles)
{
  auto rand = std::bind(
      std::uniform_int_distribution<uint64_t>(0,8192),
      std::default_random_engine((std::default_random_engine::result_type)
        std::chrono::system_clock::to_time_t(
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
      std::default_random_engine((std::default_random_engine::result_type)
        std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));
  // it always seems to generate 0 first, regardless of seed.
  (void) rand_start();

  // curve between about 0 and 10
  auto rand_length = std::bind(
      std::negative_binomial_distribution<uint64_t>(6, 0.625),
      std::default_random_engine((std::default_random_engine::result_type)
        std::chrono::system_clock::to_time_t(
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

// helper struct for checking ranges.
struct range
{
  size_t first;
  size_t last;

  range(size_t f, size_t l) : first(f), last(l) { }

  bool operator==(range const& other)
  {
    return this->first == other.first && this->last == other.last;
  }
};

TEST(SkipList, for_range)
{
  wtk::utils::SkipList<size_t> list;
  list.insert(4, 8);
  list.insert(10, 20);
  list.insert(23);
  list.insert(25, 30);
  list.insert(35, 40);

  std::vector<range> check_list;
  auto add_check_list = [&check_list](size_t f, size_t l)
  {
    check_list.emplace_back(f, l);
  };

  list.forRange(4, 8, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 8));
  check_list.clear();

  list.forRange(5, 7, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(5, 7));
  check_list.clear();

  list.forRange(3, 8, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 8));
  check_list.clear();

  list.forRange(3, 9, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 8));
  check_list.clear();

  list.forRange(5, 8, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(5, 8));
  check_list.clear();

  list.forRange(4, 4, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 4));
  check_list.clear();

  list.forRange(5, 5, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(5, 5));
  check_list.clear();

  list.forRange(8, 8, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(8, 8));
  check_list.clear();

  list.forRange(2, 5, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 5));
  check_list.clear();

  list.forRange(4, 5, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 5));
  check_list.clear();

  list.forRange(7, 8, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(7, 8));
  check_list.clear();

  list.forRange(7, 9, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(7, 8));
  check_list.clear();

  list.forRange(35, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 40));
  check_list.clear();

  list.forRange(36, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(36, 40));
  check_list.clear();

  list.forRange(33, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 40));
  check_list.clear();

  list.forRange(33, 42, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 40));
  check_list.clear();

  list.forRange(36, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(36, 40));
  check_list.clear();

  list.forRange(35, 35, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 35));
  check_list.clear();

  list.forRange(36, 36, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(36, 36));
  check_list.clear();

  list.forRange(40, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(40, 40));
  check_list.clear();

  list.forRange(33, 36, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 36));
  check_list.clear();

  list.forRange(35, 36, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(35, 36));
  check_list.clear();

  list.forRange(38, 40, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(38, 40));
  check_list.clear();

  list.forRange(38, 41, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(38, 40));
  check_list.clear();

  list.forRange(23, 23, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(23, 23));
  check_list.clear();

  list.forRange(22, 23, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(23, 23));
  check_list.clear();

  list.forRange(23, 24, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(23, 23));
  check_list.clear();

  list.forRange(15, 28, add_check_list);

  EXPECT_TRUE(check_list.size() == 3);
  EXPECT_TRUE(check_list[0] == range(15, 20));
  EXPECT_TRUE(check_list[1] == range(23, 23));
  EXPECT_TRUE(check_list[2] == range(25, 28));
  check_list.clear();

  list.forRange(15, 33, add_check_list);

  EXPECT_TRUE(check_list.size() == 3);
  EXPECT_TRUE(check_list[0] == range(15, 20));
  EXPECT_TRUE(check_list[1] == range(23, 23));
  EXPECT_TRUE(check_list[2] == range(25, 30));
  check_list.clear();

  list.forRange(9, 25, add_check_list);

  EXPECT_TRUE(check_list.size() == 3);
  EXPECT_TRUE(check_list[0] == range(10, 20));
  EXPECT_TRUE(check_list[1] == range(23, 23));
  EXPECT_TRUE(check_list[2] == range(25, 25));
  check_list.clear();

  list.clear();
  list.insert(0, 4);

  list.forRange(4, 7, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 4));
  check_list.clear();

  list.insert(13, 17);

  list.forRange(4, 7, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(4, 4));
  check_list.clear();

  list.forRange(4, 13, add_check_list);

  EXPECT_TRUE(check_list.size() == 2);
  EXPECT_TRUE(check_list[0] == range(4, 4));
  EXPECT_TRUE(check_list[1] == range(13, 13));
  check_list.clear();

  list.forRange(5, 13, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(13, 13));
  check_list.clear();

  list.forRange(17, 19, add_check_list);

  EXPECT_TRUE(check_list.size() == 1);
  EXPECT_TRUE(check_list[0] == range(17, 17));
  check_list.clear();
}

TEST(SkipList, overflow)
{
  wtk::utils::SkipList<size_t> list;

  // insert max, 0, 1
  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert max, 1, 0
  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert 1, 0, max
  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert 0, 1, max
  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert 0, max, 1
  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_FALSE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert 1, max, 0
  list.insert(1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_FALSE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_FALSE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.insert(0);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 1));
  EXPECT_TRUE(list.has(SIZE_MAX));
  EXPECT_TRUE(list.has(0));
  EXPECT_TRUE(list.has(1));
  EXPECT_FALSE(list.has(2));

  list.clear();

  // insert [max], [0 1], [2 3]
  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();

  // insert [max], [2 3], [0 1]
  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();

  // insert [2 3], [max], [0 1]
  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();

  // insert [0 1], [max], [2 3]
  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();

  // insert [0 1], [2 3], [max]
  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_FALSE(list.has(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();

  // insert [2 3], [0 1], [max]
  list.insert(2, 3);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_FALSE(list.has(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(0, 1);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_FALSE(list.has(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.insert(SIZE_MAX - 1, SIZE_MAX);

  EXPECT_TRUE(list.integrityCheck());
  EXPECT_FALSE(list.has(SIZE_MAX - 2));
  EXPECT_TRUE(list.hasAll(SIZE_MAX - 1, SIZE_MAX));
  EXPECT_TRUE(list.hasAll(0, 1));
  EXPECT_TRUE(list.hasAll(2, 3));
  EXPECT_FALSE(list.has(4, 5));

  list.clear();
}
