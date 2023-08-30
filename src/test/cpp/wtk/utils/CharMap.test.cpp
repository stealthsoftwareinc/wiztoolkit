/**
 * Copyright 2022, Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <cstring>
#include <unordered_map>
#include <map>

#include <gtest/gtest.h>

#include <wtk/utils/CharMap.h>
#include <wtk/utils/Pool.h>

#include <stealth_logging.h>
#include <stealth_timer.h>

char const* poolStrCpy(wtk::utils::Pool<char>* pool, char const* str)
{
  size_t len = strlen(str);
  char* ret = pool->allocate(len + 1);
  strcpy(ret, str);
  return ret;
}

TEST(CharMap, CharMap)
{
  wtk::utils::Pool<char> pool;
  wtk::utils::CharMap<int> map;

  map["foo"] = 1;
  map["goo"] = 2;
  map["hoo"] = 3;

  EXPECT_TRUE(map["foo"] == 1);
  EXPECT_TRUE(map["goo"] == 2);
  EXPECT_TRUE(map["hoo"] == 3);

  char const* foo = poolStrCpy(&pool, "foo");
  char const* goo = poolStrCpy(&pool, "goo");
  char const* hoo = poolStrCpy(&pool, "hoo");

  EXPECT_TRUE(map[foo] == 1);
  EXPECT_TRUE(map[goo] == 2);
  EXPECT_TRUE(map[hoo] == 3);
}

template<typename Map_T>
void bucket_counts(Map_T const& map)
{
  log_info("elements: %zu, buckets: %zu", map.size(), map.bucket_count());

  std::map<size_t, size_t> bucket_pop;
  for(size_t i = 0; i < map.bucket_count(); i++)
  {
    auto finder = bucket_pop.find(map.bucket_size(i));
    if(finder == bucket_pop.end())
    {
      bucket_pop[map.bucket_size(i)] = 1;
    }
    else
    {
      bucket_pop[map.bucket_size(i)]++;
    }
  }

  auto iter = bucket_pop.begin();
  while(iter != bucket_pop.end())
  {
    log_info("  %zu elements: %zu buckets", iter->first, iter->second);
    iter++;
  }
}

TEST(CharMap, Performance)
{
  // Build a list of strings.
  std::vector<char const*> strs;
  std::vector<std::string> strings;
  wtk::utils::Pool<char> pool;

  for(char i = 'a'; i <= 'z'; i++)
  {
    for(char j = 'a'; j <= 'z'; j++)
    {
      for(char k = 'a'; k <= 'z'; k++)
      {
        char* str = pool.allocate(4);
        str[0] = i;
        str[1] = j;
        str[2] = k;
        str[3] = '\0';
        strs.push_back(str);
        strings.emplace_back(str);
      }
    }
  }

  for(char i = 'a'; i <= 'z'; i++)
  {
    for(char j = 'a'; j <= 'z'; j++)
    {
      for(char k = 'a'; k <= 'z'; k++)
      {
        for(char l = 'a'; l <= 'z'; l++)
        {
          char* str = pool.allocate(5);
          str[0] = i;
          str[1] = j;
          str[2] = k;
          str[3] = l;
          str[4] = '\0';
          strs.push_back(str);
          strings.emplace_back(str);
        }

        char* str = pool.allocate(30);
        str[0] = i;
        str[1] = j;
        str[3] = k;
        strcpy(str + 4, "abcdefghijklmnopqrstuvwxyz");
      }

      char* str = pool.allocate(39);
      str[0] = i;
      str[1] = j;
      strcpy(str + 3, "abcdefghijklmnopqrstuvwxyz1234567890");
    }
  }

  // Time with a char map
  wtk::utils::CharMap<int> char_map;

  Timer char_timer;
  char_timer.start();

  for(size_t i = 0; i < strs.size(); i++)
  {
    char_map[strs[i]] = (int) i;
  }

  for(size_t i = 0; i < strs.size(); i++)
  {
    EXPECT_TRUE(char_map[strs[i]] == (int) i);
  }

  char_timer.stop();
  log_info("char map time: %lums", char_timer.milliseconds());
  bucket_counts(char_map);

  // Time with a string map.
  std::unordered_map<std::string, int> string_map;

  Timer string_timer;
  string_timer.start();

  for(size_t i = 0; i < strs.size(); i++)
  {
    string_map[strings[i]] = (int) i;
  }

  for(size_t i = 0; i < strs.size(); i++)
  {
    std::string str = strings[i];
    EXPECT_TRUE(string_map[str] == (int) i);
  }

  string_timer.stop();
  log_info("string map time: %lums", string_timer.milliseconds());
  bucket_counts(string_map);
}
