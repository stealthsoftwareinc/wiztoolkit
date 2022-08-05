/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
Bolt<Wire_T, Number_T>* Builder<Wire_T, Number_T>::build(
    wtk::IRTree<Number_T>* const tree)
{
  for(size_t i = 0; i < tree->size(); i++)
  {
    wtk::FunctionDeclare<Number_T>* const decl = tree->functionDeclare(i);
    std::string function_name(decl->name());
    if(this->functionMap.find(function_name)
        != this->functionMap.end())
    {
      log_error("duplicate function name \"%s\"", function_name.c_str());
    }

    Function<Wire_T, Number_T>* func = this->pool.function.allocate();
    func->numOutputs = decl->outputCount();
    func->numInputs = decl->inputCount();
    func->numInstance = decl->instanceCount();
    func->numWitness = decl->shortWitnessCount();
    func->remap.setRange(0, func->numOutputs + func->numInputs - 1);
    func->body.wires.ranges.push_back(&func->remap);

    IterBounds func_bounds;
    BoltBuildCtx func_ctx;
    func_ctx.numOutputs = func->numOutputs;
    func_ctx.numInputs = func->numInputs;
    func_ctx.bolt = &func->body;
    func_ctx.iterBounds = &func_bounds;

    if(!this->build(decl->body(), &func_ctx)) { return nullptr; }

    if(func_ctx.numInstance != decl->instanceCount()
        || func_ctx.numWitness != decl->shortWitnessCount())
    {
      log_error("Instance/short-witness counts incorrecto for function \"%s\"",
          function_name.c_str());
      return nullptr;
    }

    this->functionMap[function_name] = func;
    this->functionDeclareMap[function_name] = decl;
  }

  // Handle the "main" scope
  IterBounds main_bounds;
  BoltBuildCtx main_ctx;
  main_ctx.numOutputs = 0;
  main_ctx.numInputs = 0;
  main_ctx.bolt = &this->mainBolt;
  main_ctx.iterBounds = &main_bounds;

  if(this->build(tree->body(), &main_ctx)) { return &this->mainBolt; }
  else { return nullptr; }
}

