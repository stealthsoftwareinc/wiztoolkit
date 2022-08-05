/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
bool Evaluator<Wire_T, Number_T>::evaluate(
    Bolt<Wire_T, Number_T>* const bolt,
    wtk::InputStream<Number_T>* const instance,
    wtk::InputStream<Number_T>* const witness)
{
  bool success =
    this->evaluate(bolt, instance, nullptr, witness, nullptr, nullptr);

  Number_T dummy;
  if(witness != nullptr && witness->next(&dummy) != StreamStatus::end)
  {
    log_error("leftover witness values");
    success = false;
  }
  if(instance->next(&dummy) != StreamStatus::end)
  {
    log_error("leftover instance values");
    success = false;
  }

  return success;
}

template<typename Wire_T, typename Number_T>
bool Evaluator<Wire_T, Number_T>::evaluate(
    Bolt<Wire_T, Number_T>* const bolt,
    wtk::InputStream<Number_T>* const instance,
    SwitchStreamHandler<Wire_T>* const switch_instance,
    wtk::InputStream<Number_T>* const witness,
    SwitchStreamHandler<Wire_T>* const switch_witness,
    Wire_T const* const enabled_bit)
{
  for(size_t i = 0; i < bolt->directives.size(); i++)
  {
    switch(bolt->types[i])
    {
    case DirectiveType::ADD_GATE:
    {
      AddGate<Wire_T, Number_T>* gate = bolt->directives[i].addGate;
      this->backend->addGate(
          gate->out.deref(), gate->left.deref(), gate->right.deref());

      break;
    }
    case DirectiveType::MUL_GATE:
    {
      MulGate<Wire_T, Number_T>* gate = bolt->directives[i].mulGate;
      this->backend->mulGate(
          gate->out.deref(), gate->left.deref(), gate->right.deref());

      break;
    }
    case DirectiveType::ADDC_GATE:
    {
      AddCGate<Wire_T, Number_T>* gate = bolt->directives[i].addCGate;
      this->backend->addcGate(
          gate->out.deref(), gate->left.deref(), gate->right);

      break;
    }
    case DirectiveType::MULC_GATE:
    {
      MulCGate<Wire_T, Number_T>* gate = bolt->directives[i].mulCGate;
      this->backend->mulcGate(
          gate->out.deref(), gate->left.deref(), gate->right);

      break;
    }
    case DirectiveType::XOR_GATE:
    {
      XorGate<Wire_T, Number_T>* gate = bolt->directives[i].xorGate;
      this->backend->xorGate(
          gate->out.deref(), gate->left.deref(), gate->right.deref());

      break;
    }
    case DirectiveType::AND_GATE:
    {
      AndGate<Wire_T, Number_T>* gate = bolt->directives[i].andGate;
      this->backend->andGate(
          gate->out.deref(), gate->left.deref(), gate->right.deref());

      break;
    }
    case DirectiveType::NOT_GATE:
    {
      NotGate<Wire_T, Number_T>* gate = bolt->directives[i].notGate;
      this->backend->notGate(gate->out.deref(), gate->left.deref());

      break;
    }
    case DirectiveType::COPY:
    {
      Copy<Wire_T, Number_T>* copy = bolt->directives[i].copy;
      this->backend->copy(copy->out.deref(), copy->left.deref());

      break;
    }
    case DirectiveType::ASSIGN:
    {
      Assign<Wire_T, Number_T>* assign = bolt->directives[i].assign;
      this->backend->assign(assign->out.deref(), assign->left);

      break;
    }
    case DirectiveType::INSTANCE:
    {
      Instance<Wire_T, Number_T>* ins_dir = bolt->directives[i].instance;

      if(switch_instance != nullptr)
      {
        this->backend->copy(ins_dir->out.deref(), switch_instance->next());
      }
      else
      {
        Number_T val;
        if(UNLIKELY(
              UNLIKELY(instance->next(&val) != wtk::StreamStatus::success)
              || UNLIKELY(val >= this->backend->prime)))
        {
          log_error("instance:%zu: malformed instance value",
              instance->lineNum());
          return false;
        }

        this->backend->instance(ins_dir->out.deref(), val);
      }

      break;
    }
    case DirectiveType::WITNESS:
    {
      Witness<Wire_T, Number_T>* wit_dir = bolt->directives[i].witness;

      if(switch_witness != nullptr)
      {
        this->backend->copy(wit_dir->out.deref(), switch_witness->next());
      }
      else if(witness != nullptr)
      {
        Number_T val;
        if(UNLIKELY(
              UNLIKELY(witness->next(&val) != wtk::StreamStatus::success)
              || UNLIKELY(val >= this->backend->prime)))
        {
          log_error("witness:%zu: malformed witness value",
              witness->lineNum());
          return false;
        }

        this->backend->witness(wit_dir->out.deref(), val);
      }
      else
      {
        Number_T zero(0);
        this->backend->witness(wit_dir->out.deref(), zero);
      }

      break;
    }
    case DirectiveType::ASSERT_ZERO:
    {
      AssertZero<Wire_T, Number_T>* assert_zero =
        bolt->directives[i].assertZero;
      Wire_T* assert_wire_ptr = assert_zero->left.deref();
      Wire_T assert_wire;

      if(enabled_bit != nullptr)
      {
        if(this->backend->isBoolean)
        {
          this->backend->andGate(
              &assert_wire, assert_wire_ptr, enabled_bit);
        }
        else
        {
          this->backend->mulGate(
              &assert_wire, assert_wire_ptr, enabled_bit);
        }
        assert_wire_ptr = &assert_wire;
      }
      this->backend->assertZero(assert_wire_ptr);

      break;
    }
    case DirectiveType::INVOCATION:
    {
      Invocation<Wire_T, Number_T>* invocation =
        bolt->directives[i].invocation;

      invocation->function->remap.wires = &invocation->remap;

      if(UNLIKELY(!this->evaluate(&invocation->function->body,
          instance, switch_instance, witness, switch_witness,
          enabled_bit)))
      {
        return false;
      }
      break;
    }
    case DirectiveType::ANON_FUNCTION:
    {
      if(UNLIKELY(!this->evaluate(&bolt->directives[i].anonFunction->body,
              instance, switch_instance, witness, switch_witness, enabled_bit)))
      {
        return false;
      }
      break;
    }
    case DirectiveType::SWITCH_STATEMENT:
    {
      SwitchStatement<Wire_T, Number_T>* const switch_stmt =
        bolt->directives[i].switchStatement;

      // reserve all the instance values
      std::vector<Wire_T> instance_buffer;
      if(switch_instance != nullptr)
      {
        switch_stmt->instanceCache.values =
          switch_instance->next(switch_stmt->instanceCache.total);
      }
      else
      {
        instance_buffer.resize(switch_stmt->instanceCache.total);

        for(size_t j = 0; j < switch_stmt->instanceCache.total; j++)
        {
          Number_T num;
          if(UNLIKELY(
                UNLIKELY(wtk::StreamStatus::success != instance->next(&num))
                || UNLIKELY(num >= this->backend->prime)))
          {
            log_error("ran out of instance values, expected %zu, consumed %zu",
                switch_stmt->instanceCache.total, j);
            return false;
          }
          else
          {
            this->backend->instance(&instance_buffer[j], num);
          }
        }

        switch_stmt->instanceCache.values = instance_buffer.data();
      }

      // reserve all the witness values
      std::vector<Wire_T> witness_buffer;
      if(switch_witness != nullptr)
      {
        switch_stmt->witnessCache.values =
          switch_witness->next(switch_stmt->witnessCache.total);
      }
      else
      {
        witness_buffer.resize(switch_stmt->witnessCache.total);

        if(witness != nullptr)
        {
          for(size_t j = 0; j < switch_stmt->witnessCache.total; j++)
          {
            Number_T num;
            if(UNLIKELY(
                  UNLIKELY(wtk::StreamStatus::success != witness->next(&num))
                  || UNLIKELY(num >= this->backend->prime)))
            {
              log_error("ran out of witness values, expected %zu, consumed %zu",
                  switch_stmt->witnessCache.total, j);
              return false;
            }
            else
            {
              this->backend->witness(&witness_buffer[j], num);
            }
          }
        }
        else
        {
          Number_T zero(0);
          for(size_t j = 0; j < switch_stmt->witnessCache.total; j++)
          {
            this->backend->witness(&witness_buffer[j], zero);
          }
        }

        switch_stmt->witnessCache.values = witness_buffer.data();
      }

      // run all the cases
      Wire_T* switch_selector_wire = switch_stmt->selector.deref();
      for(size_t j = 0; j < switch_stmt->caseBlocks.size(); j++)
      {
        this->backend->caseSelect(
            &switch_stmt->selectorBits[j],
            switch_stmt->caseSelectors[j],
            switch_selector_wire);

        Wire_T* sub_selector_ptr = &switch_stmt->selectorBits[j];
        Wire_T sub_selector_tmp;
        if(enabled_bit != nullptr)
        {
          if(this->backend->isBoolean)
          {
            this->backend->andGate(
                &sub_selector_tmp, sub_selector_ptr, enabled_bit);
          }
          else
          {
            this->backend->mulGate(
                &sub_selector_tmp, sub_selector_ptr, enabled_bit);
          }

          sub_selector_ptr = &sub_selector_tmp;
        }

        switch_stmt->instanceCache.reset();
        switch_stmt->witnessCache.reset();

        switch(switch_stmt->caseTypes[j])
        {
        case SwitchStatement<Wire_T, Number_T>::INVOCATION:
        {
          Invocation<Wire_T, Number_T>* invocation =
            switch_stmt->caseBlocks[j].invocation;

          invocation->function->remap.wires = &invocation->remap;

          if(UNLIKELY(!this->evaluate(&invocation->function->body,
              instance, &switch_stmt->instanceCache,
              witness, &switch_stmt->witnessCache,
              sub_selector_ptr)))
          {
            return false;
          }

          break;
        }
        case SwitchStatement<Wire_T, Number_T>::ANON_FUNCTION:
        {
          AnonFunction<Wire_T, Number_T>* anon_function =
            switch_stmt->caseBlocks[j].anonFunction;

          if(UNLIKELY(!this->evaluate(&anon_function->body,
              instance, &switch_stmt->instanceCache,
              witness, &switch_stmt->witnessCache,
              sub_selector_ptr)))
          {
            return false;
          }
          break;
        }
        }
      }

      // add up all the dummy output values into the actual outputs
      size_t dummy_place = 0;
      for(size_t j = 0; j < switch_stmt->outputs.ranges.size(); j++)
      {
        WireRange<Wire_T, Number_T>* const range =
          switch_stmt->outputs.ranges[j];
        for(wtk::index_t k = range->first; k <= range->last; k++)
        {
          this->backend->multiplexHelper(range->deref(k),
              &switch_stmt->dummyOutputs, &switch_stmt->selectorBits,
              dummy_place);
          dummy_place++;
        }
      }

      // check that exactly 1 case was matched
      this->backend->checkSelectorBits(
          &switch_stmt->selectorBits, enabled_bit);

      break;
    }
    case DirectiveType::FOR_LOOP:
    {
      ForLoop<Wire_T, Number_T>* loop = bolt->directives[i].forLoop;

      for(wtk::index_t i = loop->first; i <= loop->last; i++)
      {
        loop->current = i;

        for(size_t i = 0; i < loop->body.wires.ranges.size(); i++)
        {
          loop->body.wires.ranges[i]->update(this->exprStack);
        }

        if(UNLIKELY(!this->evaluate(&loop->body, instance, switch_instance,
            witness, switch_witness, enabled_bit)))
        {
          return false;
        }
      }

      break;
    }
    case DirectiveType::HARD_UNROLL_FOR_LOOP:
    {
      HardUnrollForLoop<Wire_T, Number_T>* loop =
        bolt->directives[i].hardUnrollForLoop;

      PLASMASnooze<Wire_T, Number_T> snooze(this->backend);
      snooze.functions = *loop->functions;

      for(wtk::index_t i = loop->first; i <= loop->last; i++)
      {
        loop->current = i;

        PLASMASnoozeState<Wire_T, Number_T> state(
            instance, switch_instance, witness, switch_witness,
            &loop->exprBuilder, enabled_bit);

        wtk::index_t place = 0;
        for(size_t j = 0; j < loop->outputList.size(); j++)
        {
          wtk::index_t first = loop->outputList[j].first.eval(this->exprStack);
          wtk::index_t last = loop->outputList[j].second.eval(this->exprStack);

          bolt->findRanges(
              first, last, &state.wires, &place, &state.wires.pool);
        }
        wtk::index_t num_out = place;

        for(size_t j = 0; j < loop->inputList.size(); j++)
        {
          wtk::index_t first = loop->inputList[j].first.eval(this->exprStack);
          wtk::index_t last = loop->inputList[j].second.eval(this->exprStack);

          bolt->findRanges(
              first, last, &state.wires, &place, &state.wires.pool);
        }
        wtk::index_t num_in = place - num_out;

        state.wires.outputSize = num_out;
        state.wires.inputSize = num_in;

        if(num_in > 0)
        {
          state.assigned.insert(num_out, num_in - 1);
        }

        PLASMASnoozeStatus status = snooze.evaluate(loop->body, &state);

        // Safe to say because build essentially ran all the checks.
        log_assert(status != PLASMASnoozeStatus::bad_relation);

        if(status != PLASMASnoozeStatus::well_formed)
        {
          return false;
        }
      }

      break;
    }
    default:
    {
      log_error("IR1 under development");
      return false;
    }
    }
  }

  return true;
}

} } // namespace wtk::bolt
