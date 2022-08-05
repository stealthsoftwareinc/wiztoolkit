/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_WIRES_H_
#define WTK_BOLT_WIRES_H_

#include <cstddef>
#include <vector>

#include <wtk/index.h>
#include <wtk/utils/Pool.h>

#include <wtk/bolt/Expr.h>

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
struct WireRange;

template<typename Wire_T, typename Number_T>
struct WireRangePool;

/**
 * Wire reference is a reference to the memory space of a wire.
 * It can either be a direct reference, in which case it has a pointer
 * to the wire. Some wires may need to be indirect, in which case, it is
 * a pointer to a WireRange, and an index within the range.
 */
template<typename Wire_T, typename Number_T>
struct WireRef
{
  Wire_T* deref() const;

  // WireRef's are constructed as invalid, and must be initialized.
  void setDirect(Wire_T* const d);
  void setIndirect(
      WireRange<Wire_T, Number_T>* const i, wtk::index_t const idx);

private:
  union
  {
    Wire_T* direct = nullptr;
    WireRange<Wire_T, Number_T>* indirect;
  };

  // if value is UINT64_MAX, then the wire is direct
  // I'm pretty sure that it seems like this should eliminate UINT64_MAX
  // as a valid wire-number, but I don't think this is the case. It's
  // pretty much impossible for UINT64_MAX to be a non-local wire,
  // and all local-wires are going to be direct anyways.
  wtk::index_t index = 0;
};

/**
 * A WireRange has lookup capability and some variants also have ownership
 * over memory space between [first, last]. It can either assign a reference
 * to the lookup, assign a reference to a sub-range, or dereference (lookup) a
 * particular wire.
 *
 * Caller must not attempt to access wires beyond the range.
 */
template<typename Wire_T, typename Number_T>
struct WireRange
{
  // [first, last] range, inclusive.
  wtk::index_t first;
  wtk::index_t last;

  // Most WireRange's are constructed by referencing another wire range, (see
  // ref(first, last, builder) below). But the created WireRange must
  // have the range set by the caller, because this WireRange knows only
  // its own numbering, not a sub-scope's numbering.
  void setRange(wtk::index_t const f, wtk::index_t const l);

  // Get a reference to a wire in the range.
  virtual void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) = 0;

  // Get a reference to a range of wires within this range.
  virtual WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) = 0;

  // Dereference a particular wire in this range
  virtual Wire_T* deref(wtk::index_t const idx) = 0;

  // Update this range as loop iteration progresses
  virtual void update(std::vector<wtk::index_t>& stack);

  virtual ~WireRange() { /* empty */ }
};

/**
 * WireRange with ownership over wires. This is used for wires local to a
 * scope, hence the name.
 */
template<typename Wire_T, typename Number_T>
struct LocalWireRange : WireRange<Wire_T, Number_T>
{
  std::vector<Wire_T> wires;

  // constructor with first and last indexes, sizes wires vector accordingly.
  LocalWireRange(wtk::index_t const f, wtk::index_t const l);

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;
};

/**
 * WireRange with a pointer/offset directly to non-owned space. Lookup can be
 * done immediately.
 */
template<typename Wire_T, typename Number_T>
struct DirectWireRangeRef : WireRange<Wire_T, Number_T>
{
  Wire_T* const direct;

  DirectWireRangeRef(Wire_T* const d) : direct(d) { }

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;
};

/**
 * WireRange with a pointer/offset to another WireRange for lookups.
 */
template<typename Wire_T, typename Number_T>
struct IndirectWireRangeRef : WireRange<Wire_T, Number_T>
{
  WireRange<Wire_T, Number_T>* indirect = nullptr;
  wtk::index_t offset = 0;

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;
};

/**
 * Manages a list of WireRanges and looks up across all the ranges. It is
 * still forbidden to search for a wire which isn't a member of the set.
 */
template<typename Wire_T, typename Number_T>
struct WireSet
{
  // Pointer, because WireRanges will be pool-allocated by the BoltBuilder.
  std::vector<WireRange<Wire_T, Number_T>*> ranges;

  // Lookup functions for the WireRange.
  size_t findRangeIdx(wtk::index_t key) const;
  WireRange<Wire_T, Number_T>* findRange(wtk::index_t key) const;

#ifndef NDEBUG
protected:
  // BOLT has some checks that PLASMASnooze violates
  virtual bool disableChecks() const { return false; }
#endif
};

