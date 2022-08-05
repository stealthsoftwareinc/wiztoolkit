/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_TRACE_TREE_ALARM_H_
#define WTK_FIREALARM_TRACE_TREE_ALARM_H_

#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

#include <wtk/IRParameters.h>
#include <wtk/IRTree.h>
#include <wtk/utils/Indent.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/firealarm/State.h>
#include <wtk/firealarm/WireSet.h>
#include <wtk/firealarm/TreeAlarm.h>

#define LOG_IDENTIFIER "IR Trace"
#include <stealth_logging.h>

namespace wtk {
namespace firealarm {

/**
 * This class overrides some of the TreeAlarm (mostly reinvoking the same)
 * but producing a trace in the log.
 */
template<typename Wire_T>
struct TraceTreeAlarm : public TreeAlarm<Wire_T>
{
private:
  using super = TreeAlarm<Wire_T>;

protected:
  bool detailed = false;

  wtk::utils::Indent indent;

  std::vector<std::string> loopIters;

  bool binaryGateCheck(BinaryGate* gate, State<Wire_T>* state) override;

  bool binaryConstGateCheck(
      BinaryConstGate<Wire_T>* gate, State<Wire_T>* state) override;

  bool unaryGateCheck(UnaryGate* gate, State<Wire_T>* state) override;

  bool inputCheck(Input* input, State<Wire_T>* state,
      index_t* instance_count, index_t* short_witness_count) override;

  bool assignCheck(Assign<Wire_T>* assign, State<Wire_T>* state) override;

  bool assertZeroCheck(Terminal* assert_zero, State<Wire_T>* state) override;

  bool functionInvokeCheck(
      FunctionInvoke* invoke, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount) override;

  bool anonFunctionCheck(
      AnonFunction<Wire_T>* anon_func, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount) override;

  bool iterExprFunctionInvokeCheck(IterExprFunctionInvoke* invoke,
      wtk::utils::SkipList<index_t>* loop_outputs_all,
      wtk::utils::SkipList<index_t>* loop_outputs_used,
      State<Wire_T>* state, index_t* instanceCount,
      index_t* shortWitnessCount) override;

  bool iterExprAnonFunctionCheck(
      IterExprAnonFunction<Wire_T>* anon_func,
      wtk::utils::SkipList<index_t>* loop_outputs_all,
      wtk::utils::SkipList<index_t>* loop_outputs_used,
      State<Wire_T>* state, index_t* instanceCount,
      index_t* shortWitnessCount) override;

  bool forLoopCheck(ForLoop<Wire_T>* for_loop, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount) override;

  bool switchStatementCheck(
      SwitchStatement<Wire_T>* switch_stmt, State<Wire_T>* state,
      index_t* instanceCount, index_t* shortWitnessCount) override;

  bool caseFunctionInvokeCheck(CaseFunctionInvoke* invoke,
      State<Wire_T>* state, size_t outputs_size,
      std::vector<Wire_T>* instance_dups,
      std::vector<Wire_T>* short_witness_dups,
      std::vector<Wire<Wire_T>>* dummies,
      bool sub_switch_active) override;

  bool caseAnonFunctionCheck(CaseAnonFunction<Wire_T>* anon_func,
      State<Wire_T>* state, size_t outputs_size,
      std::vector<Wire_T>* instance_dups,
      std::vector<Wire_T>* short_witness_dups,
      std::vector<Wire<Wire_T>>* dummies,
      bool sub_switch_active) override;

public:
  TraceTreeAlarm(Wire_T const prime, GateSet const* gs,
      FeatureToggles const* ft, char const* fn, bool const d)
    : TreeAlarm<Wire_T>(prime, gs, ft, fn), detailed(d) { }

  bool checkTree(IRTree<Wire_T>* relation,
      InputStream<Wire_T>* instance = nullptr,
      InputStream<Wire_T>* short_witness = nullptr) override;
};

} } // namespace wtk::firealarm

#include <wtk/firealarm/TraceTreeAlarm.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_TRACE_TREE_ALARM_H_
