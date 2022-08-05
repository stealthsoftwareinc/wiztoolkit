/**
 * Copyright (C) 2022 Stealth Software Technolgies, Inc.
 */

namespace wtk {
namespace bolt {

/**
 * Because C++ random-access-iterators can't be offset by a size_t
 */
template<typename Iter_T, typename Size_T>
Iter_T iter_offset(Iter_T it, Size_T sz)
{
  std::advance(it, sz);
  return it;
}


template<typename Wire_T, typename Number_T>
bool PLASMASnoozeWireSet<Wire_T, Number_T>::isOutput(wtk::index_t const idx)
{
  return idx < this->outputSize;
}

template<typename Wire_T, typename Number_T>
bool PLASMASnoozeWireSet<Wire_T, Number_T>::isInput(wtk::index_t const idx)
{
  return idx >= this->outputSize && idx < this->outputSize + this->inputSize;
}

template<typename Wire_T, typename Number_T>
bool PLASMASnoozeWireSet<Wire_T, Number_T>::integrityCheck()
{
  size_t range_idx = 0;

  if(this->ranges.size() > 0)
  {
    if(this->outputSize != 0)
    {
      while(range_idx < this->ranges.size()
          && this->ranges[range_idx]->last < this->outputSize)
      {
        if(range_idx == 0)
        {
          if(this->ranges[range_idx]->first != 0)
          {
            log_error("integrity check fail: first output");
            return false;
          }
        }
        else if(this->ranges[range_idx]->first
            != this->ranges[range_idx - 1]->last + 1)
        {
          log_error("integrity check fail: mid output");
          return false;
        }

        range_idx++;
      }
    }

    bool first_input = true;
    if(this->inputSize != 0)
    {
      while(range_idx < this->ranges.size()
          && this->ranges[range_idx]->last < this->outputSize + this->inputSize)
      {
        if(range_idx == 0)
        {
          if(this->ranges[range_idx]->first != 0)
          {
            log_error("integrity check fail: zeroth input");
            return false;
          }
        }
        else if(first_input)
        {
          if(this->ranges[range_idx]->first != this->outputSize)
          {
            log_error("integrity check fail: first input");
            return false;
          }
        }
        else if(this->ranges[range_idx]->first
            != this->ranges[range_idx - 1]->last + 1)
        {
          log_error("integrity check fail: mid input");
          return false;
        }

        range_idx++;
        first_input = false;
      }
    }

    while(range_idx < this->ranges.size())
    {
      if(this->ranges[range_idx]->first < this->outputSize + this->inputSize)
      {
        log_error("integrity check fail: locals in remap");
        return false;
      }

      if(range_idx != 0 && this->ranges[range_idx]->first
          <= this->ranges[range_idx - 1]->last)
      {
        log_error("integrity check error: local");
        return false;
      }

      range_idx++;
    }
  }

  return true;
}

template<typename Wire_T, typename Number_T>
Wire_T const* PLASMASnoozeWireSet<Wire_T, Number_T>::retrieve(
    wtk::index_t const idx)
{
  log_assert(this->integrityCheck());

  WireRange<Wire_T, Number_T>* const range = this->findRange(idx);

  if(LIKELY(LIKELY(idx >= range->first) && LIKELY(idx <= range->last)))
  {
    return range->deref(idx);
  }
  else
  {
    return nullptr;
  }
}

wtk::index_t constexpr GROWTH_THRESHOLD = 1024;

template<typename Wire_T, typename Number_T>
Wire_T* PLASMASnoozeWireSet<Wire_T, Number_T>::insert(wtk::index_t const idx)
{
  log_assert(this->integrityCheck());

  if(UNLIKELY(this->ranges.size() == 0))
  {
    LocalWireRange<Wire_T, Number_T>* range =
      this->pool.localWireRange.allocate(1, idx, idx + GROWTH_THRESHOLD);
    this->ranges.push_back(range);

    log_assert(this->integrityCheck());
    return range->deref(idx);
  }
  else
  {
    size_t range_idx = this->findRangeIdx(idx);

    if(range_idx >= this->ranges.size())
    {
      log_assert(!this->isOutput(idx) && !this->isInput(idx));

      LocalWireRange<Wire_T, Number_T>* range =
        this->pool.localWireRange.allocate(1, idx, idx + GROWTH_THRESHOLD);
      this->ranges.push_back(range);

      log_assert(this->integrityCheck());
      return range->deref(idx);
    }
    else if(idx > this->ranges[range_idx]->last + GROWTH_THRESHOLD
        || (idx > this-> ranges[range_idx]->last
          && (this->isInput(this->ranges[range_idx]->last)
            || this->isOutput(this->ranges[range_idx]->last))))
    {
      log_assert(!this->isOutput(idx) && !this->isInput(idx));

      wtk::index_t adj_last = (range_idx < this->ranges.size() - 1
        && idx + GROWTH_THRESHOLD >= this->ranges[range_idx + 1]->first)
          ? this->ranges[range_idx + 1]->first - 1
          : idx + GROWTH_THRESHOLD;

      LocalWireRange<Wire_T, Number_T>* range =
        this->pool.localWireRange.allocate(1, idx, adj_last);
      this->ranges.insert(
          iter_offset(this->ranges.begin(), range_idx + 1), range);

      log_assert(this->integrityCheck());
      return range->deref(idx);
    }
    else if(idx > this->ranges[range_idx]->last)
    {
      log_assert(!this->isOutput(idx) && !this->isInput(idx));

      LocalWireRange<Wire_T, Number_T>* range =
        (LocalWireRange<Wire_T, Number_T>*) this->ranges[range_idx];

      wtk::index_t adj_last = (range_idx < this->ranges.size() - 1
        && range->last + GROWTH_THRESHOLD >= this->ranges[range_idx + 1]->first)
          ? this->ranges[range_idx + 1]->first - 1
          : range->last + GROWTH_THRESHOLD;

      range->wires.resize((size_t) (range->wires.size() + GROWTH_THRESHOLD));
      range->setRange(range->first, adj_last);

      log_assert(this->integrityCheck());
      return range->deref(idx);
    }
    else if(idx < this->ranges[range_idx]->first)
    {
      log_assert(!this->isOutput(idx) && !this->isInput(idx));

      wtk::index_t adj_last =
        idx + GROWTH_THRESHOLD < this->ranges[range_idx]->first
          ? idx + GROWTH_THRESHOLD
          : this->ranges[range_idx]->first - 1;
      LocalWireRange<Wire_T, Number_T>* range =
        this->pool.localWireRange.allocate(1, idx, adj_last);

      this->ranges.insert(
          iter_offset(this->ranges.begin(), range_idx), range);

      log_assert(this->integrityCheck());
      return range->deref(idx);
    }
    else // if(idx >= this->ranges[range_idx]->first
         //    && idx <= this->ranges[range_idx]->last)
    {
      log_assert(this->integrityCheck());
      return this->ranges[range_idx]->deref(idx);
    }
  }
}

template<typename Wire_T, typename Number_T>
void PLASMASnoozeWireSet<Wire_T, Number_T>::remapInput(
    wtk::index_t const idx,
    PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope)
{
  this->remapInputs(idx, idx, subscope);
}

template<typename Wire_T, typename Number_T>
void PLASMASnoozeWireSet<Wire_T, Number_T>::remapInputs(
    wtk::index_t const first, wtk::index_t const last,
    PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope)
{
  log_assert(this->integrityCheck() && subscope->integrityCheck());
  log_assert(this->ranges.size() != 0);

  size_t range_idx = this->findRangeIdx(first);
  wtk::index_t adj_first = first;
  wtk::index_t adj_last = last;

  do
  {
    log_assert(range_idx < this->ranges.size());

    WireRange<Wire_T, Number_T>* range = this->ranges[range_idx];
    adj_last = last < range->last ? last : range->last;

    WireRange<Wire_T, Number_T>* subrange =
      range->ref(adj_first, adj_last, &subscope->pool);

    subrange->setRange(subscope->outputSize + subscope->inputSize,
        subscope->outputSize + subscope->inputSize + adj_last - adj_first);
    subscope->inputSize += 1 + adj_last - adj_first;
    subscope->ranges.push_back(subrange);

    range_idx++;
    adj_first = adj_last + 1;
  } while(adj_last < last);

  log_assert(this->integrityCheck() && subscope->integrityCheck());
}

template<typename Wire_T, typename Number_T>
void PLASMASnoozeWireSet<Wire_T, Number_T>::remapOutput(
    wtk::index_t const idx,
    PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope)
{
  this->remapOutputs(idx, idx, subscope);
}

template<typename Wire_T, typename Number_T>
void PLASMASnoozeWireSet<Wire_T, Number_T>::remapOutputs(
    wtk::index_t const first, wtk::index_t const last,
    PLASMASnoozeWireSet<Wire_T, Number_T>* const subscope)
{
  log_assert(this->integrityCheck() && subscope->integrityCheck());

  if(this->ranges.size() == 0)
  {
    LocalWireRange<Wire_T, Number_T>* range =
      this->pool.localWireRange.allocate(1, first, last);
    this->ranges.push_back(range);

    WireRange<Wire_T, Number_T>* subrange = 
      range->ref(first, last, &subscope->pool);
    subrange->setRange(
        subscope->outputSize, subscope->outputSize + last - first);
    subscope->outputSize += 1 + last - first;
    subscope->ranges.push_back(subrange);
  }
  else
  {
    size_t range_idx = this->findRangeIdx(first);
    wtk::index_t adj_first = first;
    wtk::index_t adj_last = last;

    do
    {
      WireRange<Wire_T, Number_T>* subrange;

      if(this->isOutput(adj_first))
      {
        log_assert(range_idx < this->ranges.size());

        WireRange<Wire_T, Number_T>* range = this->ranges[range_idx];

        adj_last = last < range->last ? last : range->last;
        subrange = range->ref(adj_first, adj_last, &subscope->pool);
      }
      else if(UNLIKELY(this->isInput(adj_first)))
      {
        log_fatal("unreachable case");
      }
      else if(range_idx >= this->ranges.size())
      {
        adj_last = last;
        LocalWireRange<Wire_T, Number_T>* range =
          this->pool.localWireRange.allocate(1, adj_first, adj_last);

        this->ranges.push_back(range);

        subrange = range->ref(adj_first, adj_last, &subscope->pool);
      }
      else if(adj_first > this->ranges[range_idx]->last + GROWTH_THRESHOLD
          || (adj_first > this->ranges[range_idx]->last
            && (this->isInput(this->ranges[range_idx]->last)
              || this->isOutput(this->ranges[range_idx]->last))))
      {
        adj_last = (range_idx == this->ranges.size() - 1
            || last < this->ranges[range_idx + 1]->first)
          ? last : this->ranges[range_idx + 1]->first - 1;

        LocalWireRange<Wire_T, Number_T>* range =
          this->pool.localWireRange.allocate(1, adj_first, adj_last);
        this->ranges.insert(
            iter_offset(this->ranges.begin(), range_idx + 1), range);

        subrange = range->ref(adj_first, adj_last, &subscope->pool);
      }
      else if(adj_first < this->ranges[range_idx]->first)
      {
        wtk::index_t adj_last = last < this->ranges[range_idx]->first
            ? last
            : this->ranges[range_idx]->first - 1;

        LocalWireRange<Wire_T, Number_T>* range =
          this->pool.localWireRange.allocate(1, adj_first, adj_last);
        this->ranges.insert(
            iter_offset(this->ranges.begin(), range_idx), range);

        subrange = range->ref(adj_first, adj_last, &subscope->pool);
      }
      else // if(adj_first >= this->ranges[range_idx]->first)
      {
        LocalWireRange<Wire_T, Number_T>* range =
          (LocalWireRange<Wire_T, Number_T>*) this->ranges[range_idx];

        wtk::index_t adj_last = (range_idx < this->ranges.size() - 1
          && last >= this->ranges[range_idx + 1]->first)
            ? this->ranges[range_idx + 1]->first - 1
            : last;

        if(adj_last > range->last)
        {
          range->setRange(range->first, adj_last);
          range->wires.resize(1 + (size_t) (range->last - range->first));
        }

        subrange = range->ref(adj_first, adj_last, &subscope->pool);
      }

      subrange->setRange(
          subscope->outputSize, subscope->outputSize + adj_last - adj_first);
      subscope->outputSize += 1 + adj_last - adj_first;
      subscope->ranges.push_back(subrange);

      range_idx++;
      adj_first = adj_last + 1;
    } while(adj_last < last);
  }

  log_assert(this->integrityCheck() && subscope->integrityCheck());
}

template<typename Wire_T, typename Number_T>
void PLASMASnoozeWireSet<Wire_T, Number_T>::mapDummies(
    LocalWireRange<Wire_T, Number_T>* const dummies)
{
  log_assert(this->ranges.size() == 0);
  log_assert(dummies->first == 0);
  this->ranges.push_back(dummies);
  this->outputSize = dummies->last + 1;
}

template<typename Wire_T, typename Number_T>
PLASMASnoozeStatus PLASMASnooze<Wire_T, Number_T>::evaluate(
    wtk::IRTree<Number_T>* const rel_tree,
    wtk::InputStream<Number_T>* const ins_stream,
    wtk::InputStream<Number_T>* const wit_stream)
{
  for(size_t i = 0; i < rel_tree->size(); i++)
  {
    std::string name(rel_tree->functionDeclare(i)->name());
    if(UNLIKELY(this->functions.find(name) != this->functions.end()))
    {
      log_error("duplicate function: %s", name.c_str());
      return PLASMASnoozeStatus::bad_relation;
    }

    this->functions[name] = rel_tree->functionDeclare(i);
  }

  ExprBuilder main_expr_builder;
  PLASMASnoozeState<Wire_T, Number_T> main_state(
      ins_stream, nullptr,
      wit_stream, nullptr,
      &main_expr_builder, nullptr);

  PLASMASnoozeStatus status = this->evaluate(rel_tree->body(), &main_state);

  Number_T dummy;
  if(wit_stream != nullptr && wit_stream->next(&dummy) != StreamStatus::end)
  {
    log_error("leftover witness values");
    return PLASMASnoozeStatus::bad_stream;
  }
  if(ins_stream->next(&dummy) != StreamStatus::end)
  {
    log_error("leftover instance values");
    return PLASMASnoozeStatus::bad_stream;
  }

  return status;
}

template<typename Wire_T, typename Number_T>
PLASMASnoozeStatus PLASMASnooze<Wire_T, Number_T>::evaluate(
    wtk::DirectiveList<Number_T>* const dir_list,
    PLASMASnoozeState<Wire_T, Number_T>* const state)
{
  for(size_t i = 0; i < dir_list->size(); i++)
  {
    switch(dir_list->type(i))
    {
    case wtk::DirectiveList<Number_T>::BINARY_GATE:
    {
      wtk::BinaryGate* const gate = dir_list->binaryGate(i);

      if(UNLIKELY(UNLIKELY(!state->assigned.has(gate->leftWire()))
              || UNLIKELY(!state->assigned.has(gate->rightWire()))
              || UNLIKELY(!state->assigned.insert(gate->outputWire()))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T* out = state->wires.insert(gate->outputWire());
      Wire_T const* left = state->wires.retrieve(gate->leftWire());
      Wire_T const* right = state->wires.retrieve(gate->rightWire());

      log_assert(out != nullptr && left != nullptr && right != nullptr);

      switch(gate->calculation())
      {
      case wtk::BinaryGate::AND:
      {
        this->backend->andGate(out, left, right);
        break;
      }
      case wtk::BinaryGate::XOR:
      {
        this->backend->xorGate(out, left, right);
        break;
      }
      case wtk::BinaryGate::ADD:
      {
        this->backend->addGate(out, left, right);
        break;
      }
      case wtk::BinaryGate::MUL:
      {
        this->backend->mulGate(out, left, right);
        break;
      }
      default:
      {
        log_error("unreachable case");
        return PLASMASnoozeStatus::bad_relation;
      }
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::UNARY_GATE:
    {
      wtk::UnaryGate* const gate = dir_list->unaryGate(i);

      if(UNLIKELY(UNLIKELY(!state->assigned.has(gate->inputWire()))
              || UNLIKELY(!state->assigned.insert(gate->outputWire()))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T* out = state->wires.insert(gate->outputWire());
      Wire_T const* left = state->wires.retrieve(gate->inputWire());

      log_assert(out != nullptr && left != nullptr);

      switch(gate->calculation())
      {
      case wtk::UnaryGate::NOT:
      {
        this->backend->notGate(out, left);
        break;
      }
      case wtk::UnaryGate::COPY:
      {
        this->backend->copy(out, left);
        break;
      }
      default:
      {
        log_error("unreachable case");
        return PLASMASnoozeStatus::bad_relation;
      }
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      wtk::BinaryConstGate<Number_T>* const gate = dir_list->binaryConstGate(i);

      if(UNLIKELY(UNLIKELY(!state->assigned.has(gate->leftWire()))
              || UNLIKELY(gate->rightValue() >= this->backend->prime)
              || UNLIKELY(!state->assigned.insert(gate->outputWire()))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T* out = state->wires.insert(gate->outputWire());
      Wire_T const* left = state->wires.retrieve(gate->leftWire());
      Number_T const right = gate->rightValue();

      log_assert(out != nullptr && left != nullptr);

      switch(gate->calculation())
      {
      case wtk::BinaryConstGate<Number_T>::ADDC:
      {
        this->backend->addcGate(out, left, right);
        break;
      }
      case wtk::BinaryConstGate<Number_T>::MULC:
      {
        this->backend->mulcGate(out, left, right);
        break;
      }
      default:
      {
        log_error("unreachable case");
        return PLASMASnoozeStatus::bad_relation;
      }
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::INPUT:
    {
      wtk::Input* const input = dir_list->input(i);

      if(UNLIKELY(!state->assigned.insert(input->outputWire())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T* out = state->wires.insert(input->outputWire());

      log_assert(out != nullptr);

      switch(input->stream())
      {
      case wtk::Input::INSTANCE:
      {
        state->instanceCount++;

        if(state->switchInstance != nullptr)
        {
          this->backend->copy(out, state->switchInstance->next());
        }
        else
        {
          Number_T num;
          if(UNLIKELY(UNLIKELY(
                  wtk::StreamStatus::success != state->instance->next(&num))
                || UNLIKELY(num >= this->backend->prime)))
          {
            log_error("bad instance");
            return PLASMASnoozeStatus::bad_stream;
          }
          else
          {
            this->backend->instance(out, num);
          }
        }

        break;
      }
      case wtk::Input::SHORT_WITNESS:
      {
        state->witnessCount++;

        if(state->switchWitness != nullptr)
        {
          this->backend->copy(out, state->switchWitness->next());
        }
        else if(state->witness != nullptr)
        {
          Number_T num;
          if(UNLIKELY(UNLIKELY(
                  wtk::StreamStatus::success != state->witness->next(&num))
                || UNLIKELY(num >= this->backend->prime)))
          {
            log_error("bad witness");
            return PLASMASnoozeStatus::bad_stream;
          }
          else
          {
            this->backend->witness(out, num);
          }
        }
        else
        {
          Number_T zero(0);
          this->backend->witness(out, zero);
        }

        break;
      }
      default:
      {
        log_error("unreachable case");
        return PLASMASnoozeStatus::bad_relation;
      }
      }
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSIGN:
    {
      wtk::Assign<Number_T>* const assign = dir_list->assign(i);

      if(UNLIKELY(UNLIKELY(assign->constValue() >= this->backend->prime)
            || UNLIKELY(!state->assigned.insert(assign->outputWire()))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T* out = state->wires.insert(assign->outputWire());
      Number_T const right = assign->constValue();

      log_assert(out != nullptr);

      this->backend->assign(out, right);
      break;
    }
    case wtk::DirectiveList<Number_T>::ASSERT_ZERO:
    {
      wtk::Terminal* const assert_zero = dir_list->assertZero(i);

      if(UNLIKELY(!state->assigned.has(assert_zero->wire())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T const* left = state->wires.retrieve(assert_zero->wire());

      log_assert(left != nullptr);

      Wire_T const* assert_wire_ptr = left;
      Wire_T assert_wire;
      if(state->enabledBit != nullptr)
      {
        if(this->backend->isBoolean)
        {
          this->backend->andGate(
              &assert_wire, assert_wire_ptr, state->enabledBit);
        }
        else
        {
          this->backend->mulGate(
              &assert_wire, assert_wire_ptr, state->enabledBit);
        }

        assert_wire_ptr = &assert_wire;
      }

      this->backend->assertZero(assert_wire_ptr);
      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_SINGLE:
    {
      wtk::Terminal* const delete_single = dir_list->deleteSingle(i);

      if(UNLIKELY(!state->assigned.remove(delete_single->wire())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::DELETE_RANGE:
    {
      wtk::WireRange* const delete_range = dir_list->deleteRange(i);

      if(UNLIKELY(!state->assigned.remove(
              delete_range->first(), delete_range->last())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      wtk::FunctionInvoke* const invoke = dir_list->functionInvoke(i);

      std::string func_name(invoke->name());
      auto finder = this->functions.find(func_name);
      if(finder == this->functions.end())
      {
        log_error("no such function: %s", func_name.c_str());
        return PLASMASnoozeStatus::bad_relation;
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
          if(UNLIKELY(!state->assigned.has(in_list->single(i))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_inputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !state->assigned.hasAll(range->first(), range->last()))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_inputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
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
          if(UNLIKELY(!state->assigned.insert(out_list->single(i))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_outputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !state->assigned.insert(range->first(), range->last()))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_outputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      if(UNLIKELY(UNLIKELY(num_inputs != declare->inputCount())
            || UNLIKELY(num_outputs != declare->outputCount())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      ExprBuilder func_expr_builder;
      PLASMASnoozeState<Wire_T, Number_T> func_state(
          state->instance, state->switchInstance,
          state->witness, state->switchWitness,
          &func_expr_builder, state->enabledBit);
      for(size_t i = 0; i < out_list->size(); i++)
      {
        switch(out_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          state->wires.remapOutput(out_list->single(i), &func_state.wires);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          state->wires.remapOutputs(
              range->first(), range->last(), &func_state.wires);

          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      for(size_t i = 0; i < in_list->size(); i++)
      {
        switch(in_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          state->wires.remapInput(in_list->single(i), &func_state.wires);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          state->wires.remapInputs(
              range->first(), range->last(), &func_state.wires);
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      if(num_inputs > 0)
      {
        func_state.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
      }

      PLASMASnoozeStatus const sub_status =
        this->evaluate(declare->body(), &func_state);
      if(UNLIKELY(sub_status != PLASMASnoozeStatus::well_formed))
      {
        return sub_status;
      }

      if(UNLIKELY(
          UNLIKELY(func_state.instanceCount != declare->instanceCount())
          || UNLIKELY(func_state.witnessCount != declare->shortWitnessCount())
          || UNLIKELY(num_outputs != 0
            && !func_state.assigned.hasAll(0, num_outputs - 1))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::ANON_FUNCTION:
    {
      wtk::AnonFunction<Number_T>* const anon = dir_list->anonFunction(i);

      wtk::index_t num_inputs = 0;
      wtk::WireList* const in_list = anon->inputList();
      for(size_t i = 0; i < in_list->size(); i++)
      {
        switch(in_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!state->assigned.has(in_list->single(i))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_inputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !state->assigned.hasAll(range->first(), range->last()))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_inputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
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
          if(UNLIKELY(!state->assigned.insert(out_list->single(i))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_outputs++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last()) || UNLIKELY(
                  !state->assigned.insert(range->first(), range->last()))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_outputs += 1 + range->last() - range->first();
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      PLASMASnoozeState<Wire_T, Number_T> func_state(
          state->instance, state->switchInstance,
          state->witness, state->switchWitness,
          state->exprBuilder, state->enabledBit);
      for(size_t i = 0; i < out_list->size(); i++)
      {
        switch(out_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          state->wires.remapOutput(out_list->single(i), &func_state.wires);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = out_list->range(i);
          state->wires.remapOutputs(
              range->first(), range->last(), &func_state.wires);

          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      for(size_t i = 0; i < in_list->size(); i++)
      {
        switch(in_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          state->wires.remapInput(in_list->single(i), &func_state.wires);

          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = in_list->range(i);
          state->wires.remapInputs(
              range->first(), range->last(), &func_state.wires);
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      if(num_inputs > 0)
      {
        func_state.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
      }

      PLASMASnoozeStatus const sub_status =
        this->evaluate(anon->body(), &func_state);
      if(UNLIKELY(sub_status != PLASMASnoozeStatus::well_formed))
      {
        return sub_status;
      }

      if(UNLIKELY(
          UNLIKELY(func_state.instanceCount != anon->instanceCount())
          || UNLIKELY(func_state.witnessCount != anon->shortWitnessCount())
          || UNLIKELY(
            num_outputs != 0
            && !func_state.assigned.hasAll(0, num_outputs - 1))))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      break;
    }
    case wtk::DirectiveList<Number_T>::FOR_LOOP:
    {
      wtk::ForLoop<Number_T>* const for_loop = dir_list->forLoop(i);

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
        auto finder = this->functions.find(name);
        if(finder == this->functions.end())
        {
          log_error("no such function: %s", name.c_str());
          return PLASMASnoozeStatus::bad_relation;
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
        return PLASMASnoozeStatus::bad_relation;
      }
      }

      std::vector<std::pair<Expr, Expr>> output_exprs(
          output_exprs_list->size());
      std::vector<std::pair<Expr, Expr>> input_exprs(
          input_exprs_list->size());
      wtk::index_t current;
      if(UNLIKELY(!state->exprBuilder->push(for_loop->iterName(), &current)))
      {
        log_error("duplicate iterator: %s", for_loop->iterName());
        return PLASMASnoozeStatus::bad_relation;
      }

      for(size_t i = 0; i < output_exprs_list->size(); i++)
      {
        switch(output_exprs_list->type(i))
        {
        case wtk::IterExprWireList::SINGLE:
        {
          if(UNLIKELY(UNLIKELY(!state->exprBuilder->build(
                    output_exprs_list->single(i), &output_exprs[i].first))
                || UNLIKELY(!state->exprBuilder->build(
                    output_exprs_list->single(i), &output_exprs[i].second))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* const range = output_exprs_list->range(i);
          if(UNLIKELY(UNLIKELY(!state->exprBuilder->build(
                    range->first(), &output_exprs[i].first))
                || UNLIKELY(!state->exprBuilder->build(
                    range->last(), &output_exprs[i].second))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
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
          if(UNLIKELY(UNLIKELY(!state->exprBuilder->build(
                    input_exprs_list->single(i), &input_exprs[i].first))
                || UNLIKELY(!state->exprBuilder->build(
                    input_exprs_list->single(i), &input_exprs[i].second))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }
          break;
        }
        case wtk::IterExprWireList::RANGE:
        {
          wtk::IterExprWireRange* const range = input_exprs_list->range(i);
          if(UNLIKELY(UNLIKELY(!state->exprBuilder->build(
                    range->first(), &input_exprs[i].first))
                || UNLIKELY(!state->exprBuilder->build(
                    range->last(), &input_exprs[i].second))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }
          break;
        }
        }
      }

      wtk::utils::SkipList<wtk::index_t> loop_outputs;
      if(UNLIKELY(!wtk::utils::listOutputWires(
              for_loop->outputList(), &loop_outputs)))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      if(UNLIKELY(for_loop->first() > for_loop->last()))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      wtk::utils::SkipList<wtk::index_t> iter_outputs;
      std::vector<std::pair<wtk::index_t, wtk::index_t>> output_ranges(
          output_exprs.size());
      std::vector<std::pair<wtk::index_t, wtk::index_t>> input_ranges(
          input_exprs.size());
      for(current = for_loop->first(); current <= for_loop->last(); current++)
      {
        wtk::index_t num_inputs = 0;
        for(size_t i = 0; i < input_exprs.size(); i++)
        {
          input_ranges[i].first = input_exprs[i].first.eval(this->exprStack);
          input_ranges[i].second = input_exprs[i].second.eval(this->exprStack);

          if(UNLIKELY(!state->assigned.hasAll(
                  input_ranges[i].first, input_ranges[i].second)))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_inputs += 1 + input_ranges[i].second - input_ranges[i].first;
        }

        wtk::index_t num_outputs = 0;
        for(size_t i = 0; i < output_exprs.size(); i++)
        {
          output_ranges[i].first = output_exprs[i].first.eval(this->exprStack);
          output_ranges[i].second =
            output_exprs[i].second.eval(this->exprStack);

          if(UNLIKELY(UNLIKELY(!state->assigned.insert(
                  output_ranges[i].first, output_ranges[i].second))
                || UNLIKELY(!iter_outputs.insert(
                  output_ranges[i].first, output_ranges[i].second))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          num_outputs += 1 + output_ranges[i].second - output_ranges[i].first;
        }

        if(is_invoke && UNLIKELY(UNLIKELY(num_outputs != expected_output)
              || UNLIKELY(num_inputs != expected_input)))
        {
          log_error("bad form");
          return PLASMASnoozeStatus::bad_relation;
        }

        ExprBuilder func_expr_builder;
        ExprBuilder* expr_builder =
          is_invoke ? &func_expr_builder : state->exprBuilder;
        PLASMASnoozeState<Wire_T, Number_T> sub_state(
            state->instance, state->switchInstance,
            state->witness, state->switchWitness,
            expr_builder, state->enabledBit);

        for(size_t i = 0; i < output_ranges.size(); i++)
        {
          state->wires.remapOutputs(
              output_ranges[i].first, output_ranges[i].second,
              &sub_state.wires);
        }

        for(size_t i = 0; i < input_ranges.size(); i++)
        {
          state->wires.remapInputs(
              input_ranges[i].first, input_ranges[i].second,
              &sub_state.wires);
        }

        if(num_inputs > 0)
        {
          sub_state.assigned.insert(num_outputs, num_outputs + num_inputs - 1);
        }

        PLASMASnoozeStatus const sub_status =
          this->evaluate(loop_dir_list, &sub_state);
        if(UNLIKELY(sub_status != PLASMASnoozeStatus::well_formed))
        {
          return sub_status;
        }

        if(UNLIKELY(UNLIKELY(sub_state.instanceCount != expected_instance)
            || UNLIKELY(sub_state.witnessCount != expected_witness)
            || UNLIKELY(num_outputs != 0
              && !sub_state.assigned.hasAll(0, num_outputs - 1))))
        {
          log_error("bad form");
          return PLASMASnoozeStatus::bad_relation;
        }
        state->instanceCount += sub_state.instanceCount;
        state->witnessCount += sub_state.witnessCount;
      }

      if(UNLIKELY(!wtk::utils::SkipList<wtk::index_t>::equivalent(
            &loop_outputs, &iter_outputs)))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      state->exprBuilder->pop();
      break;
    }
    case wtk::DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      wtk::SwitchStatement<Number_T>* const switch_stmt =
        dir_list->switchStatement(i);

      wtk::index_t num_output =
        wtk::utils::countWireList(switch_stmt->outputList());

      wtk::index_t num_instance = 0;
      wtk::index_t num_witness = 0;
      if(UNLIKELY(!wtk::utils::maxInsWit<Number_T>(
            switch_stmt, &this->functions, &num_instance, &num_witness)))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      // reserve all the instance values
      std::vector<Wire_T> instance_buffer;
      SwitchStreamHandler<Wire_T> switch_instance((size_t) num_instance);
      if(state->switchInstance != nullptr)
      {
        switch_instance.values =
          state->switchInstance->next((size_t) num_instance);
      }
      else
      {
        instance_buffer.resize((size_t) num_instance);

        for(size_t j = 0; j < num_instance; j++)
        {
          Number_T num;
          if(wtk::StreamStatus::success != state->instance->next(&num)
              || num > this->backend->prime)
          {
            log_error("bad instance");
            return PLASMASnoozeStatus::bad_stream;
          }
          else
          {
            this->backend->instance(&instance_buffer[j], num);
          }
        }

        switch_instance.values = instance_buffer.data();
      }

      // reserve all the witness values
      std::vector<Wire_T> witness_buffer;
      SwitchStreamHandler<Wire_T> switch_witness((size_t) num_witness);
      if(state->switchWitness != nullptr)
      {
        switch_witness.values =
          state->switchWitness->next((size_t) num_witness);
      }
      else
      {
        witness_buffer.resize((size_t) num_witness);

        if(state->witness != nullptr)
        {
          for(size_t j = 0; j < num_witness; j++)
          {
            Number_T num;
            if(wtk::StreamStatus::success != state->witness->next(&num)
                || num > this->backend->prime)
            {
              log_error("bad witness");
              return PLASMASnoozeStatus::bad_stream;
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
          for(size_t j = 0; j < num_witness; j++)
          {
            this->backend->witness(&witness_buffer[j], zero);
          }
        }

        switch_witness.values = witness_buffer.data();
      }

      // Get the condition wire
      if(UNLIKELY(!state->assigned.has(switch_stmt->condition())))
      {
        log_error("bad form");
        return PLASMASnoozeStatus::bad_relation;
      }

      Wire_T const* const condition_wire =
        state->wires.retrieve(switch_stmt->condition());

      // run all the cases
      wtk::utils::SkipList<Number_T> cases;
      wtk::utils::Pool<LocalWireRange<Wire_T, Number_T>> dummiesPool;
      std::vector<LocalWireRange<Wire_T, Number_T>*> dummies;
      dummies.reserve(switch_stmt->size());

      for(size_t i = 0; i < switch_stmt->size(); i++)
      {
        if(num_output > 0)
        {
          dummies.push_back(dummiesPool.allocate(1, 0, num_output - 1));
        }
      }

      std::vector<Wire_T> selection_bits(switch_stmt->size());
      for(size_t j = 0; j < switch_stmt->size(); j++)
      {
        wtk::CaseBlock<Number_T>* const case_blk = switch_stmt->caseBlock(j);
        if(UNLIKELY(UNLIKELY(!cases.insert(case_blk->match()))
              || UNLIKELY(case_blk->match() >= this->backend->prime)))
        {
          log_error("bad case <%s>",
              wtk::utils::dec(case_blk->match()).c_str());
          return PLASMASnoozeStatus::bad_relation;
        }

        this->backend->caseSelect(
            &selection_bits[j], case_blk->match(), condition_wire);

        Wire_T* enabled_bit_ptr = &selection_bits[j];
        Wire_T enabled_tmp;
        if(state->enabledBit != nullptr)
        {
          if(this->backend->isBoolean)
          {
            this->backend->andGate(
                &enabled_tmp, enabled_bit_ptr, state->enabledBit);
          }
          else
          {
            this->backend->mulGate(
                &enabled_tmp, enabled_bit_ptr, state->enabledBit);
          }

          enabled_bit_ptr = &enabled_tmp;
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
          auto finder = this->functions.find(func_name);
          if(UNLIKELY(finder == this->functions.end()))
          {
            log_error("no such function: %s", invoke->name());
            return PLASMASnoozeStatus::bad_relation;
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
          return PLASMASnoozeStatus::bad_relation;
        }
        }

        ExprBuilder invoke_expr_builder;
        ExprBuilder* case_expr_builder =
          is_invoke ? &invoke_expr_builder : state->exprBuilder;
        PLASMASnoozeState<Wire_T, Number_T> case_state(
            state->instance, &switch_instance,
            state->witness, &switch_witness,
            case_expr_builder, enabled_bit_ptr);
        if(num_output > 0)
        {
          case_state.wires.mapDummies(dummies[j]);
        }

        wtk::index_t num_input = 0;
        for(size_t i = 0; i < input_list->size(); i++)
        {
          switch(input_list->type(i))
          {
          case wtk::WireList::SINGLE:
          {
            if(UNLIKELY(!state->assigned.has(input_list->single(i))))
            {
              log_error("bad form");
              return PLASMASnoozeStatus::bad_relation;
            }

            state->wires.remapInput(input_list->single(i), &case_state.wires);
            num_input += 1;
            break;
          }
          case wtk::WireList::RANGE:
          {
            wtk::WireRange* const range = input_list->range(i);
            if(UNLIKELY(UNLIKELY(range->first() > range->last())
                  || UNLIKELY(
                    !state->assigned.has(range->first(), range->last()))))
            {
              log_error("bad form");
              return PLASMASnoozeStatus::bad_relation;
            }

            state->wires.remapInputs(
                range->first(), range->last(), &case_state.wires);
            num_input += 1 + range->last() - range->first();
            break;
          }
          default:
          {
            log_error("unreachable case");
            return PLASMASnoozeStatus::bad_relation;
          }
          }
        }

        if(is_invoke && UNLIKELY(UNLIKELY(num_output != expected_output)
              || UNLIKELY(num_input != expected_input)))
        {
          log_error("bad form");
          return PLASMASnoozeStatus::bad_relation;
        }

        if(num_input != 0)
        {
          case_state.assigned.insert(num_output, num_output + num_input - 1);
        }

        switch_instance.reset();
        switch_witness.reset();

        PLASMASnoozeStatus case_status =
          this->evaluate(case_body, &case_state);
        if(UNLIKELY(case_status != PLASMASnoozeStatus::well_formed))
        {
          return case_status;
        }

        if(UNLIKELY(UNLIKELY(case_state.instanceCount != expected_instance)
              || UNLIKELY(case_state.witnessCount != expected_witness)
              || UNLIKELY(num_output != 0
              && !case_state.assigned.hasAll(0, num_output - 1))))
        {
          log_error("bad form");
          return PLASMASnoozeStatus::bad_relation;
        }
      }

      state->instanceCount += num_instance;
      state->witnessCount += num_witness;

      // add up all the dummy output values into the actual outputs
      wtk::WireList* const output_list = switch_stmt->outputList();
      size_t dummy_place = 0;
      for(size_t i = 0; i < output_list->size(); i++)
      {
        switch(output_list->type(i))
        {
        case wtk::WireList::SINGLE:
        {
          if(UNLIKELY(!state->assigned.insert(output_list->single(i))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          this->backend->multiplexHelper(
              state->wires.insert(output_list->single(i)),
              &dummies, &selection_bits, dummy_place);
          dummy_place++;
          break;
        }
        case wtk::WireList::RANGE:
        {
          wtk::WireRange* const range = output_list->range(i);
          if(UNLIKELY(UNLIKELY(range->first() > range->last())
                || UNLIKELY(
                  !state->assigned.insert(range->first(), range->last()))))
          {
            log_error("bad form");
            return PLASMASnoozeStatus::bad_relation;
          }

          for(wtk::index_t k = range->first(); k <= range->last(); k++)
          {
            this->backend->multiplexHelper(state->wires.insert(k),
                &dummies, &selection_bits, dummy_place);
            dummy_place++;
          }
          break;
        }
        default:
        {
          log_error("unreachable case");
          return PLASMASnoozeStatus::bad_relation;
        }
        }
      }

      log_assert(dummy_place == num_output);

      // Assert that one case was matched.
      this->backend->checkSelectorBits(&selection_bits, state->enabledBit);

      break;
    }
    default:
    {
      log_error("unhandled directive");
      return PLASMASnoozeStatus::bad_relation;
    }
    }
  }

  return PLASMASnoozeStatus::well_formed;
}

} } // namespace wtk::bolt
