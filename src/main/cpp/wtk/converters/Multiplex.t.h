/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace converters {

inline wtk::index_t WireAdjuster::selector()
{
  return this->numOutput + this->numInput;
}

inline wtk::index_t WireAdjuster::adjust(wtk::index_t wire)
{
  if(wire < this->numOutput + this->numInput) { return wire; }
  else { return wire + this->numInstance + this->numWitness + 1; }
}

inline wtk::index_t WireAdjuster::nextInstance()
{
  wtk::index_t ret = this->numOutput + this->numInput + 1 + this->currInstance;
  this->currInstance++;
  return ret;
}

inline wtk::index_t WireAdjuster::nextWitness()
{
  wtk::index_t ret = this->numOutput + this->numInput + 1 +
    this->numInstance + this->currWitness;
  this->currWitness++;
  return ret;
}

template<typename Number_T>
void WireAdjuster::adjust(
    WireListBuilder<Number_T>* builder, wtk::WireList* list)
{
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::WireList::SINGLE:
    {
      builder->single(this->adjust(list->single(i)));
      break;
    }
    case wtk::WireList::RANGE:
    {
      wtk::WireRange* range = list->range(i);
      wtk::index_t first = range->first();
      wtk::index_t last = range->last();

      if(first < this->numOutput + this->numInput
          && last < this->numOutput + this->numInput)
      {
        builder->range(first, last);
      }
      else if(first < this->numOutput + this->numInput &&
          last >= this->numOutput + this->numInput)
      {
        if(first == this->numOutput + this->numInput + 1)
        {
          builder->single(first);
        }
        else
        {
          builder->range(first, this->numOutput + this->numInput - 1);
        }
        if(last == this->numOutput + this->numInput)
        {
          builder->single(last + this->numInstance + this->numWitness + 1);
        }
        else
        {
          builder->range(
              this->numOutput + this->numInput + this->numInstance
              + this->numWitness + 1,
              last + this->numInstance + this->numWitness + 1);
        }
      }
      else
      {
        builder->range(first + this->numInstance + this->numWitness + 1,
            last + this->numInstance + this->numWitness + 1);
      }

      break;
    }
    }
  }
}

template<typename Number_T>
void WireAdjuster::nextInstances(
    WireListBuilder<Number_T>* builder, wtk::index_t num)
{
  if(num == 0) { return; }
  else if(num == 1)
  {
    builder->single(this->numOutput + this->numInput + 1 + this->currInstance);
    this->currInstance++;
  }
  else
  {
    builder->range(this->numOutput + this->numInput + 1 + this->currInstance,
        this->numOutput + this->numInput + this->currInstance + num);
    this->currInstance += num;
  }
}

template<typename Number_T>
void WireAdjuster::nextWitnesses(
    WireListBuilder<Number_T>* builder, wtk::index_t num)
{
  if(num == 0) { return; }
  else if(num == 1)
  {
    builder->single( this->numOutput + this->numInput + 1
        + this->numInstance + this->currWitness);
    this->currWitness++;
  }
  else
  {
    builder->range(this->numOutput + this->numInput + 1 + this->numInstance
        + this->currWitness,
        this->numOutput + this->numInput + this->numInstance
        + this->currWitness + num);
    this->currWitness += num;
  }
}

template<typename Number_T>
Multiplex<Number_T>::Multiplex(Number_T c, bool ib)
  : characteristic(c), isBoolean(ib) { }

template<typename Number_T>
wtk::IRTree<Number_T>* Multiplex<Number_T>::transform(
    wtk::IRTree<Number_T>* original)
{
  /* Special function gate for selector bit */
  this->generateCaseChecker();

  /* function gates */
  for(size_t i = 0; i < original->size(); i++)
  {
    wtk::FunctionDeclare<Number_T>* orig_func = original->functionDeclare(i);

    TreeBuilder<Number_T> builder(&this->pool);
    std::vector<IterBounds> bounds;
    if(!this->transform(orig_func->body(), &builder, bounds))
    {
      return nullptr;
    }

    // duplicate/remove switches from the function
    wtk::irregular::TextFunctionDeclare<Number_T>* new_func =
      this->pool.functionDeclares.allocate();
    new_func->outCount = orig_func->outputCount();
    new_func->inCount = orig_func->inputCount();
    new_func->insCount = orig_func->instanceCount();
    new_func->witCount = orig_func->shortWitnessCount();
    new_func->name_ = this->pool.strdup(orig_func->name());
    new_func->directives = builder.node;

    // add the function to the output tree
    this->pool.tree.functionDeclares.emplace_back(new_func);

    // add it to a map incase it is used as a case-block
    std::string name_string(new_func->name());
    this->functions.emplace(name_string, new_func);
  }

  /* Circuit body */
  TreeBuilder<Number_T> builder(&this->pool);
  std::vector<IterBounds> bounds;
  if(!this->transform(original->body(), &builder, bounds)) { return nullptr; }
  this->pool.tree.directives = builder.node;
  return &this->pool.tree;
}

/**
 * At the top level duplicate everything except a switch-case (obviously)
 */
