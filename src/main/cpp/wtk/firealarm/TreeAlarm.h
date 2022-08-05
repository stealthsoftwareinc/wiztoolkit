/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_TREE_ALARM_H_
#define WTK_FIREALARM_TREE_ALARM_H_

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include <wtk/IRParameters.h>
#include <wtk/IRTree.h>
#include <wtk/QueueInputStream.h>
#include <wtk/utils/SkipList.h>

#include <wtk/firealarm/State.h>
#include <wtk/firealarm/WireSet.h>

#define LOG_IDENTIFIER "TreeAlarm"
#include <stealth_logging.h>

namespace wtk {
namespace firealarm {

/**
 * This class is intended to implement the IR spec as closely as possible.
 *
 * Note that while the spec separates itself into sections for resource and
 * evaluation validity, almost at function-boundaries,, this implementation
 * intermingles both into the same function. Evaluation validity code is
 * typically guarded by "if(evaluationCheck)" guards. It will check for
 * evaluation validity when it is given instance and short witness input
 * streams.
 */
template<typename Wire_T>
struct TreeAlarm
{
protected:
  // prime, gate set and feature toggles.
  Wire_T const p;
  GateSet const* gateSet;
  FeatureToggles const* toggles;

  // gate counting information stuff
  size_t numXor = 0;
  size_t numAnd = 0;
  size_t numNot = 0;
  size_t numAdd = 0;
  size_t numAddc = 0;
  size_t numMul = 0;
  size_t numMulc = 0;
  size_t numCopy = 0;
  size_t numAssign = 0;
  size_t numInstance = 0;
  size_t numShortWitness = 0;
  size_t numAssertZero = 0;
  size_t numAssertZeroDisabled = 0;

  // for file name error printing.
  char const* fileName;

  // used during recursion check.
  std::unordered_set<std::string> functionsSet;

  // used during resource/evaluation checks for finding function bodies.
  std::unordered_map<std::string, FunctionDeclare<Wire_T>*> functionsMap;

  /**
   * Helper to extract a value from the instance stream.
   */
  virtual bool extractInstanceValue(
      wtk::InputStream<Wire_T>* ins, Wire_T* value, size_t lineNum);

  /**
   * Helper to extract a value from the short witness stream.
   */
  virtual bool extractShortWitnessValue(
      wtk::InputStream<Wire_T>* wit, Wire_T* value, size_t lineNum);

  /**
   * (recursively) Traverses a <directive-list> to find function-invokes
   * during recursion checking.
   */
  virtual bool directiveListRecursionCheck(DirectiveList<Wire_T>* dir_list);

  /**
   * Implements the recursion_check body of a <function-declare>
   */
  virtual bool functionDeclareRecursionCheck(
      FunctionDeclare<Wire_T>* func_declare);

  /**
   * Implements the recursion_check body of a <function-invoke>
   */
  virtual bool functionInvokeRecursionCheck(FunctionInvoke* invoke);

  /**
   * Implements the recursion_check body of a <for-loop>, or rather the
   * <iter-expr-function-invoke> body of a <for-loop>.
   */
  virtual bool forLoopRecursionCheck(ForLoop<Wire_T>* for_loop);

  /**
   * Implements the recursion_check body of a <switch-statement>, or rather
   * the <case-function-invoke> body of a case block.
   */
  virtual bool switchStatementRecursionCheck(
      SwitchStatement<Wire_T>* switch_stmt);

  /**
   * Implements the specification for a <directive-list>
   */
  virtual bool directiveListCheck(
      DirectiveList<Wire_T>* dir_list, State<Wire_T>* state,
      index_t* instance_count, index_t* short_witness_wount);

  /**
   * Implements the specification for a <binary-gate>
   */
  virtual bool binaryGateCheck(BinaryGate* gate, State<Wire_T>* state);

  /**
   * Implements the specification for a <binary-const-gate>
   */
  virtual bool binaryConstGateCheck(
      BinaryConstGate<Wire_T>* gate, State<Wire_T>* state);

  /**
   * Implements the specification for a <unary-gate>
   */
  virtual bool unaryGateCheck(UnaryGate* gate, State<Wire_T>* state);

  /**
   * Implements the specification for a <input>
   */
  virtual bool inputCheck(Input* input, State<Wire_T>* state,
      index_t* instance_count, index_t* short_witness_count);

  /**
   * Implements the specification for a <assign>
   */
  virtual bool assignCheck(Assign<Wire_T>* assign, State<Wire_T>* state);

  /**
   * Implements the specification for a <assert-zero>
   */
  virtual bool assertZeroCheck(Terminal* assert_zero, State<Wire_T>* state);