template<typename Wire_T, typename Number_T>
bool Builder<Wire_T, Number_T>::build(
    wtk::DirectiveList<Number_T>* const dir_list, BoltBuildCtx* const ctx)
{
  // Start condition is that input wires are assigned.
  if(ctx->numInputs != 0)
  {
    ctx->all.insert(ctx->numOutputs, ctx->numOutputs + ctx->numInputs - 1);
    ctx->assigned.insert(ctx->numOutputs, ctx->numOutputs + ctx->numInputs - 1);
  }

  if(!wtk::utils::listAssignedWires(dir_list, &ctx->all))
  {
    return false;
  }

  ctx->all.forEach([this, &ctx](wtk::index_t l, wtk::index_t r) -> void
      {
        if(l >= ctx->numOutputs + ctx->numInputs)
        {
          ctx->bolt->wires.ranges.emplace_back(
              this->pool.wireRangePool.localWireRange.allocate(1, l, r));
        }
        else if(r >= ctx->numOutputs + ctx->numInputs)
        {
          ctx->bolt->wires.ranges.emplace_back(
              this->pool.wireRangePool.localWireRange.allocate(
                1, ctx->numOutputs + ctx->numInputs, r));
        }
      });

  for(size_t i = 0; i < dir_list->size(); i++)
  {
    switch(dir_list->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      wtk::BinaryGate* gate = dir_list->binaryGate(i);

      wtk::index_t const out = gate->outputWire();
      wtk::index_t const left = gate->leftWire();
      wtk::index_t const right = gate->rightWire();

      if(!ctx->assigned.has(left) || ctx->deleted.has(left))
      {
        log_error("left input wire $%" PRIu64 " is not usable", left);
        return false;
      }

      if(!ctx->assigned.has(right) || ctx->deleted.has(right))
      {
        log_error("right input wire $%" PRIu64 " is not usable", right);
        return false;
      }

      if(!ctx->assigned.insert(out))
      {
        log_error("cannot assign output wire $%" PRIu64, out);
        return false;
      }

      switch(gate->calculation())
      {
      case wtk::BinaryGate::ADD:
      {
        AddGate<Wire_T, Number_T>* g = this->pool.addGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->findWire(right, &g->right);
        ctx->bolt->types.emplace_back(DirectiveType::ADD_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().addGate = g;
        break;
      }
      case wtk::BinaryGate::MUL:
      {
        MulGate<Wire_T, Number_T>* g = this->pool.mulGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->findWire(right, &g->right);
        ctx->bolt->types.emplace_back(DirectiveType::MUL_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().mulGate = g;
        break;
      }
      case wtk::BinaryGate::XOR:
      {
        XorGate<Wire_T, Number_T>* g = this->pool.xorGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->findWire(right, &g->right);
        ctx->bolt->types.emplace_back(DirectiveType::XOR_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().xorGate = g;
        break;
      }
      case wtk::BinaryGate::AND:
      {
        AndGate<Wire_T, Number_T>* g = this->pool.andGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->findWire(right, &g->right);
        ctx->bolt->types.emplace_back(DirectiveType::AND_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().andGate = g;
        break;
      }
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      wtk::BinaryConstGate<Number_T>* gate = dir_list->binaryConstGate(i);

      wtk::index_t const out = gate->outputWire();
      wtk::index_t const left = gate->leftWire();
      Number_T const right = gate->rightValue();

      if(!ctx->assigned.has(left) || ctx->deleted.has(left))
      {
        log_error("left input wire $%" PRIu64 " is not usable", left);
        return false;
      }

      if(right >= this->characteristic)
      {
        log_error("right input value <%s> exceeds characteristic <%s>",
            wtk::utils::dec(right).c_str(),
            wtk::utils::dec(this->characteristic).c_str());
        return false;
      }

      if(!ctx->assigned.insert(out))
      {
        log_error("cannot assign output wire $%" PRIu64, out);
        return false;
      }

      switch(gate->calculation())
      {
      case wtk::BinaryConstGate<Number_T>::ADDC:
      {
        AddCGate<Wire_T, Number_T>* g = this->pool.addCGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        g->right = right;
        ctx->bolt->types.emplace_back(DirectiveType::ADDC_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().addCGate = g;
        break;
      }
      case wtk::BinaryConstGate<Number_T>::MULC:
      {
        MulCGate<Wire_T, Number_T>* g = this->pool.mulCGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        g->right = right;
        ctx->bolt->types.emplace_back(DirectiveType::MULC_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().mulCGate = g;
        break;
      }
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      wtk::UnaryGate* gate = dir_list->unaryGate(i);

      wtk::index_t const out = gate->outputWire();
      wtk::index_t const left = gate->inputWire();

      if(!ctx->assigned.has(left) || ctx->deleted.has(left))
      {
        log_error("left input wire $%" PRIu64 " is not usable", left);
        return false;
      }

      if(!ctx->assigned.insert(out))
      {
        log_error("cannot assign output wire $%" PRIu64, out);
        return false;
      }

      switch(gate->calculation())
      {
      case wtk::UnaryGate::NOT:
      {
        NotGate<Wire_T, Number_T>* g = this->pool.notGate.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->types.emplace_back(DirectiveType::NOT_GATE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().notGate = g;
        break;
      }
      case wtk::UnaryGate::COPY:
      {
        Copy<Wire_T, Number_T>* g = this->pool.copy.allocate();
        ctx->bolt->findWire(out, &g->out);
        ctx->bolt->findWire(left, &g->left);
        ctx->bolt->types.emplace_back(DirectiveType::COPY);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().copy = g;
        break;
      }
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      wtk::Assign<Number_T>* assign = dir_list->assign(i);

      wtk::index_t const out = assign->outputWire();
      Number_T const left = assign->constValue();

      if(left >= this->characteristic)
      {
        log_error("input value <%s> exceeds characteristic <%s>",
            wtk::utils::dec(left).c_str(),
            wtk::utils::dec(this->characteristic).c_str());
        return false;
      }

      if(!ctx->assigned.insert(out))
      {
        log_error("cannot assign output wire $%" PRIu64, out);
        return false;
      }

      Assign<Wire_T, Number_T>* a = this->pool.assign.allocate();
      ctx->bolt->findWire(out, &a->out);
      a->left = left;
      ctx->bolt->types.emplace_back(DirectiveType::ASSIGN);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().assign = a;
      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      wtk::Input* input = dir_list->input(i);

      wtk::index_t const out = input->outputWire();

      if(!ctx->assigned.insert(out))
      {
        log_error("cannot assign output wire $%" PRIu64, out);
        return false;
      }

      switch(input->stream())
      {
      case wtk::Input::INSTANCE:
      {
        ctx->numInstance += 1;

        Instance<Wire_T, Number_T>* i = this->pool.instance.allocate();
        ctx->bolt->findWire(out, &i->out);
        ctx->bolt->types.emplace_back(DirectiveType::INSTANCE);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().instance = i;
        break;
      }
      case wtk::Input::SHORT_WITNESS:
      {
        ctx->numWitness += 1;

        Witness<Wire_T, Number_T>* w = this->pool.witness.allocate();
        ctx->bolt->findWire(out, &w->out);
        ctx->bolt->types.emplace_back(DirectiveType::WITNESS);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().witness = w;
        break;
      }
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO:
    {
      wtk::Terminal* assert_zero = dir_list->assertZero(i);

      wtk::index_t left = assert_zero->wire();

      if(!ctx->assigned.has(left) || ctx->deleted.has(left))
      {
        log_error("left input wire $%" PRIu64 " is not usable", left);
        return false;
      }

      AssertZero<Wire_T, Number_T>* a = this->pool.assertZero.allocate();
      ctx->bolt->findWire(left, &a->left);
      ctx->bolt->types.emplace_back(DirectiveType::ASSERT_ZERO);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().assertZero = a;
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE:
    {
      wtk::Terminal* del_single = dir_list->deleteSingle(i);
      wtk::index_t wire = del_single->wire();

      if(wire >= ctx->numOutputs + ctx->numInputs)
      {
        if(!ctx->assigned.has(wire))
        {
          log_error("Wire $%" PRIu64 " is not yet assigned.", wire);
          return false;
        }

        if(!ctx->deleted.insert(wire))
        {
          log_error("Wire $%" PRIu64 " was previously deleted.", wire);
          return false;
        }

        // Other than well-formedness checking, we're going to ignore the
        // delete hints for now.
      }
      else
      {
        log_error("Cannot delete non-local wire $%" PRIu64 ".", wire);
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE:
    {
      wtk::WireRange* del_range = dir_list->deleteRange(i);
      wtk::index_t first = del_range->first();
      wtk::index_t last = del_range->last();

      if(first > last)
      {
        log_error(
            "Invalid wire range $%" PRIu64 " ... $%" PRIu64 ".", first, last);
        return false;
      }

      if(first >= ctx->numOutputs + ctx->numInputs)
      {
        if(!ctx->assigned.hasAll(first, last))
        {
          log_error(
              "Wires $%" PRIu64 " ... $%" PRIu64 " are not yet assigned.",
              first, last);
          return false;
        }

        if(!ctx->deleted.insert(first, last))
        {
          log_error(
              "Wires $%" PRIu64 " ... $%" PRIu64 " were previously deleted.",
              first, last);
          return false;
        }

        // Other than well-formedness checking, we're going to ignore the
        // delete hints for now.
      }
      else
      {
        log_error(
            "Cannot delete non-local Wires $%" PRIu64 " ... $%" PRIu64,
            first, last);
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      wtk::FunctionInvoke* invoke = dir_list->functionInvoke(i);
      std::string func_name(invoke->name());

      Function<Wire_T, Number_T>* function;
      {
        auto finder = this->functionMap.find(func_name);
        if(finder == this->functionMap.end())
        {
          log_error("No such function \"%s\"", func_name.c_str());
          return false;
        }
        else
        {
          function = finder->second;
        }
      }

      // Check that the inputs are usable
      wtk::WireList* in_list = invoke->inputList();
      for(size_t j = 0; j < in_list->size(); j++)
      {
        switch(in_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          if(!ctx->assigned.has(in_list->single(j))
              || ctx->deleted.has(in_list->single(j)))
          {
            log_error("input wire $%" PRIu64 " is not useable",
                in_list->single(j));
            return false;
          }

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = in_list->range(j);
          if(range->first() > range->last())
          {
            log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                range->first(), range->last());
            return false;
          }

          if(!ctx->assigned.hasAll(range->first(), range->last())
              || ctx->deleted.has(range->first(), range->last()))
          {
            log_error("input wires $%" PRIu64 " ... $%" PRIu64
                " are not useable", range->first(), range->last());
            return false;
          }

          break;
        }
        }
      }

      Invocation<Wire_T, Number_T>* invocation =
        this->pool.invocation.allocate();

      wtk::WireList* out_list = invoke->outputList();
      wtk::index_t place = 0;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          wtk::index_t single = out_list->single(j);
          if(!ctx->assigned.insert(single))
          {
            log_error("output wire $%" PRIu64 " was already assigned", single);
            return false;
          }

          ctx->bolt->findRanges(
              single, single, &invocation->remap, &place,
              &this->pool.wireRangePool);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);
          if(range->first() > range->last())
          {
            log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                range->first(), range->last());
            return false;
          }

          if(!ctx->assigned.insert(range->first(), range->last()))
          {
            log_error("output wires $%" PRIu64 " ... $%" PRIu64
                " was already assigned", range->first(), range->last());
            return false;
          }

          ctx->bolt->findRanges(
              range->first(), range->last(), &invocation->remap, &place,
              &this->pool.wireRangePool);

          break;
        }
        }
      }

      if(place != function->numOutputs)
      {
        log_error("Wrong output size");
        return false;
      }

      for(size_t j = 0; j < in_list->size(); j++)
      {
        switch(in_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          wtk::index_t single = in_list->single(j);
          ctx->bolt->findRanges(
              single, single, &invocation->remap, &place,
              &this->pool.wireRangePool);
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = in_list->range(j);
          ctx->bolt->findRanges(
              range->first(), range->last(), &invocation->remap, &place,
              &this->pool.wireRangePool);
          break;
        }
        }
      }

      if(place != function->numOutputs + function->numInputs)
      {
        log_error("Wrong input size");
        return false;
      }

      ctx->numInstance += function->numInstance;
      ctx->numWitness += function->numWitness;

      invocation->function = function;

      ctx->bolt->types.emplace_back(DirectiveType::INVOCATION);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().invocation = invocation;

      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      wtk::AnonFunction<Number_T>* anon = dir_list->anonFunction(i);

      // Check that the inputs are usable
      wtk::WireList* in_list = anon->inputList();
      for(size_t j = 0; j < in_list->size(); j++)
      {
        switch(in_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          if(!ctx->assigned.has(in_list->single(j))
              || ctx->deleted.has(in_list->single(j)))
          {
            log_error("input wire $%" PRIu64 " is not useable",
                in_list->single(j));
            return false;
          }

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = in_list->range(j);
          if(range->first() > range->last())
          {
            log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                range->first(), range->last());
            return false;
          }

          if(!ctx->assigned.hasAll(range->first(), range->last())
              || ctx->deleted.has(range->first(), range->last()))
          {
            log_error("input wires $%" PRIu64 " ... $%" PRIu64
                " are not useable", range->first(), range->last());
            return false;
          }

          break;
        }
        }
      }

      AnonFunction<Wire_T, Number_T>* anonFunction =
        this->pool.anonFunction.allocate();

      wtk::WireList* out_list = anon->outputList();
      wtk::index_t place = 0;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          wtk::index_t single = out_list->single(j);
          if(!ctx->assigned.insert(single))
          {
            log_error("output wire $%" PRIu64 " was already assigned", single);
            return false;
          }

          ctx->bolt->findRanges(
              single, single, &anonFunction->body.wires, &place,
              &this->pool.wireRangePool);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);
          if(range->first() > range->last())
          {
            log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                range->first(), range->last());
            return false;
          }

          if(!ctx->assigned.insert(range->first(), range->last()))
          {
            log_error("output wires $%" PRIu64 " ... $%" PRIu64
                " was already assigned", range->first(), range->last());
            return false;
          }

          ctx->bolt->findRanges(
              range->first(), range->last(), &anonFunction->body.wires,
              &place, &this->pool.wireRangePool);

          break;
        }
        }
      }

      BoltBuildCtx anon_ctx;
      anon_ctx.numOutputs = place;
      anon_ctx.bolt = &anonFunction->body;
      anon_ctx.iterBounds = ctx->iterBounds;

      for(size_t j = 0; j < in_list->size(); j++)
      {
        switch(in_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          wtk::index_t single = in_list->single(j);
          ctx->bolt->findRanges(
              single, single, &anonFunction->body.wires, &place,
              &this->pool.wireRangePool);
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = in_list->range(j);
          ctx->bolt->findRanges(
              range->first(), range->last(), &anonFunction->body.wires,
              &place, &this->pool.wireRangePool);
          break;
        }
        }
      }

      anon_ctx.numInputs = place - anon_ctx.numOutputs;

      if(!this->build(anon->body(), &anon_ctx)) { return false; }

      if(anon_ctx.numInstance != anon->instanceCount()
          || anon_ctx.numWitness != anon->shortWitnessCount())
      {
        log_error("Anonymous function declares wrong count of"
            " instance/short-witness inputs");
        return false;
      }
      else
      {
        ctx->numInstance += anon->instanceCount();
        ctx->numWitness += anon->shortWitnessCount();
      }

      ctx->bolt->types.emplace_back(DirectiveType::ANON_FUNCTION);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().anonFunction = anonFunction;
      break;
    }
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      wtk::SwitchStatement<Number_T>* const switch_stmt =
        dir_list->switchStatement(i);

      // Should be guaranteed by grammar
      log_assert(switch_stmt->size() > 0);

      wtk::index_t num_ins;
      wtk::index_t num_wit;
      if(!wtk::utils::maxInsWit(
            switch_stmt, &this->functionDeclareMap, &num_ins, &num_wit))
      {
        return false;
      }

      ctx->numInstance += num_ins;
      ctx->numWitness += num_wit;

      SwitchStatement<Wire_T, Number_T>* const out_switch =
        this->pool.switchStatement.allocate(
            1, switch_stmt->size(), num_ins, num_wit);

      wtk::index_t const num_outputs =
        wtk::utils::countWireList(switch_stmt->outputList());

      wtk::utils::SkipList<Number_T> cases;
      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        wtk::CaseBlock<Number_T>* const case_blk = switch_stmt->caseBlock(j);

        if(case_blk->match() >= this->characteristic)
        {
          log_error("Case <%s> exceeds characteristic <%s>.",
              wtk::utils::dec(case_blk->match()).c_str(),
              wtk::utils::dec(this->characteristic).c_str());
          return false;
        }
        if(!cases.insert(case_blk->match()))
        {
          log_error("duplicate case <%s>",
              wtk::utils::dec(case_blk->match()).c_str());
          return false;
        }
        out_switch->caseSelectors[j] = case_blk->match();

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* invoke = case_blk->invokeBody();
          std::string func_name(invoke->name());

          // Find the function to reference
          Function<Wire_T, Number_T>* function;
          {
            auto finder = this->functionMap.find(func_name);
            if(finder == this->functionMap.end())
            {
              log_error("No such function \"%s\"", func_name.c_str());
              return false;
            }
            else
            {
              function = finder->second;
            }
          }

          // Allocate an invocation
          Invocation<Wire_T, Number_T>* invocation =
            this->pool.invocation.allocate();
          invocation->function = function;

          // map in some "dummy outputs"
          if(num_outputs != function->numOutputs)
          {
            log_error("Wrong output size");
            return false;
          }

          out_switch->dummyOutputs[j] =
              this->pool.wireRangePool.localWireRange.allocate(
                  1, 0, num_outputs - 1);
          invocation->remap.ranges.emplace_back(
              out_switch->dummyOutputs[j]);

          // Check that the inputs are usable, and remap them.
          wtk::WireList* in_list = invoke->inputList();
          wtk::index_t place = num_outputs;
          for(size_t k = 0; k < in_list->size(); k++)
          {
            switch(in_list->type(k))
            {
            case wtk::WireList::SINGLE:
            {
              if(!ctx->assigned.has(in_list->single(k))
                  || ctx->deleted.has(in_list->single(k)))
              {
                log_error("input wire $%" PRIu64 " is not useable",
                    in_list->single(k));
                return false;
              }

              ctx->bolt->findRanges(in_list->single(k), in_list->single(k),
                  &invocation->remap, &place, &this->pool.wireRangePool);

              break;
            }
            case wtk::WireList::RANGE:
            {
              wtk::WireRange* range = in_list->range(k);
              if(range->first() > range->last())
              {
                log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                    range->first(), range->last());
                return false;
              }

              if(!ctx->assigned.hasAll(range->first(), range->last())
                  || ctx->deleted.has(range->first(), range->last()))
              {
                log_error("input wires $%" PRIu64 " ... $%" PRIu64
                    " are not useable", range->first(), range->last());
                return false;
              }

              ctx->bolt->findRanges(range->first(), range->last(),
                  &invocation->remap, &place, &this->pool.wireRangePool);

              break;
            }
            }
          }

          if(place != function->numOutputs + function->numInputs)
          {
            log_error("Wrong intput size");
            return false;
          }

          out_switch->caseTypes[j] =
            SwitchStatement<Wire_T, Number_T>::INVOCATION;
          out_switch->caseBlocks[j].invocation = invocation;
          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();

          // Allocate an anonymous function
          AnonFunction<Wire_T, Number_T>* anon_function =
            this->pool.anonFunction.allocate();

          // map in some "dummy outputs"
          out_switch->dummyOutputs[j] =
              this->pool.wireRangePool.localWireRange.allocate(
                  1, 0, num_outputs - 1);
          anon_function->body.wires.ranges.emplace_back(
              out_switch->dummyOutputs[j]);

          // Check that the inputs are usable, and remap them.
          wtk::WireList* in_list = anon->inputList();
          wtk::index_t place = num_outputs;
          for(size_t k = 0; k < in_list->size(); k++)
          {
            switch(in_list->type(k))
            {
            case wtk::WireList::SINGLE:
            {
              if(!ctx->assigned.has(in_list->single(k))
                  || ctx->deleted.has(in_list->single(k)))
              {
                log_error("input wire $%" PRIu64 " is not useable",
                    in_list->single(k));
                return false;
              }

              ctx->bolt->findRanges(in_list->single(k), in_list->single(k),
                  &anon_function->body.wires, &place,
                  &this->pool.wireRangePool);

              break;
            }
            case wtk::WireList::RANGE:
            {
              wtk::WireRange* range = in_list->range(k);
              if(range->first() > range->last())
              {
                log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                    range->first(), range->last());
                return false;
              }

              if(!ctx->assigned.hasAll(range->first(), range->last())
                  || ctx->deleted.has(range->first(), range->last()))
              {
                log_error("input wires $%" PRIu64 " ... $%" PRIu64
                    " are not useable", range->first(), range->last());
                return false;
              }

              ctx->bolt->findRanges(range->first(), range->last(),
                  &anon_function->body.wires, &place,
                  &this->pool.wireRangePool);

              break;
            }
            }
          }

          BoltBuildCtx anon_ctx;
          anon_ctx.numOutputs = num_outputs;
          anon_ctx.numInputs = place - num_outputs;
          anon_ctx.bolt = &anon_function->body;
          anon_ctx.iterBounds = ctx->iterBounds;

          if(!this->build(anon->body(), &anon_ctx)) { return false; }
          if(anon_ctx.numInstance != anon->instanceCount()
              || anon_ctx.numWitness != anon->shortWitnessCount())
          {
            log_error("Anonymous function declares wrong count of"
                " instance/short-witness inputs");
            return false;
          }

          out_switch->caseTypes[j] =
              SwitchStatement<Wire_T, Number_T>::ANON_FUNCTION;
          out_switch->caseBlocks[j].anonFunction = anon_function;
          break;
        }
        }
      }

      wtk::index_t selector = switch_stmt->condition();
      if(!ctx->assigned.has(selector) || ctx->deleted.has(selector))
      {
        log_error("Switch selector wire $%" PRIu64 " is not usable", selector);
        return false;
      }
      ctx->bolt->findWire(selector, &out_switch->selector);

      wtk::WireList* out_list = switch_stmt->outputList();
      wtk::index_t place = 0;
      for(size_t j = 0; j < out_list->size(); j++)
      {
        switch(out_list->type(j))
        {
        case wtk::WireList::SINGLE:
        {
          wtk::index_t single = out_list->single(j);
          if(!ctx->assigned.insert(single))
          {
            log_error("output wire $%" PRIu64 " was already assigned", single);
            return false;
          }

          ctx->bolt->findRanges(
              single, single, &out_switch->outputs, &place,
              &this->pool.wireRangePool);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* range = out_list->range(j);
          if(range->first() > range->last())
          {
            log_error("invalid range $%" PRIu64 " ... $%" PRIu64,
                range->first(), range->last());
            return false;
          }

          if(!ctx->assigned.insert(range->first(), range->last()))
          {
            log_error("output wires $%" PRIu64 " ... $%" PRIu64
                " was already assigned", range->first(), range->last());
            return false;
          }

          ctx->bolt->findRanges(range->first(), range->last(),
              &out_switch->outputs, &place, &this->pool.wireRangePool);

          break;
        }
        }
      }

      ctx->bolt->types.emplace_back(DirectiveType::SWITCH_STATEMENT);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().switchStatement = out_switch;
      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      if(!this->buildLoop(dir_list->forLoop(i), ctx)) { return false; }
      break;
    }
    default:
    {
      log_error("unsupported directive");
      return false;
    }
    }
  }

  if(!wtk::utils::SkipList<index_t>::equivalent(&ctx->all, &ctx->assigned))
  {
    log_error("not all wires were assigned");
    return false;
  }

  if(ctx->numOutputs != 0 && !ctx->all.hasAll(0, ctx->numOutputs - 1))
  {
    log_error("not all output wires were assigned");
    return false;
  }

  return true;
}