template<typename Number_T>
bool Multiplex<Number_T>::transform(
    wtk::DirectiveList<Number_T>* original,
    TreeBuilder<Number_T>* builder,
    std::vector<IterBounds>& bounds)
{
  // ephemeral wires used in the multiplexers.
  wtk::index_t next_ephemeral = 1ULL << 63;

  // duplicate most of the gates, except for switch-case.
  for(size_t i = 0; i < original->size(); i++)
  {
    switch(original->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      builder->binaryGate(original->binaryGate(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      builder->binaryConstGate(original->binaryConstGate(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      builder->unaryGate(original->unaryGate(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      builder->assign(original->assign(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      builder->input(original->input(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE:
    {
      builder->deleteSingle(original->deleteSingle(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE:
    {
      builder->deleteRange(original->deleteRange(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO:
    {
      builder->assertZero(original->assertZero(i));
      break;
    }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      wtk::FunctionInvoke* orig_invoke = original->functionInvoke(i);
      WireListBuilder<Number_T> outputs(&this->pool);
      WireListBuilder<Number_T> inputs(&this->pool);

      outputs.duplicate(orig_invoke->outputList());
      inputs.duplicate(orig_invoke->inputList());
      builder->functionInvoke(&outputs, orig_invoke->name(), &inputs);
      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      wtk::AnonFunction<Number_T>* orig_anon = original->anonFunction(i);
      WireListBuilder<Number_T> outputs(&this->pool);
      WireListBuilder<Number_T> inputs(&this->pool);

      outputs.duplicate(orig_anon->outputList());
      inputs.duplicate(orig_anon->inputList());

      TreeBuilder<Number_T> sub_builder(&this->pool);
      if(!this->transform(orig_anon->body(), &sub_builder, bounds))
      {
        return false;
      }

      builder->anonFunction(&outputs, &inputs, orig_anon->instanceCount(),
          orig_anon->shortWitnessCount(), &sub_builder);
      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      wtk::ForLoop<Number_T>* for_loop = original->forLoop(i);

      WireListBuilder<Number_T> out(&this->pool);
      out.duplicate(for_loop->outputList());

      IterExprBuilder<Number_T> ieb(&this->pool);
      IterExprWireListBuilder<Number_T> iter_out(&this->pool);
      IterExprWireListBuilder<Number_T> iter_in(&this->pool);

      this->pool.registerIter(for_loop->iterName());
      switch(for_loop->bodyType())
      {
      case wtk::ForLoop<Number_T>::INVOKE:
      {
        wtk::IterExprFunctionInvoke* invoke = for_loop->invokeBody();
        iter_out.duplicate(invoke->outputList(), &ieb);
        iter_in.duplicate(invoke->inputList(), &ieb);

        builder->forLoopInvoke(&out, for_loop->iterName(),
            for_loop->first(), for_loop->last(), &iter_out, invoke->name(),
            &iter_in);
        break;
      }
      case wtk::ForLoop<Number_T>::ANONYMOUS:
      {
        wtk::IterExprAnonFunction<Number_T>* anon = for_loop->anonymousBody();
        iter_out.duplicate(anon->outputList(), &ieb);
        iter_in.duplicate(anon->inputList(), &ieb);

        TreeBuilder<Number_T> body(&this->pool);
        bounds.emplace_back();
        bounds.back().iter = for_loop->iterName();
        bounds.back().first = for_loop->first();
        bounds.back().last = for_loop->last();
        this->transform(anon->body(), &body, bounds);
        bounds.pop_back();

        builder->forLoopAnonymous(&out, for_loop->iterName(),
            for_loop->first(), for_loop->last(), &iter_out, &iter_in,
            anon->instanceCount(), anon->shortWitnessCount(), &body);
        break;
      }
      }

      this->pool.unregisterIter();
      break;
    }
    // multiplex the switch-cases
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      wtk::SwitchStatement<Number_T>* switch_stmt =
        original->switchStatement(i);
      log_assert(switch_stmt->size() != 0);

      // The grammar should disallow size() == 0
      if(switch_stmt->size() == 1)
      {
        wtk::CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(0);

        // assert that the case matches the condition.
        if(this->isBoolean)
        {
          if(case_blk->match() == Number_T(0))
          {
            builder->assertZero(switch_stmt->condition());
          }
          else
          {
            builder->notGate(next_ephemeral, switch_stmt->condition());
            builder->assertZero(next_ephemeral++);
          }
        }
        else
        {
          builder->addcGate(next_ephemeral, switch_stmt->condition(),
              (this->characteristic - 1) * case_blk->match());
          builder->assertZero(next_ephemeral++);
        }

        wtk::WireList* out_list = switch_stmt->outputList();
        WireListBuilder<Number_T> out_list_bldr(&this->pool);
        out_list_bldr.duplicate(out_list);
        WireListBuilder<Number_T> in_list_bldr(&this->pool);

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* orig_invoke = case_blk->invokeBody();
          in_list_bldr.duplicate(orig_invoke->inputList());

          builder->functionInvoke(
              &out_list_bldr, orig_invoke->name(), &in_list_bldr);
          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* orig_anon =
            case_blk->anonymousBody();
          in_list_bldr.duplicate(orig_anon->inputList());

          TreeBuilder<Number_T> sub_builder(&this->pool);
          if(!this->transform(orig_anon->body(), &sub_builder, bounds))
          {
            return false;
          }

          builder->anonFunction(&out_list_bldr, &in_list_bldr,
              orig_anon->instanceCount(), orig_anon->shortWitnessCount(),
              &sub_builder);
          break;
        }
        }

        break;
      }

      wtk::index_t ins_count;
      wtk::index_t wit_count;
      if(!wtk::utils::maxInsWit(
          switch_stmt, &this->functions, &ins_count, &wit_count))
      {
        return false;
      }

      wtk::index_t const ins_first = next_ephemeral;
      if(ins_count == 1) { builder->instance(next_ephemeral++); }
      else if(ins_count > 1)
      {
        next_ephemeral += ins_count;
        WireListBuilder<Number_T> ins_loop_outs(&this->pool);
        IterExprBuilder<Number_T> iters(&this->pool);
        IterExprWireListBuilder<Number_T> ins_loop_iter_outs(&this->pool);
        IterExprWireListBuilder<Number_T> ins_loop_iter_ins(&this->pool);
        TreeBuilder<Number_T> ins_loop_body(&this->pool);

        ins_loop_outs.range(ins_first, ins_first + ins_count - 1);
        ins_loop_iter_outs.single(iters.name("wtk::mux::i"));
        ins_loop_body.instance(0);

        builder->forLoopAnonymous(&ins_loop_outs, "wtk::mux::i",
            ins_first, ins_first + ins_count - 1,
            &ins_loop_iter_outs, &ins_loop_iter_ins, 1, 0, &ins_loop_body);
      }

      wtk::index_t const wit_first = next_ephemeral;
      if(wit_count == 1) { builder->shortWitness(next_ephemeral++); }
      else if(wit_count > 1)
      {
        next_ephemeral += wit_count;
        WireListBuilder<Number_T> wit_loop_outs(&this->pool);
        IterExprBuilder<Number_T> iters(&this->pool);
        IterExprWireListBuilder<Number_T> wit_loop_iter_outs(&this->pool);
        IterExprWireListBuilder<Number_T> wit_loop_iter_ins(&this->pool);
        TreeBuilder<Number_T> wit_loop_body(&this->pool);

        wit_loop_outs.range(wit_first, wit_first + wit_count - 1);
        wit_loop_iter_outs.single(iters.name("wtk::mux::i"));
        wit_loop_body.shortWitness(0);

        builder->forLoopAnonymous(&wit_loop_outs, "wtk::mux::i",
            wit_first, wit_first + wit_count - 1,
            &wit_loop_iter_outs, &wit_loop_iter_ins, 0, 1, &wit_loop_body);
      }

      wtk::index_t const num_outputs =
        wtk::utils::countWireList(switch_stmt->outputList());

      wtk::index_t const cond_wire = switch_stmt->condition();

      wtk::index_t const first_selector = this->muxStart(
          switch_stmt, &next_ephemeral, cond_wire, builder);

      wtk::index_t const first_output = next_ephemeral;
      next_ephemeral += switch_stmt->size() * num_outputs;
      // output each case.
      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        wtk::CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(j);
        
        WireListBuilder<Number_T> outputs(&this->pool);
        if(num_outputs == 1)
        {
          outputs.single(first_output + j);
        }
        else if(num_outputs > 1)
        {
          outputs.range(first_output + (j * num_outputs),
              first_output + ((j + 1) * num_outputs) - 1);
        }

        WireListBuilder<Number_T> inputs(&this->pool);
        TreeBuilder<Number_T> case_builder(&this->pool);

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* invoke = case_blk->invokeBody();

          auto finder = this->functions.find(std::string(invoke->name()));
          if(finder == this->functions.end())
          {
            log_error("couldn't find function \'%s\'", invoke->name());
            return false;
          }
          wtk::FunctionDeclare<Number_T>* declare = finder->second;

          inputs.duplicate(invoke->inputList());
          inputs.single(first_selector + j);
          if(declare->instanceCount() == 1) { inputs.single(ins_first); }
          else if(declare->instanceCount() > 1)
          {
            inputs.range(ins_first, ins_first + declare->instanceCount() - 1);
          }

          if(declare->shortWitnessCount() == 1) { inputs.single(wit_first); }
          else if(declare->shortWitnessCount() > 1)
          {
            inputs.range(
                wit_first, wit_first + declare->shortWitnessCount() - 1);
          }

          WireAdjuster adjuster;
          adjuster.numOutput = num_outputs;
          adjuster.numInput = wtk::utils::countWireList(invoke->inputList());
          adjuster.numInstance = declare->instanceCount();
          adjuster.numWitness = declare->shortWitnessCount();

          std::vector<IterBounds> sub_bounds;
          if(!this->transformAdjust(
                declare->body(), &case_builder, &adjuster, sub_bounds))
          {
            return false;
          }

          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();

          inputs.duplicate(anon->inputList());
          inputs.single(first_selector + j);
          if(anon->instanceCount() == 1) { inputs.single(ins_first); }
          else if(anon->instanceCount() > 1)
          {
            inputs.range(ins_first, ins_first + anon->instanceCount() - 1);
          }

          if(anon->shortWitnessCount() == 1) { inputs.single(wit_first); }
          else if(anon->shortWitnessCount() > 1)
          {
            inputs.range(wit_first, wit_first + anon->shortWitnessCount() - 1);
          }

          WireAdjuster adjuster;
          adjuster.numOutput = num_outputs;
          adjuster.numInput = wtk::utils::countWireList(anon->inputList());
          adjuster.numInstance = anon->instanceCount();
          adjuster.numWitness = anon->shortWitnessCount();

          if(!this->transformAdjust(
                anon->body(), &case_builder, &adjuster, bounds))
          {
            return false;
          }

          break;
        }
        }

        builder->anonFunction(&outputs, &inputs, 0, 0, &case_builder);
      }

      this->muxFinish(switch_stmt, &next_ephemeral, first_selector,
          first_output, num_outputs, nullptr, builder);

      break;
    }
    }
  }

  return true;
}

template<typename Number_T>
void exponentiate(TreeBuilder<Number_T>* builder,
    Number_T exponent, wtk::index_t in_wire, wtk::index_t* curr_wire)
{
  if(exponent == 1) { return; }
  else if(exponent % 2 == 1)
  {
    exponentiate<Number_T>(builder, exponent - 1, in_wire, curr_wire);
    builder->mulGate(*curr_wire + 1, *curr_wire, in_wire);
    *curr_wire += 1;
  }
  else
  {
    exponentiate<Number_T>(builder, exponent / 2, in_wire, curr_wire);
    builder->mulGate(*curr_wire + 1, *curr_wire, *curr_wire);
    *curr_wire += 1;
  }
}

template<typename Number_T>
void Multiplex<Number_T>::generateCaseChecker()
{
  if(!this->isBoolean)
  {
    // output is the selector bit (0 or 1)
    wtk::index_t const output_count = 1;
    // input 1 is the switch condition wire.
    // input 2 is the additive inverse of the case selector value.
    wtk::index_t const input_count = 2;
    // no instance/witness
    wtk::index_t const instance_count = 0;
    wtk::index_t const witness_count = 0;

    TreeBuilder<Number_T> builder(&this->pool);

    // add the two inputs for (condition - case)
    builder.addGate(3, 1, 2);

    wtk::index_t wire = 3;
    exponentiate<Number_T>(&builder, this->characteristic - 1, wire, &wire);

    builder.mulcGate(wire + 1, wire, this->characteristic - 1);
    builder.addcGate(0, wire + 1, Number_T(1));

    wtk::irregular::TextFunctionDeclare<Number_T>* func =
      this->pool.functionDeclares.allocate();
    func->name_ = this->pool.strdup("wtk::mux::check_case");
    func->outCount = output_count;
    func->inCount = input_count;
    func->insCount = instance_count;
    func->witCount = witness_count;
    func->directives = builder.node;

    this->pool.tree.functionDeclares.emplace_back(func);
  }
}

template<typename Number_T>
bool Multiplex<Number_T>::transformAdjust(
    wtk::DirectiveList<Number_T>* original,
    TreeBuilder<Number_T>* builder,
    WireAdjuster* adjuster,
    std::vector<IterBounds>& bounds)
{
  wtk::index_t next_ephemeral = 1ULL << 63;
  (void) next_ephemeral;

  // duplicate most of the gates, except for switch-case.
  for(size_t i = 0; i < original->size(); i++)
  {
    switch(original->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      wtk::BinaryGate* gate = original->binaryGate(i);
      builder->binaryGate(
          adjuster->adjust(gate->outputWire()),
          gate->calculation(),
          adjuster->adjust(gate->leftWire()),
          adjuster->adjust(gate->rightWire()));
      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      wtk::BinaryConstGate<Number_T>* gate = original->binaryConstGate(i);
      builder->binaryConstGate(
          adjuster->adjust(gate->outputWire()),
          gate->calculation(),
          adjuster->adjust(gate->leftWire()),
          gate->rightValue());
      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      wtk::UnaryGate* gate = original->unaryGate(i);
      builder->unaryGate(
          adjuster->adjust(gate->outputWire()),
          gate->calculation(),
          adjuster->adjust(gate->inputWire()));
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      wtk::Assign<Number_T>* assign = original->assign(i);
      builder->assign(
          adjuster->adjust(assign->outputWire()),
          assign->constValue());
      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      wtk::Input* input = original->input(i);
      if(input->stream() == wtk::Input::INSTANCE)
      {
        builder->copy(adjuster->adjust(input->outputWire()),
            adjuster->nextInstance());
      }
      else
      {
        builder->copy(adjuster->adjust(input->outputWire()),
            adjuster->nextWitness());
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE:
    {
      builder->deleteSingle(
          adjuster->adjust(original->deleteSingle(i)->wire()));
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE:
    {
      wtk::WireRange* range = original->deleteRange(i);
      builder->deleteRange(
          adjuster->adjust(range->first()), adjuster->adjust(range->last()));
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO:
    {
      if(this->isBoolean)
      {
        builder->andGate(next_ephemeral, adjuster->selector(),
            adjuster->adjust(original->assertZero(i)->wire()));
      }
      else
      {
        builder->mulGate(next_ephemeral, adjuster->selector(),
            adjuster->adjust(original->assertZero(i)->wire()));
      }
      builder->assertZero(next_ephemeral++);
      break;
    }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      wtk::FunctionInvoke* orig_invoke = original->functionInvoke(i);
      WireListBuilder<Number_T> outputs(&this->pool);
      WireListBuilder<Number_T> inputs(&this->pool);

      auto finder = this->functions.find(orig_invoke->name());
      if(finder == this->functions.end())
      {
        log_error("could not find function \"%s\"", orig_invoke->name());
        return false;
      }
      wtk::FunctionDeclare<Number_T>* orig_declare = finder->second;

      adjuster->adjust(&outputs, orig_invoke->outputList());
      adjuster->adjust(&inputs, orig_invoke->inputList());
      inputs.single(adjuster->selector());
      adjuster->nextInstances(&inputs, orig_declare->instanceCount());
      adjuster->nextWitnesses(&inputs, orig_declare->shortWitnessCount());

      WireAdjuster sub_adjuster;
      sub_adjuster.numOutput = orig_declare->outputCount();
      sub_adjuster.numInput = orig_declare->inputCount();
      sub_adjuster.numInstance = orig_declare->instanceCount();
      sub_adjuster.numWitness = orig_declare->shortWitnessCount();

      TreeBuilder<Number_T> sub_builder(&this->pool);
      std::vector<IterBounds> sub_bounds;
      if(!this->transformAdjust(
            orig_declare->body(), &sub_builder, &sub_adjuster, sub_bounds))
      {
        return false;
      }

      builder->anonFunction(&outputs, &inputs, 0, 0, &sub_builder);
      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      wtk::AnonFunction<Number_T>* orig_anon = original->anonFunction(i);
      WireListBuilder<Number_T> outputs(&this->pool);
      WireListBuilder<Number_T> inputs(&this->pool);

      adjuster->adjust(&outputs, orig_anon->outputList());
      adjuster->adjust(&inputs, orig_anon->inputList());
      inputs.single(adjuster->selector());
      adjuster->nextInstances(&inputs, orig_anon->instanceCount());
      adjuster->nextWitnesses(&inputs, orig_anon->shortWitnessCount());

      WireAdjuster sub_adjuster;
      sub_adjuster.numOutput =
        wtk::utils::countWireList(orig_anon->outputList());
      sub_adjuster.numInput =
        wtk::utils::countWireList(orig_anon->inputList());
      sub_adjuster.numInstance = orig_anon->instanceCount();
      sub_adjuster.numWitness = orig_anon->shortWitnessCount();

      TreeBuilder<Number_T> sub_builder(&this->pool);
      if(!this->transformAdjust(
            orig_anon->body(), &sub_builder, &sub_adjuster, bounds))
      {
        return false;
      }

      builder->anonFunction(&outputs, &inputs, 0, 0, &sub_builder);

      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      wtk::ForLoop<Number_T>* for_loop = original->forLoop(i);

      WireListBuilder<Number_T> outputs(&this->pool);
      adjuster->adjust(&outputs, for_loop->outputList());

      wtk::IterExprWireList* iter_out_list;
      wtk::IterExprWireList* iter_in_list;
      wtk::index_t ins_count;
      wtk::index_t wit_count;
      wtk::DirectiveList<Number_T>* orig_body;
      bool has_name = false;

      switch(for_loop->bodyType())
      {
      case wtk::ForLoop<Number_T>::INVOKE:
      {
        wtk::IterExprFunctionInvoke* orig_invoke = for_loop->invokeBody();

        auto finder = this->functions.find(orig_invoke->name());
        if(finder == this->functions.end())
        {
          log_error("could not find function \"%s\"", orig_invoke->name());
          return false;
        }
        wtk::FunctionDeclare<Number_T>* orig_declare = finder->second;

        iter_out_list = orig_invoke->outputList();
        iter_in_list = orig_invoke->inputList();
        ins_count = orig_declare->instanceCount();
        wit_count = orig_declare->shortWitnessCount();
        orig_body = orig_declare->body();
        has_name = true;

        break;
      }
      case wtk::ForLoop<Number_T>::ANONYMOUS:
      {
        wtk::IterExprAnonFunction<Number_T>* anon = for_loop->anonymousBody();

        iter_out_list = anon->outputList();
        iter_in_list = anon->inputList();
        ins_count = anon->instanceCount();
        wit_count = anon->shortWitnessCount();
        orig_body = anon->body();

        break;
      }
      default: { log_error("unreachable"); return false; }
      }

      IterExprBuilder<Number_T> ieb(&this->pool);
      IterExprWireListBuilder<Number_T> iter_out_new(&this->pool);
      IterExprWireListBuilder<Number_T> iter_in_new(&this->pool);

      bounds.emplace_back();
      bounds.back().iter = for_loop->iterName();
      bounds.back().first = for_loop->first();
      bounds.back().last = for_loop->last();
      this->pool.registerIter(for_loop->iterName());

      wtk::index_t const threshold_val =
        adjuster->numOutput + adjuster->numInput;
      wtk::index_t const adjustment =
        adjuster->numInstance + adjuster->numWitness + 1;

      wtk::index_t num_outputs = 0;
      for(size_t i = 0; i < iter_out_list->size(); i++)
      {
        switch(iter_out_list->type(i))
        {
        case wtk::IterExprWireList::SINGLE:
        {
          IterThreshold threshold_met = this->iterExprStraddlesThreshold(
              iter_out_list->single(i), bounds, threshold_val);

          switch(threshold_met)
          {
          case IterThreshold::lessthan:
          {
            iter_out_new.single(ieb.duplicate(iter_out_list->single(i)));
            break;
          }
          case IterThreshold::straddle:
          {
            log_error("Multiplex Converter cannot handle iterator expressions"
                " straddling the input to local transition (output)");
            return false;
          }
          case IterThreshold::greaterthan:
          {
            iter_out_new.single(ieb.add(ieb.literal(adjustment),
                  ieb.duplicate(iter_out_list->single(i))));
            break;
          }
          }

          num_outputs++;
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* range = iter_out_list->range(i);
          IterThreshold threshold_met = this->iterExprRangeStraddlesThreshold(
              range->first(), range->last(), bounds, threshold_val);

          switch(threshold_met)
          {
          case IterThreshold::lessthan:
          {
            iter_out_new.range(
                ieb.duplicate(range->first()), ieb.duplicate(range->last()));
            break;
          }
          case IterThreshold::straddle:
          {
            log_error("Multiplex Converter cannot handle iterator expression"
                " ranges straddling the input to local transition (output)");
            return false;
          }
          case IterThreshold::greaterthan:
          {
            iter_out_new.range(
                ieb.add(
                  ieb.literal(adjustment), ieb.duplicate(range->first())),
                ieb.add(
                  ieb.literal(adjustment), ieb.duplicate(range->last())));
            break;
          }
          }

          num_outputs += this->iterExprRangeSize(
              range->first(), range->last(), bounds);
          break;
        }
        }
      }

      wtk::index_t num_inputs = 0;
      for(size_t i = 0; i < iter_in_list->size(); i++)
      {
        switch(iter_in_list->type(i))
        {
        case wtk::IterExprWireList::SINGLE:
        {
          IterThreshold threshold_met = this->iterExprStraddlesThreshold(
              iter_in_list->single(i), bounds, threshold_val);

          switch(threshold_met)
          {
          case IterThreshold::lessthan:
          {
            iter_in_new.single(ieb.duplicate(iter_in_list->single(i)));
            break;
          }
          case IterThreshold::straddle:
          {
            log_error("Multiplex Converter cannot handle iterator expressions"
                " straddling the input to local transition (input)");
            return false;
          }
          case IterThreshold::greaterthan:
          {
            iter_in_new.single(ieb.add(ieb.literal(adjustment),
                  ieb.duplicate(iter_in_list->single(i))));
            break;
          }
          }

          num_inputs++;
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* range = iter_in_list->range(i);
          IterThreshold threshold_met = this->iterExprRangeStraddlesThreshold(
              range->first(), range->last(), bounds, threshold_val);

          switch(threshold_met)
          {
          case IterThreshold::lessthan:
          {
            iter_in_new.range(
                ieb.duplicate(range->first()), ieb.duplicate(range->last()));
            break;
          }
          case IterThreshold::straddle:
          {
            log_error("Multiplex Converter cannot handle iterator expression"
                " ranges straddling the input to local transition (input)");
            return false;
          }
          case IterThreshold::greaterthan:
          {
            iter_in_new.range(
                ieb.add(
                  ieb.literal(adjustment), ieb.duplicate(range->first())),
                ieb.add(
                  ieb.literal(adjustment), ieb.duplicate(range->last())));
            break;
          }
          }

          num_inputs += this->iterExprRangeSize(
              range->first(), range->last(), bounds);
          break;
        }
        }
      }

      iter_in_new.single(ieb.literal(adjuster->selector()));

      if(ins_count == 1)
      {
        iter_in_new.single(ieb.add(ieb.name(for_loop->iterName()),
            ieb.literal(adjuster->numOutput + adjuster->numInput + 1
              + adjuster->currInstance - for_loop->first())));
        adjuster->currInstance += for_loop->last() - for_loop->first() + 1;
      }
      else if(ins_count > 1)
      {
        iter_in_new.range(
            ieb.add(
              ieb.literal(adjuster->numOutput + adjuster->numInput
                + adjuster->currInstance + 1),
              ieb.mul(
                ieb.sub(
                  ieb.name(for_loop->iterName()),
                  ieb.literal(for_loop->first())),
                ieb.literal(ins_count))),
            ieb.add(
              ieb.literal(adjuster->numOutput + adjuster->numInput
                + adjuster->currInstance + 1 + ins_count),
              ieb.mul(
                ieb.sub(
                  ieb.name(for_loop->iterName()),
                  ieb.literal(for_loop->first())),
                ieb.literal(ins_count))));
        adjuster->currInstance +=
          (for_loop->last() - for_loop->first() + 1) * ins_count;
      }

      if(wit_count == 1)
      {
        iter_in_new.single(ieb.add(ieb.name(for_loop->iterName()),
            ieb.literal(adjuster->numOutput + adjuster->numInput + 1
              + adjuster->numInstance + adjuster->currWitness
              - for_loop->first())));
        adjuster->currWitness += for_loop->last() - for_loop->first() + 1;
      }
      else if(wit_count > 1)
      {
        iter_in_new.range(
            ieb.add(
              ieb.literal(adjuster->numOutput + adjuster->numInput
                + adjuster->numInstance + adjuster->currWitness + 1),
              ieb.mul(
                ieb.sub(
                  ieb.name(for_loop->iterName()),
                  ieb.literal(for_loop->first())),
                ieb.literal(wit_count))),
            ieb.add(
              ieb.literal(adjuster->numOutput + adjuster->numInput
                + adjuster->numInstance + adjuster->currWitness + 1
                + wit_count),
              ieb.mul(
                ieb.sub(
                  ieb.name(for_loop->iterName()),
                  ieb.literal(for_loop->first())),
                ieb.literal(wit_count))));
        adjuster->currWitness +=
          (for_loop->last() - for_loop->first() + 1) * wit_count;
      }

      WireAdjuster sub_adjuster;
      sub_adjuster.numOutput = num_outputs;
      sub_adjuster.numInput = num_inputs;
      sub_adjuster.numInstance = ins_count;
      sub_adjuster.numWitness = wit_count;
      TreeBuilder<Number_T> sub_builder(&this->pool);

      if(has_name)
      {
        std::vector<IterBounds> sub_bounds;
        if(!this->transformAdjust(
              orig_body, &sub_builder, &sub_adjuster, sub_bounds))
        {
          return false;
        }
      }
      else
      {
        if(!this->transformAdjust(
              orig_body, &sub_builder, &sub_adjuster, bounds))
        {
          return false;
        }
      }

      builder->forLoopAnonymous(&outputs, for_loop->iterName(),
          for_loop->first(), for_loop->last(), &iter_out_new,
          &iter_in_new, 0, 0, &sub_builder);

      bounds.pop_back();
      this->pool.unregisterIter();
      break;
    }
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      wtk::SwitchStatement<Number_T>* switch_stmt =
        original->switchStatement(i);

      // The grammar should disallow size() == 0
      if(switch_stmt->size() == 1)
      {
        wtk::CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(0);

        // assert that the case matches the condition.
        if(this->isBoolean)
        {
          if(case_blk->match() == Number_T(0))
          {
            builder->andGate(next_ephemeral,
                adjuster->selector(), switch_stmt->condition());
            builder->assertZero(next_ephemeral++);
          }
          else
          {
            builder->notGate(
                next_ephemeral, adjuster->adjust(switch_stmt->condition()));
            builder->andGate(
                next_ephemeral + 1, next_ephemeral, adjuster->selector());
            builder->assertZero(next_ephemeral + 1);
            next_ephemeral += 2;
          }
        }
        else
        {
          builder->addcGate(next_ephemeral,
              adjuster->adjust(switch_stmt->condition()),
              (this->characteristic - 1) * case_blk->match());
          builder->mulGate(
              next_ephemeral + 1, next_ephemeral, adjuster->selector());
          builder->assertZero(next_ephemeral + 1);
          next_ephemeral += 2;
        }

        WireListBuilder<Number_T> outputs(&this->pool);
        WireListBuilder<Number_T> inputs(&this->pool);
        wtk::DirectiveList<Number_T>* sub_body = nullptr;
        WireAdjuster sub_adjuster;

        adjuster->adjust(&outputs, switch_stmt->outputList());
        sub_adjuster.numOutput =
          wtk::utils::countWireList(switch_stmt->outputList());
        TreeBuilder<Number_T> sub_builder(&this->pool);

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* orig_invoke = case_blk->invokeBody();

          auto finder = this->functions.find(orig_invoke->name());
          if(finder == this->functions.end())
          {
            log_error("could not find function \"%s\"", orig_invoke->name());
            return false;
          }
          wtk::FunctionDeclare<Number_T>* orig_decl = finder->second;

          sub_adjuster.numInput =
            wtk::utils::countWireList(orig_invoke->inputList());
          adjuster->adjust(&inputs, orig_invoke->inputList());
          inputs.single(adjuster->selector());
          adjuster->nextInstances(&inputs, orig_decl->instanceCount());
          sub_adjuster.numInstance = orig_decl->instanceCount();
          adjuster->nextWitnesses(&inputs, orig_decl->shortWitnessCount());
          sub_adjuster.numWitness = orig_decl->shortWitnessCount();

          sub_body = orig_decl->body();

          std::vector<IterBounds> sub_bounds;
          if(!this->transformAdjust(
                sub_body, &sub_builder, &sub_adjuster, sub_bounds))
          {
            return false;
          }

          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* orig_anon =
            case_blk->anonymousBody();

          sub_adjuster.numInput =
            wtk::utils::countWireList(orig_anon->inputList());
          adjuster->adjust(&inputs, orig_anon->inputList());
          inputs.single(adjuster->selector());
          adjuster->nextInstances(&inputs, orig_anon->instanceCount());
          sub_adjuster.numInstance = orig_anon->instanceCount();
          adjuster->nextWitnesses(&inputs, orig_anon->shortWitnessCount());
          sub_adjuster.numWitness = orig_anon->shortWitnessCount();

          sub_body = orig_anon->body();

          if(!this->transformAdjust(
                sub_body, &sub_builder, &sub_adjuster, bounds))
          {
            return false;
          }

          break;
        }
        }


        builder->anonFunction(&outputs, &inputs, 0, 0, &sub_builder);
        break;
      }

      wtk::index_t ins_count;
      wtk::index_t wit_count;
      if(!wtk::utils::maxInsWit(
          switch_stmt, &this->functions, &ins_count, &wit_count))
      {
        return false;
      }

      wtk::index_t const ins_first = adjuster->nextInstance();
      wtk::index_t const wit_first = adjuster->nextWitness();
      adjuster->currInstance += ins_count - 1;
      adjuster->currWitness += wit_count - 1;

      wtk::index_t const num_outputs =
        wtk::utils::countWireList(switch_stmt->outputList());
      wtk::index_t const cond_wire = switch_stmt->condition();
      wtk::index_t const first_selector = this->muxStart(
          switch_stmt, &next_ephemeral, cond_wire, builder);
      wtk::index_t const actual_selector = next_ephemeral;
      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        if(this->isBoolean)
        {
          builder->andGate(next_ephemeral++,
              first_selector + (wtk::index_t) j, adjuster->selector());
        }
        else
        {
          builder->mulGate(next_ephemeral++,
              first_selector + (wtk::index_t) j, adjuster->selector());
        }
      }
      wtk::index_t const first_output = next_ephemeral;
      next_ephemeral += switch_stmt->size() * num_outputs;

      // output each case.
      // Adjust the inputs to have selector-bit and instance/witness.
      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        wtk::CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(j);
        
        WireListBuilder<Number_T> outputs(&this->pool);
        if(num_outputs == 1)
        {
          outputs.single(first_output + j);
        }
        else if(num_outputs > 1)
        {
          outputs.range(first_output + (j * num_outputs),
              first_output + ((j + 1) * num_outputs) - 1);
        }

        WireListBuilder<Number_T> inputs(&this->pool);
        TreeBuilder<Number_T> case_builder(&this->pool);

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* invoke = case_blk->invokeBody();

          auto finder = this->functions.find(std::string(invoke->name()));
          if(finder == this->functions.end())
          {
            log_error("couldn't find function \'%s\'", invoke->name());
            return false;
          }
          wtk::FunctionDeclare<Number_T>* declare = finder->second;

          inputs.duplicate(invoke->inputList());

          // Selector wire and instance/witnesses are converted to inputs.
          inputs.single(actual_selector + j);
          if(declare->instanceCount() == 1) { inputs.single(ins_first); }
          else if(declare->instanceCount() > 1)
          {
            inputs.range(ins_first, ins_first + declare->instanceCount() - 1);
          }

          if(declare->shortWitnessCount() == 1) { inputs.single(wit_first); }
          else if(declare->shortWitnessCount() > 1)
          {
            inputs.range(
                wit_first, wit_first + declare->shortWitnessCount() - 1);
          }

          WireAdjuster adjuster;
          adjuster.numOutput = num_outputs;
          adjuster.numInput = wtk::utils::countWireList(invoke->inputList());
          adjuster.numInstance = declare->instanceCount();
          adjuster.numWitness = declare->shortWitnessCount();

          std::vector<IterBounds> sub_bounds;
          if(!this->transformAdjust(
                declare->body(), &case_builder, &adjuster, sub_bounds))
          {
            return false;
          }

          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();

          inputs.duplicate(anon->inputList());

          // Selector wire and instance/witnesses are converted to inputs.
          inputs.single(actual_selector + j);
          if(anon->instanceCount() == 1) { inputs.single(ins_first); }
          else if(anon->instanceCount() > 1)
          {
            inputs.range(ins_first, ins_first + anon->instanceCount() - 1);
          }

          if(anon->shortWitnessCount() == 1) { inputs.single(wit_first); }
          else if(anon->shortWitnessCount() > 1)
          {
            inputs.range(wit_first, wit_first + anon->shortWitnessCount() - 1);
          }

          WireAdjuster adjuster;
          adjuster.numOutput = num_outputs;
          adjuster.numInput = wtk::utils::countWireList(anon->inputList());
          adjuster.numInstance = anon->instanceCount();
          adjuster.numWitness = anon->shortWitnessCount();

          if(!this->transformAdjust(
                anon->body(), &case_builder, &adjuster, bounds))
          {
            return false;
          }

          break;
        }
        }

        builder->anonFunction(&outputs, &inputs, 0, 0, &case_builder);
      }

      wtk::index_t outer_selector = adjuster->selector();
      this->muxFinish(switch_stmt, &next_ephemeral, first_selector,
          first_output, num_outputs, &outer_selector, builder);

      break;
    }
    default:
    {
      log_error("feature unimplemented");
      break;
    }
    }
  }

  return true;
}

template<typename Number_T>
wtk::index_t Multiplex<Number_T>::muxStart(
    wtk::SwitchStatement<Number_T>* switch_stmt,
      wtk::index_t* const next_ephemeral,
      wtk::index_t const cond_wire,
      TreeBuilder<Number_T>* const builder)
{
  if(this->isBoolean)
  {
    // assume there's either 1 or 2 cases because its boolean.
    wtk::index_t const first_selector = *next_ephemeral;

    for(size_t i = 0; i < 2; i++)
    {
      if(switch_stmt->caseBlock(i)->match() == Number_T(0))
      {
        builder->notGate(*next_ephemeral, cond_wire);
      }
      else
      {
        builder->copy(*next_ephemeral, cond_wire);
      }
      *next_ephemeral += 1;
    }
    return first_selector;
  }
  else
  {
    // make all the case match wires
    wtk::index_t const first_case = *next_ephemeral;
    for(size_t j = 0; j < switch_stmt->size(); j++)
    {
      builder->assign((*next_ephemeral)++,
          (switch_stmt->caseBlock(j)->match() * (this->characteristic - 1))
          % this->characteristic);
    }

    // make each cases selector bit.
    wtk::index_t first_selector = *next_ephemeral;
    *next_ephemeral += switch_stmt->size();

    WireListBuilder<Number_T> sel_loop_outs(&this->pool);
    IterExprBuilder<Number_T> iters(&this->pool);
    IterExprWireListBuilder<Number_T> sel_loop_iter_outs(&this->pool);
    IterExprWireListBuilder<Number_T> sel_loop_iter_ins(&this->pool);

    sel_loop_outs.range(
        first_selector, first_selector + switch_stmt->size() - 1);
    sel_loop_iter_outs.single(iters.add(
          iters.literal(first_selector), iters.name("wtk::mux::i")));
    sel_loop_iter_ins.single(iters.add(
          iters.literal(first_case), iters.name("wtk::mux::i")));
    sel_loop_iter_ins.single(iters.literal(cond_wire));

    builder->forLoopInvoke(&sel_loop_outs, "wtk::mux::i",
        0, switch_stmt->size() - 1, &sel_loop_iter_outs,
        "wtk::mux::check_case", &sel_loop_iter_ins);

    return first_selector;
  }
}

template<typename Number_T>
void Multiplex<Number_T>::muxFinish(
    wtk::SwitchStatement<Number_T>* switch_stmt,
    wtk::index_t* const next_ephemeral,
    wtk::index_t const first_selector,
    wtk::index_t const first_output,
    wtk::index_t const num_outputs,
    wtk::index_t const* const outer_selector,
    TreeBuilder<Number_T>* const builder)
{
  if(this->isBoolean)
  {
    // Skip checking that 1 case was selected, because single case is
    // handled elsewhere, and 2-case is implied by case-exclusivity and by
    // field-coverage.

    log_assert(switch_stmt->size() == 2);

    // multiply each wire by its selector bit, and sum all the outputs
    // into the final output.
    {
      // with two cases, sum them directly into the last one.
      wtk::WireList* out_list = switch_stmt->outputList();
      wtk::index_t output_place = first_output;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          builder->andGate(*next_ephemeral, output_place, first_selector);
          builder->andGate(*next_ephemeral + 1,
              output_place + num_outputs, first_selector + 1);
          builder->xorGate(
              out_list->single(j), *next_ephemeral, *next_ephemeral + 1);
          *next_ephemeral += 2;
          output_place++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);

          WireListBuilder<Number_T> sub_out_list(&this->pool);
          sub_out_list.range(range);
          IterExprBuilder<Number_T> expr(&this->pool);
          IterExprWireListBuilder<Number_T> iter_out_list(&this->pool);
          iter_out_list.single(expr.add(
                expr.literal(range->first()), expr.name("wtk::mux::i")));
          IterExprWireListBuilder<Number_T> iter_in_list(&this->pool);
          iter_in_list.single(expr.add(
                expr.literal(output_place), expr.name("wtk::mux::i")));
          iter_in_list.single(expr.add(
                expr.literal(output_place + num_outputs),
                expr.name("wtk::mux::i")));
          iter_in_list.single(expr.literal(first_selector));
          iter_in_list.single(expr.literal(first_selector + 1));
          TreeBuilder<Number_T> sub_builder(&this->pool);
          sub_builder.andGate(5, 1, 3);
          sub_builder.andGate(6, 2, 4);
          sub_builder.xorGate(0, 5, 6);

          wtk::index_t const n_iters = range->last() - range->first();
          builder->forLoopAnonymous(&sub_out_list, "wtk::mux::i",
              0, n_iters, &iter_out_list, &iter_in_list, 0, 0, &sub_builder);
          output_place += n_iters + 1;
        }
        }
      }
    }
  }
  else
  {
    // sum the selectors and assert one
    {
      builder->addGate((*next_ephemeral)++, first_selector, first_selector + 1);
      // two cases only needsthe initial add
      if(switch_stmt->size() == 3)
      {
        // three cases need an additional add
        builder->addGate(
            *next_ephemeral, *next_ephemeral - 1, first_selector + 2);
        (*next_ephemeral)++;
      }
      else if(switch_stmt->size() > 3)
      {
        // each additional case needs another add, so just loop it.
        wtk::index_t const n_iters = switch_stmt->size() - 3;
        WireListBuilder<Number_T> out_list(&this->pool);
        out_list.range(*next_ephemeral, *next_ephemeral + n_iters);
        IterExprBuilder<Number_T> iter(&this->pool);
        IterExprWireListBuilder<Number_T> out_iter_list(&this->pool);
        out_iter_list.single(iter.add(
              iter.literal(*next_ephemeral), iter.name("wtk::mux::i")));
        IterExprWireListBuilder<Number_T> in_iter_list(&this->pool);
        in_iter_list.single(iter.add(
              iter.literal(*next_ephemeral - 1), iter.name("wtk::mux::i")));
        in_iter_list.single(iter.add(
              iter.literal(first_selector + 2), iter.name("wtk::mux::i")));
        TreeBuilder<Number_T> sub_builder(&this->pool);
        sub_builder.addGate(0, 1, 2);

        builder->forLoopAnonymous(&out_list, "wtk::mux::i", 0, n_iters,
            &out_iter_list, &in_iter_list, 0, 0, &sub_builder);
        *next_ephemeral += n_iters + 1;
      }

      builder->addcGate(
          *next_ephemeral, *next_ephemeral - 1, this->characteristic - 1);

      if(outer_selector != nullptr)
      {
        (*next_ephemeral)++;
        builder->mulGate(
            *next_ephemeral, *next_ephemeral - 1, *outer_selector);
      }

      builder->assertZero(*next_ephemeral);
      (*next_ephemeral)++;
    }

    // multiply each wire by its selector bit, and sum all the outputs
    // into the final output.
    if(switch_stmt->size() == 2)
    {
      // with two cases, sum them directly into the last one.
      wtk::WireList* out_list = switch_stmt->outputList();
      wtk::index_t output_place = first_output;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          builder->mulGate(*next_ephemeral, output_place, first_selector);
          builder->mulGate(*next_ephemeral + 1,
              output_place + num_outputs, first_selector + 1);
          builder->addGate(
              out_list->single(j), *next_ephemeral, *next_ephemeral + 1);
          *next_ephemeral += 2;
          output_place++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);

          WireListBuilder<Number_T> sub_out_list(&this->pool);
          sub_out_list.range(range);
          IterExprBuilder<Number_T> expr(&this->pool);
          IterExprWireListBuilder<Number_T> iter_out_list(&this->pool);
          iter_out_list.single(expr.add(
                expr.literal(range->first()), expr.name("wtk::mux::i")));
          IterExprWireListBuilder<Number_T> iter_in_list(&this->pool);
          iter_in_list.single(expr.add(
                expr.literal(output_place), expr.name("wtk::mux::i")));
          iter_in_list.single(expr.add(
                expr.literal(output_place + num_outputs),
                expr.name("wtk::mux::i")));
          iter_in_list.single(expr.literal(first_selector));
          iter_in_list.single(expr.literal(first_selector + 1));
          TreeBuilder<Number_T> sub_builder(&this->pool);
          sub_builder.mulGate(5, 1, 3);
          sub_builder.mulGate(6, 2, 4);
          sub_builder.addGate(0, 5, 6);

          wtk::index_t const n_iters = range->last() - range->first();
          builder->forLoopAnonymous(&sub_out_list, "wtk::mux::i",
              0, n_iters, &iter_out_list, &iter_in_list, 0, 0, &sub_builder);
          output_place += n_iters + 1;
        }
        }
      }
    }
    else
    {
      // add first two cases' outputs together.
      wtk::index_t prev_sum;
      {
        if(num_outputs == 1)
        {
          // if theres only one switch-output, then no loop needed
          builder->mulGate((*next_ephemeral)++, first_output, first_selector);
          builder->mulGate(
              (*next_ephemeral)++, first_output + 1, first_selector + 1);
          builder->addGate(
              *next_ephemeral, *next_ephemeral - 1, *next_ephemeral - 2);
          prev_sum = *next_ephemeral;
          (*next_ephemeral)++;
        }
        else if(num_outputs > 1)
        {
          // if more than one, then a loop is needed.
          WireListBuilder<Number_T> out_list(&this->pool);
          out_list.range(*next_ephemeral, *next_ephemeral + num_outputs - 1);
          IterExprBuilder<Number_T> iter(&this->pool);
          IterExprWireListBuilder<Number_T> out_iter_list(&this->pool);
          out_iter_list.single(iter.add(
                iter.literal(*next_ephemeral), iter.name("wtk::mux::i")));
          IterExprWireListBuilder<Number_T> in_iter_list(&this->pool);
          in_iter_list.single(iter.add(
                iter.literal(first_output), iter.name("wtk::mux::i")));
          in_iter_list.single(iter.add(
                iter.literal(first_output + num_outputs),
                iter.name("wtk::mux::i")));
          in_iter_list.single(iter.literal(first_selector));
          in_iter_list.single(iter.literal(first_selector + 1));
          TreeBuilder<Number_T> sub_builder(&this->pool);
          sub_builder.mulGate(5, 1, 3);
          sub_builder.mulGate(6, 2, 4);
          sub_builder.addGate(0, 5, 6);

          builder->forLoopAnonymous(&out_list,
              "wtk::mux::i", 0, num_outputs - 1,
              &out_iter_list, &in_iter_list, 0, 0, &sub_builder);
          prev_sum = *next_ephemeral;
          *next_ephemeral += num_outputs;
        }
        else
        {
          prev_sum = 0;
        }
      }

      // two cases was previously handled. three cases is handled as the first
      // and last cases. Four cases are handled here, and five or more will
      // need an additional loop.
      if(switch_stmt->size() == 4)
      {
        if(num_outputs == 1)
        {
          builder->mulGate(
              (*next_ephemeral)++, first_output + 2, first_selector + 2);
          builder->addGate(*next_ephemeral, prev_sum, *next_ephemeral - 1);
          prev_sum = *next_ephemeral;
          (*next_ephemeral)++;
        }
        else if(num_outputs > 1)
        {
          WireListBuilder<Number_T> out_list(&this->pool);
          out_list.range(*next_ephemeral, *next_ephemeral + num_outputs - 1);
          IterExprBuilder<Number_T> expr(&this->pool);
          IterExprWireListBuilder<Number_T> out_iter_list(&this->pool);
          out_iter_list.single(expr.add(
                expr.literal(*next_ephemeral), expr.name("wtk::mux::i")));
          IterExprWireListBuilder<Number_T> in_iter_list(&this->pool);
          in_iter_list.single(expr.add(
                expr.literal(first_output + 2 * num_outputs),
                expr.name("wtk::mux::i")));
          in_iter_list.single(expr.add(
                expr.literal(prev_sum), expr.name("wtk::mux::i")));
          in_iter_list.single(expr.literal(first_selector + 2));
          TreeBuilder<Number_T> sub_builder(&this->pool);
          sub_builder.mulGate(4, 1, 3);
          sub_builder.addGate(0, 4, 2);

          builder->forLoopAnonymous(&out_list, "wtk::mux::i", 0, num_outputs - 1,
              &out_iter_list, &in_iter_list, 0, 0, &sub_builder);

          prev_sum = *next_ephemeral;
          *next_ephemeral += num_outputs;
        }
      }
      else if(switch_stmt->size() > 4)
      {
        // requires a loop (or loop of loops) to handle more cases
        wtk::index_t const n_iters = switch_stmt->size() - 3;
        WireListBuilder<Number_T> outer_outs(&this->pool);
        outer_outs.range(
            *next_ephemeral, *next_ephemeral - 1 + num_outputs * n_iters);
        IterExprBuilder<Number_T> exprs(&this->pool);
        IterExprWireListBuilder<Number_T> outer_iter_outs(&this->pool);
        IterExprWireListBuilder<Number_T> outer_iter_ins(&this->pool);
        TreeBuilder<Number_T> outer_builder(&this->pool);

        if(num_outputs == 1)
        {
          outer_iter_outs.single(exprs.add(
                exprs.literal(*next_ephemeral), exprs.name("wtk::mux::i")));
          outer_iter_ins.single(exprs.add(
                exprs.literal(first_output + 2), exprs.name("wtk::mux::i")));
          outer_iter_ins.single(exprs.add(
                exprs.literal(prev_sum), exprs.name("wtk::mux::i")));
          outer_iter_ins.single(exprs.add(
                exprs.literal(first_selector + 2), exprs.name("wtk::mux::i")));
          outer_builder.mulGate(4, 1, 3);
          outer_builder.addGate(0, 4, 2);
        }
        else if(num_outputs > 1)
        {
          outer_iter_outs.range(exprs.add(exprs.literal(*next_ephemeral),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))),
              exprs.add(exprs.literal(*next_ephemeral + num_outputs - 1),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))));

          outer_iter_ins.range(exprs.add(
                exprs.literal(first_output + 2 * num_outputs),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))),
              exprs.add(exprs.literal(first_output + 3 * num_outputs - 1),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))));
          outer_iter_ins.range(exprs.add(exprs.literal(prev_sum),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))),
              exprs.add(exprs.literal(prev_sum + num_outputs - 1),
                exprs.mul(exprs.name("wtk::mux::i"),
                  exprs.literal(num_outputs))));
          outer_iter_ins.single(exprs.add(
                exprs.literal(first_selector + 2), exprs.name("wtk::mux::i")));

          WireListBuilder<Number_T> inner_outs(&this->pool);
          IterExprWireListBuilder<Number_T> inner_iter_outs(&this->pool);
          IterExprWireListBuilder<Number_T> inner_iter_ins(&this->pool);
          TreeBuilder<Number_T> inner_builder(&this->pool);
          inner_outs.range(0, num_outputs - 1);
          inner_iter_outs.single(exprs.name("wtk::mux::j"));
          inner_iter_ins.single(exprs.add(
                exprs.literal(num_outputs), exprs.name("wtk::mux::j")));
          inner_iter_ins.single(exprs.add(
                exprs.literal(2 * num_outputs), exprs.name("wtk::mux::j")));
          inner_iter_ins.single(exprs.literal(3 * num_outputs));
          inner_builder.mulGate(4, 1, 3);
          inner_builder.addGate(0, 4, 2);

          outer_builder.forLoopAnonymous(&inner_outs, "wtk::mux::j", 0,
              num_outputs - 1, &inner_iter_outs, & inner_iter_ins, 0, 0,
              &inner_builder);
        }

        builder->forLoopAnonymous(&outer_outs, "wtk::mux::i", 0, n_iters - 1,
            &outer_iter_outs, &outer_iter_ins, 0, 0, &outer_builder);
        prev_sum += n_iters * num_outputs;
        *next_ephemeral += n_iters * num_outputs;
      }

      // Sum the last case, while assigning to the original output wires.
      wtk::WireList* out_list = switch_stmt->outputList();
      wtk::index_t output_place =
        first_output + (switch_stmt->size() - 1) * num_outputs;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          builder->mulGate(*next_ephemeral,
              output_place, first_selector + switch_stmt->size() - 1);
          builder->addGate(
              out_list->single(j), *next_ephemeral, prev_sum);
          (*next_ephemeral)++;
          output_place++;
          prev_sum++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);

          WireListBuilder<Number_T> new_out_list(&this->pool);
          new_out_list.range(range);
          IterExprBuilder<Number_T> expr(&this->pool);
          IterExprWireListBuilder<Number_T> iter_out_list(&this->pool);
          iter_out_list.single(expr.add(
                expr.literal(range->first()), expr.name("wtk::mux::i")));
          IterExprWireListBuilder<Number_T> iter_in_list(&this->pool);
          iter_in_list.single(expr.add(
                expr.literal(output_place), expr.name("wtk::mux::i")));
          iter_in_list.single(expr.add(
                expr.literal(prev_sum), expr.name("wtk::mux::i")));
          iter_in_list.single(
              expr.literal(first_selector + switch_stmt->size() - 1));
          TreeBuilder<Number_T> sub_builder(&this->pool);
          sub_builder.mulGate(4, 1, 3);
          sub_builder.addGate(0, 4, 2);

          wtk::index_t const n_iters = range->last() - range->first();
          builder->forLoopAnonymous(&new_out_list, "wtk::mux::i",
              0, n_iters, &iter_out_list, &iter_in_list, 0, 0, &sub_builder);
          output_place += n_iters + 1;
          prev_sum += n_iters + 1;
        }
        }
      }
    }
  }
}

