/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_BUILDER_H_
#define WTK_BOLT_BUILDER_H_

#include <cstddef>
#include <vector>
#include <unordered_map>

#include <wtk/index.h>
#include <wtk/utils/Pool.h>
#include <wtk/utils/SkipList.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/IRTree.h>

#include <wtk/bolt/wires.h>
#include <wtk/bolt/directives.h>
#include <wtk/bolt/Expr.h>

namespace wtk {
namespace bolt {

template<typename T>
using Pool=wtk::utils::Pool<T>;

// Pool allocation for directives and wires, and maybe other things.
template<typename Wire_T, typename Number_T>
struct BoltPool
{
  // Simple gates
  Pool<AddGate<Wire_T, Number_T>> addGate;
  Pool<MulGate<Wire_T, Number_T>> mulGate;
  Pool<AddCGate<Wire_T, Number_T>> addCGate;
  Pool<MulCGate<Wire_T, Number_T>> mulCGate;
  Pool<XorGate<Wire_T, Number_T>> xorGate;
  Pool<AndGate<Wire_T, Number_T>> andGate;
  Pool<NotGate<Wire_T, Number_T>> notGate;

  // Common directives
  Pool<Copy<Wire_T, Number_T>> copy;
  Pool<Assign<Wire_T, Number_T>> assign;
  Pool<Instance<Wire_T, Number_T>> instance;
  Pool<Witness<Wire_T, Number_T>> witness;
  Pool<AssertZero<Wire_T, Number_T>> assertZero;

  // Scope directives
  Pool<Function<Wire_T, Number_T>> function;
  Pool<Invocation<Wire_T, Number_T>> invocation;
  Pool<AnonFunction<Wire_T, Number_T>> anonFunction;
  Pool<SwitchStatement<Wire_T, Number_T>> switchStatement;
  Pool<ForLoop<Wire_T, Number_T>> forLoop;
  Pool<HardUnrollForLoop<Wire_T, Number_T>> hardUnrollForLoop;

  // Wire Ranges
  WireRangePool<Wire_T, Number_T> wireRangePool;
};

/**
 * The Bolt Builder conducts the first stage of BOLT invocation -- building
 * an augmented syntax tree. It does its best to do this in O(s) time where
 * s is the size of the syntax-tree. This is easy to do for most of the IR,
 * however, certain classes of for-loops require work proportional to either
 * number of iterations ("soft" unrolling), or proportional to iterations and
 * size of the sub-syntax tree ("hard" unrolling).
 */
template<typename Wire_T, typename Number_T>
struct Builder
{
private:
  // Ownership of BoltPool
  BoltPool<Wire_T, Number_T> pool;

  // Ownership of a top-level Bolt object
  Bolt<Wire_T, Number_T> mainBolt;

  Number_T const characteristic;

public:
  Builder(Number_T const c) : characteristic(c) { }

  /*
   * Builds a Bolt for an IRTree, will return nullptr if the relation is
   * not well-formed.
   */
  Bolt<Wire_T, Number_T>* build(wtk::IRTree<Number_T>* const tree);

private:
  struct BoltBuildCtx
  {
    wtk::utils::SkipList<wtk::index_t> all;
    wtk::utils::SkipList<wtk::index_t> assigned;
    wtk::utils::SkipList<wtk::index_t> deleted;

    // Assigned by caller indicating how many wires are mapped out/in
    wtk::index_t numOutputs;
    wtk::index_t numInputs;
    // Assigned during build, and checked for correctness by caller of build.
    wtk::index_t numInstance = 0;
    wtk::index_t numWitness = 0;

    Bolt<Wire_T, Number_T>* bolt;

    // Used by for-loops, needs to be passed as baggage by other features.
    IterBounds* iterBounds;
  };

  bool build(
      wtk::DirectiveList<Number_T>* const dir_list, BoltBuildCtx* const ctx);

  // maps of function names to bodies.
  std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*>
    functionDeclareMap;
  std::unordered_map<std::string, Function<Wire_T, Number_T>*> functionMap;

  // For loop stuff.
  struct ExprBuildCtx
  {
    Expr first;
    Expr last;
    wtk::index_t totalMin = UINT64_MAX;
    wtk::index_t totalMax = 0;
    wtk::index_t expectedSpan;
    bool shortcut;
    bool constant = false; // used by inputs only?
  };

  bool buildLoop(
      wtk::ForLoop<Number_T>* const loop_dir, BoltBuildCtx* const ctx);

  enum SoftUnrollStatus
  {
    OKAY, // Okay to soft unroll
    HARD, // Attempt to hard unroll
    FAIL  // Loop is poorly formed
  };

  SoftUnrollStatus checkSoftUnroll(
      std::vector<ExprBuildCtx>* const outputs,
      std::vector<ExprBuildCtx>* const inputs,
      wtk::utils::SkipList<wtk::index_t>* const loop_outputs,
      BoltBuildCtx* const ctx);

  SoftUnrollStatus checkSoftUnrollHelper(
      std::vector<ExprBuildCtx>* const outputs,
      std::vector<ExprBuildCtx>* const inputs,
      wtk::index_t const expected_outputs, wtk::index_t const expected_inputs,
      wtk::utils::SkipList<wtk::index_t>* const loop_outputs,
      BoltBuildCtx* const ctx, size_t const place);

  bool buildHardLoop(
      wtk::ForLoop<Number_T>* const loop_dir, BoltBuildCtx* const ctx);

  struct HardUnrollCtx
  {
    // defines assignability of wires.
    wtk::utils::SkipList<wtk::index_t> assigned;
    wtk::utils::SkipList<wtk::index_t> deleted;

    // Counted in the body of a function and compared to declared counts after.
    wtk::index_t instanceCount = 0;
    wtk::index_t witnessCount = 0;

    // Space from 0 up to noDelete shouldn't be deleted (remapped space).
    wtk::index_t noDelete;

    // Loop iterator things.
    ExprBuilder* exprBuilder;
  };

  // Iterates every iteration of the hard-unrolled loop (including outer loops)
  bool iterateHardUnroll(
      HardUnrollForLoop<Wire_T, Number_T>* const loop,
      BoltBuildCtx* const bolt_ctx,
      HardUnrollCtx* const unroll_ctx,
      wtk::utils::SkipList<wtk::index_t>* const loop_outs,
      size_t const depth = 0);

  // Basically hard-unrolling will pass the loop on to PLASMASnooze during
  // evaluation. Unfortunately, PLASMASnooze catches relation poor-form at
  // evaluation, which breaks the guarantee of BOLT Builder, in that it catches
  // all relation poor-form during build. Thus we need to 'pre-evaluate' it,
  // which is what this does.
  bool checkHardUnrollIteration(
      wtk::DirectiveList<Number_T>* const body, HardUnrollCtx* const ctx);

  std::vector<wtk::index_t> exprStack;
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

#include <wtk/bolt/Builder.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_BUILDER_H_