/**
 * A WireRange which performs lookups through a referenced WireSet.
 */
template<typename Wire_T, typename Number_T>
struct WireSetWireRangeRef : public WireRange<Wire_T, Number_T>
{
  // These should have continuous ranges. The entire stretch should match
  // this's range.
  WireSet<Wire_T, Number_T>* wires;

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;
};

/**
 * In shortcut loop evaluation, only the mapping-offset needs to update.
 * The range is constant.
 */
template<typename Wire_T, typename Number_T>
struct ShortcutLoopWireSetWireRangeRef : public WireRange<Wire_T, Number_T>
{
  // Just a pointer to the containing-scope's WireSet.
  WireSet<Wire_T, Number_T>* wires;

  // Offset of the mapping in the parent-scope.
  // Updated by the epxr on each iteration.
  wtk::index_t mappingOffset;

  // An expression to update the mapping offset.
  Expr offsetExpr;

  // Constructor has pointer to wires and mover for expression
  ShortcutLoopWireSetWireRangeRef(
      WireSet<Wire_T, Number_T>* const w, Expr&& e);

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;

  // Has to update the offset
  void update(std::vector<wtk::index_t>& stack) override;
};

/**
 * This is an optimization to Shortcut ranges where if the total-range in
 * the outer-scope is a single WireRange, it can be an indirect reference
 * to the single range.
 */
template<typename Wire_T, typename Number_T>
struct ShortcutLoopIndirectWireRangeRef : public WireRange<Wire_T, Number_T>
{
  WireRange<Wire_T, Number_T>* indirect;

  // Offset of the mapping in the parent-scope.
  // Updated by the epxr on each iteration.
  wtk::index_t mappingOffset;

  // An expression to update the mapping offset.
  Expr offsetExpr;

  // Constructor has pointer to wires and mover for expression
  ShortcutLoopIndirectWireRangeRef(
      WireRange<Wire_T, Number_T>* const i, Expr&& e);

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;

  // Has to update the offset
  void update(std::vector<wtk::index_t>& stack) override;
};

/**
 * For Soft unrolled loops, we'll have a single range to cover all the inputs
 * and outputs, then it does an internal translation then a lookup through
 * the outer-scope's WireSet.
 */
template<typename Wire_T, typename Number_T>
struct SoftLoopWireRangeRef : public WireRange<Wire_T, Number_T>
{
  // Just a pointer to the containing-scope's WireSet.
  WireSet<Wire_T, Number_T> const* const wires;

  struct Translation
  {
    wtk::index_t const first;
    wtk::index_t const last;
    wtk::index_t const offset;

    Translation(
        wtk::index_t const f, wtk::index_t const l, wtk::index_t const o)
      : first(f), last(l), offset(o) { }
  };

  struct Mapping
  {
    Expr first;
    Expr last;

    Mapping(Expr&& f, Expr&& l) : first(std::move(f)), last(std::move(l)) { }
  };

  // Updated on each iteration.
  std::vector<Translation> translations;
  // Used to rebuild translations on each iteration.
  std::vector<Mapping> mappings;

  SoftLoopWireRangeRef(
      WireSet<Wire_T, Number_T> const* const w, size_t const n_mappings);

  void ref(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref) override;

  WireRange<Wire_T, Number_T>* ref(
      wtk::index_t const first, wtk::index_t const last,
      WireRangePool<Wire_T, Number_T>* const pool) override;

  Wire_T* deref(wtk::index_t const idx) override;

  // Has to update the mappings
  void update(std::vector<wtk::index_t>& stack) override;
};

template<typename T>
using Pool=wtk::utils::Pool<T>;

/**
 * Pool object for all types of WireRanges.
 */
template<typename Wire_T, typename Number_T>
struct WireRangePool
{
  Pool<LocalWireRange<Wire_T, Number_T>> localWireRange;
  Pool<DirectWireRangeRef<Wire_T, Number_T>> directWireRangeRef;
  Pool<IndirectWireRangeRef<Wire_T, Number_T>> indirectWireRangeRef;
  Pool<ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>>
    shortcutLoopWireSetWireRangeRef;
  Pool<ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>>
    shortcutLoopIndirectWireRangeRef;
  Pool<SoftLoopWireRangeRef<Wire_T, Number_T>> softLoopWireRangeRef;
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

#include <wtk/bolt/wires.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_WIRES_H_