// This checks that evaluations of the expression do not "straddle" the
// threshold. It is for determining if a loop can be muxified without issue.
//
// This uses the heuristic of the first and last values being less and
// greater than the threshold. This heuristich should hold with the following
// restrictions, in which case it falls back to a brute-force approach.
//  - the expression is of "array index" form (formally a linear dependence
//    with each iterator), implying it should be monotone with respect to
//    each of the iterators
//  - no overflow in evaluation of first and last. Inductively, if one of the
//    middle iterations were to overflow, then the first or last would also
//    overflow, and if a middle iteration overflowed then the expression might
//    not be increasing or decreasing.
template<typename Number_T>
IterThreshold Multiplex<Number_T>::iterExprStraddlesThreshold(
    wtk::IterExpr* expr,
    std::vector<IterBounds>& bounds,
    wtk::index_t threshold)
{

  std::unordered_map<std::string, wtk::index_t> table;
  wtk::index_t max = 0;
  wtk::index_t min = UINT64_MAX;
  if(wtk::utils::iterExprLinear(expr))
  {
    if(!this->fastIterExprStraddlesThresholdHelper(
        expr, bounds, table, 0, &max, &min))
    {
      this->iterExprStraddlesThresholdHelper(
          expr, bounds, table, 0, &max, &min);
    }
  }
  else
  {
    this->iterExprStraddlesThresholdHelper(
        expr, bounds, table, 0, &max, &min);
  }

  log_assert(max >= min);


  if(max < threshold && min < threshold)
  {
    return IterThreshold::lessthan;
  }
  else if(max >= threshold && min >= threshold)
  {
    return IterThreshold::greaterthan;
  }
  else
  {
    return IterThreshold::straddle;
  }
}

