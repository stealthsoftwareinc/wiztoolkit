/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_SKIP_LIST_H_
#define WTK_UTILS_SKIP_LIST_H_

#include <cstdio>
#include <memory>
#include <functional>

#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace utils {

template<typename Number_T>
struct Range;

/**
 * SkipList is basically just a numeric set that uses a list of ranges
 * of elements instead of a list of exact elements.
 */
template<typename Number_T>
class SkipList
{
  std::unique_ptr<Range<Number_T>> list =
    std::unique_ptr<Range<Number_T>>(nullptr);

public:
  SkipList() = default;
  SkipList(SkipList const& copy);

  /**
   * Checks the integrity of the skip list. There are two checks here.
   *  - for each range, last >= first
   *  - for each range which isn't the end, first > next.last+1
   */
  bool integrityCheck();

  /**
   * Prints out the list on stdout. (this is mainly for debugging).
   */
  void print();

  /**
   * Returns true if it has the element within the set.
   */
  bool has(Number_T n);

  /**
   * Returns true if any element within the given range is an element
   * of the set.
   * 
   * fails if first > last
   */
  bool has(Number_T first, Number_T last);

  /**
   * Returns true if all elements within the range are elements of the set.
   *
   * Fails if first > last.
   */
  bool hasAll(Number_T first, Number_T last);

  /**
   * Insert a single element into the set.
   *
   * Fails by returning false if the element is already contained.
   */
  bool insert(Number_T n);

  /**
   * Insert a range of elements into the set.
   *
   * Fails by returning false if any element of the new range is already
   * contained by the set. Also fails if first > last.
   */
  bool insert(Number_T first, Number_T last);

  /**
   * Removes an element from the set. returns false, without modifying,
   * if the item is not an element of the set.
   */
  bool remove(Number_T n);

  /**
   * Remove a range from the set. returns false, without modifying,
   * if all items in the range are not elements of this set.
   */
  bool remove(Number_T first, Number_T last);

  /**
   * Execute a lambda on each range in the list. The lambda shouldn't
   * modify the list otherwise it might break. Function is void(first,last).
   */
  void forEach(std::function<void(Number_T, Number_T)> func);

  /**
   * Indicates if two SkipLists represent equivalent sets.
   */
  static bool equivalent(SkipList<Number_T>* a, SkipList<Number_T>* b);

  /**
   * empties the list.
   */
  void clear();
};

} } // namespace wtk::utils

#include <wtk/utils/SkipList.t.h>

#endif // WTK_UTILS_SKIP_LIST_H_