template<typename Wire_T, typename Number_T>
bool Builder<Wire_T, Number_T>::buildLoop(
    wtk::ForLoop<Number_T>* const dir_loop, BoltBuildCtx* const ctx)
{
  /* Step 0a. Push the loop bounds */
  ctx->iterBounds->reset();
  ForLoop<Wire_T, Number_T>* loop = this->pool.forLoop.allocate();
  loop->first = dir_loop->first();
  loop->last = dir_loop->last();

  if(!ctx->iterBounds->push(dir_loop->iterName(),
      dir_loop->first(), dir_loop->last(), &loop->current))
  {
      return false;
  }

  /* Step 0b. calculate and check the loop's length */
  if(dir_loop->last() < dir_loop->first())
  {
    log_error("Loop with negative iterations is undefined behavior");
    return false;
  }

  wtk::index_t loop_len = 1 + dir_loop->last() - dir_loop->first();

  /* Step 1a. Add the loop's output wires to a skiplist */
  wtk::utils::SkipList<wtk::index_t> loop_outs;
  if(!wtk::utils::listOutputWires(dir_loop->outputList(), &loop_outs))
  {
    log_error("Loop output wire list is invalid");
    return false;
  }

  /* Step 1b. Attempt shortcut evaluation for output expressions. */
  wtk::IterExprWireList* dir_outputs;
  switch(dir_loop->bodyType())
  {
  case wtk::ForLoop<Number_T>::INVOKE:
  {
    dir_outputs = dir_loop->invokeBody()->outputList();
    break;
  }
  case wtk::ForLoop<Number_T>::ANONYMOUS:
  {
    dir_outputs = dir_loop->anonymousBody()->outputList();
    break;
  }
  default:
  {
    log_error("unreachable case");
    return false;
  }
  }

  std::vector<ExprBuildCtx> output_ctxs;
  output_ctxs.reserve(dir_outputs->size());
  bool can_shortcut = true;
  wtk::utils::SkipList<wtk::index_t> iteration_outs;
  for(size_t i = 0; i < dir_outputs->size(); i++)
  {
    wtk::IterExpr* first_expr;
    wtk::IterExpr* last_expr;
    switch(dir_outputs->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      first_expr = dir_outputs->single(i);
      last_expr = dir_outputs->single(i);
      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      first_expr = dir_outputs->range(i)->first();
      last_expr = dir_outputs->range(i)->last();
      break;
    }
    default:
    {
      log_error("unreachable case");
      return false;
    }
    }

    // Build BOLT Expressions.
    output_ctxs.emplace_back();
    if(!ctx->iterBounds->build(first_expr, &output_ctxs.back().first)
        || !ctx->iterBounds->build(last_expr, &output_ctxs.back().last))
    {
      log_error("Loop iterator expression has unbound iterator names");
      return false;
    }

    // Check iterator usage in-case soft-unroll is necessary.
    ctx->iterBounds->checkIterUsage(first_expr);
    ctx->iterBounds->checkIterUsage(last_expr);

    bool linear_expr = wtk::utils::iterExprLinear(first_expr)
      && wtk::utils::iterExprLinear(last_expr);

    bool local_only =
      wtk::utils::iterExprSoleDependence(first_expr, dir_loop->iterName())
      && wtk::utils::iterExprSoleDependence(last_expr, dir_loop->iterName());


    // Eval at first.
    loop->current = loop->first;

    wtk::index_t first_first = output_ctxs.back().first.eval(this->exprStack);
    wtk::index_t first_last = output_ctxs.back().last.eval(this->exprStack);

    // Eval at second to check if first and second are adjacent.
    bool adjacent_1_2 = true;
    if(loop_len > 1)
    {
      loop->current = loop->first + 1;

      wtk::index_t second_first =
        output_ctxs.back().first.eval(this->exprStack);
      wtk::index_t second_last =
        output_ctxs.back().last.eval(this->exprStack);

      adjacent_1_2 =
        first_last == second_first - 1 || first_first == second_last + 1;
    }

    // Encapsulates checks at the bounds
    //  - No overflow
    //  - Span is non-negative
    //  - Span is constant
    output_ctxs.back().expectedSpan = 1 + first_last - first_first;
    bool bounds_okay = ctx->iterBounds->evalBounds(first_expr, last_expr,
        &output_ctxs.back().totalMin, &output_ctxs.back().totalMax,
        output_ctxs.back().expectedSpan);

    output_ctxs.back().shortcut =
      linear_expr && local_only && adjacent_1_2 && bounds_okay;
    can_shortcut = can_shortcut && output_ctxs.back().shortcut;

    if(output_ctxs.back().shortcut
        && (!loop_outs.hasAll(
            output_ctxs.back().totalMin, output_ctxs.back().totalMax)
          || !iteration_outs.insert(
            output_ctxs.back().totalMin, output_ctxs.back().totalMax)))
    {
      log_error("Shortcut iterator range is not valid for assignments");
      return false;
    }
  }

  if(can_shortcut && !wtk::utils::SkipList<wtk::index_t>::equivalent(
        &loop_outs, &iteration_outs))
  {
    log_error("Loop outputs are not assigned by iterations");
    return false;
  }

  /* Step 2. Attempt shortcut evaluation for input expressions. */
  wtk::IterExprWireList* dir_inputs;

  switch(dir_loop->bodyType())
  {
  case wtk::ForLoop<Number_T>::INVOKE:
  {
    dir_inputs = dir_loop->invokeBody()->inputList();
    break;
  }
  case wtk::ForLoop<Number_T>::ANONYMOUS:
  {
    dir_inputs = dir_loop->anonymousBody()->inputList();
    break;
  }
  default:
  {
    log_error("unreachable case");
    return false;
  }
  }

  bool parallel = can_shortcut;

  std::vector<ExprBuildCtx> input_ctxs;
  input_ctxs.reserve(dir_inputs->size());
  for(size_t i = 0; i < dir_inputs->size(); i++)
  {
    wtk::IterExpr* first_expr;
    wtk::IterExpr* last_expr;
    switch(dir_inputs->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      first_expr = dir_inputs->single(i);
      last_expr = dir_inputs->single(i);
      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      first_expr = dir_inputs->range(i)->first();
      last_expr = dir_inputs->range(i)->last();
      break;
    }
    default:
    {
      log_error("unreachable case");
      return false;
    }
    }

    // Build BOLT Expressions.
    input_ctxs.emplace_back();
    if(!ctx->iterBounds->build(first_expr, &input_ctxs.back().first)
        || !ctx->iterBounds->build(last_expr, &input_ctxs.back().last))
    {
      log_error("Loop iterator expression has unbound iterator names");
      return false;
    }

    // Check iterator usage in-case soft-unroll is necessary.
    ctx->iterBounds->checkIterUsage(first_expr);
    ctx->iterBounds->checkIterUsage(last_expr);

    bool linear_expr = wtk::utils::iterExprLinear(first_expr)
      && wtk::utils::iterExprLinear(last_expr);


    // Eval at first.
    loop->current = loop->first;

    wtk::index_t first_first = input_ctxs.back().first.eval(this->exprStack);
    wtk::index_t first_last = input_ctxs.back().last.eval(this->exprStack);

    // Encapsulates checks at the bounds
    //  - No overflow
    //  - Span is non-negative
    //  - Span is constant
    input_ctxs.back().expectedSpan = 1 + first_last - first_first;
    bool bounds_okay = ctx->iterBounds->evalBounds(first_expr, last_expr,
        &input_ctxs.back().totalMin, &input_ctxs.back().totalMax,
        input_ctxs.back().expectedSpan);

    bool assigned_before_loop = ctx->assigned.hasAll(
          input_ctxs.back().totalMin, input_ctxs.back().totalMax)
      && !ctx->deleted.has(
          input_ctxs.back().totalMin, input_ctxs.back().totalMax);

    input_ctxs.back().shortcut =
      linear_expr && bounds_okay && assigned_before_loop;

    parallel = parallel && input_ctxs.back().shortcut;

    // If it can shortcut it might also be constant.
    if(input_ctxs.back().shortcut)
    {
      input_ctxs.back().constant =
        wtk::utils::iterExprConstant(first_expr)
        && wtk::utils::iterExprConstant(last_expr);
    }
    // If it can't shortcut, check if it can shortcut via being sequential
    // or if un-shortcuts any outputs.
    else
    {
      bool sequential = linear_expr && bounds_okay;
      for(size_t i = 0; i < output_ctxs.size(); i++)
      {
        if(output_ctxs[i].shortcut
            && ((input_ctxs.back().totalMin >= output_ctxs[i].totalMin
                && input_ctxs.back().totalMin <= output_ctxs[i].totalMax)
              || (input_ctxs.back().totalMax >= output_ctxs[i].totalMin
                && input_ctxs.back().totalMax <= output_ctxs[i].totalMax)))
        {
          // Test if the input is sequential to the output
          loop->current = loop->first;
          wtk::index_t const out_first =
            output_ctxs[i].first.eval(this->exprStack);
          wtk::index_t const in_first = first_first;
          loop->current = loop->last;
          wtk::index_t const out_last =
            output_ctxs[i].first.eval(this->exprStack);
          wtk::index_t const in_last =
            input_ctxs.back().first.eval(this->exprStack);

          if(!(wtk::utils::iterExprSoleDependence(
                  first_expr, dir_loop->iterName())
              && wtk::utils::iterExprSoleDependence(
                last_expr, dir_loop->iterName())
              && out_first != in_first
              // This also implies same direction of travel
              && (out_first - in_first) == (out_last - in_last)
              && (input_ctxs.back().totalMin < output_ctxs[i].totalMin
                ? (ctx->assigned.hasAll(
                    input_ctxs.back().totalMin, output_ctxs[i].totalMin - 1)
                  && !ctx->deleted.has(
                    input_ctxs.back().totalMin, output_ctxs[i].totalMin - 1)
                  && output_ctxs[i].totalMin - input_ctxs.back().totalMin
                    >= input_ctxs.back().expectedSpan)
                : (ctx->assigned.hasAll(
                    output_ctxs[i].totalMax + 1, input_ctxs.back().totalMax)
                  && !ctx->deleted.has(
                    output_ctxs[i].totalMax + 1, input_ctxs.back().totalMax)
                  && input_ctxs.back().totalMax - output_ctxs[i].totalMax
                    >= input_ctxs.back().expectedSpan))))
          {
            output_ctxs[i].shortcut = false;
            sequential = false;
            // Since this output will be soft-unrolled, leaving this will
            // throw off soft-unrolling.
            iteration_outs.remove(
                output_ctxs[i].totalMin, output_ctxs[i].totalMax);
          }
        }
      }

      input_ctxs.back().shortcut = sequential;
    }

    can_shortcut = can_shortcut && input_ctxs.back().shortcut;
  }

  /* Step 3a. If possible, build the shortcut */
  if(can_shortcut)
  {
    loop->parallelizable = parallel;

    wtk::index_t place = 0;
    for(size_t i = 0; i < output_ctxs.size(); i++)
    {
      WireRange<Wire_T, Number_T>* indirect =
        ctx->bolt->wires.findRange(output_ctxs[i].totalMin);
      if(indirect->last >= output_ctxs[i].totalMax)
      {
        ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>* range =
          this->pool.wireRangePool.shortcutLoopIndirectWireRangeRef.allocate(1,
              indirect, std::move(output_ctxs[i].first));
        wtk::index_t const last = place + output_ctxs[i].expectedSpan - 1;
        range->setRange(place, last);
        place = last + 1;
        loop->body.wires.ranges.push_back(range);
      }
      else
      {
        ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>* range =
          this->pool.wireRangePool.shortcutLoopWireSetWireRangeRef.allocate(1,
              &ctx->bolt->wires, std::move(output_ctxs[i].first));
        wtk::index_t const last = place + output_ctxs[i].expectedSpan - 1;
        range->setRange(place, last);
        place = last + 1;
        loop->body.wires.ranges.push_back(range);
      }
    }
    wtk::index_t const num_outs = place;

    for(size_t i = 0; i < input_ctxs.size(); i++)
    {
      if(input_ctxs[i].constant)
      {
        ctx->bolt->findRanges(input_ctxs[i].totalMin, input_ctxs[i].totalMax,
            &loop->body.wires, &place, &this->pool.wireRangePool);
      }
      else
      {
        WireRange<Wire_T, Number_T>* indirect =
          ctx->bolt->wires.findRange(input_ctxs[i].totalMin);
        if(indirect->last >= input_ctxs[i].totalMax)
        {
          ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>* range =
            this->pool.wireRangePool.shortcutLoopIndirectWireRangeRef
                .allocate(1, indirect, std::move(input_ctxs[i].first));
          wtk::index_t const last = place + input_ctxs[i].expectedSpan - 1;
          range->setRange(place, last);
          place = last + 1;
          loop->body.wires.ranges.push_back(range);
        }
        else
        {
          ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>* range =
            this->pool.wireRangePool.shortcutLoopWireSetWireRangeRef
            .allocate(1, &ctx->bolt->wires, std::move(input_ctxs[i].first));
          wtk::index_t const last = place + input_ctxs[i].expectedSpan - 1;
          range->setRange(place, last);
          place = last + 1;
          loop->body.wires.ranges.push_back(range);
        }
      }
    }
    wtk::index_t const num_ins = place - num_outs;

    BoltBuildCtx loop_ctx;
    loop_ctx.numOutputs = num_outs;
    loop_ctx.numInputs = num_ins;
    loop_ctx.bolt = &loop->body;

    switch(dir_loop->bodyType())
    {
    case wtk::ForLoop<Number_T>::INVOKE:
    {
      wtk::IterExprFunctionInvoke* invoke = dir_loop->invokeBody();
      std::string name_str(invoke->name());
      auto finder = this->functionDeclareMap.find(name_str);
      if(finder == this->functionDeclareMap.end())
      {
        log_error("No such function-gate \"%s\"", invoke->name());
        return false;
      }

      wtk::FunctionDeclare<Number_T>* declare = finder->second;

      if(num_outs != declare->outputCount()
          || num_ins != declare->inputCount())
      {
        log_error("number of loop-iteration inputs and outputs does not match "
            "function \"%s\"", invoke->name());
        return false;
      }

      IterBounds sub_iter_bounds;
      loop_ctx.iterBounds = &sub_iter_bounds;
      if(!this->build(declare->body(), &loop_ctx)) { return false; }

      if(loop_ctx.numInstance != declare->instanceCount()
          || loop_ctx.numWitness != declare->shortWitnessCount())
      {
        log_error("number of instances or short-witnesses does not match");
        return false;
      }

      ctx->numInstance +=
        (1 + loop->last - loop->first) * declare->instanceCount();
      ctx->numWitness +=
        (1 + loop->last - loop->first) * declare->shortWitnessCount();

      break;
    }
    case wtk::ForLoop<Number_T>::ANONYMOUS:
    {
      wtk::IterExprAnonFunction<Number_T>* anon = dir_loop->anonymousBody();

      loop_ctx.iterBounds = ctx->iterBounds;
      if(!this->build(anon->body(), &loop_ctx)) { return false; }

      if(loop_ctx.numInstance != anon->instanceCount()
          || loop_ctx.numWitness != anon->shortWitnessCount())
      {
        log_error("number of instances or short-witnesses does not match");
        return false;
      }

      ctx->numInstance +=
        (1 + loop->last - loop->first) * anon->instanceCount();
      ctx->numWitness +=
        (1 + loop->last - loop->first) * anon->shortWitnessCount();

      break;
    }
    }

    bool okay = true;
    loop_outs.forEach([&ctx, &okay](wtk::index_t const f, wtk::index_t const l)
      {
        okay = ctx->assigned.insert(f, l) && okay;
      });

    if(!okay)
    {
      log_error("Loop outputs are not assignable");
      return false;
    }
    else
    {
      ctx->iterBounds->pop();

      ctx->bolt->types.push_back(DirectiveType::FOR_LOOP);
      ctx->bolt->directives.emplace_back();
      ctx->bolt->directives.back().forLoop = loop;

      return true;
    }
  }
  /* Step 3b. Otherwise, attempt Soft unrolling */
  else
  {
    wtk::utils::SkipList<wtk::index_t> unchecked_loop_outs(loop_outs);
    iteration_outs.forEach(
      [&unchecked_loop_outs](wtk::index_t const f, wtk::index_t const l)
        -> void
      {
        unchecked_loop_outs.remove(f, l);
      });

    SoftUnrollStatus status = this->checkSoftUnroll(
        &output_ctxs, &input_ctxs, &unchecked_loop_outs, ctx);

    if(status == OKAY)
    {
      /* Step 4a. Build via soft-unrolling */
      wtk::index_t num_remapped = 0;
      SoftLoopWireRangeRef<Wire_T, Number_T>* soft_range =
        this->pool.wireRangePool.softLoopWireRangeRef.allocate(
            1, &ctx->bolt->wires, output_ctxs.size() + input_ctxs.size());
      for(size_t i = 0; i < output_ctxs.size(); i++)
      {
        num_remapped += output_ctxs[i].expectedSpan;

        soft_range->mappings.emplace_back(
            std::move(output_ctxs[i].first), std::move(output_ctxs[i].last));
      }
      wtk::index_t const num_outs = num_remapped;

      for(size_t i = 0; i < input_ctxs.size(); i++)
      {
        num_remapped += input_ctxs[i].expectedSpan;

        soft_range->mappings.emplace_back(
            std::move(input_ctxs[i].first), std::move(input_ctxs[i].last));
      }
      wtk::index_t const num_ins = num_remapped - num_outs;

      soft_range->setRange(0, num_remapped);
      loop->body.wires.ranges.push_back(soft_range);

      BoltBuildCtx loop_ctx;
      loop_ctx.numOutputs = num_outs;
      loop_ctx.numInputs = num_ins;
      loop_ctx.bolt = &loop->body;

      switch(dir_loop->bodyType())
      {
      case wtk::ForLoop<Number_T>::INVOKE:
      {
        wtk::IterExprFunctionInvoke* invoke = dir_loop->invokeBody();
        std::string name_str(invoke->name());
        auto finder = this->functionDeclareMap.find(name_str);
        if(finder == this->functionDeclareMap.end())
        {
          log_error("No such function-gate \"%s\"", invoke->name());
          return false;
        }

        wtk::FunctionDeclare<Number_T>* declare = finder->second;

        if(num_outs != declare->outputCount()
            || num_ins != declare->inputCount())
        {
          log_error("number of loop-iteration inputs and outputs does not match "
              "function \"%s\"", invoke->name());
          return false;
        }

        IterBounds sub_iter_bounds;
        loop_ctx.iterBounds = &sub_iter_bounds;
        if(!this->build(declare->body(), &loop_ctx)) { return false; }

        if(loop_ctx.numInstance != declare->instanceCount()
            || loop_ctx.numWitness != declare->shortWitnessCount())
        {
          log_error("number of instances or short-witnesses does not match");
          return false;
        }

        ctx->numInstance +=
          (1 + loop->last - loop->first) * declare->instanceCount();
        ctx->numWitness +=
          (1 + loop->last - loop->first) * declare->shortWitnessCount();

        break;
      }
      case wtk::ForLoop<Number_T>::ANONYMOUS:
      {
        wtk::IterExprAnonFunction<Number_T>* anon = dir_loop->anonymousBody();

        loop_ctx.iterBounds = ctx->iterBounds;
        if(!this->build(anon->body(), &loop_ctx)) { return false; }

        if(loop_ctx.numInstance != anon->instanceCount()
            || loop_ctx.numWitness != anon->shortWitnessCount())
        {
          log_error("number of instances or short-witnesses does not match");
          return false;
        }

        ctx->numInstance +=
          (1 + loop->last - loop->first) * anon->instanceCount();
        ctx->numWitness +=
          (1 + loop->last - loop->first) * anon->shortWitnessCount();

        break;
      }
      }

      bool okay = true;
      loop_outs.forEach([&ctx, &okay](wtk::index_t const f, wtk::index_t const l)
        {
          okay = ctx->assigned.insert(f, l) && okay;
        });

      if(!okay)
      {
        log_error("Loop outputs are not assignable");
        return false;
      }
      else
      {
        ctx->iterBounds->pop();

        ctx->bolt->types.push_back(DirectiveType::FOR_LOOP);
        ctx->bolt->directives.emplace_back();
        ctx->bolt->directives.back().forLoop = loop;

        return true;
      }
    }
    else if(status == HARD)
    {
      /* Step 4b. Fall back to hard unrolling */
      ctx->iterBounds->pop(); // hard-unrolling does its own thing.
      return this->buildHardLoop(dir_loop, ctx);
    }
    else
    {
      return false;
    }
  }

  return true;
}

