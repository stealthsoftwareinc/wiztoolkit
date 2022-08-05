/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

/**
 * Because C++ random-access-iterators can't be offset by a size_t
 */
template<typename Iter_T, typename Size_T>
Iter_T iter_offset(Iter_T it, Size_T sz)
{
  std::advance(it, sz);
  return it;
}

template<typename Wire_T>
bool WireSet<Wire_T>::isOutput(index_t const idx)
{
  return idx < this->outputSize();
}

template<typename Wire_T>
bool WireSet<Wire_T>::isInput(index_t const idx)
{
  return idx < this->outputSize() + this->inputSize();
}

template<typename Wire_T>
index_t WireSet<Wire_T>::adjustLocal(index_t const idx)
{
  return idx - (this->outputSize() + this->inputSize());
}

template<typename Wire_T>
size_t findRangeIdx(
    std::vector<WireRangeRef<Wire_T>>* ranges, index_t const idx_adj)
{
  // Short circuit if there's one or zero ranges.
  if(ranges->size() <= 1) { return 0; }

  size_t l = 0;
  size_t h = ranges->size() - 1;

  while(l != h)
  {
    size_t const mid = l + ((1 + h - l) / 2);
    if((*ranges)[mid].first > idx_adj) { h = mid - 1; }
    else { l = mid; }
  }

  log_assert(l < ranges->size());
  log_assert(l == 0 || idx_adj >= (*ranges)[l].first);
  return l;
}

template<typename Wire_T>
Wire<Wire_T>* WireSet<Wire_T>::findOutput(index_t const idx)
{
  log_assert(idx < this->outputSize());

  WireRangeRef<Wire_T>* range =
    &this->outputs[findRangeIdx(&this->outputs, idx)];

  // The outputs should be contiguous, so these assertions are okay
  log_assert(idx >= range->first && idx <= range->last);
  size_t idx_in_range = range->offset + (size_t) (idx - range->first);
  log_assert(idx_in_range < range->wires->size());
  return &(*range->wires)[idx_in_range];
}

template<typename Wire_T>
Wire<Wire_T>* WireSet<Wire_T>::findInput(index_t const idx)
{
  log_assert(idx >= this->outputSize() &&
      idx < this->outputSize() + this->inputSize());

  WireRangeRef<Wire_T>* range =
    &this->inputs[findRangeIdx(&this->inputs, idx)];

  // The inputs should be contiguous, so these assertions are okay
  log_assert(idx >= range->first && idx <= range->last);
  size_t idx_in_range = range->offset + (size_t) (idx - range->first);
  log_assert(idx_in_range < range->wires->size());
  return &(*range->wires)[idx_in_range];
}

template<typename Wire_T>
Wire<Wire_T>* WireSet<Wire_T>::findLocal(index_t const idx)
{
  log_assert(idx >= this->outputSize() + this->inputSize());

  index_t const idx_adj = this->adjustLocal(idx);
  size_t range_idx = findRangeIdx(&this->locals, idx_adj);

  if(range_idx >= this->locals.size()
      || idx_adj < this->locals[range_idx].first
      || idx_adj > this->locals[range_idx].last)
  {
    return nullptr;
  }
  else
  {
    size_t idx_in_range = (size_t) (idx_adj - this->locals[range_idx].first);
    log_assert(idx_in_range < this->locals[range_idx].wires->size());
    return &(*this->locals[range_idx].wires)[idx_in_range];
  }
}

template<typename Wire_T>
Wire<Wire_T>* WireSet<Wire_T>::findIndex(index_t idx)
{
  if(this->isOutput(idx))     { return this->findOutput(idx); }
  else if(this->isInput(idx)) { return this->findInput(idx); }
  else                        { return this->findLocal(idx); }
}