template<typename Number_T>
void Multiplex<Number_T>::iterExprStraddlesThresholdHelper(
    wtk::IterExpr* expr,
    std::vector<IterBounds>& bounds,
    std::unordered_map<std::string, wtk::index_t>& table,
    size_t bounds_place,
    wtk::index_t* max, wtk::index_t* min)
{
  if(bounds_place == bounds.size())
  {
    wtk::index_t val = wtk::utils::iterExprEval(expr, table);
    if(val > *max) { *max = val; }
    if(val < *min) { *min = val; }
  }
  else
  {
    for(wtk::index_t i = bounds[bounds_place].first;
        i <= bounds[bounds_place].last; i++)
    {
      table[bounds[bounds_place].iter] = i;
      this->iterExprStraddlesThresholdHelper(
          expr, bounds, table, bounds_place + 1, max, min);
    }
  }
}

template<typename Number_T>
bool Multiplex<Number_T>::fastIterExprStraddlesThresholdHelper(
    wtk::IterExpr* expr,
    std::vector<IterBounds>& bounds,
    std::unordered_map<std::string, wtk::index_t>& table,
    size_t bounds_place,
    wtk::index_t* max, wtk::index_t* min)
{
  if(bounds_place == bounds.size())
  {
    wtk::index_t val;
    bool no_overflow = !wtk::utils::iterExprEvalOverflow(&val, expr, table);
    if(val > *max) { *max = val; }
    if(val < *min) { *min = val; }

    return no_overflow;
  }
  else
  {
    table[bounds[bounds_place].iter] = bounds[bounds_place].first;
    bool no_overflow = this->fastIterExprStraddlesThresholdHelper(
        expr, bounds, table, bounds_place + 1, max, min);
    table[bounds[bounds_place].iter] = bounds[bounds_place].last;
    no_overflow = this->fastIterExprStraddlesThresholdHelper(
        expr, bounds, table, bounds_place + 1, max, min) && no_overflow;

    return no_overflow;
  }
}