template<typename Wire_T, typename Number_T>
typename Builder<Wire_T, Number_T>::SoftUnrollStatus
Builder<Wire_T, Number_T>::checkSoftUnroll(
    std::vector<ExprBuildCtx>* const outputs,
    std::vector<ExprBuildCtx>* const inputs,
    wtk::utils::SkipList<wtk::index_t>* const loop_outputs,
    BoltBuildCtx* const ctx)
{
  wtk::index_t expected_outputs = 0;
  for(size_t i = 0; i < outputs->size(); i++)
  {
    if(!(*outputs)[i].shortcut)
    {
      expected_outputs += (*outputs)[i].expectedSpan;
    }
  }

  wtk::index_t expected_inputs = 0;
  for(size_t i = 0; i < inputs->size(); i++)
  {
    if(!(*inputs)[i].shortcut)
    {
      expected_inputs += (*inputs)[i].expectedSpan;
    }
  }

  return this->checkSoftUnrollHelper(
      outputs, inputs, expected_outputs, expected_inputs,
      loop_outputs, ctx, 0);
}

template<typename Wire_T, typename Number_T>
typename Builder<Wire_T, Number_T>::SoftUnrollStatus
Builder<Wire_T, Number_T>::checkSoftUnrollHelper(
    std::vector<ExprBuildCtx>* const outputs,
    std::vector<ExprBuildCtx>* const inputs,
    wtk::index_t const expected_outputs, wtk::index_t const expected_inputs,
    wtk::utils::SkipList<wtk::index_t>* const loop_outputs,
    BoltBuildCtx* const ctx, size_t const place)
{
  if(place == ctx->iterBounds->iterators.size() - 1)
  {
    // run the local loop.
    wtk::utils::SkipList<wtk::index_t> assigned_copy(ctx->assigned);
    wtk::utils::SkipList<wtk::index_t> unrolled_outputs;

    for(wtk::index_t i = ctx->iterBounds->firsts[place];
        i <= ctx->iterBounds->lasts[place]; i++)
    {
      *ctx->iterBounds->iterators[place] = i;

      wtk::index_t iteration_inputs = 0;
      for(size_t j = 0; j < inputs->size(); j++)
      {
        if(!(*inputs)[j].shortcut)
        {
          wtk::index_t first = (*inputs)[j].first.eval(this->exprStack);
          wtk::index_t last = (*inputs)[j].last.eval(this->exprStack);

          if(first > last || !assigned_copy.hasAll(first, last)
              || ctx->deleted.has(first, last))
          {
            log_error("Input expression is poorly formed in soft unroll");
            return FAIL;
          }

          iteration_inputs += 1 + last - first;
        }
      }

      wtk::index_t iteration_outputs = 0;
      for(size_t j = 0; j < outputs->size(); j++)
      {
        if(!(*outputs)[j].shortcut)
        {
          wtk::index_t first = (*outputs)[j].first.eval(this->exprStack);
          wtk::index_t last = (*outputs)[j].last.eval(this->exprStack);

          if(first > last || !assigned_copy.insert(first, last)
              || !unrolled_outputs.insert(first, last)
              || !loop_outputs->hasAll(first, last))
          {
            log_error("Output expression is poorly formed in soft unroll");
            return FAIL;
          }
          iteration_outputs += 1 + last - first;
        }
      }

      if(iteration_outputs != expected_outputs
          || iteration_inputs != expected_inputs)
      {
        return HARD;
      }
    }

    if(!wtk::utils::SkipList<wtk::index_t>::equivalent(
          &unrolled_outputs, loop_outputs))
    {
      log_error("Loop outputs are not assigned in soft unroll");
      return FAIL;
    }

    return OKAY;
  }
  else if(ctx->iterBounds->iterUsage[place])
  {
    // soft unroll the outer loop.
    for(wtk::index_t i = ctx->iterBounds->firsts[place];
        i <= ctx->iterBounds->lasts[place]; i++)
    {
      *ctx->iterBounds->iterators[place] = i;

      SoftUnrollStatus status =
        this->checkSoftUnrollHelper(
            outputs, inputs, expected_outputs, expected_inputs,
            loop_outputs, ctx, place + 1);
      if(status != OKAY)
      {
        return status;
      }
    }

    return OKAY;
  }
  else
  {
    // pass through the outer loop.
    return this->checkSoftUnrollHelper(
        outputs, inputs, expected_outputs, expected_inputs,
        loop_outputs, ctx, place + 1);
  }
}

