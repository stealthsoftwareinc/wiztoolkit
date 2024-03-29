/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_SKIP_LIST_H_
#define WTK_UTILS_SKIP_LIST_H_

#include <cstdio>
#include <memory>
#include <functional>
#include <string>

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
  SkipList& operator=(SkipList const& copy);

  SkipList(SkipList&&) = default;
  SkipList& operator=(SkipList&&) = default;

  ~SkipList() = default;

  /**
   * Checks the integrity of the skip list. There are two checks here.
   *  - for each range, last >= first
   *  - for each range which isn't the end, first > next.last+1
   */
  bool integrityCheck() const;

  /**
   * Returns a string form of the list. (mainly for debugging).
   */
  std::string toString() const;

  /**
   * Prints out the list on stdout. (this is mainly for debugging).
   */
  void print() const;

  /**
   * Returns true if it has the element within the set.
   */
  bool has(Number_T const n) const;

  /**
   * Returns true if any element within the given range is an element
   * of the set.
   * 
   * fails if first > last
   */
  bool has(Number_T const first, Number_T const last) const;

  /**
   * Returns true if all elements within the range are elements of the set.
   *
   * Fails if first > last.
   */
  bool hasAll(Number_T const first, Number_T const last) const;

  /**
   * Insert a single element into the set.
   *
   * Fails by returning false if the element is already contained.
   */
  bool insert(Number_T const n);

  /**
   * Insert a range of elements into the set.
   *
   * Fails by returning false if any element of the new range is already
   * contained by the set. Also fails if first > last.
   */
  bool insert(Number_T const first, Number_T const last);

  /**
   * Removes an element from the set. returns false, without modifying,
   * if the item is not an element of the set.
   */
  bool remove(Number_T const n);

  /**
   * Remove a range from the set. returns false, without modifying,
   * if all items in the range are not elements of this set.
   */
  bool remove(Number_T const first, Number_T const last);

  /**
   * Execute func on each range in the list. The lambda shouldn't modify the
   * list otherwise it might break. Function is void(first,last).
   */
  void forEach(std::function<void(Number_T, Number_T)> func) const;

  /**
   * Execute func on every range in intersect(this, [range_first, range_last].
   * The lambda shouldn't modify the list otherwise it might break. Function
   * is void(first,last).
   */
  void forRange(Number_T const range_first, Number_T const range_last,
      std::function<void(Number_T, Number_T)> func) const;

  /**
   * Indicates if two SkipLists represent equivalent sets.
   */
  static bool equivalent(
      SkipList<Number_T> const* const a, SkipList<Number_T> const* const b);

  /**
   * empties the list.
   */
  void clear();
};

} } // namespace wtk::utils

#include <wtk/utils/SkipList.t.h>

#endif // WTK_UTILS_SKIP_LIST_H_
