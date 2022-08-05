/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_DIRECTIVES_H_
#define WTK_BOLT_DIRECTIVES_H_

#include <cstddef>
#include <vector>
#include <utility>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/bolt/wires.h>
#include <wtk/bolt/SwitchStreamHandler.h>

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
struct AddGate;

template<typename Wire_T, typename Number_T>
struct MulGate;

template<typename Wire_T, typename Number_T>
struct AddCGate;

template<typename Wire_T, typename Number_T>
struct MulCGate;

template<typename Wire_T, typename Number_T>
struct XorGate;

template<typename Wire_T, typename Number_T>
struct AndGate;

template<typename Wire_T, typename Number_T>
struct NotGate;

template<typename Wire_T, typename Number_T>
struct Copy;

template<typename Wire_T, typename Number_T>
struct Assign;

template<typename Wire_T, typename Number_T>
struct Instance;

template<typename Wire_T, typename Number_T>
struct Witness;

template<typename Wire_T, typename Number_T>
struct AssertZero;

template<typename Wire_T, typename Number_T>
struct Function;

template<typename Wire_T, typename Number_T>
struct Invocation;

template<typename Wire_T, typename Number_T>
struct AnonFunction;

template<typename Wire_T, typename Number_T>
struct SwitchStatement;

template<typename Wire_T, typename Number_T>
struct ForLoop;

template<typename Wire_T, typename Number_T>
struct HardUnrollForLoop;

enum class DirectiveType
{
  ADD_GATE,
  MUL_GATE,
  ADDC_GATE,
  MULC_GATE,
  XOR_GATE,
  AND_GATE,
  NOT_GATE,
  COPY,
  ASSIGN,
  INSTANCE,
  WITNESS,
  ASSERT_ZERO,
  INVOCATION,
  ANON_FUNCTION,
  SWITCH_STATEMENT,
  FOR_LOOP,
  HARD_UNROLL_FOR_LOOP
};

template<typename Wire_T, typename Number_T>
union Directive
{
  AddGate<Wire_T, Number_T>* addGate = nullptr;
  MulGate<Wire_T, Number_T>* mulGate;
  AddCGate<Wire_T, Number_T>* addCGate;
  MulCGate<Wire_T, Number_T>* mulCGate;
  XorGate<Wire_T, Number_T>* xorGate;
  AndGate<Wire_T, Number_T>* andGate;
  NotGate<Wire_T, Number_T>* notGate;
  Copy<Wire_T, Number_T>* copy;
  Assign<Wire_T, Number_T>* assign;
  Instance<Wire_T, Number_T>* instance;
  Witness<Wire_T, Number_T>* witness;
  AssertZero<Wire_T, Number_T>* assertZero;
  Invocation<Wire_T, Number_T>* invocation;
  AnonFunction<Wire_T, Number_T>* anonFunction;
  SwitchStatement<Wire_T, Number_T>* switchStatement;
  ForLoop<Wire_T, Number_T>* forLoop;
  HardUnrollForLoop<Wire_T, Number_T>* hardUnrollForLoop;
};

template<typename Wire_T, typename Number_T>
struct Bolt
{
  WireSet<Wire_T, Number_T> wires;

  std::vector<DirectiveType> types;
  std::vector<Directive<Wire_T, Number_T>> directives;

  void findWire(
      wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref);

  void findRanges(wtk::index_t const first, wtk::index_t const last,
      WireSet<Wire_T, Number_T>* const wires, wtk::index_t* const place,
      WireRangePool<Wire_T, Number_T>* const pool);

  virtual ~Bolt() { /* empty */ }
};

/**
 * These structs aggregates WireRefs and numeric constants to compose a
 * circuit. The BoltBuilder will form lists of these structs as Bolt bodies.
 */

template<typename Wire_T, typename Number_T>
struct AddGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  WireRef<Wire_T, Number_T> right;
};

template<typename Wire_T, typename Number_T>
struct MulGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  WireRef<Wire_T, Number_T> right;
};

template<typename Wire_T, typename Number_T>
struct AddCGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  Number_T right = Number_T(0);
};

template<typename Wire_T, typename Number_T>
struct MulCGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  Number_T right = Number_T(0);
};

template<typename Wire_T, typename Number_T>
struct XorGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  WireRef<Wire_T, Number_T> right;
};

template<typename Wire_T, typename Number_T>
struct AndGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
  WireRef<Wire_T, Number_T> right;
};

template<typename Wire_T, typename Number_T>
struct NotGate
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
};

template<typename Wire_T, typename Number_T>
struct Copy
{
  WireRef<Wire_T, Number_T> out;
  WireRef<Wire_T, Number_T> left;
};

template<typename Wire_T, typename Number_T>
struct Assign
{
  WireRef<Wire_T, Number_T> out;
  Number_T left = Number_T(0);
};