  /**
   * Implements the specification for a <delete-single>
   */
  virtual bool deleteSingleCheck(Terminal* del, State<Wire_T>* state);

  /**
   * Implements the specification for a <delete-range>
   */
  virtual bool deleteRangeCheck(WireRange* del, State<Wire_T>* state);

  /**
   * Implements the relation valididty body of a <function-declare>.
   */
  virtual bool functionDeclareCheck(FunctionDeclare<Wire_T>* functionDeclare);

  /**
   * Implements the specification for a <function-invoke>
   */
  virtual bool functionInvokeCheck(
      FunctionInvoke* invoke, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount);

  /**
   * Implements the specification for a <anon-function>
   */
  virtual bool anonFunctionCheck(
      AnonFunction<Wire_T>* anon_func, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount);

  /**
   * Evaluates an <iter-expr> as defined in the spec.
   */
  virtual bool iterExprEval(IterExpr* expr, index_t* result,
      std::unordered_map<std::string, index_t>* iterators);

  /**
   * Implements the specification for a <iter-expr-function-invoke>
   *
   * Loop_outputs_* are for tracking the usage of loop outputs between
   * invocation outputs. All is the list of all the loop's outputs. it
   * starts full. used are the ones which have been used in an iteration.
   *
   * The spec doesn't call for warning if two iterations overlap eachother,
   * it will still produce errors, however non-obvious as to the issue.
   * For convenience, FIREALARM will opt for the warning, it is supposed to
   * be friendly.
   */
  virtual bool iterExprFunctionInvokeCheck(IterExprFunctionInvoke* invoke,
      wtk::utils::SkipList<index_t>* loop_outputs_all,
      wtk::utils::SkipList<index_t>* loop_outputs_used,
      State<Wire_T>* state, index_t* instanceCount,
      index_t* shortWitnessCount);

  /**
   * Implements the specification for a <iter-expr-anon-function>
   *
   * Loop_outputs_* are for tracking the usage of loop outputs between
   * invocation outputs. All is the list of all the loop's outputs. it
   * starts full. used are the ones which have been used in an iteration.
   *
   * The spec doesn't call for warning if two iterations overlap eachother,
   * it will still produce errors, however non-obvious as to the issue.
   * For convenience, FIREALARM will opt for the warning, it is supposed to
   * be friendly.
   */
  virtual bool iterExprAnonFunctionCheck(
      IterExprAnonFunction<Wire_T>* anon_func,
      wtk::utils::SkipList<index_t>* loop_outputs_all,
      wtk::utils::SkipList<index_t>* loop_outputs_used,
      State<Wire_T>* state, index_t* instanceCount,
      index_t* shortWitnessCount);

  /**
   * Implements the specification for a <for-loop>
   */
  virtual bool forLoopCheck(ForLoop<Wire_T>* for_loop, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount);

  /**
   * Implements the specification for a <switch-statement>
   */
  virtual bool switchStatementCheck(
      SwitchStatement<Wire_T>* switch_stmt, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount);

  /**
   * Implements the specification for a <case-function-invoke>
   */
  virtual bool caseFunctionInvokeCheck(CaseFunctionInvoke* invoke,
      State<Wire_T>* state, size_t outputs_size,
      std::vector<Wire_T>* instance_dups,
      std::vector<Wire_T>* short_witness_dups,
      std::vector<Wire<Wire_T>>* dummies, bool sub_switch_active);

  /**
   * Implements the specification for a <case-anon-function>
   */
  virtual bool caseAnonFunctionCheck(CaseAnonFunction<Wire_T>* anon_func,
      State<Wire_T>* state, size_t outputs_size,
      std::vector<Wire_T>* instance_dups,
      std::vector<Wire_T>* short_witness_dups,
      std::vector<Wire<Wire_T>>* dummies, bool sub_switch_active);

public:
  TreeAlarm(Wire_T const prime,
      GateSet const* gs, FeatureToggles const* ft, char const* fn)
    : p(prime), gateSet(gs), toggles(ft), fileName(fn) { }

  /**
   * Checks that the relation is valid. If instance and short_witness it also
   * checks if the evaluation is valid.
   */
  virtual bool checkTree(IRTree<Wire_T>* relation,
      InputStream<Wire_T>* instance = nullptr,
      InputStream<Wire_T>* short_witness = nullptr);

  /**
   * Logs the counts of each simple directive.
   */
  virtual void logCounts(bool checked_evaluation = false);
};

} } // namespace wtk::firealarm

#include <wtk/firealarm/TreeAlarm.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_TREE_ALARM_H_