template<typename Number_T>
IterThreshold Multiplex<Number_T>::iterExprRangeStraddlesThreshold(
    wtk::IterExpr* first, wtk::IterExpr* last,
    std::vector<IterBounds>& bounds,
    wtk::index_t threshold)
{

  std::unordered_map<std::string, wtk::index_t> table;
  wtk::index_t max = 0;
  wtk::index_t min = UINT64_MAX;
  this->iterExprRangeStraddlesThresholdHelper(
      first, last, bounds, table, 0, &max, &min);
  if(wtk::utils::iterExprLinear(first) && wtk::utils::iterExprLinear(last))
  {
    if(!this->fastIterExprRangeStraddlesThresholdHelper(
        first, last, bounds, table, 0, &max, &min))
    {
      this->iterExprRangeStraddlesThresholdHelper(
          first, last, bounds, table, 0, &max, &min);
    }
  }
  else
  {
    this->iterExprRangeStraddlesThresholdHelper(
        first, last, bounds, table, 0, &max, &min);
  }

  log_assert(max >= min);


  if(max < threshold && min < threshold)
  {
    return IterThreshold::lessthan;
  }
  else if(max >= threshold && min >= threshold)
  {
    return IterThreshold::greaterthan;
  }
  else
  {
    return IterThreshold::straddle;
  }
}