template<typename Wire_T, typename Number_T>
struct Instance
{
  WireRef<Wire_T, Number_T> out;
};

template<typename Wire_T, typename Number_T>
struct Witness
{
  WireRef<Wire_T, Number_T> out;
};

template<typename Wire_T, typename Number_T>
struct AssertZero
{
  WireRef<Wire_T, Number_T> left;
};

/**
 * This is the declaration of a named function. It should be paired with
 * an invocation. The inputs and outputs of its body are tied to the remap
 * range, which can be switched to refer to different invocations.
 */
template<typename Wire_T, typename Number_T>
struct Function
{
  WireSetWireRangeRef<Wire_T, Number_T> remap;

  Bolt<Wire_T, Number_T> body;

  // copied from FunctionDeclare, checked when building invocation
  wtk::index_t numOutputs;
  wtk::index_t numInputs;
  wtk::index_t numInstance;
  wtk::index_t numWitness;
};

/**
 * The invocation's remap is a WireSet which can be pointed into the functions
 * remap during evaluation.
 */
template<typename Wire_T, typename Number_T>
struct Invocation
{
  WireSet<Wire_T, Number_T> remap;

  Function<Wire_T, Number_T>* function;
};

/**
 * The anonymous function's body doesn't need to be reused, so it can map
 * directly, not needing a remap.
 */
template<typename Wire_T, typename Number_T>
struct AnonFunction
{
  Bolt<Wire_T, Number_T> body;
};

template<typename Wire_T, typename Number_T>
struct SwitchStatement
{
  // The selector wire is an input wire which must have equality to one
  // of the case selectors below.
  WireRef<Wire_T, Number_T> selector;

  // The list of switch output wires. It should have the same effective length
  // as every range in the dummy outputs list. The dummy outputs are summed
  // and assigned to these outputs.
  WireSet<Wire_T, Number_T> outputs;

  std::vector<LocalWireRange<Wire_T, Number_T>*> dummyOutputs;

  // Eval should "bundle" some instance and witness values for all cases to
  // reuse. These are the numbers on each.
  wtk::index_t numInstance = 0;
  wtk::index_t numWitness = 0;

  // These streams are to cache instance/witness values for reuse in each case.
  SwitchStreamHandler<Wire_T> instanceCache;
  SwitchStreamHandler<Wire_T> witnessCache;

  // Case Selector, Type and Block define a "case". The case is activated
  // if the case selector matches the switch selector, the type indicates
  // whether to use an invocation or an anonymous body, and, lastly, the
  // case block is a union of invocation or anonymous function pointers for
  // the actual block body.
  enum CaseType
  {
    INVOCATION,
    ANON_FUNCTION
  };

  union CaseBlock
  {
    Invocation<Wire_T, Number_T>* invocation;
    AnonFunction<Wire_T, Number_T>* anonFunction;
  };

  std::vector<Number_T> caseSelectors;
  std::vector<CaseType> caseTypes;
  std::vector<CaseBlock> caseBlocks;

  // Selector bits are constructed at eval-time from the switch selector and
  // case-selectors. They indicate which case is active, Each is 0 for
  // inactive and 1 for active. If the entire switch is within an inactive
  // case, then all are 0.
  std::vector<Wire_T> selectorBits;

  SwitchStatement(
      size_t const nc, wtk::index_t const ni, wtk::index_t const nw)
    : dummyOutputs(nc),
      numInstance(ni),
      numWitness(nw),
      instanceCache((size_t) ni),
      witnessCache((size_t) nw),
      caseSelectors(nc),
      caseTypes(nc),
      caseBlocks(nc),
      selectorBits(nc) { }
};

/**
 * The for loop has a body which is repeated a bunch of times.
 * It also has an iterator which increments on each loop.
 * The iterator is pointed at by expressions.
 */
template<typename Wire_T, typename Number_T>
struct ForLoop
{
  wtk::index_t first = 0;
  wtk::index_t last = 0;
  wtk::index_t current = 0;

  Bolt<Wire_T, Number_T> body;

  bool parallelizable = false;
};

/**
 * Special for loop implementation for Hard Unrolls which are farmed out
 * to PLASMASnooze.
 */
template<typename Wire_T, typename Number_T>
struct HardUnrollForLoop
{
  wtk::index_t first = 0;
  wtk::index_t last = 0;
  wtk::index_t current = 0;

  ExprBuilder exprBuilder;

  std::vector<std::pair<Expr, Expr>> outputList;
  std::vector<std::pair<Expr, Expr>> inputList;

  wtk::DirectiveList<Number_T>* body;

  wtk::index_t instanceCount;
  wtk::index_t witnessCount;

  std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*>* functions;
};

} } // namespace wtk::bolt

#include <wtk/bolt/directives.t.h>

#endif//WTK_BOLT_H_
