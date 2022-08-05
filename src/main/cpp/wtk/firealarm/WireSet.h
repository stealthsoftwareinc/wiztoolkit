/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_WIRE_SET_H_
#define WTK_FIREALARM_WIRE_SET_H_

#include <cstddef>
#include <cstdint>

#include <memory>
#include <unordered_map>
#include <vector>
#include <iterator>

#include <wtk/index.h>
#include <wtk/utils/Pool.h>
#include <wtk/utils/hints.h>

namespace wtk {
namespace firealarm {

// Enumerates failure conditions.
enum class WireSetFail
{
  success,
  deleted,
  assigned,
  not_assigned,
  null_deref,
  cant_delete,
  invalid_range,
  nonempty_subscope
};

// Return a string describing a failure event.
char const* wireSetFailString(WireSetFail fail);

template<typename Wire_T>
struct Wire
{
  bool assigned = false;
  bool deleted = false;
  // uint64_t used = 0; // TODO: log information about used/unused wires.

  Wire_T wire = Wire_T(0);
};

template<typename Wire_T>
struct WireRangeRef
{
  std::vector<Wire<Wire_T>>* wires = nullptr;
  size_t offset = 0;

  wtk::index_t first = 0;
  wtk::index_t last = 0;
};

template<typename Wire_T>
class WireSet
{
  // To avoid movement on resizing/insertion, wire vectors need to be pooled
  wtk::utils::Pool<std::vector<Wire<Wire_T>>, 1> wiresPool;

  /**
   * Stores references to output wires.
   */
  std::vector<WireRangeRef<Wire_T>> outputs;

  /**
   * Stores references to input wires.
   */
  std::vector<WireRangeRef<Wire_T>> inputs;

  /**
   * This is owning storage for local wires.
   */
  std::vector<WireRangeRef<Wire_T>> locals;
  
  // helpers to indicate if a wire is an input or output
  bool isOutput(index_t const idx);
  bool isInput(index_t const idx);

  // The spec internally adjusts local-wires to be zero-indexed, so this
  // helper does that.
  index_t adjustLocal(index_t const idx);

  // helpers for findIndex(idx) (to be reimplemented with optimizations)
  Wire<Wire_T>* findOutput(index_t const idx);
  Wire<Wire_T>* findInput(index_t const idx);
  Wire<Wire_T>* findLocal(index_t const idx);

  // Isomorphic to spec.
  Wire<Wire_T>* findIndex(index_t const idx);

  bool memLayoutCheck();

  public:
  /**
   * Indicates if it is okay to retrieve a wire during resource validity check.
   */
  WireSetFail retrieveResource(index_t const idx);

  /**
   * Returns the value at the index during evaluation validity check. Undefined
   * behavior if retrieveResource(idx) failed.
   */
  Wire_T retrieveEvaluation(index_t const idx);

  /**
   * Indicates if it is okay to insert a wire during resource validity check.
   */
  WireSetFail insertResource(index_t const idx);

  /**
   * Inserts the value at idx during evaluation validity check. Undefined
   * behavior if insertResource(idx) failed.
   */
  void insertEvaluation(index_t const idx, Wire_T value);

  /**
   * Removes the index if possible, otherwise return a failure without
   * modifying the WireSet.
   */
  WireSetFail remove(index_t const idx);

  /**
   * Remove the range if possible, otherwise return a failure without
   * modifying the WireSet.
   */
  WireSetFail remove(index_t const first, index_t const last);

  /**
   * Maps a wire as an input wire from this scope (the containing scope) into
   * the referenced sub-scope.
   *
   * Used during resource validity check.
   */
  WireSetFail remapInput(index_t idx, WireSet<Wire_T>* subscope);

  /**
   * Maps a range of wires as inputs from this scope (the containing scope)
   * into the referenced sub-scope.
   *
   * Used during resource validity check.
   *
   * Note: the spec doesn't actually define a ranged version of remap, but
   * the implementation could be made more efficient with a ranged version.
   */
  WireSetFail remapInputs(
      index_t first, index_t last, WireSet<Wire_T>* subscope);

  /**
   * Maps a wire as an output wire from this scope (the containing scope) into
   * the referenced sub-scope.
   *
   * Used during resource validity check.
   *
   * Note: the spec doesn't actually define a ranged version of remap, but
   * the implementation could be made more efficient with a ranged version.
   */
  WireSetFail remapOutput(index_t const idx, WireSet<Wire_T>* subscope);

  /**
   * Maps a range of wires as outputs from this scope (the containing scope)
   * into the referenced sub-scope.
   *
   * Used during resource validity check.
   *
   * Note: the spec doesn't actually define a ranged version of remap, but
   * the implementation could be made more efficient with a ranged version.
   */
  WireSetFail remapOutputs(
      index_t const first, index_t const last, WireSet<Wire_T>* subscope);

  /**
   * Maps "dummy wires" into this scope, as output wires. This should be used
   * by switch case statements to "ignore" non-selected cases.
   *
   * It will insert count many dummies, their storage emplaced into the
   * the dummies vector. The dummies vector should have sufficient capacity
   * for count many wires without resizing, otherwise dangling pointers will
   * happen.
   *
   * Used during resource validity check.
   */
  WireSetFail mapDummies(
      index_t const count, std::vector<Wire<Wire_T>>& dummies);

  /**
   * Indicates the number of remapped inputs.
   */
  index_t inputSize();

  /**
   * Indicates the number of remapped outputs.
   */
  index_t outputSize();
};

} } // namespace wtk::firealarm

#define LOG_IDENTIFIER "wtk::firealarm::WireSet"
#include <stealth_logging.h>

#include <wtk/firealarm/WireSet.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif // WTK_FIREALARM_WIRE_SET_H_
