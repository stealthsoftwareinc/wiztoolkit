/**
 * Copyright (C) 2022 Stealth Software Technolgies, Inc.
 */

#ifndef WTK_BOLT_PLASMASNOOZE_H_
#define WTK_BOLT_PLASMASNOOZE_H_

#include <cstddef>
#include <vector>
#include <string>
#include <unordered_map>
#include <iterator>
#include <utility>

#include <wtk/Parser.h>
#include <wtk/IRTree.h>
#include <wtk/index.h>
#include <wtk/utils/SkipList.h>
#include <wtk/utils/Pool.h>
#include <wtk/utils/hints.h>

#include <wtk/bolt/wires.h>
#include <wtk/bolt/Expr.h>
#include <wtk/bolt/Backend.h>
#include <wtk/bolt/SwitchStreamHandler.h>

/**
 * PLASMASnooze:
 * Practical Local Acceleration for Single-pass with Malleable Assumptions
 * Snooze: opposite of Alarm, because FIREALARM adheres strictly to the
 * IR Spec, while PLASMASnooze fails to detect deviations where they are deemed
 * harmless.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
class Evaluator;

//  A WireSet specifically for PLASMASnooze. It extends the BOLT WireSet with
//  some specific operations other than lookup. They are largely patterned off
//  similar functions from FIREALARM's WireSet, but these generally assume
//  operations are checked for bad form elsewhere.
template<typename Wire_T, typename Number_T>
class PLASMASnoozeWireSet : private WireSet<Wire_T, Number_T>
{
  wtk::index_t outputSize = 0;
  wtk::index_t inputSize = 0;

  // helpers to indicate if a wire is an input or output
  bool isOutput(wtk::index_t const idx);
  bool isInput(wtk::index_t const idx);

  // Pool of WireRanges (because they have to come from somewhere)
  WireRangePool<Wire_T, Number_T> pool;

  bool integrityCheck();

#ifndef NDEBUG
protected:
  bool disableChecks() const override { return true; }
#endif

public:

  // Retrieve an existing wire, returning either the wire by pointer or a null.
  Wire_T const* retrieve(wtk::index_t const idx);

  // Inserts the value to idx, returning either the wire by pointer or a null.
  Wire_T* insert(wtk::index_t const idx);

  // Maps a wire as an input wire from this scope (the containing scope) into
  // the referenced sub-scope.
  void remapInput(wtk::index_t const idx,
      PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope);

  // Maps a range of wires as inputs from this scope (the containing scope)
  // into the referenced sub-scope.
  void remapInputs(
      wtk::index_t const first, wtk::index_t const last,
      PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope);

  // Maps a wire as an output wire from this scope (the containing scope) into
  // the referenced sub-scope.
  void remapOutput(wtk::index_t const idx,
      PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope);

  // Maps a range of wires as outputs from this scope (the containing scope)
  // into the referenced sub-scope.
  void remapOutputs(
      wtk::index_t const first, wtk::index_t const last,
      PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope);

  // Inserts some dummy wires into this wire set.
  void mapDummies(LocalWireRange<Wire_T, Number_T>* const dummies);

  friend class Evaluator<Wire_T, Number_T>;
};

/**
 * Indicates success/failures that PLASMASnooze can detect.
 */
enum class PLASMASnoozeStatus
{
  bad_relation, /** poorly formed relation */
  bad_stream,   /** poorly formed instance or witness */
  well_formed   /** well-formed relation, instance, and witness */
};

template<typename Wire_T, typename Number_T>
struct PLASMASnoozeState
{
  // Storage for wires.
  PLASMASnoozeWireSet<Wire_T, Number_T> wires;

  // The WireSet doesn't define correctness, this SkipList does.
  // (relaxed to allow overwriting deleted wires)
  wtk::utils::SkipList<wtk::index_t> assigned;

  // Input Streams
  wtk::InputStream<Number_T>* const instance;
  wtk::InputStream<Number_T>* const witness;

  // Input Streams fo switch statements
  SwitchStreamHandler<Wire_T>* const switchInstance;
  SwitchStreamHandler<Wire_T>* const switchWitness;

  // Counted in the body of a function and compared to declared counts after.
  wtk::index_t instanceCount = 0;
  wtk::index_t witnessCount = 0;

  // Loop iterator things.
  ExprBuilder* const exprBuilder;

  // Bit indicating if the current execution path is enabled.
  Wire_T const* const enabledBit;

  PLASMASnoozeState(
      wtk::InputStream<Number_T>* const i,
      SwitchStreamHandler<Wire_T>* const si,
      wtk::InputStream<Number_T>* const w,
      SwitchStreamHandler<Wire_T>* const sw,
      ExprBuilder* const eb,
      Wire_T const* const e_bit)
    : instance(i), witness(w),
      switchInstance(si), switchWitness(sw),
      exprBuilder(eb),
      enabledBit(e_bit) { }
};

/**
 * Main driver object for PLASMASnooze, it walks a syntax tree and at each
 * gate it invokes the corresponding gate of the Backend API.
 */
template<typename Wire_T, typename Number_T>
class PLASMASnooze
{
  // Pointer to the backend
  Backend<Wire_T, Number_T>* const backend;

  // Map of names to function declarations
  std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*> functions;

  // Expr evaluation stack.
  std::vector<wtk::index_t> exprStack;

public:
  /**
   * Construction requires just a Backend.
   */
  PLASMASnooze(Backend<Wire_T, Number_T>* const b) : backend(b) { }

  /**
   * To evaluate, pass a relation syntax tree, instance stream, and
   * witness stream.
   *
   * If the witness is nullptr, the Evaluator assumes that the backend is a
   * verifier and feeds zeroes in place of the witness.
   */
  PLASMASnoozeStatus evaluate(
      wtk::IRTree<Number_T>* const rel_tree,
      wtk::InputStream<Number_T>* const ins_stream,
      wtk::InputStream<Number_T>* const wit_stream);

private:
  PLASMASnoozeStatus evaluate(
      wtk::DirectiveList<Number_T>* const dir_list,
      PLASMASnoozeState<Wire_T, Number_T>* const state);

  friend class Evaluator<Wire_T, Number_T>;
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "PLASMASnooze"
#include <stealth_logging.h>

#include <wtk/bolt/PLASMASnooze.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_PLASMASNOOZE_H_
