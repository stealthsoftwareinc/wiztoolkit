/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_INDEX_H_
#define WTK_INDEX_H_

#include <cstdint>

namespace wtk {

/**
 * The index refers to wire-numbers and similar in the IR.
 * As a guideline, use it where a number would have been a
 * numeric literal in the IR.
 *
 * For the number of items in a list in the IR prefer size_t
 * (e.g. wireList.size() or directiveList.size())
 */
using index_t = uint64_t;

} // namespace wtk

#endif // WTK_INDEX_H_
