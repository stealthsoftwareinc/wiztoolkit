/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_POOL_H_
#define WTK_UTILS_POOL_H_

#include <cstddef>
#include <cstdlib>
#include <vector>
#include <utility>

#include <wtk/utils/hints.h>

namespace wtk {
namespace utils {

/**
 * The pool encapsulates a batch allocation scheme, where a bunch of
 * objects are allocated in array simultaneously, and pointers to these
 * are returned one at a time. Eventually all objects are deleted
 * simultaneously. This reduces time spent in the allocator and memory
 * management overhead.
 *
 * Default batch size will fill a 2MB large memory page.
 */
template<typename T, size_t batch_size = (2 << 20) / sizeof(T)>
class Pool
{
  // each element is a pointer to batch-size many items, and 
  std::vector<std::pair<T*, size_t>> spaces;

  T* makeSpace(size_t n);

public:
  /**
   * Return a pointer to n many items. Each will be default intialized.
   * N must be less than the batch_size.
   */
  T* allocate(size_t n = 1);

  /**
   * Return a pointer to n many items. Each will be initialized with args.
   * N must be less than the batch_size.
   */
  template<typename... Args>
  T* allocate(size_t n, Args&&... args);

  ~Pool();
};

} } // namespace wtk::utils

#include <wtk/utils/Pool.t.h>

#endif // WTK_UTILS_POOL_H_
