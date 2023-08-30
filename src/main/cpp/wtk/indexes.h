/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_INDEX_H_
#define WTK_INDEX_H_

#include <cstddef>
#include <cstdint>

#include <wtk/utils/hints.h>

namespace wtk {

/**
 * The wire_idx refers to wire-numbers and similar in the IR.
 * As a guideline, use it where a number would have been a
 * $num in the IR.
 *
 * For the number of items in a list in the IR prefer size_t
 * (e.g. wireList.size() or directiveList.size())
 */
using wire_idx = uint64_t;

wire_idx constexpr WIRE_IDX_MAX = UINT64_MAX;

/**
 * The type_idx refers to type indexes in the IR.
 */
using type_idx = uint8_t;

/**
 * Indicates if x can be converted to a size_t without overflow.
 *
 * With any luck, the compiler should recognize this as tautological on
 * 64-bit systems.
 */
ALWAYS_INLINE constexpr static inline bool canConvertWireIdxToSize(wire_idx idx)
{
  return !((wire_idx) SIZE_MAX < WIRE_IDX_MAX) || idx < (wire_idx) SIZE_MAX;
}

} // namespace wtk

#endif // WTK_INDEX_H_