// Only used in debug mode for assertions.
template<typename Wire_T>
bool WireSet<Wire_T>::memLayoutCheck()
{
  /* Outputs */
  if(this->outputs.size() > 0)
  {
    if(this->outputs[0].first != 0)
    {
      log_error("mem layout 1");
      return false;
    }

    for(size_t i = 0; i < this->outputs.size(); i++)
    {
      if(this->outputs[i].first > this->outputs[i].last)
      {
        log_error("mem layout 2");
        return false;
      }
      if(this->outputs[i].offset + 1
          + (size_t) (this->outputs[i].last - this->outputs[i].first)
          > this->outputs[i].wires->size())
      {
        log_error("mem layout 3");
        return false;
      }
      if(i > 0 && this->outputs[i - 1].last + 1 != this->outputs[i].first)
      {
        log_error("mem layout 4");
        return false;
      }
    }
  }

  /* Inputs */
  if(this->inputs.size() > 0)
  {
    index_t const first = this->outputSize();
    if(this->inputs[0].first != first)
    {
      log_error("mem layout 5");
      return false;
    }

    for(size_t i = 0; i < this->inputs.size(); i++)
    {
      if(this->inputs[i].first > this->inputs[i].last)
      {
        log_error("mem layout 6");
        return false;
      }
      if(this->inputs[i].offset + 1
          + (size_t) (this->inputs[i].last - this->inputs[i].first)
          > this->inputs[i].wires->size())
      {
        log_error("mem layout 7");
        return false;
      }
      if(i > 0 && this->inputs[i - 1].last + 1 != this->inputs[i].first)
      {
        log_error("mem layout 8");
        return false;
      }
    }
  }

  /* Locals */
  if(this->locals.size() > 0)
  {
    for(size_t i = 0; i < this->locals.size(); i++)
    {
      if(i > 0 && this->locals[i - 1].last >= this->locals[i].first)
      {
        log_error("mem layout 9");
        return false;
      }
      if(this->locals[i].last < this->locals[i].first)
      {
        log_error("mem layout 10");
        return false;
      }
      if(1 + this->locals[i].last - this->locals[i].first
          != this->locals[i].wires->size())
      {
        log_error("mem layout 11");
        return false;
      }
    }
  }

  return true;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::retrieveResource(index_t const idx)
{
  log_assert(this->memLayoutCheck());

  Wire<Wire_T>* w = this->findIndex(idx);
  if(w == nullptr)
  {
    return WireSetFail::not_assigned;
  }

  if(w->deleted)  { return WireSetFail::deleted; }
  if(!w->assigned) { return WireSetFail::not_assigned; }

  return WireSetFail::success;
}

template<typename Wire_T>
Wire_T WireSet<Wire_T>::retrieveEvaluation(index_t const idx)
{
  Wire<Wire_T>* w = this->findIndex(idx);
  return w->wire;
}

// chosen somewhat arbitrarily
size_t constexpr GROWTH_THRESHOLD = 1024;

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::insertResource(index_t const idx)
{
  log_assert(this->memLayoutCheck());

  if(this->isOutput(idx))
  {
    Wire<Wire_T>* w = this->findOutput(idx);

    // I'm fairly certain null is impossible, but spec calls for the check.
    if(w == nullptr) { return WireSetFail::null_deref; }

    if(w->deleted)  { return WireSetFail::deleted; }
    if(w->assigned) { return WireSetFail::assigned; }

    w->assigned = true;
  }
  else if(!this->isInput(idx)) // It's a local
  {
    // deviates slightly from spec logic due to use of heuristically sized
    // arrays rather than a hashmap. The spec only checks for null, but we'll
    // check for not-found or found and !(deleted || assigned). There are also
    // a few cases of not-found (empty, grow, or insert).
    index_t const idx_adj = this->adjustLocal(idx);
    size_t range_idx = findRangeIdx(&this->locals, idx_adj);

    if(this->locals.size() == 0)
    {
      this->locals.emplace_back();
      this->locals.back().first = idx_adj;
      this->locals.back().last = idx_adj;
      this->locals.back().wires = this->wiresPool.allocate();
      this->locals.back().wires->emplace_back();
      this->locals.back().wires->back().assigned = true;
    }
    else if(idx_adj >= this->locals[range_idx].first
        && idx_adj <= this->locals[range_idx].last)
    {
      size_t const idx_in_range =
        (size_t) (idx_adj - this->locals[range_idx].first);
      log_assert(idx_in_range < this->locals[range_idx].wires->size());

      Wire<Wire_T>* r = &(*this->locals[range_idx].wires)[idx_in_range];
      if(r->deleted)       { return WireSetFail::deleted; }
      else if(r->assigned) { return WireSetFail::assigned; }
      else { r->assigned = true; }
    }
    else if(idx_adj < this->locals[range_idx].first)
    {
      this->locals.emplace(iter_offset(this->locals.begin(), range_idx));
      this->locals[range_idx].first = idx_adj;
      this->locals[range_idx].last = idx_adj;
      this->locals[range_idx].wires = this->wiresPool.allocate();
      this->locals[range_idx].wires->emplace_back();
      (*this->locals[range_idx].wires)[0].assigned = true;
    }
    else if(idx_adj <= this->locals[range_idx].last + GROWTH_THRESHOLD)
    {
      size_t const idx_in_range =
        (size_t) (idx_adj - this->locals[range_idx].first);
      this->locals[range_idx].wires->resize(idx_in_range + 1);
      this->locals[range_idx].wires->back().assigned = true;
      this->locals[range_idx].last = idx_adj;
      log_assert(idx_in_range + 1 == this->locals[range_idx].wires->size());
    }
    else
    {
      this->locals.emplace(iter_offset(this->locals.begin(), range_idx + 1));
      this->locals[range_idx + 1].first = idx_adj;
      this->locals[range_idx + 1].last = idx_adj;
      this->locals[range_idx + 1].wires = this->wiresPool.allocate();
      this->locals[range_idx + 1].wires->emplace_back();
      (*this->locals[range_idx + 1].wires)[0].assigned = true;
    }
  }
  else
  {
    // its an input wire, should be assigned previously by definition.
    return WireSetFail::assigned;
  }

  log_assert(this->memLayoutCheck());
  return WireSetFail::success;
}

template<typename Wire_T>
void WireSet<Wire_T>::insertEvaluation(index_t const idx, Wire_T value)
{
  Wire<Wire_T>* w = this->findIndex(idx);
  w->wire = value;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remove(index_t const idx)
{
  if(idx < this->outputSize() + this->inputSize())
  {
    return WireSetFail::cant_delete;
  }

  Wire<Wire_T>* w = this->findLocal(idx);
  if(w == nullptr) { return WireSetFail::not_assigned; }
  if(w->deleted) { return WireSetFail::deleted; }
  if(!w->assigned) { return WireSetFail::not_assigned; }

  w->deleted = true;

  return WireSetFail::success;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remove(index_t const first,
    index_t const last)
{
  for(index_t i = first; i <= last; i++)
  {
    WireSetFail fail = this->remove(i);
    if(fail != WireSetFail::success) { return fail; }
  }

  return WireSetFail::success;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remapInput(index_t idx, WireSet<Wire_T>* subscope)
{
  return this->remapInputs(idx, idx, subscope);
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remapInputs(
    index_t first, index_t last, WireSet<Wire_T>* subscope)
{
  log_assert(this->memLayoutCheck());
  log_assert(subscope->memLayoutCheck());

  if(subscope->locals.size() != 0) { return WireSetFail::nonempty_subscope; }

  if(first > last) { return WireSetFail::invalid_range; }
  else if(first < this->outputSize() && last >= this->outputSize())
  {
    WireSetFail fail =
      this->remapInputs(first, this->outputSize() - 1, subscope);
    if(fail == WireSetFail::success)
    {
      return this->remapInputs(this->outputSize(), last, subscope);
    }
    else { return fail; }
  }
  else if(first < this->outputSize() + this->inputSize()
      && last >= this->outputSize() + this->inputSize())
  {
    WireSetFail fail = this->remapInputs(
        first, this->outputSize() + this->inputSize() - 1, subscope);
    if(fail == WireSetFail::success)
    {
      return this->remapInputs(
          this->outputSize() + this->inputSize(), last, subscope);
    }
    else { return fail; }
  }
  else if(last < this->outputSize())
  {
    size_t range_idx = findRangeIdx(&this->outputs, first);
    index_t adj_first = first;

    do
    {
      index_t const adj_last = this->outputs[range_idx].last < last
        ? this->outputs[range_idx].last
        : last;

      size_t const offset = this->outputs[range_idx].offset;
      for(size_t i = (size_t) adj_first + offset; i <=
          (size_t) adj_last + offset; i++)
      {
        if((*this->outputs[range_idx].wires)[i].deleted)
        {
          return WireSetFail::deleted;
        }
        if(!(*this->outputs[range_idx].wires)[i].assigned)
        {
          return WireSetFail::not_assigned;
        }
      }

      index_t sub_first = (subscope->inputs.size() == 0)
        ? subscope->outputSize()
        : subscope->inputs.back().last + 1;
      index_t sub_last = sub_first + adj_last - adj_first;
      subscope->inputs.emplace_back();
      subscope->inputs.back().first = sub_first;
      subscope->inputs.back().last = sub_last;
      subscope->inputs.back().wires = this->outputs[range_idx].wires;
      subscope->inputs.back().offset = this->outputs[range_idx].offset
        + (size_t) (adj_first - this->outputs[range_idx].first);

      range_idx += 1;
      adj_first = adj_last + 1;
    } while(adj_first <= last);
  }
  else if(first < this->outputSize() + this->inputSize())
  {
    size_t range_idx = findRangeIdx(&this->inputs, first);
    index_t adj_first = first;

    do
    {
      index_t adj_last = this->inputs[range_idx].last < last
        ? this->inputs[range_idx].last
        : last;

      // Can skip checking deleted || !assigned because these are guaranteed
      // for input wires.

      index_t sub_first = (subscope->inputs.size() == 0)
        ? subscope->outputSize()
        : subscope->inputs.back().last + 1;
      index_t sub_last = sub_first + adj_last - adj_first;
      subscope->inputs.emplace_back();
      subscope->inputs.back().first = sub_first;
      subscope->inputs.back().last = sub_last;
      subscope->inputs.back().wires = this->inputs[range_idx].wires;
      subscope->inputs.back().offset = this->inputs[range_idx].offset
        + (size_t) (adj_first - this->inputs[range_idx].first);

      range_idx += 1;
      adj_first = adj_last + 1;
    } while(adj_first <= last);
  }
  else // Local wires
  {
    index_t const adj_first = this->adjustLocal(first);
    size_t range_idx = findRangeIdx(&this->locals, adj_first);
    index_t const adj_last = this->adjustLocal(last);

    if(adj_first >= this->locals[range_idx].first
        && adj_last <= this->locals[range_idx].last)
    {
      index_t sub_first = (subscope->inputs.size() == 0)
        ? subscope->outputSize()
        : subscope->inputs.back().last + 1;
      index_t sub_last = sub_first + adj_last - adj_first;

      std::vector<Wire<Wire_T>>* sub_wires = this->locals[range_idx].wires;
      size_t sub_offset = (size_t) (adj_first - this->locals[range_idx].first);

      for(size_t i = 0; i <= (size_t) (sub_last - sub_first); i++)
      {
        if((*sub_wires)[sub_offset + i].deleted)
        {
          return WireSetFail::deleted;
        }
        if(!(*sub_wires)[sub_offset + i].assigned)
        {
          return WireSetFail::not_assigned;
        }
      }

      subscope->inputs.emplace_back();
      subscope->inputs.back().first = sub_first;
      subscope->inputs.back().last = sub_last;
      subscope->inputs.back().wires = sub_wires;
      subscope->inputs.back().offset = sub_offset;
    }
    else
    {
      return WireSetFail::not_assigned;
    }
  }

  log_assert(this->memLayoutCheck());
  log_assert(subscope->memLayoutCheck());
  return WireSetFail::success;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remapOutput(
    index_t const idx, WireSet<Wire_T>* subscope)
{
  return this->remapOutputs(idx, idx, subscope);
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::remapOutputs(
    index_t const first, index_t const last, WireSet<Wire_T>* subscope)
{
  log_assert(this->memLayoutCheck());
  log_assert(subscope->memLayoutCheck());

  if(subscope->locals.size() != 0) { return WireSetFail::nonempty_subscope; }

  if(first > last) { return WireSetFail::invalid_range; }
  else if(first < this->outputSize() && last >= this->outputSize())
  {
    WireSetFail fail =
      this->remapOutputs(first, this->outputSize() - 1, subscope);
    if(fail == WireSetFail::success)
    {
      return this->remapOutputs(this->outputSize(), last, subscope);
    }
    else { return fail; }
  }
  else if(last < this->outputSize())
  {
    size_t range_idx = findRangeIdx(&this->outputs, first);
    index_t adj_first = first;

    do
    {
      index_t adj_last = this->outputs[range_idx].last < last
        ? this->outputs[range_idx].last
        : last;

      size_t offset = this->outputs[range_idx].offset;
      for(size_t i = (size_t) adj_first + offset;
          i <= (size_t) adj_last + offset; i++)
      {
        if((*this->outputs[range_idx].wires)[i].assigned)
        {
          return WireSetFail::assigned;
        }
      }

      index_t sub_first = (subscope->outputs.size() == 0)
        ? 0
        : subscope->outputs.back().last + 1;
      index_t sub_last = sub_first + adj_last - adj_first;
      subscope->outputs.emplace_back();
      subscope->outputs.back().first = sub_first;
      subscope->outputs.back().last = sub_last;
      subscope->outputs.back().wires = this->outputs[range_idx].wires;
      subscope->outputs.back().offset = this->outputs[range_idx].offset
        + (size_t) (adj_first - this->outputs[range_idx].first);

      range_idx += 1;
      adj_first = adj_last + 1;
    } while(adj_first <= last);
  }
  else if(first < this->outputSize() + this->inputSize())
  {
    // Attempting to overwrite this scope's input wires.
    return WireSetFail::assigned;
  }
  else // Local wires
  {
    index_t adj_first = this->adjustLocal(first);
    size_t range_idx = findRangeIdx(&this->locals, adj_first);
    index_t const adj_actual_last = this->adjustLocal(last);

    do
    {
      index_t const sub_first = (subscope->outputs.size() == 0)
        ? 0
        : subscope->outputs.back().last + 1;
      index_t sub_last;
      std::vector<Wire<Wire_T>>* sub_wires;
      size_t sub_offset;

      if(this->locals.size() == 0)
      {
        this->locals.emplace_back();
        this->locals[0].first = adj_first;
        this->locals[0].last = adj_actual_last;
        this->locals[0].wires = this->wiresPool.allocate();
        this->locals[0].wires->resize(
            1 + (size_t) (adj_actual_last - adj_first));

        sub_last = sub_first + adj_actual_last - adj_first;
        sub_wires = this->locals[0].wires;
        sub_offset = 0;

        range_idx += 1;
        adj_first = adj_actual_last + 1;
      }
      else if(adj_first >= this->locals[range_idx].first
          && adj_actual_last <= this->locals[range_idx].last)
      {
        sub_wires = this->locals[range_idx].wires;
        sub_last = sub_first + adj_actual_last - adj_first;
        sub_offset = (size_t) (adj_first - this->locals[range_idx].first);

        for(size_t i = 0; i <= (size_t) (sub_last - sub_first); i++)
        {
          if((*sub_wires)[sub_offset + i].deleted)
          {
            return WireSetFail::deleted;
          }
          if((*sub_wires)[sub_offset + i].assigned)
          {
            return WireSetFail::assigned;
          }
        }

        range_idx += 1;
        adj_first = adj_actual_last + 1;
      }
      else if(adj_first < this->locals[range_idx].first)
      {
        index_t adj_last = this->locals[range_idx].first < adj_actual_last
          ? this->locals[range_idx].first - 1
          : adj_actual_last;

        this->locals.emplace(iter_offset(this->locals.begin(), range_idx));
        this->locals[range_idx].first = adj_first;
        this->locals[range_idx].last = adj_last;
        this->locals[range_idx].wires = this->wiresPool.allocate();
        this->locals[range_idx].wires->resize(
            1 + (size_t) (adj_last - this->locals[range_idx].first));

        sub_last = sub_first + adj_last - adj_first;
        sub_wires = this->locals[range_idx].wires;
        sub_offset = 0;

        range_idx += 1;
        adj_first = adj_last + 1;
      }
      else if(adj_first <= this->locals[range_idx].last + GROWTH_THRESHOLD)
      {
        index_t const adj_last = (range_idx + 1 < this->locals.size()
          && adj_actual_last >= this->locals[range_idx + 1].first)
          ? this->locals[range_idx + 1].first - 1
          : adj_actual_last;

        sub_wires = this->locals[range_idx].wires;
        sub_last = sub_first + adj_last - adj_first;
        sub_offset = (size_t) (adj_first - this->locals[range_idx].first);

        index_t curr_last = this->locals[range_idx].last;
        for(index_t i = adj_first; i <= curr_last; i++)
        {
          if((*this->locals[range_idx].wires)[
              (size_t) (i - this->locals[range_idx].first)].deleted)
          {
            return WireSetFail::deleted;
          }
          if((*this->locals[range_idx].wires)[
              (size_t) (i - this->locals[range_idx].first)].assigned)
          {
            return WireSetFail::assigned;
          }
        }

        size_t const last_in_range =
          (size_t) (adj_last - this->locals[range_idx].first);
        this->locals[range_idx].wires->resize(last_in_range + 1);
        this->locals[range_idx].last = adj_last;

        range_idx += 1;
        adj_first = adj_last + 1;
      }
      else
      {
        index_t const adj_last = (range_idx + 1 < this->locals.size()
          && adj_actual_last >= this->locals[range_idx + 1].first)
          ? this->locals[range_idx + 1].first - 1
          : adj_actual_last;


        this->locals.emplace(iter_offset(this->locals.begin(), range_idx + 1));
        this->locals[range_idx + 1].first = adj_first;
        this->locals[range_idx + 1].last = adj_last;
        this->locals[range_idx + 1].wires = this->wiresPool.allocate();
        this->locals[range_idx + 1].wires->resize(
            1 + (size_t) (adj_last - this->locals[range_idx + 1].first));

        sub_last = sub_first + adj_last - adj_first;
        sub_wires = this->locals[range_idx + 1].wires;
        sub_offset = 0;

        range_idx += 2;
        adj_first = adj_last + 1;
      }

      subscope->outputs.emplace_back();
      subscope->outputs.back().first = sub_first;
      subscope->outputs.back().last = sub_last;
      subscope->outputs.back().wires = sub_wires;
      subscope->outputs.back().offset = sub_offset;
    } while(adj_first <= adj_actual_last);
  }

  log_assert(this->memLayoutCheck());
  log_assert(subscope->memLayoutCheck());
  return WireSetFail::success;
}

template<typename Wire_T>
WireSetFail WireSet<Wire_T>::mapDummies(
    index_t const count, std::vector<Wire<Wire_T>>& dummies)
{
  log_assert(dummies.size() == 0);
  if(this->outputs.size() != 0) { return WireSetFail::nonempty_subscope; }

  dummies.resize((size_t) count);
  this->outputs.emplace_back();
  this->outputs[0].first = 0;
  this->outputs[0].last = count - 1;
  this->outputs[0].wires = &dummies;

  log_assert(this->memLayoutCheck());

  return WireSetFail::success;
}

template<typename Wire_T>
index_t WireSet<Wire_T>::inputSize()
{
  if(this->inputs.size() == 0) { return 0; }
  else { return 1 + this->inputs.back().last - this->inputs[0].first; }
}

template<typename Wire_T>
index_t WireSet<Wire_T>::outputSize()
{
  if(this->outputs.size() == 0) { return 0; }
  else { return 1 + this->outputs.back().last; }
}

} } // namespace wtk::firealarm
