/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Wire_T>
bool TreeAlarm<Wire_T>::extractInstanceValue(
    wtk::InputStream<Wire_T>* ins, Wire_T* value, size_t lineNum)
{
  StreamStatus status = ins->next(value);
  if(status == StreamStatus::success)
  {
    if(*value >= this->p)
    {
      log_error(
          "%s:%zu: (instance:%zu) Instance value < %s > exceeds p < %s >.",
          this->fileName, lineNum, ins->lineNum(),
          wtk::utils::dec(*value).c_str(), wtk::utils::dec(this->p).c_str());
      return false;
    }
  }
  else if(status == StreamStatus::end)
  {
    log_error("%s:%zu: (instance:%zu) Instance input stream has reached"
        " end of file.", this->fileName, lineNum, ins->lineNum());
    return false;
  }
  else // error
  {
    log_error("%s:%zu: (instance:%zu) Error reading instance stream.",
        this->fileName, lineNum, ins->lineNum());
    return false;
  }

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::extractShortWitnessValue(
    wtk::InputStream<Wire_T>* wit, Wire_T* value, size_t lineNum)
{
  StreamStatus status = wit->next(value);
  if(status == StreamStatus::success)
  {
    if(*value >= this->p)
    {
      log_error(
          "%s:%zu: (short witness:%zu) Short witness value < %s > exceeds p "
          "< %s >.", this->fileName, lineNum, wit->lineNum(),
          wtk::utils::dec(*value).c_str(), wtk::utils::dec(this->p).c_str());
      return false;
    }
  }
  else if(status == StreamStatus::end)
  {
    log_error("%s:%zu: (short witness:%zu) short witness input stream has "
        "reached end of file.", this->fileName, lineNum, wit->lineNum());
    return false;
  }
  else // error
  {
    log_error("%s:%zu: (short witness:%zu) Error reading short witness "
        "stream.", this->fileName, lineNum, wit->lineNum());
    return false;
  }

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::directiveListRecursionCheck(
    DirectiveList<Wire_T>* dir_list)
{
  bool success = true;
  for(size_t i = 0; i < dir_list->size(); i++)
  {
    switch(dir_list->type(i))
    {
    case DirectiveList<Wire_T>::FUNCTION_INVOKE:
    {
      if(!this->functionInvokeRecursionCheck(dir_list->functionInvoke(i)))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::ANON_FUNCTION:
    {
      if(!this->directiveListRecursionCheck(dir_list->anonFunction(i)->body()))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::FOR_LOOP:
    {
      if(!this->forLoopRecursionCheck(dir_list->forLoop(i)))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::SWITCH_STATEMENT:
    {
      if(!this->switchStatementRecursionCheck(dir_list->switchStatement(i)))
      {
        success = false;
      }
      break;
    }
    default:
    {
      /* other directives don't need to be checked, because they couldn't
       * have <function-invoke> children. */
      break;
    }
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::functionDeclareRecursionCheck(
      FunctionDeclare<Wire_T>* func_declare)
{
  bool success = this->directiveListRecursionCheck(func_declare->body());

  std::string name(func_declare->name());
  if(this->functionsSet.find(name) != this->functionsSet.end())
  {
    log_error("%s:%zu: Function \"%s\" was previously defined.",
        this->fileName, func_declare->lineNum(), func_declare->name());
    success = false;
  }
  else if(success) { this->functionsSet.insert(name); }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::functionInvokeRecursionCheck(FunctionInvoke* invoke)
{
  std::string name(invoke->name());
  if(this->functionsSet.find(name) == this->functionsSet.end())
  {
    log_error("%s:%zu: Function \"%s\" was not previously defined.",
        this->fileName, invoke->lineNum(), invoke->name());
    return false;
  }

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::forLoopRecursionCheck(ForLoop<Wire_T>* for_loop)
{
  switch(for_loop->bodyType())
  {
  case ForLoop<Wire_T>::INVOKE:
  {
    std::string name(for_loop->invokeBody()->name());
    if(this->functionsSet.find(name) == this->functionsSet.end())
    {
      log_error("%s:%zu: Function \"%s\" was not previously defined.",
          this->fileName, for_loop->invokeBody()->lineNum(),
          for_loop->invokeBody()->name());
      return false;
    }
    else
    {
      return true;
    }
  }
  case ForLoop<Wire_T>::ANONYMOUS:
  {
    return this->directiveListRecursionCheck(
        for_loop->anonymousBody()->body());
  }
  }

  return false;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::switchStatementRecursionCheck(
    SwitchStatement<Wire_T>* switch_stmt)
{
  bool success = true;
  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    CaseBlock<Wire_T>* case_blk = switch_stmt->caseBlock(i);
    switch(case_blk->bodyType())
    {
    case CaseBlock<Wire_T>::INVOKE:
    {
      std::string name(case_blk->invokeBody()->name());
      if(this->functionsSet.find(name) == this->functionsSet.end())
      {
        log_error("%s:%zu: Function \"%s\" was not previously defined.",
            this->fileName, case_blk->invokeBody()->lineNum(),
            case_blk->invokeBody()->name());
        success = false;
      }
      break;
    }
    case CaseBlock<Wire_T>::ANONYMOUS:
    {
      success = success && this->directiveListRecursionCheck(
          case_blk->anonymousBody()->body());
      break;
    }
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::directiveListCheck(
    DirectiveList<Wire_T>* dir_list, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  // return failures lazily, allowing additional errors to accumulate.
  bool success = true;
  *instance_count = 0;
  *short_witness_count = 0;

  for(size_t i = 0; i < dir_list->size(); i++)
  {
    switch(dir_list->type(i))
    {
    case DirectiveList<Wire_T>::BINARY_GATE:
    {
      if(!this->binaryGateCheck(dir_list->binaryGate(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::BINARY_CONST_GATE:
    {
      if(!this->binaryConstGateCheck(dir_list->binaryConstGate(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::UNARY_GATE:
    {
      if(!this->unaryGateCheck(dir_list->unaryGate(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::INPUT:
    {
      index_t sub_instance_count;
      index_t sub_short_witness_count;

      if(!this->inputCheck(dir_list->input(i), state,
            &sub_instance_count, &sub_short_witness_count))
      {
        success = false;
      }

      *instance_count += sub_instance_count;
      *short_witness_count += sub_short_witness_count;
      break;
    }
    case DirectiveList<Wire_T>::ASSIGN:
    {
      if(!this->assignCheck(dir_list->assign(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::ASSERT_ZERO:
    {
      if(!this->assertZeroCheck(dir_list->assertZero(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::DELETE_SINGLE:
    {
      if(!this->deleteSingleCheck(dir_list->deleteSingle(i), state))
      {
        success = false;
      }
      break;
    }
    case DirectiveList<Wire_T>::DELETE_RANGE:
    {
      if(!this->deleteRangeCheck(dir_list->deleteRange(i), state))
      {
        success = false;
      }
      break;
    }
    /* The non-nesting directives all attempt to "cover up" semantic errors
     * so that future directives are minimally affected (during relation
     * checking at least). The nesting-directives do not, so they will
     * fail eagerly. */
    case DirectiveList<Wire_T>::FUNCTION_INVOKE:
    {
      index_t sub_instance_count;
      index_t sub_short_witness_count;


      if(!this->functionInvokeCheck(dir_list->functionInvoke(i), state,
            &sub_instance_count, &sub_short_witness_count))
      {
        return false;
      }

      *instance_count += sub_instance_count;
      *short_witness_count += sub_short_witness_count;
      break;
    }
    case DirectiveList<Wire_T>::ANON_FUNCTION:
    {
      index_t sub_instance_count;
      index_t sub_short_witness_count;

      if(!this->anonFunctionCheck(dir_list->anonFunction(i), state,
            &sub_instance_count, &sub_short_witness_count))
      {
        return false;
      }

      *instance_count += sub_instance_count;
      *short_witness_count += sub_short_witness_count;
      break;
    }
    case DirectiveList<Wire_T>::FOR_LOOP:
    {
      index_t sub_instance_count;
      index_t sub_short_witness_count;

      if(!this->forLoopCheck(dir_list->forLoop(i), state,
            &sub_instance_count, &sub_short_witness_count))
      {
        return false;
      }

      *instance_count += sub_instance_count;
      *short_witness_count += sub_short_witness_count;
      break;
    }
    case DirectiveList<Wire_T>::SWITCH_STATEMENT:
    {
      index_t sub_instance_count;
      index_t sub_short_witness_count;

      if(!this->switchStatementCheck(dir_list->switchStatement(i), state,
            &sub_instance_count, &sub_short_witness_count))
      {
        return false;
      }

      *instance_count += sub_instance_count;
      *short_witness_count += sub_short_witness_count;
      break;
    }
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::binaryGateCheck(BinaryGate* gate, State<Wire_T>* state)
{
  // Count gates
  switch(gate->calculation())
  {
  case BinaryGate::AND:
  {
    this->numAnd++;
    break;
  }
  case BinaryGate::XOR:
  {
    this->numXor++;
    break;
  }
  case BinaryGate::ADD:
  {
    this->numAdd++;
    break;
  }
  case BinaryGate::MUL:
  {
    this->numMul++;
    break;
  }
  }

  // then check
  WireSetFail left_fail = state->wires->retrieveResource(gate->leftWire());
  if(left_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Left input wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->leftWire(),
        wireSetFailString(left_fail));
  }

  WireSetFail right_fail = state->wires->retrieveResource(gate->rightWire());
  if(right_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Right input wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->rightWire(),
        wireSetFailString(right_fail));
  }

  WireSetFail out_fail = state->wires->insertResource(gate->outputWire());
  if(out_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Output wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->outputWire(),
        wireSetFailString(out_fail));
  }

  bool bad_calc = false;
  if(gate->calculation() == BinaryGate::AND
      && (this->gateSet->gateSet != GateSet::boolean
        || !this->gateSet->enableAnd))
  {
    bad_calc = true;
  }
  else if(gate->calculation() == BinaryGate::XOR
      && (this->gateSet->gateSet != GateSet::boolean
        || !this->gateSet->enableXor))
  {
    bad_calc = true;
  }
  else if(gate->calculation() == BinaryGate::ADD
      && (this->gateSet->gateSet != GateSet::arithmetic
        || !this->gateSet->enableAdd))
  {
    bad_calc = true;
  }
  else if(gate->calculation() == BinaryGate::MUL
      && (this->gateSet->gateSet != GateSet::arithmetic
        || !this->gateSet->enableMul))
  {
    bad_calc = true;
  }

  if(left_fail == WireSetFail::success && right_fail == WireSetFail::success
      && out_fail == WireSetFail::success && !bad_calc)
  {
    if(state->checkEvaluation())
    {
      Wire_T left_val = state->wires->retrieveEvaluation(gate->leftWire());
      Wire_T right_val = state->wires->retrieveEvaluation(gate->rightWire());
      switch(gate->calculation())
      {
      case BinaryGate::AND:
      {
        state->wires->insertEvaluation(gate->outputWire(),
            (left_val & right_val) & 0x01);
        break;
      }
      case BinaryGate::XOR:
      {
        state->wires->insertEvaluation(gate->outputWire(),
            (left_val ^ right_val) & 0x01);
        break;
      }
      case BinaryGate::ADD:
      {
        state->wires->insertEvaluation(
            gate->outputWire(), Wire_T(left_val + right_val) % this->p);
        break;
      }
      case BinaryGate::MUL:
      {
        state->wires->insertEvaluation(
            gate->outputWire(), Wire_T(left_val * right_val) % this->p);
        break;
      }
      }
    }
    return true;
  }
  else { return false; }
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::binaryConstGateCheck(
    BinaryConstGate<Wire_T>* gate, State<Wire_T>* state)
{
  // count gates
  switch(gate->calculation())
  {
  case BinaryConstGate<Wire_T>::ADDC:
  {
    this->numAddc++;
    break;
  }
  case BinaryConstGate<Wire_T>::MULC:
  {
    this->numMulc++;
    break;
  }
  }

  // then check
  WireSetFail left_fail = state->wires->retrieveResource(gate->leftWire());
  if(left_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Left input wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->leftWire(),
        wireSetFailString(left_fail));
  }

  bool right_val_ok = true;
  if(this->p <= gate->rightValue())
  {
    log_error("%s:%zu: Right input value < %s > exceeds p < %s >",
        this->fileName, gate->lineNum(),
        wtk::utils::dec(gate->rightValue()).c_str(),
        wtk::utils::dec(this->p).c_str());
    right_val_ok = false;
  }

  WireSetFail out_fail = state->wires->insertResource(gate->outputWire());
  if(out_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Output wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->outputWire(),
        wireSetFailString(out_fail));
  }

  bool bad_calc = false;
  if(gate->calculation() == BinaryConstGate<Wire_T>::ADDC
      && (this->gateSet->gateSet != GateSet::arithmetic
        || !this->gateSet->enableAddC))
  {
    bad_calc = true;
  }
  else if(gate->calculation() == BinaryConstGate<Wire_T>::MULC
      && (this->gateSet->gateSet != GateSet::arithmetic
        || !this->gateSet->enableMulC))
  {
    bad_calc = true;
  }

  if(left_fail == WireSetFail::success && right_val_ok
      && out_fail == WireSetFail::success && !bad_calc)
  {
    if(state->checkEvaluation())
    {
      Wire_T left_val = state->wires->retrieveEvaluation(gate->leftWire());
      Wire_T right_val = gate->rightValue();
      switch(gate->calculation())
      {
      case BinaryConstGate<Wire_T>::ADDC:
      {
        state->wires->insertEvaluation(
            gate->outputWire(), Wire_T(left_val + right_val) % this->p);
        break;
      }
      case BinaryConstGate<Wire_T>::MULC:
      {
        state->wires->insertEvaluation(
            gate->outputWire(), Wire_T(left_val * right_val) % this->p);
        break;
      }
      }
    }
    return true;
  }
  else { return false; }
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::unaryGateCheck(UnaryGate* gate, State<Wire_T>* state)
{
  // Spec has different rules for unary gate and copy gate (which are bundled
  // here by the parser, because of similar structure). Fortunately the
  // spec's rules are mostly the same for unary and copy gates.

  // count gates
  switch(gate->calculation())
  {
  case UnaryGate::NOT:
  {
    this->numNot++;
    break;
  }
  case UnaryGate::COPY:
  {
    this->numCopy++;
    break;
  }
  }

  // then check
  WireSetFail in_fail = state->wires->retrieveResource(gate->inputWire());
  if(in_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Input wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->inputWire(),
        wireSetFailString(in_fail));
  }

  WireSetFail out_fail = state->wires->insertResource(gate->outputWire());
  if(out_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Output wire $%" PRIu64 " is invalid: %s",
        this->fileName, gate->lineNum(), gate->outputWire(),
        wireSetFailString(out_fail));
  }

  bool bad_calc = false;
  if(gate->calculation() == UnaryGate::NOT
      && (this->gateSet->gateSet != GateSet::boolean
        || !this->gateSet->enableNot))
  {
    bad_calc = true;
  }

  if(in_fail == WireSetFail::success && out_fail == WireSetFail::success
      && !bad_calc)
  {
    if(state->checkEvaluation())
    {
      Wire_T val = state->wires->retrieveEvaluation(gate->inputWire());
      switch(gate->calculation())
      {
      case UnaryGate::NOT:
      {
        state->wires->insertEvaluation(gate->outputWire(), (~val) & 0x01);
        break;
      }
      case UnaryGate::COPY:
      {
        state->wires->insertEvaluation(gate->outputWire(), val);
        break;
      }
      }
    }
    return true;
  }
  else { return false; }
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::inputCheck(Input* input, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  WireSetFail fail = state->wires->insertResource(input->outputWire());
  if(fail != WireSetFail::success)
  {
    log_error("%s:%zu: Output wire $%" PRIu64 " is invalid: %s",
        this->fileName, input->lineNum(), input->outputWire(),
        wireSetFailString(fail));
  }

  if(input->stream() == Input::INSTANCE)
  {
    this->numInstance++;
    *instance_count = 1;
    *short_witness_count = 0;
  }
  else
  {
    this->numShortWitness++;
    *instance_count = 0;
    *short_witness_count = 1;
  }
  
  if(fail == WireSetFail::success)
  {
    if(state->checkEvaluation())
    {
      if(input->stream() == Input::INSTANCE)
      {
        Wire_T value;
        if(this->extractInstanceValue(
              state->instance, &value, input->lineNum()))
        {
          state->wires->insertEvaluation(input->outputWire(), value);
          return true;
        }
      }
      else if(input->stream() == Input::SHORT_WITNESS)
      {
        Wire_T value;
        if(this->extractShortWitnessValue(
              state->shortWitness, &value, input->lineNum()))
        {
          state->wires->insertEvaluation(input->outputWire(), value);
          return true;
        }
      }
    }
    else { return true; }
  }

  return false;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::assignCheck(
    Assign<Wire_T>* assign, State<Wire_T>* state)
{
  // count gates
  this->numAssign++;

  // then check
  bool input_val_ok = true;
  if(this->p <= assign->constValue())
  {
    log_error("%s:%zu: Input value < %s > exceeds p < %s >",
        this->fileName, assign->lineNum(),
        wtk::utils::dec(assign->constValue()).c_str(),
        wtk::utils::dec(this->p).c_str());
    input_val_ok = false;
  }

  WireSetFail out_fail = state->wires->insertResource(assign->outputWire());
  if(out_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Output wire $%" PRIu64 " is invalid: %s",
        this->fileName, assign->lineNum(), assign->outputWire(),
        wireSetFailString(out_fail));
  }

  if(input_val_ok && out_fail == WireSetFail::success)
  {
    if(state->checkEvaluation())
    {
      state->wires->insertEvaluation(
          assign->outputWire(), assign->constValue());
    }
    return true;
  }
  else { return false; }
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::assertZeroCheck(
    Terminal* assert_zero, State<Wire_T>* state)
{
  // count directives
  this->numAssertZero++;

  // then check
  WireSetFail fail = state->wires->retrieveResource(assert_zero->wire());
  if(fail != WireSetFail::success)
  {
    log_error("%s:%zu: Assertion wire $%" PRIu64 " is invalid: %s",
        this->fileName, assert_zero->lineNum(), assert_zero->wire(),
        wireSetFailString(fail));
  }
  
  if(fail == WireSetFail::success)
  {
    if(state->checkEvaluation() && state->switchActive)
    {
      Wire_T val = state->wires->retrieveEvaluation(assert_zero->wire());
      if(val == Wire_T(0)) { return true; }
      else
      {
        log_error("%s:%zu: Assertion failure on wire $%" PRIu64
            ", < %s > is not < 0 >", this->fileName, assert_zero->lineNum(),
            assert_zero->wire(), wtk::utils::dec(val).c_str());
      }
    }
    else { return true; }

    if(state->checkEvaluation() && !state->switchActive)
    {
      // count assert zeros which were disabled by switch statements.
      this->numAssertZeroDisabled++;
    }
  }

  return false;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::deleteSingleCheck(Terminal* del, State<Wire_T>* state)
{
  WireSetFail fail = state->wires->remove(del->wire());
  if(fail != WireSetFail::success)
  {
    log_error("%s:%zu: Failed to remove wire $%" PRIu64 ". %s",
        this->fileName, del->lineNum(), del->wire(), wireSetFailString(fail));
    return false;
  }

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::deleteRangeCheck(WireRange* del, State<Wire_T>* state)
{
  if(del->first() > del->last())
  {
    log_error("%s:%zu: Deletion wire range is invalid. First wire $%" PRIu64
        " exceeds last wire $%" PRIu64, this->fileName, del->lineNum(),
        del->first(), del->last());
    return false;
  }

  WireSetFail fail = state->wires->remove(del->first(), del->last());
  if(fail != WireSetFail::success)
  {
    log_error("%s:%zu: Failed to remove wire range $%" PRIu64 "through $%"
        PRIu64 ". %s", this->fileName, del->lineNum(), del->first(),
        del->last(), wireSetFailString(fail));
    return false;
  }

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::functionDeclareCheck(
      FunctionDeclare<Wire_T>* func_declare)
{
  if(!this->toggles->functionToggle)
  {
    log_error("%s:%zu: Functions are disabled.",
        this->fileName, func_declare->lineNum());
    return false;
  }

  std::string name(func_declare->name());
  this->functionsMap[name] = func_declare;

  return true;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::functionInvokeCheck(
    FunctionInvoke* invoke, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  if(!this->toggles->functionToggle)
  {
    log_error("%s:%zu: Functions are disabled.",
        this->fileName, invoke->lineNum());
    return false;
  }

  std::string name(invoke->name());
  auto iter = this->functionsMap.find(name);
  if(iter == this->functionsMap.end())
  {
    log_error("%s:%zu: Function \"%s\" was not previously declared.",
        this->fileName, invoke->lineNum(), invoke->name());
    return false;
  }

  FunctionDeclare<Wire_T>* function = iter->second;
  WireSet<Wire_T> sub_wires;

  /* The spec doesn't actually call for the output list to have distinct
   * elements, but doing so will cause issues when the function actually
   * runs. I'll emit a warning so it isn't as jarring. */
  wtk::utils::SkipList<index_t> distinct_outputs;
  for(size_t i = 0; i < invoke->outputList()->size(); i++)
  {
    switch(invoke->outputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = invoke->outputList()->single(i);

      if(!distinct_outputs.insert(single))
      {
        log_warn("%s:%zu: Function gate output wire $%" PRIu64
            " is duplicated.", this->fileName, invoke->outputList()->lineNum(),
            single);
      }

      WireSetFail single_fail = state->wires->remapOutput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Function gate output wire $%" PRIu64
            " is invalid: %s", this->fileName, invoke->outputList()->lineNum(),
            single, wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = invoke->outputList()->range(i);

      if(!distinct_outputs.insert(range->first(), range->last()))
      {
        log_warn("%s:%zu: Function gate output range $%" PRIu64 " through $%"
            PRIu64 " has duplicates.", this->fileName,
            invoke->outputList()->lineNum(), range->first(), range->last());
      }

      WireSetFail range_fail = state->wires->remapOutputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Function gate output range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            invoke->outputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  for(size_t i = 0; i < invoke->inputList()->size(); i++)
  {
    switch(invoke->inputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = invoke->inputList()->single(i);
      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Function gate input wire $%" PRIu64
            " is invalid: %s", this->fileName, invoke->inputList()->lineNum(),
            single, wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = invoke->inputList()->range(i);
      WireSetFail range_fail = state->wires->remapInputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Function gate input range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            invoke->inputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  if(sub_wires.outputSize() != function->outputCount())
  {
    log_error("%s:%zu: Function outputs size mismatch. Expected: %" PRIu64
        " but found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->outputCount(), sub_wires.outputSize());
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
    return false;
  }

  if(sub_wires.inputSize() != function->inputCount())
  {
    log_error("%s:%zu: Function inputs size mismatch. Expected: %" PRIu64
        " but found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->inputCount(), sub_wires.inputSize());
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
    return false;
  }

  bool success = true;
  std::unordered_map<std::string, index_t> sub_iterators;
  State<Wire_T> new_state(
      &sub_wires, &sub_iterators, nullptr, nullptr);

  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < function->instanceCount(); i++)
    {
      Wire_T value;
      if(this->extractInstanceValue(
            state->instance, &value, invoke->lineNum()))
      {
        sub_instance.insert(value, state->instance->lineNum());
      }
      else { return false; }
    }

    for(size_t i = 0; i < function->shortWitnessCount(); i++)
    {
      Wire_T value;
      if(this->extractShortWitnessValue(
            state->shortWitness, &value, invoke->lineNum()))
      {
        sub_short_witness.insert(value, state->shortWitness->lineNum());
      }
      else { return false; }
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = state->switchActive;
  }

  index_t sub_instance_count;
  index_t sub_short_witness_count;
  if(!this->directiveListCheck(function->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != function->instanceCount())
  {
    log_error("%s:%zu: Instance count  mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->instanceCount(), sub_instance_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  if(sub_short_witness_count != function->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->shortWitnessCount(), sub_short_witness_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  *instance_count = function->instanceCount();
  *short_witness_count = function->shortWitnessCount();

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error("%s:%zu: Function \"%s\" failed to assign its output wire $%"
          PRIu64 " (sub-scope numbering)", this->fileName, invoke->lineNum(),
          name.c_str(), i);
      log_info("%s:%zu: Function \"%s\" defined here.", this->fileName,
          function->lineNum(), name.c_str());
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_instance.size(), name.c_str());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_short_witness.size(), name.c_str());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::anonFunctionCheck(
    AnonFunction<Wire_T>* anon_func, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  if(!this->toggles->functionToggle)
  {
    log_error("%s:%zu: Functions are disabled.",
        this->fileName, anon_func->lineNum());
    return false;
  }

  WireSet<Wire_T> sub_wires;

  /* The spec doesn't actually call for the output list to have distinct
   * elements, but doing so will cause issues when the function actually
   * runs. I'll emit a warning so it isn't as jarring. */
  wtk::utils::SkipList<index_t> distinct_outputs;
  for(size_t i = 0; i < anon_func->outputList()->size(); i++)
  {
    switch(anon_func->outputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = anon_func->outputList()->single(i);

      if(!distinct_outputs.insert(single))
      {
        log_warn("%s:%zu: Anonymous Function gate output wire $%" PRIu64
            " is duplicated.", this->fileName,
            anon_func->outputList()->lineNum(), single);
      }

      WireSetFail single_fail = state->wires->remapOutput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Anonymous function gate output wire $%" PRIu64
            " is invalid: %s", this->fileName,
            anon_func->outputList()->lineNum(), single,
            wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = anon_func->outputList()->range(i);

      if(!distinct_outputs.insert(range->first(), range->last()))
      {
        log_warn("%s:%zu: Anonymous function gate output range $%" PRIu64
            " through $%" PRIu64 " has duplicates.", this->fileName,
            anon_func->outputList()->lineNum(), range->first(), range->last());
      }

      WireSetFail range_fail = state->wires->remapOutputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Anonymous function output range $%" PRIu64
            " through $%" PRIu64 " is invalid: %s", this->fileName,
            anon_func->outputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  for(size_t i = 0; i < anon_func->inputList()->size(); i++)
  {
    switch(anon_func->inputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = anon_func->inputList()->single(i);
      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Anonymous function input wire $%" PRIu64
            " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), single,
            wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = anon_func->inputList()->range(i);
      WireSetFail range_fail = state->wires->remapInputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Anonymous function input range $%" PRIu64
            " through $%" PRIu64 " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  bool success = true;
  State<Wire_T> new_state(
      &sub_wires, state->iterators, nullptr, nullptr);

  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < anon_func->instanceCount(); i++)
    {
      Wire_T value;
      if(this->extractInstanceValue(
            state->instance, &value, anon_func->lineNum()))
      {
        sub_instance.insert(value, state->instance->lineNum());
      }
      else { return false; }
    }

    for(size_t i = 0; i < anon_func->shortWitnessCount(); i++)
    {
      Wire_T value;
      if(this->extractShortWitnessValue(
            state->shortWitness, &value, anon_func->lineNum()))
      {
        sub_short_witness.insert(value, state->shortWitness->lineNum());
      }
      else { return false; }
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = state->switchActive;
  }

  index_t sub_instance_count;
  index_t sub_short_witness_count;

  if(!this->directiveListCheck(anon_func->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != anon_func->instanceCount())
  {
    log_error("%s:%zu: Instance count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->instanceCount(), sub_instance_count);
    success = false;
  }

  if(sub_short_witness_count != anon_func->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->shortWitnessCount(), sub_short_witness_count);
    success = false;
  }

  *instance_count = anon_func->instanceCount();
  *short_witness_count = anon_func->shortWitnessCount();

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error(
          "%s:%zu: Anonymous function failed to assign its output wire $%"
          PRIu64 " (sub-scope numbering)", this->fileName,
          anon_func->lineNum(), i);
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "anonymous function", this->fileName, anon_func->lineNum(),
          sub_instance.size());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          "anonymous function", this->fileName, anon_func->lineNum(),
          sub_short_witness.size());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::iterExprEval(IterExpr* expr, index_t* result,
    std::unordered_map<std::string, index_t>* iterators)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    *result = expr->literal();
    return true;
  }
  case IterExpr::ITERATOR:
  {
    std::string name(expr->name());
    auto finder = iterators->find(name);
    if(finder != iterators->end())
    {
      *result = finder->second;
      return true;
    }
    else
    {
      *result = 0;
      log_error("%s:%zu: Loop iterator \"%s\" not found.", this->fileName,
          expr->lineNum(), expr->name());
      return false;
    }
  }
  case IterExpr::ADD:
  {
    index_t left, right;
    bool success = this->iterExprEval(expr->lhs(), &left, iterators);
    success = this->iterExprEval(expr->rhs(), &right, iterators) && success;
    *result = left + right;
    return success;
  }
  case IterExpr::SUB:
  {
    index_t left, right;
    bool success = this->iterExprEval(expr->lhs(), &left, iterators);
    success = this->iterExprEval(expr->rhs(), &right, iterators) && success;
    *result = left - right;
    return success;
  }
  case IterExpr::MUL:
  {
    index_t left, right;
    bool success = this->iterExprEval(expr->lhs(), &left, iterators);
    success = this->iterExprEval(expr->rhs(), &right, iterators) && success;

    *result = left * right;
    return success;
  }
  case IterExpr::DIV:
  {
    bool success = true;
    index_t right = expr->literal();
    if(right == 0)
    {
      log_error("%s:%zu: Constant division would divide by zero.",
          this->fileName, expr->lineNum());
      success = false;
      right = 1;
    }
    index_t left;
    success = this->iterExprEval(expr->lhs(), &left, iterators) && success;

    *result = left / right;
    return success;
  }
  }

  return false;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::iterExprFunctionInvokeCheck(
    IterExprFunctionInvoke* invoke,
    wtk::utils::SkipList<index_t>* loop_outputs_all,
    wtk::utils::SkipList<index_t>* loop_outputs_used,
    State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  if(!this->toggles->functionToggle)
  {
    log_error("%s:%zu: Functions are disabled.",
        this->fileName, invoke->lineNum());
    return false;
  }

  std::string name(invoke->name());
  auto iter = this->functionsMap.find(name);
  if(iter == this->functionsMap.end())
  {
    log_error("%s:%zu: Function \"%s\" was not previously declared.",
        this->fileName, invoke->lineNum(), invoke->name());
    return false;
  }

  FunctionDeclare<Wire_T>* function = iter->second;
  WireSet<Wire_T> sub_wires;

  /* Uses loop_outputs_all to check that the function output is a member of
   * the loop output (called for by spec). and it uses loop_outputs_used to
   * check that output wires aren't reused (convenience because although the
   * spec doesn't require this, it would produce confuzzling errors).
   */
  for(size_t i = 0; i < invoke->outputList()->size(); i++)
  {
    switch(invoke->outputList()->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      index_t single;
      if(!this->iterExprEval(
            invoke->outputList()->single(i), &single, state->iterators))
      {
        return false;
      }

      if(!loop_outputs_all->has(single))
      {
        log_error("%s:%zu: Loop iteration output $%" PRIu64 " is not a member"
            " of the loop's output", this->fileName,
            invoke->outputList()->lineNum(), single);
        return false;
      }
      if(!loop_outputs_used->insert(single))
      {
        log_warn("%s:%zu: Loop output wire $%" PRIu64 " is duplicated.",
            this->fileName, invoke->outputList()->lineNum(), single);
      }

      WireSetFail single_fail = state->wires->remapOutput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration output wire $%" PRIu64
            " is invalid: %s", this->fileName, invoke->outputList()->lineNum(),
            single, wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case IterExprWireList::RANGE:
    {
      IterExprWireRange* range = invoke->outputList()->range(i);
      index_t first, last;
      if(!this->iterExprEval(range->first(), &first, state->iterators)
          || !this->iterExprEval(range->last(), &last, state->iterators))
      {
        return false;
      }

      if(!loop_outputs_all->has(first, last))
      {
        log_error("%s:%zu: Loop iteration outputs $%" PRIu64 " through $%"
            PRIu64 " are not a members of the loop's output", this->fileName,
            invoke->outputList()->lineNum(), first, last);
        return false;
      }
      if(!loop_outputs_used->insert(first, last))
      {
        log_warn("%s:%zu: Loop output wires $%" PRIu64 " through $%" PRIu64
            "are duplicated.", this->fileName, invoke->outputList()->lineNum(),
            first, last);
      }

      WireSetFail range_fail =
        state->wires->remapOutputs(first, last, &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration output range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            invoke->outputList()->lineNum(), first, last,
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  for(size_t i = 0; i < invoke->inputList()->size(); i++)
  {
    switch(invoke->inputList()->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      index_t single;
      if(!this->iterExprEval(
            invoke->inputList()->single(i), &single, state->iterators))
      {
        return false;
      }

      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration input wire $%" PRIu64
            " is invalid: %s", this->fileName, invoke->inputList()->lineNum(),
            single, wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case IterExprWireList::RANGE:
    {
      IterExprWireRange* range = invoke->inputList()->range(i);
      index_t first, last;
      if(!this->iterExprEval(range->first(), &first, state->iterators)
          || !this->iterExprEval(range->last(), &last, state->iterators))
      {
        return false;
      }

      WireSetFail range_fail =
        state->wires->remapInputs(first, last, &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration input range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            invoke->inputList()->lineNum(), first, last,
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  if(sub_wires.outputSize() != function->outputCount())
  {
    log_error("%s:%zu: Loop iteration invocation outputs size mismatch. "
        "Expected: %" PRIu64 " but found: %" PRIu64, this->fileName,
        invoke->lineNum(), function->outputCount(), sub_wires.outputSize());
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
    return false;
  }

  if(sub_wires.inputSize() != function->inputCount())
  {
    log_error("%s:%zu: Loop iteration invocation inputs size mismatch. "
        "Expected: %" PRIu64 " but found: %" PRIu64, this->fileName,
        invoke->lineNum(), function->inputCount(), sub_wires.inputSize());
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
    return false;
  }

  bool success = true;
  std::unordered_map<std::string, index_t> sub_iterators;
  State<Wire_T> new_state(
      &sub_wires, &sub_iterators, nullptr, nullptr);

  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < function->instanceCount(); i++)
    {
      Wire_T value;
      if(this->extractInstanceValue(
            state->instance, &value, invoke->lineNum()))
      {
        sub_instance.insert(value, state->instance->lineNum());
      }
      else { return false; }
    }

    for(size_t i = 0; i < function->shortWitnessCount(); i++)
    {
      Wire_T value;
      if(this->extractShortWitnessValue(
            state->shortWitness, &value, invoke->lineNum()))
      {
        sub_short_witness.insert(value, state->shortWitness->lineNum());
      }
      else { return false; }
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = state->switchActive;
  }

  index_t sub_instance_count;
  index_t sub_short_witness_count;

  if(!this->directiveListCheck(function->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != function->instanceCount())
  {
    log_error("%s:%zu: Instance count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->instanceCount(), sub_instance_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  if(sub_short_witness_count != function->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->shortWitnessCount(), sub_short_witness_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  *instance_count = function->instanceCount();
  *short_witness_count = function->shortWitnessCount();

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error("%s:%zu: Function \"%s\" failed to assign its output wire $%"
          PRIu64 " (sub-scope numbering)", this->fileName, invoke->lineNum(),
          name.c_str(), i);
      log_info("%s:%zu: Function \"%s\" defined here.", this->fileName,
          function->lineNum(), name.c_str());
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_instance.size(), name.c_str());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_short_witness.size(), name.c_str());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::iterExprAnonFunctionCheck(
    IterExprAnonFunction<Wire_T>* anon_func,
    wtk::utils::SkipList<index_t>* loop_outputs_all,
    wtk::utils::SkipList<index_t>* loop_outputs_used,
    State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  WireSet<Wire_T> sub_wires;
 
  /* Uses loop_outputs_all to check that the function output is a member of
   * the loop output (called for by spec). and it uses loop_outputs_used to
   * check that output wires aren't reused (convenience because although the
   * spec doesn't require this, it would produce confuzzling errors).
   */
  for(size_t i = 0; i < anon_func->outputList()->size(); i++)
  {
    switch(anon_func->outputList()->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      index_t single;
      if(!this->iterExprEval(
            anon_func->outputList()->single(i), &single, state->iterators))
      {
        return false;
      }

      if(!loop_outputs_all->has(single))
      {
        log_error("%s:%zu: Loop iteration output $%" PRIu64 " is not a member"
            " of the loop's output", this->fileName,
            anon_func->outputList()->lineNum(), single);
        return false;
      }
      if(!loop_outputs_used->insert(single))
      {
        log_warn("%s:%zu: Loop output wire $%" PRIu64 " is duplicated.",
            this->fileName, anon_func->outputList()->lineNum(), single);
      }

      WireSetFail single_fail = state->wires->remapOutput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration output wire $%" PRIu64
            " is invalid: %s", this->fileName,
            anon_func->outputList()->lineNum(), single,
            wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case IterExprWireList::RANGE:
    {
      IterExprWireRange* range = anon_func->outputList()->range(i);
      index_t first, last;
      if(!this->iterExprEval(range->first(), &first, state->iterators)
          || !this->iterExprEval(range->last(), &last, state->iterators))
      {
        return false;
      }

      if(!loop_outputs_all->has(first, last))
      {
        log_error("%s:%zu: Loop iteration outputs $%" PRIu64 " through $%"
            PRIu64 " are not a members of the loop's output", this->fileName,
            anon_func->outputList()->lineNum(), first, last);
        return false;
      }
      if(!loop_outputs_used->insert(first, last))
      {
        log_warn("%s:%zu: Loop output wires $%" PRIu64 " through $%" PRIu64
            "are duplicated.", this->fileName,
            anon_func->outputList()->lineNum(), first, last);
      }

      WireSetFail range_fail =
        state->wires->remapOutputs(first, last, &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration output range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            anon_func->outputList()->lineNum(), first, last,
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  for(size_t i = 0; i < anon_func->inputList()->size(); i++)
  {
    switch(anon_func->inputList()->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      index_t single;
      if(!this->iterExprEval(
            anon_func->inputList()->single(i), &single, state->iterators))
      {
        return false;
      }

      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration input wire $%" PRIu64
            " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), single,
            wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case IterExprWireList::RANGE:
    {
      IterExprWireRange* range = anon_func->inputList()->range(i);
      index_t first, last;
      if(!this->iterExprEval(range->first(), &first, state->iterators)
          || !this->iterExprEval(range->last(), &last, state->iterators))
      {
        return false;
      }

      WireSetFail range_fail =
        state->wires->remapInputs(first, last, &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Loop iteration input range $%" PRIu64 " through $%"
            PRIu64 " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), first, last,
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  bool success = true;
  State<Wire_T> new_state(
      &sub_wires, state->iterators, nullptr, nullptr);

  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < anon_func->instanceCount(); i++)
    {
      Wire_T value;
      if(this->extractInstanceValue(
            state->instance, &value, anon_func->lineNum()))
      {
        sub_instance.insert(value, state->instance->lineNum());
      }
      else { return false; }
    }

    for(size_t i = 0; i < anon_func->shortWitnessCount(); i++)
    {
      Wire_T value;
      if(this->extractShortWitnessValue(
            state->shortWitness, &value, anon_func->lineNum()))
      {
        sub_short_witness.insert(value, state->shortWitness->lineNum());
      }
      else { return false; }
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = state->switchActive;
  }

  index_t sub_instance_count;
  index_t sub_short_witness_count;

  if(!this->directiveListCheck(anon_func->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != anon_func->instanceCount())
  {
    log_error("%s:%zu: Instance count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->instanceCount(), sub_instance_count);
    success = false;
  }

  if(sub_short_witness_count != anon_func->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->shortWitnessCount(), sub_short_witness_count);
    success = false;
  }

  *instance_count = anon_func->instanceCount();
  *short_witness_count = anon_func->shortWitnessCount();

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error(
          "%s:%zu: Loop iteration Anonymous Function failed to assign its "
          "output wire $%" PRIu64 " (sub-scope numbering)", this->fileName,
          anon_func->lineNum(), i);
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "loop iteration anonymous function", this->fileName,
          anon_func->lineNum(), sub_instance.size());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          "loop iteration anonymous function", this->fileName,
          anon_func->lineNum(), sub_short_witness.size());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::forLoopCheck(
    ForLoop<Wire_T>* for_loop, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  if(!this->toggles->forLoopToggle)
  {
    log_error("%s:%zu: for-loops are disabled.",
        this->fileName, for_loop->lineNum());
    return false;
  }

  std::string iterName(for_loop->iterName());
  if(state->iterators->find(iterName) != state->iterators->end())
  {
    log_error("%s:%zu: for-loop iterator \"%s\" is already in use",
        this->fileName, for_loop->lineNum(), iterName.c_str());
    return false;
  }
  (*state->iterators)[iterName] = 0;

  wtk::utils::SkipList<index_t> loop_outputs_all;
  wtk::utils::SkipList<index_t> loop_outputs_used;

  for(size_t i = 0; i < for_loop->outputList()->size(); i++)
  {
    switch(for_loop->outputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = for_loop->outputList()->single(i);
      if(!loop_outputs_all.insert(single))
      {
        log_warn("%s:%zu: For loop output wire $%" PRIu64 " is duplicated",
            this->fileName, for_loop->outputList()->lineNum(), single);
      }
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = for_loop->outputList()->range(i);
      if(!loop_outputs_all.insert(range->first(), range->last()))
      {
        log_warn("%s:%zu: For loop output wire range $%" PRIu64 " through $%"
            PRIu64 " is duplicated", this->fileName,
            for_loop->outputList()->lineNum(), range->first(), range->last());
      }
      break;
    }
    }
  }

  *instance_count = 0;
  *short_witness_count = 0;

  bool success = true;

  // spec says loop is inclusive of both bounds.
  for(index_t i = for_loop->first(); i <= for_loop->last(); i++)
  {
    (*state->iterators)[iterName] = i;
    bool fail = false;
    index_t sub_instance_count;
    index_t sub_short_witness_count;

    switch(for_loop->bodyType())
    {
    case ForLoop<Wire_T>::INVOKE:
    {
      if(!this->iterExprFunctionInvokeCheck(for_loop->invokeBody(),
            &loop_outputs_all, &loop_outputs_used, state,
            &sub_instance_count, &sub_short_witness_count))
      {
        fail = true;
      }
      break;
    }
    case ForLoop<Wire_T>::ANONYMOUS:
    {
      if(!this->iterExprAnonFunctionCheck(for_loop->anonymousBody(),
            &loop_outputs_all, &loop_outputs_used, state,
            &sub_instance_count, &sub_short_witness_count))
      {
        fail = true;
      }
      break;
    }
    }

    *instance_count += sub_instance_count;
    *short_witness_count += sub_short_witness_count;

    if(fail) { success = false; break; }
  }

  state->iterators->erase(iterName);

  if(!wtk::utils::SkipList<index_t>::equivalent(
        &loop_outputs_all, &loop_outputs_used))
  {
    log_error("%s:%zu: For loop did not assign all of its output wires",
        this->fileName, for_loop->lineNum());
    success = false;
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::switchStatementCheck(
    SwitchStatement<Wire_T>* switch_stmt, State<Wire_T>* state,
    index_t* instance_count, index_t* short_witness_count)
{
  if(!this->toggles->switchCaseToggle)
  {
    log_error("%s:%zu: Switch statements are disabled", this->fileName,
        switch_stmt->lineNum());
    return false;
  }

  // Check that condition wire is okay
  bool success = true;
  WireSetFail cond_fail =
    state->wires->retrieveResource(switch_stmt->condition());
  if(cond_fail != WireSetFail::success)
  {
    log_error("%s:%zu: Switch statement condition wire $%" PRIu64
        " is invalid: %s", this->fileName, switch_stmt->lineNum(),
        switch_stmt->condition(), wireSetFailString(cond_fail));
    success = false;
  }

  // check that output wires are okay, and count outputs.
  size_t outputs_size = 0;
  for(size_t i = 0; i < switch_stmt->outputList()->size(); i++)
  {
    switch(switch_stmt->outputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = switch_stmt->outputList()->single(i);
      outputs_size++;

      WireSetFail in_fail = state->wires->insertResource(single);
      if(in_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Switch statement output wire $%" PRIu64
            " is invalid: %s", this->fileName,
            switch_stmt->outputList()->lineNum(), single,
            wireSetFailString(in_fail));
        success = false;
      }
      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = switch_stmt->outputList()->range(i);
      // spec calls for inclusive bounds.
      for(index_t j = range->first(); j <= range->last(); j++)
      {
        outputs_size++;

        WireSetFail in_fail = state->wires->insertResource(j);
        if(in_fail != WireSetFail::success)
        {
          log_error("%s:%zu: Switch statement output wire $%" PRIu64
              " is invalid: %s (in range $%" PRIu64 " through $%" PRIu64 ")",
              this->fileName, switch_stmt->outputList()->lineNum(), j,
              wireSetFailString(in_fail), range->first(), range->last());
          success = false;
        }
      }

      break;
    }
    }
  }

  // Check that case selector values are okay and find the maximum
  // instance/witness lengths
  wtk::utils::SkipList<Wire_T> case_selectors;
  *instance_count = 0;
  *short_witness_count = 0;
  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    CaseBlock<Wire_T>* case_blk = switch_stmt->caseBlock(i);
    Wire_T selector = case_blk->match();
    if(selector >= this->p)
    {
      log_error("%s:%zu: Case selector < %s > is not a member of field (p=%s)",
          this->fileName, case_blk->lineNum(),
          wtk::utils::dec(selector).c_str(), wtk::utils::dec(this->p).c_str());
      success = false;
    }

    if(!case_selectors.insert(selector))
    {
      log_error("%s:%zu: Case selector < %s > is duplicated", this->fileName,
          case_blk->lineNum(), wtk::utils::dec(selector).c_str());
      success = false;
    }

    switch(case_blk->bodyType())
    {
    case CaseBlock<Wire_T>::INVOKE:
    {
      CaseFunctionInvoke* invoke = case_blk->invokeBody();
      std::string name(invoke->name());
      auto finder = this->functionsMap.find(name);
      if(finder != this->functionsMap.end())
      {
        FunctionDeclare<Wire_T>* func = finder->second;
        if(func->instanceCount() > *instance_count)
        {
          *instance_count = func->instanceCount();
        }
        if(func->shortWitnessCount() > *short_witness_count)
        {
          *short_witness_count = func->shortWitnessCount();
        }
      }
      break;
    }
    case CaseBlock<Wire_T>::ANONYMOUS:
    {
      CaseAnonFunction<Wire_T>* anon_func = case_blk->anonymousBody();
      if(anon_func->instanceCount() > *instance_count)
      {
        *instance_count = anon_func->instanceCount();
      }
      if(anon_func->shortWitnessCount() > *short_witness_count)
      {
        *short_witness_count = anon_func->shortWitnessCount();
      }
      break;
    }
    }
  }

  std::vector<Wire_T> instance_dup;
  std::vector<Wire_T> short_witness_dup;
  if(state->checkEvaluation())
  {
    instance_dup.reserve((size_t) *instance_count);
    for(size_t i = 0; i < *instance_count; i++)
    {
      Wire_T val(0);
      if(!this->extractInstanceValue(
            state->instance, &val, switch_stmt->lineNum()))
      {
        success = false;
      }

      instance_dup.push_back(val);
    }

    short_witness_dup.reserve((size_t) *short_witness_count);
    for(size_t i = 0; i < *short_witness_count; i++)
    {
      Wire_T val(0);
      if(!this->extractShortWitnessValue(
            state->shortWitness, &val, switch_stmt->lineNum()))
      {
        success = false;
      }

      short_witness_dup.push_back(val);
    }
  }

  // Get the value of the condition wire.
  Wire_T cond_val(0);
  if(state->checkEvaluation())
  {
    cond_val = state->wires->retrieveEvaluation(switch_stmt->condition());
  }
 
  // invoke all the case bodies.
  bool matched_case = false;
  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    CaseBlock<Wire_T>* case_blk = switch_stmt->caseBlock(i);
    // this collects the values assigned to dummy output wires.
    std::vector<Wire<Wire_T>> dummies;
    dummies.reserve(state->checkEvaluation() ? outputs_size : 0);

    bool yes_case = true;
    if(state->checkEvaluation()) { yes_case = cond_val == case_blk->match(); }

    switch(case_blk->bodyType())
    {
    case CaseBlock<Wire_T>::INVOKE:
    {
      if(!this->caseFunctionInvokeCheck(case_blk->invokeBody(), state,
            outputs_size, &instance_dup, &short_witness_dup, &dummies,
            yes_case && state->switchActive))
      {
        success = false;
      }
      break;
    }
    case CaseBlock<Wire_T>::ANONYMOUS:
    {
      if(!this->caseAnonFunctionCheck(case_blk->anonymousBody(), state,
            outputs_size, &instance_dup, &short_witness_dup, &dummies,
            yes_case && state->switchActive))
      {
        success = false;
      }
      break;
    }
    }

    if(success && state->checkEvaluation())
    {
      if(yes_case)
      {
        matched_case = true;
        WireList* out_list = switch_stmt->outputList();
        size_t output_place = 0;
        for(size_t j = 0; j < out_list->size(); j++)
        {
          switch(out_list->type(j))
          {
          case WireList::SINGLE:
          {
            state->wires->insertEvaluation(out_list->single(j),
                dummies[output_place++].wire);
            break;
          }
          case WireList::RANGE:
          {
            WireRange* range = out_list->range(j);
            // loop has inclusive bounds
            for(index_t k = range->first(); k <= range->last(); k++)
            {
              state->wires->insertEvaluation(
                  k, dummies[output_place++].wire);
            }
            break;
          }
          }
        }
      }
    }
  }

  if(state->checkEvaluation() && state->switchActive && !matched_case)
  {
    log_error("%s:%zu: Switch statement failed to match a cases",
        this->fileName, switch_stmt->lineNum());
    success = false;
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::caseFunctionInvokeCheck(CaseFunctionInvoke* invoke,
    State<Wire_T>* state, size_t outputs_size,
    std::vector<Wire_T>* instance_dups,
    std::vector<Wire_T>* short_witness_dups,
    std::vector<Wire<Wire_T>>* dummies,
    bool sub_switch_active)
{
  if(!this->toggles->functionToggle)
  {
    log_error("%s:%zu: Functions are disabled.",
        this->fileName, invoke->lineNum());
    return false;
  }

  std::string name(invoke->name());
  auto iter = this->functionsMap.find(name);
  if(iter == this->functionsMap.end())
  {
    log_error("%s:%zu: Function \"%s\" was not previously declared.",
        this->fileName, invoke->lineNum(), invoke->name());
    return false;
  }

  FunctionDeclare<Wire_T>* function = iter->second;
  if(function->outputCount() != outputs_size)
  {
    log_error("%s:%zu: Case function outputs size mismatch. Expected: %"
        PRIu64 " but found %zu", this->fileName, invoke->lineNum(),
        function->outputCount(), outputs_size);
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
  }

  WireSet<Wire_T> sub_wires;
  WireSetFail dummy_fail = sub_wires.mapDummies(outputs_size, *dummies);
  if(dummy_fail != WireSetFail::success)
  {
    log_info("%s:%zu: Case function failed to map dummy outputs: %s",
        this->fileName, invoke->lineNum(), wireSetFailString(dummy_fail));
    return false;
  }

  for(size_t i = 0; i < invoke->inputList()->size(); i++)
  {
    switch(invoke->inputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = invoke->inputList()->single(i);
      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Case function gate input wire $%" PRIu64
            " is invalid: %s", this->fileName, invoke->inputList()->lineNum(),
            single, wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = invoke->inputList()->range(i);
      WireSetFail range_fail = state->wires->remapInputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Case function gate input range $%" PRIu64
            " through $%" PRIu64 " is invalid: %s", this->fileName,
            invoke->inputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  if(sub_wires.inputSize() != function->inputCount())
  {
    log_error("%s:%zu: Function inputs size mismatch. Expected: %" PRIu64
        " but found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->inputCount(), sub_wires.inputSize());
    log_info("%s:%zu: Function \"%s\" declared here.", this->fileName,
        function->lineNum(), function->name());
    return false;
  }

  std::unordered_map<std::string, index_t> iterators;
  State<Wire_T> new_state(&sub_wires, &iterators, nullptr, nullptr);

  // spec says that <switch-statement> should creates the dup lists and sub
  // streams, and pass only the sub streams to <case-invoke-function>, but
  // it was easier to implement to pass the dup list here.
  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < function->instanceCount(); i++)
    {
      sub_instance.insert((*instance_dups)[i], 0);
    }
    for(size_t i = 0; i < function->shortWitnessCount(); i++)
    {
      sub_short_witness.insert((*short_witness_dups)[i], 0);
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = sub_switch_active;
  }

  bool success = true;

  index_t sub_instance_count; 
  index_t sub_short_witness_count; 

  if(!this->directiveListCheck(function->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != function->instanceCount())
  {
    log_error("%s:%zu: Instance count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->instanceCount(), sub_instance_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  if(sub_short_witness_count != function->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, invoke->lineNum(),
        function->shortWitnessCount(), sub_short_witness_count);
    log_info("%s:%zu: Function \"%s\" declared here", this->fileName,
        function->lineNum(), function->name());
    success = false;
  }

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error("%s:%zu: Function \"%s\" failed to assign its output wire $%"
          PRIu64 " (sub-scope numbering)", this->fileName, invoke->lineNum(),
          name.c_str(), i);
      log_info("%s:%zu: Function \"%s\" defined here.", this->fileName,
          function->lineNum(), name.c_str());
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_instance.size(), name.c_str());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          "function \"%s\"", this->fileName, invoke->lineNum(),
          sub_short_witness.size(), name.c_str());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::caseAnonFunctionCheck(
    CaseAnonFunction<Wire_T>* anon_func,
    State<Wire_T>* state, size_t outputs_size,
    std::vector<Wire_T>* instance_dups,
    std::vector<Wire_T>* short_witness_dups,
    std::vector<Wire<Wire_T>>* dummies, bool sub_switch_active)
{
  WireSet<Wire_T> sub_wires;
  WireSetFail dummy_fail = sub_wires.mapDummies(outputs_size, *dummies);
  if(dummy_fail != WireSetFail::success)
  {
    log_info("%s:%zu: Case anonymous function failed to map dummy outputs: %s",
        this->fileName, anon_func->lineNum(), wireSetFailString(dummy_fail));
    return false;
  }

  for(size_t i = 0; i < anon_func->inputList()->size(); i++)
  {
    switch(anon_func->inputList()->type(i))
    {
    case WireList::SINGLE:
    {
      index_t single = anon_func->inputList()->single(i);
      WireSetFail single_fail = state->wires->remapInput(single, &sub_wires);
      if(single_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Case anonymous function gate input wire $%" PRIu64
            " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), single,
            wireSetFailString(single_fail));
        return false;
      }

      break;
    }
    case WireList::RANGE:
    {
      WireRange* range = anon_func->inputList()->range(i);
      WireSetFail range_fail = state->wires->remapInputs(
          range->first(), range->last(), &sub_wires);
      if(range_fail != WireSetFail::success)
      {
        log_error("%s:%zu: Case anonymous function gate input range $%" PRIu64
            " through $%" PRIu64 " is invalid: %s", this->fileName,
            anon_func->inputList()->lineNum(), range->first(), range->last(),
            wireSetFailString(range_fail));
        return false;
      }

      break;
    }
    }
  }

  State<Wire_T> new_state(&sub_wires, state->iterators, nullptr, nullptr);

  // spec says that <switch-statement> should creates the dup lists and sub
  // streams, and pass only the sub streams to <case-invoke-function>, but
  // it was easier to implement to pass the dup list here.
  wtk::QueueInputStream<Wire_T> sub_instance;
  wtk::QueueInputStream<Wire_T> sub_short_witness;
  if(state->checkEvaluation())
  {
    for(size_t i = 0; i < anon_func->instanceCount(); i++)
    {
      sub_instance.insert((*instance_dups)[i], 0);
    }
    for(size_t i = 0; i < anon_func->shortWitnessCount(); i++)
    {
      sub_short_witness.insert((*short_witness_dups)[i], 0);
    }

    new_state.instance = &sub_instance;
    new_state.shortWitness = &sub_short_witness;
    new_state.switchActive = sub_switch_active;
  }

  bool success = true;

  index_t sub_instance_count;
  index_t sub_short_witness_count;

  if(!this->directiveListCheck(anon_func->body(), &new_state,
        &sub_instance_count, &sub_short_witness_count))
  {
    success = false;
  }

  if(sub_instance_count != anon_func->instanceCount())
  {
    log_error("%s:%zu: Instance count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->instanceCount(), sub_instance_count);
    success = false;
  }

  if(sub_short_witness_count != anon_func->shortWitnessCount())
  {
    log_error("%s:%zu: Short Witness count mismatch. Expected: %" PRIu64
        ", found: %" PRIu64, this->fileName, anon_func->lineNum(),
        anon_func->shortWitnessCount(), sub_short_witness_count);
    success = false;
  }

  for(index_t i = 0; i < sub_wires.outputSize(); i++)
  {
    WireSetFail fail = sub_wires.retrieveResource(i);
    if(fail != WireSetFail::success)
    {
      log_error("%s:%zu: Case anonymous function failed to assign its output "
          "wire $%" PRIu64 " (sub-scope numbering)", this->fileName,
          anon_func->lineNum(), i);
      success = false;
    }
  }

  if(state->checkEvaluation())
  {
    if(sub_instance.size() != 0)
    {
      log_error("%s:%zu: Instance has %zu leftover values after processing "
          "anonymous function", this->fileName, anon_func->lineNum(),
          sub_instance.size());
      success = false;
    }

    if(sub_short_witness.size() != 0)
    {
      log_error("%s:%zu: ShortWitness has %zu leftover values after processing"
          " anonymous function", this->fileName, anon_func->lineNum(),
          sub_short_witness.size());
      success = false;
    }
  }

  return success;
}

template<typename Wire_T>
bool TreeAlarm<Wire_T>::checkTree(wtk::IRTree<Wire_T>* relation,
    InputStream<Wire_T>* instance,
    InputStream<Wire_T>* short_witness)
{

  for(size_t i = 0; i < relation->size(); i++)
  {
    if(!this->functionDeclareRecursionCheck(relation->functionDeclare(i)))
    {
      return false;
    }
  }

  for(size_t i = 0; i < relation->size(); i++)
  {
    if(!this->functionDeclareCheck(relation->functionDeclare(i)))
    {
      return false;
    }
  }

  WireSet<Wire_T> wires;
  std::unordered_map<std::string, index_t> iterators;
  State<Wire_T> state(&wires, &iterators, instance, short_witness);

  index_t instance_count;
  index_t short_witness_count;

  if(!this->directiveListCheck(relation->body(), &state,
        &instance_count, &short_witness_count))
  {
    return false;
  }

  // spec doesn't use these at top level
  (void) instance_count;
  (void) short_witness_count;

  return true;
}

template<typename Wire_T>
void TreeAlarm<Wire_T>::logCounts(bool checked_evaluation)
{
  log_info("Gate Counts:");
  if(this->gateSet->gateSet == GateSet::boolean)
  {
    if(this->gateSet->enableXor)
    {
      log_info("  @xor:           %zu", this->numXor);
    }
    else { log_info("  @xor:           (disabled)"); }

    if(this->gateSet->enableAnd)
    {
      log_info("  @and:           %zu", this->numAnd);
    }
    else { log_info("  @and:           (disabled)"); }

    if(this->gateSet->enableNot)
    {
      log_info("  @not:           %zu", this->numNot);
    }
    else { log_info("  @not:           (disabled)"); }
  }
  else
  {
    if(this->gateSet->enableAdd)
    {
      log_info("  @add:           %zu", this->numAdd);
    }
    else { log_info("  @add:           (disabled)"); }

    if(this->gateSet->enableAddC)
    {
      log_info("  @addc:          %zu", this->numAddc);
    }
    else { log_info("  @addc:          (disabled)"); }

    if(this->gateSet->enableMul)
    {
      log_info("  @mul:           %zu", this->numMul);
    }
    else { log_info("  @mul:           (disabled)"); }

    if(this->gateSet->enableMulC)
    {
      log_info("  @mulc:          %zu", this->numMulc);
    }
    else { log_info("  @mulc:          (disabled)"); }
  }

  log_info("  Copy:           %zu", this->numCopy);
  log_info("  Assign:         %zu", this->numAssign);
  log_info("  @instance:      %zu", this->numInstance);
  log_info("  @short_witness: %zu", this->numShortWitness);
  log_info("  @assert_zero:   %zu", this->numAssertZero);
  if(checked_evaluation)
  {
    log_info("  @assert_zero:   %zu (in nonselected switch-cases)",
        this->numAssertZeroDisabled);
  }
}

} } // namespace wtk::firealarm