template<typename Wire_T, typename Number_T>
bool Builder<Wire_T, Number_T>::buildHardLoop(
    wtk::ForLoop<Number_T>* const dir_loop, BoltBuildCtx* const ctx)
{
  if(dir_loop->bodyType() == wtk::ForLoop<Number_T>::INVOKE)
  {
    log_error("Cannot hard-unroll an named function, inputs/outputs size "
        "is non constant");
    return false;
  }

  HardUnrollForLoop<Wire_T, Number_T>* const hard_loop =
    this->pool.hardUnrollForLoop.allocate();

  hard_loop->first = dir_loop->first();
  hard_loop->last = dir_loop->last();
  hard_loop->body = dir_loop->anonymousBody()->body();
  hard_loop->instanceCount = dir_loop->anonymousBody()->instanceCount();
  hard_loop->witnessCount = dir_loop->anonymousBody()->shortWitnessCount();
  hard_loop->functions = &this->functionDeclareMap;

  ctx->numInstance +=
    hard_loop->instanceCount * (1 + hard_loop->last - hard_loop->first);
  ctx->numWitness +=
    hard_loop->witnessCount * (1 + hard_loop->last - hard_loop->first);

  for(size_t i = 0; i < ctx->iterBounds->names.size(); i++)
  {
    hard_loop->exprBuilder.push(
        ctx->iterBounds->names[i].c_str(), ctx->iterBounds->iterators[i]);
  }

  hard_loop->exprBuilder.push(dir_loop->iterName(), &hard_loop->current);

  wtk::IterExprWireList* const out_list =
    dir_loop->anonymousBody()->outputList();
  hard_loop->outputList.reserve(out_list->size());
  for(size_t i = 0; i < out_list->size(); i++)
  {
    hard_loop->outputList.emplace_back();

    switch(out_list->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      if(!hard_loop->exprBuilder.build(
            out_list->single(i), &hard_loop->outputList.back().first)
          || !hard_loop->exprBuilder.build(
            out_list->single(i), &hard_loop->outputList.back().second))
      {
        log_error("Malformed loop iteration output expression");
        return false;
      }

      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      wtk::IterExprWireRange* const range = out_list->range(i);
      if(!hard_loop->exprBuilder.build(
            range->first(), &hard_loop->outputList.back().first)
          || !hard_loop->exprBuilder.build(
            range->last(), &hard_loop->outputList.back().second))
      {
        log_error("Malformed loop iteration output expression");
        return false;
      }

      break;
    }
    }
  }

  wtk::IterExprWireList* in_list = dir_loop->anonymousBody()->inputList();
  hard_loop->inputList.reserve(in_list->size());
  for(size_t i = 0; i < in_list->size(); i++)
  {
    hard_loop->inputList.emplace_back();

    switch(in_list->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      if(!hard_loop->exprBuilder.build(
            in_list->single(i), &hard_loop->inputList.back().first)
          || !hard_loop->exprBuilder.build(
            in_list->single(i), &hard_loop->inputList.back().second))
      {
        log_error("Malformed loop iteration input expression");
        return false;
      }

      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      wtk::IterExprWireRange* const range = in_list->range(i);
      if(!hard_loop->exprBuilder.build(
            range->first(), &hard_loop->inputList.back().first)
          || !hard_loop->exprBuilder.build(
            range->last(), &hard_loop->inputList.back().second))
      {
        log_error("Malformed loop iteration input expression");
        return false;
      }

      break;
    }
    }
  }

  // List of the loop's output wires.
  wtk::utils::SkipList<wtk::index_t> loop_outputs;
  if(!wtk::utils::listOutputWires(
          dir_loop->outputList(), &loop_outputs))
  {
    log_error("Loop output list is malformed");
    return false;
  }

  HardUnrollCtx unroll_ctx;
  unroll_ctx.exprBuilder = &hard_loop->exprBuilder;

  if(!this->iterateHardUnroll(hard_loop, ctx, &unroll_ctx, &loop_outputs))
  {
    return false;
  }

  bool success = true;
  loop_outputs.forEach(
      [&ctx, &success](wtk::index_t first, wtk::index_t last) -> void
    {
      success = ctx->assigned.insert(first, last) && success;
    });

  ctx->bolt->types.emplace_back(DirectiveType::HARD_UNROLL_FOR_LOOP);
  ctx->bolt->directives.emplace_back();
  ctx->bolt->directives.back().hardUnrollForLoop = hard_loop;

  return success;
}

