/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::binaryGateCheck(
    BinaryGate* gate, State<Wire_T>* state)
{
  bool ret  = this->super::binaryGateCheck(gate, state);

  if(this->detailed)
  {
    char const* name = "";
    switch(gate->calculation())
    {
    case BinaryGate::AND: name = "@and"; break;
    case BinaryGate::XOR: name = "@xor"; break;
    case BinaryGate::ADD: name = "@add"; break;
    case BinaryGate::MUL: name = "@mul"; break;
    }

    log_info("(%s:%zu) %s$%" PRIu64 " <- %s($%" PRIu64 ", $%" PRIu64 ");",
        this->fileName, gate->lineNum(), this->indent.get(),
        gate->outputWire(), name, gate->leftWire(), gate->rightWire());
    if(state->checkEvaluation())
    {
      log_info("(%s:%zu) %s  -> <%s>;", this->fileName, gate->lineNum(),
          this->indent.get(), wtk::utils::dec(
            state->wires->retrieveEvaluation(gate->outputWire())).c_str());
    }
  }

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::binaryConstGateCheck(
    BinaryConstGate<Wire_T>* gate, State<Wire_T>* state)
{
  bool ret  = this->super::binaryConstGateCheck(gate, state);

  if(this->detailed)
  {
    char const* name = "";
    switch(gate->calculation())
    {
    case BinaryConstGate<Wire_T>::ADDC: name = "@addc"; break;
    case BinaryConstGate<Wire_T>::MULC: name = "@mulc"; break;
    }

    log_info("(%s:%zu) %s$%" PRIu64 " <- %s($%" PRIu64 ", < %s >);",
        this->fileName, gate->lineNum(), this->indent.get(),
        gate->outputWire(), name, gate->leftWire(),
        wtk::utils::dec(gate->rightValue()).c_str());
    if(state->checkEvaluation())
    {
      log_info("(%s:%zu) %s  -> <%s>;", this->fileName, gate->lineNum(),
          this->indent.get(), wtk::utils::dec(
            state->wires->retrieveEvaluation(gate->outputWire())).c_str());
    }
  }

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::unaryGateCheck(
    UnaryGate* gate, State<Wire_T>* state)
{
  bool ret  = this->super::unaryGateCheck(gate, state);

  if(this->detailed)
  {
    if(gate->calculation() == UnaryGate::NOT)
    {
      log_info("(%s:%zu) %s$%" PRIu64 " <- @not($%" PRIu64 ");",
          this->fileName, gate->lineNum(), this->indent.get(),
          gate->outputWire(), gate->inputWire());
    }
    else
    {
      log_info("(%s:%zu) %s$%" PRIu64 " <- $%" PRIu64 ";",
          this->fileName, gate->lineNum(), this->indent.get(),
          gate->outputWire(), gate->inputWire());
    }

    if(state->checkEvaluation())
    {
      log_info("(%s:%zu) %s  -> <%s>;", this->fileName, gate->lineNum(),
          this->indent.get(), wtk::utils::dec(
            state->wires->retrieveEvaluation(gate->outputWire())).c_str());
    }
  }

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::inputCheck(Input* input, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  bool ret = this->super::inputCheck(
      input, state, instance_count, short_witness_count);

  if(input->stream() == Input::INSTANCE)
  {
    log_info("(%s:%zu) %s$%" PRIu64 " <- @instance;",
        this->fileName, input->lineNum(), this->indent.get(),
        input->outputWire());
  }
  else
  {
    log_info("(%s:%zu) %s$%" PRIu64 " <- @short_witness;",
        this->fileName, input->lineNum(), this->indent.get(),
        input->outputWire());
  }

  if(state->checkEvaluation())
  {
    log_info("(%s:%zu) %s  -> <%s>;", this->fileName, input->lineNum(),
        this->indent.get(), wtk::utils::dec(
          state->wires->retrieveEvaluation(input->outputWire())).c_str());
  }

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::assignCheck(
    Assign<Wire_T>* assign, State<Wire_T>* state)
{
  bool ret  = this->super::assignCheck(assign, state);

  if(this->detailed)
  {
    log_info("(%s:%zu) %s$%" PRIu64 " <- < %s >;",
        this->fileName, assign->lineNum(), this->indent.get(),
        assign->outputWire(), wtk::utils::dec(assign->constValue()).c_str());
    if(state->checkEvaluation())
    {
      log_info("(%s:%zu) %s  -> <%s>;", this->fileName, assign->lineNum(),
          this->indent.get(), wtk::utils::dec(
            state->wires->retrieveEvaluation(assign->outputWire())).c_str());
    }
  }

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::assertZeroCheck(
    Terminal* assert_zero, State<Wire_T>* state)
{
  bool ret = this->super::assertZeroCheck(assert_zero, state);

  log_info("(%s:%zu) %s@assert_zero($%" PRIu64 ");",
      this->fileName, assert_zero->lineNum(), this->indent.get(),
      assert_zero->wire());

  if(state->checkEvaluation())
  {
    Wire_T w = state->wires->retrieveEvaluation(assert_zero->wire());
    if(!state->switchActive)
    {
      log_info("(%s:%zu) %s  <%s> -> (@case not selected)", this->fileName,
          assert_zero->lineNum(), this->indent.get(),
          wtk::utils::dec(w).c_str());
    }
    else if(w == Wire_T(0))
    {
      log_info("(%s:%zu) %s  <%s> -> accept", this->fileName,
          assert_zero->lineNum(), this->indent.get(),
          wtk::utils::dec(w).c_str());
    }
    else
    {
      log_error("(%s:%zu) %s  <%s> -> failure", this->fileName,
          assert_zero->lineNum(), this->indent.get(),
          wtk::utils::dec(w).c_str());
    }
  }
  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::functionInvokeCheck(
    FunctionInvoke* invoke, State<Wire_T>* state,
    index_t* instanceCount, index_t* shortWitnessCount)
{
  log_info("(%s:%zu) %s@call(%s)", this->fileName, invoke->lineNum(),
      this->indent.get(), invoke->name());
  this->indent.inc();

  bool ret = this->super::functionInvokeCheck(
      invoke, state, instanceCount, shortWitnessCount);

  this->indent.dec();
  log_info("(%s:%zu) %s@end", this->fileName, invoke->lineNum(),
      this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::anonFunctionCheck(
    AnonFunction<Wire_T>* anon_func, State<Wire_T>* state,
    index_t* instanceCount, index_t* shortWitnessCount)
{
  log_info("(%s:%zu) %s@anon_call", this->fileName, anon_func->lineNum(),
      this->indent.get());
  this->indent.inc();

  bool ret = this->super::anonFunctionCheck(
      anon_func, state, instanceCount, shortWitnessCount);

  this->indent.dec();
  log_info("(%s:%zu) %s@end", this->fileName, anon_func->lineNum(),
      this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::iterExprFunctionInvokeCheck(
    IterExprFunctionInvoke* invoke,
    wtk::utils::SkipList<index_t>* loop_outputs_all,
    wtk::utils::SkipList<index_t>* loop_outputs_used,
    State<Wire_T>* state, index_t* instanceCount,
    index_t* shortWitnessCount)
{
  index_t iter = 0;
  auto finder = state->iterators->find(this->loopIters.back());
  if(finder != state->iterators->end()) { iter = finder->second; }

  log_info("(%s:%zu) %s@call(%s) (@for %s, iteration %" PRIu64")",
      this->fileName, invoke->lineNum(), this->indent.get(), invoke->name(),
      this->loopIters.back().c_str(), iter);
  this->indent.inc();

  bool ret = this->super::iterExprFunctionInvokeCheck(invoke, loop_outputs_all,
      loop_outputs_used, state, instanceCount, shortWitnessCount);

  this->indent.dec();
  log_info("(%s:%zu) %s@end", this->fileName, invoke->lineNum(),
      this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::iterExprAnonFunctionCheck(
    IterExprAnonFunction<Wire_T>* anon_func,
    wtk::utils::SkipList<index_t>* loop_outputs_all,
    wtk::utils::SkipList<index_t>* loop_outputs_used,
    State<Wire_T>* state, index_t* instanceCount,
    index_t* shortWitnessCount)
{
  index_t iter = 0;
  auto finder = state->iterators->find(this->loopIters.back());
  if(finder != state->iterators->end()) { iter = finder->second; }

  log_info("(%s:%zu) %s@anon_call (@for %s, iteration %" PRIu64")",
      this->fileName, anon_func->lineNum(), this->indent.get(),
      this->loopIters.back().c_str(), iter);
  this->indent.inc();

  bool ret = this->super::iterExprAnonFunctionCheck(anon_func,
      loop_outputs_all, loop_outputs_used, state, instanceCount,
      shortWitnessCount);

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, anon_func->lineNum(), this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::forLoopCheck(ForLoop<Wire_T>* for_loop,
    State<Wire_T>* state, index_t* instanceCount, index_t* shortWitnessCount)
{
  log_info("(%s:%zu) %s@for @first %" PRIu64 " @last %" PRIu64,
      this->fileName, for_loop->lineNum(), this->indent.get(),
      for_loop->first(), for_loop->last());
  this->indent.inc();

  this->loopIters.push_back(for_loop->iterName());
  bool ret = this->super::forLoopCheck(
      for_loop, state, instanceCount, shortWitnessCount);
  this->loopIters.pop_back();

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, for_loop->lineNum(), this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::switchStatementCheck(
    SwitchStatement<Wire_T>* switch_stmt, State<Wire_T>* state,
    index_t* instanceCount, index_t* shortWitnessCount)
{
  log_info("(%s:%zu) %s@switch($%" PRIu64 ")",
      this->fileName, switch_stmt->lineNum(), this->indent.get(),
      switch_stmt->condition());
  this->indent.inc();

  bool ret = this->super::switchStatementCheck(switch_stmt, state,
      instanceCount, shortWitnessCount);

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, switch_stmt->lineNum(), this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::caseFunctionInvokeCheck(
    CaseFunctionInvoke* invoke,
    State<Wire_T>* state, size_t outputs_size,
    std::vector<Wire_T>* instance_dups,
    std::vector<Wire_T>* short_witness_dups,
    std::vector<Wire<Wire_T>>* dummies,
    bool sub_switch_active)
{
  char const* selected = (state->checkEvaluation() && state->switchActive)
    ? " (active)"
    : "";

  log_info("(%s:%zu) %s@case @call(%s)%s",
      this->fileName, invoke->lineNum(), this->indent.get(),
      invoke->name(), selected);
  this->indent.inc();

  bool ret = this->super::caseFunctionInvokeCheck(invoke, state, outputs_size,
      instance_dups, short_witness_dups, dummies, sub_switch_active);

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, invoke->lineNum(), this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::caseAnonFunctionCheck(
    CaseAnonFunction<Wire_T>* anon_func,
    State<Wire_T>* state, size_t outputs_size,
    std::vector<Wire_T>* instance_dups,
    std::vector<Wire_T>* short_witness_dups,
    std::vector<Wire<Wire_T>>* dummies,
    bool sub_switch_active)
{
  char const* selected = (state->checkEvaluation() && sub_switch_active)
    ? " (active)"
    : "";

  log_info("(%s:%zu) %s@case @anon_call%s",
      this->fileName, anon_func->lineNum(), this->indent.get(), selected);
  this->indent.inc();

  bool ret = this->super::caseAnonFunctionCheck(anon_func, state, outputs_size,
      instance_dups, short_witness_dups, dummies, sub_switch_active);

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, anon_func->lineNum(), this->indent.get());

  return ret;
}

template<typename Wire_T>
bool TraceTreeAlarm<Wire_T>::checkTree(IRTree<Wire_T>* relation,
    InputStream<Wire_T>* instance,
    InputStream<Wire_T>* short_witness)
{
  log_info("(%s:%zu) %s@begin",
      this->fileName, relation->lineNum(), this->indent.get());
  this->indent.inc();

  bool ret = this->super::checkTree(relation, instance, short_witness);

  this->indent.dec();
  log_info("(%s:%zu) %s@end",
      this->fileName, relation->lineNum(), this->indent.get());

  return ret;
}

} } // namespace wtk::firealarm
