/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_SCOPE_H_
#define WTK_NAILS_SCOPE_H_

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <memory>
#include <string>

#include <wtk/indexes.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/SkipList.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace nails {

/**
 * The scope handles memory management within a single field.
 */

// The Range is internal to Scope and is essentially a managed pointer
// for wires.
template<typename Wire_T>
struct Range
{
  // Actual length of the range in allocated memory
  size_t length;

  // Pointer to the actual wires
  Wire_T* wires;

  // indicates if the range was constructed with newRange (size fixed by IR)
  bool newRange = false;

  // indicates if this range is remapped from a calling scope
  bool remapped = false;

  // indicates if this range can be grown (may start as true, then turn false,
  // but cannot turn true once false).
  bool canGrow = true;

  // Constructor for either a new or remapped range.
  // nr indicates if its new (true) or remapped (false)
  Range(size_t l, Wire_T* ws, bool nr);

  // Constructor for a non-new range, that can be grown.
  Range(size_t l, Wire_T* ws);

  // Copying is forbidden
  Range(Range const&) = delete;
  Range& operator=(Range const&) = delete;

  // Moving is allowable
  Range(Range&&);
  Range& operator=(Range&&);

  ~Range();
};

enum class ScopeError
{
  success,
  outOfMem,
  alreadyExists,
  cannotDeleteRemap,
  unmatchedDelete,
  notAssigned,
  deleted,
  discontiguous
};

char const* scopeErrorString(ScopeError err);

/**
 * The Scope manages wire memory for one Field within a single function.
 */
template<typename Wire_T>
struct Scope
{
  // Coindexed lists offsets and ranges. Offsets is the offset from 0 of the
  // first wire in each range. It is sorted ascending and is binary searched
  // by findRange. Once the range index is retrieved, ranges may be indexed
  // and its wires retrieved.
  std::vector<wire_idx> offsets;
  std::vector<Range<Wire_T>> ranges;

  // Set of all wires which were assigned (included those deleted)
  wtk::utils::SkipList<wire_idx> assigned;

  // Set of all assigned wires, with deleted wires removed.
  wtk::utils::SkipList<wire_idx> active;

  // index of the first local wire. Indices < firstLocal must be remapped and
  // indices >= firstLocal must be local.
  wire_idx firstLocal = 0;

  // Given a wire index, this function finds corresponding range's index.
  size_t findRange(wire_idx idx) const;

  /**
   * Create a new range from first through last. Returns nullptr on failure and
   * sets *err.
   */
  Wire_T* newRange(
      wire_idx const first, wire_idx const last, ScopeError* const err);

  /**
   * Deletes the existing range from first through last. Returns success or
   * an error code.
   */
  ScopeError deleteRange(wire_idx const first, wire_idx const last);

  /**
   * Retrieves a single wire. Returns either the wire or nullptr and sets *err.
   */
  Wire_T const* retrieve(wire_idx const wire, ScopeError* const err) const;

  /**
   * Assigns a single wire. Returns either the wire or nullptr and sets *err.
   */
  Wire_T* assign(wire_idx const wire, ScopeError* const err);

  /**
   * Finds (or creates) a range of unassigned wires ready to be remapped as
   * output wires. Returns nullptr on failure and sets *err.
   */
  Wire_T* findOutputs(
      wire_idx const first, wire_idx const last, ScopeError* const err);

  /**
   * Finds a range of previously assigned wires which will can be remapped as
   * input wires. Returns nullptr on failure and sets *err.
   */
  Wire_T* findInputs(
      wire_idx const first, wire_idx const last, ScopeError* const err);

  /**
   * Maps an output range into this scope. This must not be called after a
   * local wire is assigned or an input wire is remapped.
   */
  void mapOutputs(size_t const length, Wire_T* const wires);

  /**
   * Maps an input range into this scope. This must not be called after a
   * local wire is assigned.
   */
  void mapInputs(size_t const length, Wire_T* const wires);

  std::string toString() const;

  Scope() = default;

  // No copying
  Scope(Scope<Wire_T> const&) = delete;
  Scope<Wire_T>& operator=(Scope<Wire_T> const&) = delete;

  // Only moving
  Scope(Scope<Wire_T>&&) = default;
  Scope<Wire_T>& operator=(Scope<Wire_T>&&) = default;

  ~Scope();
};

} } // namespace wtk::nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Scope.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_SCOPE_H_