template<typename Wire_T, typename Number_T>
bool Builder<Wire_T, Number_T>::iterateHardUnroll(
    HardUnrollForLoop<Wire_T, Number_T>* const loop,
    BoltBuildCtx* const bolt_ctx,
    HardUnrollCtx* const unroll_ctx,
    wtk::utils::SkipList<wtk::index_t>* const loop_outs,
    size_t const depth)
{
  if(depth == bolt_ctx->iterBounds->names.size())
  {
    wtk::utils::SkipList<wtk::index_t> iter_outs;
    wtk::utils::SkipList<wtk::index_t> assigned_copy(bolt_ctx->assigned);

    for(wtk::index_t i = loop->first; i <= loop->last; i++)
    {
      loop->current = i;

      wtk::index_t num_inputs = 0;
      for(size_t j = 0; j < loop->inputList.size(); j++)
      {
        wtk::index_t const first =
          loop->inputList[j].first.eval(this->exprStack);
        wtk::index_t const last =
          loop->inputList[j].second.eval(this->exprStack);

        if(!assigned_copy.hasAll(first, last)
            || bolt_ctx->deleted.has(first, last))
        {
          log_error("Loop iteration inputs are not useable");
          return false;
        }

        num_inputs += 1 + last - first;
      }

      wtk::index_t num_outputs = 0;
      for(size_t j = 0; j < loop->outputList.size(); j++)
      {
        wtk::index_t const first =
          loop->outputList[j].first.eval(this->exprStack);
        wtk::index_t const last =
          loop->outputList[j].second.eval(this->exprStack);

        if(!assigned_copy.insert(first, last)
            || bolt_ctx->deleted.has(first, last)
            || !iter_outs.insert(first, last))
        {
          log_error("Loop iteration outputs are not useable");
          return false;
        }

        num_outputs += 1 + last - first;
      }

      unroll_ctx->assigned.clear();
      unroll_ctx->deleted.clear();
      if(num_inputs > 0)
      {
        unroll_ctx->assigned.insert(num_outputs, num_outputs + num_inputs - 1);
      }

      unroll_ctx->instanceCount = 0;
      unroll_ctx->witnessCount = 0;
      unroll_ctx->noDelete = num_outputs + num_inputs;

      if(!this->checkHardUnrollIteration(loop->body, unroll_ctx))
      {
        return false;
      }

      if(unroll_ctx->instanceCount != loop->instanceCount
          || unroll_ctx->witnessCount != loop->witnessCount
          ||(num_outputs != 0
            && !unroll_ctx->assigned.hasAll(0, num_outputs - 1)))
      {
        log_error("Hard unrolled loop body has wrong instance consumption");
        return false;
      }
    }

    if(!wtk::utils::SkipList<wtk::index_t>::equivalent(&iter_outs, loop_outs))
    {
      log_error("Iterations' outputs do not cover the loop's outputs.");
      return false;
    }
  }
  else
  {
    for(wtk::index_t i = bolt_ctx->iterBounds->firsts[depth];
        i <= bolt_ctx->iterBounds->lasts[depth]; i++)
    {
      *bolt_ctx->iterBounds->iterators[depth] = i;

      if(!this->iterateHardUnroll(
            loop, bolt_ctx, unroll_ctx, loop_outs, depth + 1))
      {
        return false;
      }
    }
  }

  return true;
}