template<typename Number_T>
void Multiplex<Number_T>::iterExprRangeStraddlesThresholdHelper(
    wtk::IterExpr* first, wtk::IterExpr* last,
    std::vector<IterBounds>& bounds,
    std::unordered_map<std::string, wtk::index_t>& table,
    size_t bounds_place,
    wtk::index_t* max, wtk::index_t* min)
{
  if(bounds_place == bounds.size())
  {
    wtk::index_t first_val = wtk::utils::iterExprEval(first, table);
    wtk::index_t last_val = wtk::utils::iterExprEval(last, table);

    log_assert(first_val <= last_val);

    if(last_val > *max) { *max = last_val; }
    if(first_val < *min) { *min = first_val; }
  }
  else
  {
    for(wtk::index_t i = bounds[bounds_place].first;
        i <= bounds[bounds_place].last; i++)
    {
      table[bounds[bounds_place].iter] = i;
      this->iterExprRangeStraddlesThresholdHelper(
          first, last, bounds, table, bounds_place + 1, max, min);
    }
  }
}

template<typename Number_T>
bool Multiplex<Number_T>::fastIterExprRangeStraddlesThresholdHelper(
    wtk::IterExpr* first, wtk::IterExpr* last,
    std::vector<IterBounds>& bounds,
    std::unordered_map<std::string, wtk::index_t>& table,
    size_t bounds_place,
    wtk::index_t* max, wtk::index_t* min)
{
  if(bounds_place == bounds.size())
  {
    wtk::index_t first_val;
    bool no_overflow =
      !wtk::utils::iterExprEvalOverflow(&first_val, first, table);
    wtk::index_t last_val;
    no_overflow =
      !wtk::utils::iterExprEvalOverflow(&last_val, last, table) && no_overflow;

    log_assert(first_val <= last_val);

    if(last_val > *max) { *max = last_val; }
    if(first_val < *min) { *min = first_val; }

    return no_overflow;
  }
  else
  {
    table[bounds[bounds_place].iter] = bounds[bounds_place].first;
    bool no_overflow = this->fastIterExprRangeStraddlesThresholdHelper(
        first, last, bounds, table, bounds_place + 1, max, min);
    table[bounds[bounds_place].iter] = bounds[bounds_place].last;
    no_overflow = this->fastIterExprRangeStraddlesThresholdHelper(
        first, last, bounds, table, bounds_place + 1, max, min) && no_overflow;

    return no_overflow;
  }
}

template<typename Number_T>
wtk::index_t Multiplex<Number_T>::iterExprRangeSize(
    wtk::IterExpr* first, wtk::IterExpr* last,
    std::vector<IterBounds>& bounds)
{
  // On the assumption that every iteration has the same range size, this
  // computes the size using only the first bound. There are cases where
  // the first and last range size could differ. but thats only useful if
  // the total list size is the same on each iteration. Its possible to
  // construct something where the total list size differs but that isn't
  // really useful, because the body is more or less fixed size.

  std::unordered_map<std::string, wtk::index_t> table;

  for(IterBounds& bound : bounds)
  {
    table[bound.iter] = bound.first;
  }

  wtk::index_t first_val = wtk::utils::iterExprEval(first, table);
  wtk::index_t last_val = wtk::utils::iterExprEval(last, table);
  log_assert(first_val <= last_val);

  return last_val - first_val + 1;
}

} } // namespace wtk::converters