template<typename Wire_T, typename Number_T>
bool Builder<Wire_T, Number_T>::checkHardUnrollIteration(
    wtk::DirectiveList<Number_T>* const body, HardUnrollCtx* const ctx)
{
  for(size_t i = 0; i < body->size(); i++)
  {
    switch(body->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      wtk::BinaryGate* const gate = body->binaryGate(i);

      if(UNLIKELY(UNLIKELY(!ctx->assigned.has(gate->leftWire()))
            || UNLIKELY(ctx->deleted.has(gate->leftWire()))
            || UNLIKELY(!ctx->assigned.has(gate->rightWire()))
            || UNLIKELY(ctx->deleted.has(gate->rightWire()))
            || UNLIKELY(!ctx->assigned.insert(gate->outputWire()))
            || UNLIKELY(ctx->deleted.has(gate->outputWire()))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      wtk::UnaryGate* const gate = body->unaryGate(i);

      if(UNLIKELY(UNLIKELY(!ctx->assigned.has(gate->inputWire()))
            || UNLIKELY(ctx->deleted.has(gate->inputWire()))
            || UNLIKELY(!ctx->assigned.insert(gate->outputWire()))
            || UNLIKELY(ctx->deleted.has(gate->outputWire()))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      wtk::BinaryConstGate<Number_T>* const gate = body->binaryConstGate(i);

      if(UNLIKELY(UNLIKELY(!ctx->assigned.has(gate->leftWire()))
            || UNLIKELY(ctx->deleted.has(gate->leftWire()))
            || UNLIKELY(gate->rightValue() >= this->characteristic)
            || UNLIKELY(!ctx->assigned.insert(gate->outputWire()))
            || UNLIKELY(ctx->deleted.has(gate->outputWire()))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      wtk::Input* const input = body->input(i);

      if(UNLIKELY(!ctx->assigned.insert(input->outputWire()))
          || UNLIKELY(ctx->deleted.has(input->outputWire())))
      {
        log_error("Hard unroll failure");
        return false;
      }

      switch(input->stream())
      {
      case wtk::Input::INSTANCE: { ctx->instanceCount++; break; }
      case wtk::Input::SHORT_WITNESS: { ctx->witnessCount++; break; }
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      wtk::Assign<Number_T>* const assign = body->assign(i);

      if(UNLIKELY(UNLIKELY(assign->constValue() >= this->characteristic)
            || UNLIKELY(!ctx->assigned.insert(assign->outputWire()))
            || UNLIKELY(ctx->deleted.has(assign->outputWire()))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO:
    {
      wtk::Terminal* const assert_zero = body->assertZero(i);

      if(UNLIKELY(!ctx->assigned.has(assert_zero->wire()))
          || UNLIKELY(ctx->deleted.has(assert_zero->wire())))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE:
    {
      wtk::Terminal* const delete_single = body->deleteSingle(i);

      if(UNLIKELY(!ctx->deleted.insert(delete_single->wire()))
          || UNLIKELY(delete_single->wire() < ctx->noDelete))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE:
    {
      wtk::WireRange* const delete_range = body->deleteRange(i);

      if(UNLIKELY(!ctx->deleted.insert(
              delete_range->first(), delete_range->last()))
          || UNLIKELY(delete_range->first() < ctx->noDelete))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      wtk::FunctionInvoke* const invoke = body->functionInvoke(i);

      std::string func_name(invoke->name());
      auto finder = this->functionDeclareMap.find(func_name);
      if(finder == this->functionDeclareMap.end())
      {
        log_error("no such function: %s", func_name.c_str());
        return false;
      }

      wtk::FunctionDeclare<Number_T>* const declare = finder->second;

      wtk::index_t num_inputs = 0;
      wtk::WireList* const in_list = invoke->inputList();
      for(size_t i = 0; i < in_list->size(); i++)
      {
        switch(in_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!ctx->assigned.has(in_list->single(i)))
              || UNLIKELY(ctx->deleted.has(in_list->single(i))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_inputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !ctx->assigned.hasAll(range->first(), range->last()))
                || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_inputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("Hard unroll failure");
          return false;
        }
        }
      }

      wtk::index_t num_outputs = 0;
      wtk::WireList* const out_list = invoke->outputList();
      for(size_t i = 0; i < out_list->size(); i++)
      {
        switch(out_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!ctx->assigned.insert(out_list->single(i)))
              || UNLIKELY(ctx->deleted.has(out_list->single(i))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_outputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !ctx->assigned.insert(range->first(), range->last()))
                || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_outputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("Hard unroll failure");
          return false;
        }
        }
      }

      if(UNLIKELY(UNLIKELY(num_inputs != declare->inputCount())
            || UNLIKELY(num_outputs != declare->outputCount())))
      {
        log_error("Hard unroll failure");
        return false;
      }

      ExprBuilder func_expr_builder;
      HardUnrollCtx sub_ctx;
      sub_ctx.exprBuilder = &func_expr_builder;
      sub_ctx.noDelete = num_outputs + num_inputs;

      if(num_inputs > 0)
      {
        sub_ctx.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
      }

      if(!this->checkHardUnrollIteration(declare->body(), &sub_ctx))
      {
        return false;
      }

      if(UNLIKELY(
          UNLIKELY(sub_ctx.instanceCount != declare->instanceCount())
          || UNLIKELY(sub_ctx.witnessCount != declare->shortWitnessCount())
          || UNLIKELY(num_outputs != 0
            && !sub_ctx.assigned.hasAll(0, num_outputs - 1))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      wtk::AnonFunction<Number_T>* const anon = body->anonFunction(i);

      wtk::index_t num_inputs = 0;
      wtk::WireList* const in_list = anon->inputList();
      for(size_t i = 0; i < in_list->size(); i++)
      {
        switch(in_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!ctx->assigned.has(in_list->single(i)))
              || UNLIKELY(!ctx->deleted.has(in_list->single(i))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_inputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !ctx->assigned.hasAll(range->first(), range->last()))
                || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_inputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return false;
        }
        }
      }

      wtk::index_t num_outputs = 0;
      wtk::WireList* const out_list = anon->outputList();
      for(size_t i = 0; i < out_list->size(); i++)
      {
        switch(out_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!ctx->assigned.insert(out_list->single(i)))
              || UNLIKELY(ctx->deleted.has(out_list->single(i))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_outputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !ctx->assigned.insert(range->first(), range->last()))
                || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_outputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return false;
        }
        }
      }

      HardUnrollCtx sub_ctx;
      sub_ctx.exprBuilder = ctx->exprBuilder;
      sub_ctx.noDelete = num_outputs + num_inputs;

      if(num_inputs > 0)
      {
        sub_ctx.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
      }

      if(!this->checkHardUnrollIteration(anon->body(), &sub_ctx))
      {
        return false;
      }

      if(UNLIKELY(
          UNLIKELY(sub_ctx.instanceCount != anon->instanceCount())
          || UNLIKELY(sub_ctx.witnessCount != anon->shortWitnessCount())
          || UNLIKELY(num_outputs != 0
            && !sub_ctx.assigned.hasAll(0, num_outputs - 1))))
      {
        log_error("Hard unroll failure");
        return false;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      wtk::ForLoop<Number_T>* const for_loop = body->forLoop(i);

      wtk::IterExprWireList* output_exprs_list;
      wtk::IterExprWireList* input_exprs_list;
      wtk::DirectiveList<Number_T>* loop_dir_list;
      bool is_invoke;
      wtk::index_t expected_output;
      wtk::index_t expected_input;
      wtk::index_t expected_instance;
      wtk::index_t expected_witness;

      switch(for_loop->bodyType())
      {
      case wtk::ForLoop<Number_T>::INVOKE:
      {
        wtk::IterExprFunctionInvoke* const invoke = for_loop->invokeBody();

        output_exprs_list = invoke->outputList();
        input_exprs_list = invoke->inputList();

        std::string name(invoke->name());
        auto finder = this->functionDeclareMap.find(name);
        if(finder == this->functionDeclareMap.end())
        {
          log_error("no such function: %s", name.c_str());
          return false;
        }

        wtk::FunctionDeclare<Number_T>* declare = finder->second;
        loop_dir_list = declare->body();
        is_invoke = true;
        expected_output = declare->outputCount();
        expected_input = declare->inputCount();
        expected_instance = declare->instanceCount();
        expected_witness = declare->shortWitnessCount();
        break;
      }
      case wtk::ForLoop<Number_T>::ANONYMOUS:
      {
        wtk::IterExprAnonFunction<Number_T>* const anon =
          for_loop->anonymousBody();

        output_exprs_list = anon->outputList();
        input_exprs_list = anon->inputList();
        loop_dir_list = anon->body();
        is_invoke = false;
        expected_output = 0;
        expected_input = 0;
        expected_instance = anon->instanceCount();
        expected_witness = anon->shortWitnessCount();
        break;
      }
      default:
      {
        log_error("unreachable case");
        return false;
      }
      }

      std::vector<std::pair<Expr, Expr>> output_exprs(
          output_exprs_list->size());
      std::vector<std::pair<Expr, Expr>> input_exprs(
          input_exprs_list->size());
      wtk::index_t current;
      if(UNLIKELY(!ctx->exprBuilder->push(for_loop->iterName(), &current)))
      {
        log_error("duplicate iterator: %s", for_loop->iterName());
        return false;
      }

      for(size_t i = 0; i < output_exprs_list->size(); i++)
      {
        switch(output_exprs_list->type(i))
        {
        case wtk::IterExprWireList::SINGLE:
        {
          if(UNLIKELY(UNLIKELY(!ctx->exprBuilder->build(
                    output_exprs_list->single(i), &output_exprs[i].first))
                || UNLIKELY(!ctx->exprBuilder->build(
                    output_exprs_list->single(i), &output_exprs[i].second))))
          {
            log_error("Hard unroll failure");
            return false;
          }
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* const range = output_exprs_list->range(i);
          if(UNLIKELY(UNLIKELY(!ctx->exprBuilder->build(
                    range->first(), &output_exprs[i].first))
                || UNLIKELY(!ctx->exprBuilder->build(
                    range->last(), &output_exprs[i].second))))
          {
            log_error("Hard unroll failure");
            return false;
          }
          break;
        }
        }
      }

      for(size_t i = 0; i < input_exprs_list->size(); i++)
      {
        switch(input_exprs_list->type(i))
        {
        case wtk::IterExprWireList::SINGLE:
        {
          if(UNLIKELY(UNLIKELY(!ctx->exprBuilder->build(
                    input_exprs_list->single(i), &input_exprs[i].first))
                || UNLIKELY(!ctx->exprBuilder->build(
                    input_exprs_list->single(i), &input_exprs[i].second))))
          {
            log_error("Hard unroll failure");
            return false;
          }
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* const range = input_exprs_list->range(i);
          if(UNLIKELY(UNLIKELY(!ctx->exprBuilder->build(
                    range->first(), &input_exprs[i].first))
                || UNLIKELY(!ctx->exprBuilder->build(
                    range->last(), &input_exprs[i].second))))
          {
            log_error("Hard unroll failure");
            return false;
          }
          break;
        }
        }
      }

      wtk::utils::SkipList<wtk::index_t> loop_outputs;
      if(UNLIKELY(!wtk::utils::listOutputWires(
              for_loop->outputList(), &loop_outputs)))
      {
        log_error("Hard unroll failure");
        return false;
      }

      if(UNLIKELY(for_loop->first() > for_loop->last()))
      {
        log_error("Hard unroll failure");
        return false;
      }

      wtk::utils::SkipList<wtk::index_t> iter_outputs;
      for(current = for_loop->first(); current <= for_loop->last(); current++)
      {
        wtk::index_t num_inputs = 0;
        for(size_t i = 0; i < input_exprs.size(); i++)
        {
          wtk::index_t first = input_exprs[i].first.eval(this->exprStack);
          wtk::index_t second = input_exprs[i].second.eval(this->exprStack);

          if(UNLIKELY(!ctx->assigned.hasAll(first, second))
              || UNLIKELY(ctx->deleted.has(first, second)))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_inputs += 1 + second - first;
        }

        wtk::index_t num_outputs = 0;
        for(size_t i = 0; i < output_exprs.size(); i++)
        {
          wtk::index_t first = output_exprs[i].first.eval(this->exprStack);
          wtk::index_t second = output_exprs[i].second.eval(this->exprStack);

          if(UNLIKELY(UNLIKELY(!ctx->assigned.insert(first, second))
                || UNLIKELY(ctx->deleted.has(first, second))
                || UNLIKELY(!iter_outputs.insert(first, second))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          num_outputs += 1 + second - first;
        }

        if(is_invoke && UNLIKELY(UNLIKELY(num_outputs != expected_output)
              || UNLIKELY(num_inputs != expected_input)))
        {
          log_error("Hard unroll failure");
          return false;
        }

        ExprBuilder func_expr_builder;
        HardUnrollCtx sub_ctx;
        sub_ctx.exprBuilder = 
          is_invoke ? &func_expr_builder : ctx->exprBuilder;
        sub_ctx.noDelete = num_outputs + num_inputs;

        if(num_inputs > 0)
        {
          sub_ctx.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
        }

        if(!this->checkHardUnrollIteration(loop_dir_list, &sub_ctx))
        {
          return false;
        }

        if(UNLIKELY(UNLIKELY(sub_ctx.instanceCount != expected_instance)
            || UNLIKELY(sub_ctx.witnessCount != expected_witness)
            || UNLIKELY(num_outputs != 0
              && !sub_ctx.assigned.hasAll(0, num_outputs - 1))))
        {
          log_error("Hard unroll failure");
          return false;
        }
        ctx->instanceCount += sub_ctx.instanceCount;
        ctx->witnessCount += sub_ctx.witnessCount;
      }

      if(UNLIKELY(!wtk::utils::SkipList<wtk::index_t>::equivalent(
            &loop_outputs, &iter_outputs)))
      {
        log_error("Hard unroll failure");
        return false;
      }

      ctx->exprBuilder->pop();
      break;
    }
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      wtk::SwitchStatement<Number_T>* const switch_stmt =
        body->switchStatement(i);

      wtk::index_t num_output =
        wtk::utils::countWireList(switch_stmt->outputList());

      wtk::index_t num_instance = 0;
      wtk::index_t num_witness = 0;
      if(UNLIKELY(!wtk::utils::maxInsWit<Number_T>(switch_stmt,
              &this->functionDeclareMap, &num_instance, &num_witness)))
      {
        log_error("Hard unroll failure");
        return false;
      }

      // Get the condition wire
      if(UNLIKELY(!ctx->assigned.has(switch_stmt->condition()))
          || UNLIKELY(ctx->deleted.has(switch_stmt->condition())))
      {
        log_error("Hard unroll failure");
        return false;
      }

      // run all the cases
      wtk::utils::SkipList<Number_T> cases;

      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        wtk::CaseBlock<Number_T>* const case_blk = switch_stmt->caseBlock(j);
        if(UNLIKELY(!cases.insert(case_blk->match()))
            || UNLIKELY(case_blk->match() >= this->characteristic))
        {
          log_error("bad case <%s>",
              wtk::utils::dec(case_blk->match()).c_str());
          return false;
        }

        wtk::DirectiveList<Number_T>* case_body;
        wtk::WireList* input_list;
        bool is_invoke;
        wtk::index_t expected_output;
        wtk::index_t expected_input;
        wtk::index_t expected_instance;
        wtk::index_t expected_witness;

        switch(case_blk->bodyType())
        {
        case wtk::CaseBlock<Number_T>::INVOKE:
        {
          wtk::CaseFunctionInvoke* const invoke = case_blk->invokeBody();

          std::string func_name(invoke->name());
          auto finder = this->functionDeclareMap.find(func_name);
          if(UNLIKELY(finder == this->functionDeclareMap.end()))
          {
            log_error("no such function: %s", invoke->name());
            return false;
          }

          wtk::FunctionDeclare<Number_T>* const declare = finder->second;

          case_body = declare->body();
          input_list = invoke->inputList();
          is_invoke = true;
          expected_output = declare->outputCount();
          expected_input = declare->inputCount();
          expected_instance = declare->instanceCount();
          expected_witness = declare->shortWitnessCount();
          break;
        }
        case wtk::CaseBlock<Number_T>::ANONYMOUS:
        {
          wtk::CaseAnonFunction<Number_T>* const anon =
            case_blk->anonymousBody();

          case_body = anon->body();
          input_list = anon->inputList();
          is_invoke = false;
          expected_output = 0;
          expected_input = 0;
          expected_instance = anon->instanceCount();
          expected_witness = anon->shortWitnessCount();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return false;
        }
        }

        ExprBuilder invoke_expr_builder;
        HardUnrollCtx sub_ctx;
        sub_ctx.exprBuilder =
          is_invoke ? &invoke_expr_builder : ctx->exprBuilder;

        wtk::index_t num_input = 0;
        for(size_t i = 0; i < input_list->size(); i++)
        {
          switch(input_list->type(i))
          {
          case wtk::WireList::SINGLE:
          {
            if(UNLIKELY(!ctx->assigned.has(input_list->single(i)))
                || UNLIKELY(ctx->deleted.has(input_list->single(i))))
            {
              log_error("Hard unroll failure");
              return false;
            }

            num_input += 1;
            break;
          }
          case wtk::WireList::RANGE:
          {
            wtk::WireRange* const range = input_list->range(i);
            if(UNLIKELY(UNLIKELY(range->first() > range->last())
                  || UNLIKELY(!ctx->assigned.has(range->first(), range->last()))
                  || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
            {
              log_error("Hard unroll failure");
              return false;
            }

            num_input += 1 + range->last() - range->first();
            break;
          }
          default:
          {
            log_error("unreachable case");
            return false;
          }
          }
        }

        sub_ctx.noDelete = num_output + num_input;

        if(is_invoke && UNLIKELY(UNLIKELY(num_output != expected_output)
              || UNLIKELY(num_input != expected_input)))
        {
          log_error("Hard unroll failure");
          return false;
        }

        if(num_input != 0)
        {
          sub_ctx.assigned.insert(num_output, num_output + num_input - 1);
        }

        if(!this->checkHardUnrollIteration(case_body, &sub_ctx))
        {
          return false;
        }

        if(UNLIKELY(UNLIKELY(sub_ctx.instanceCount != expected_instance)
              || UNLIKELY(sub_ctx.witnessCount != expected_witness)
              || UNLIKELY(num_output != 0
              && !sub_ctx.assigned.hasAll(0, num_output - 1))))
        {
          log_error("Hard unroll failure");
          return false;
        }

      }

      ctx->instanceCount += num_instance;
      ctx->witnessCount += num_witness;

      // add up all the dummy output values into the actual outputs
      wtk::WireList* const output_list = switch_stmt->outputList();
      for(size_t i = 0; i < output_list->size(); i++)
      {
        switch(output_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!ctx->assigned.insert(output_list->single(i)))
              || UNLIKELY(ctx->deleted.has(output_list->single(i))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = output_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last())
                || UNLIKELY(
                  !ctx->assigned.insert(range->first(), range->last()))
                || UNLIKELY(ctx->deleted.has(range->first(), range->last()))))
          {
            log_error("Hard unroll failure");
            return false;
          }

          break;
        }
        default:
        {
          log_error("unreachable case");
          return false;
        }
        }
      }

      break;
    }
    default:
    {
      log_error("unhandled directive");
      return false;
    }
    }
  }

  return true;
}

} } // namespace wtk::bolt
