/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

/**
 * Because C++ random-access-iterators can't be offset by a size_t (with the
 * -Wconversion flag)
 */
template<typename Iter_T, typename Size_T>
Iter_T iter_offset(Iter_T it, Size_T sz)
{
  std::advance(it, sz);
  return it;
}

template<typename Wire_T>
Range<Wire_T>::Range(size_t l, Wire_T* ws, bool nr)
  : length(l), wires(ws), newRange(nr), remapped(!nr), canGrow(false) { }

template<typename Wire_T>
Range<Wire_T>::Range(size_t l, Wire_T* ws)
  : length(l), wires(ws), newRange(false), remapped(false), canGrow(true) { }

template<typename Wire_T>
Range<Wire_T>::Range(Range<Wire_T>&& move)
  : length(move.length), wires(move.wires),
    newRange(move.newRange), remapped(move.remapped), canGrow(move.canGrow)

{
  move.wires = nullptr;
}

template<typename Wire_T>
Range<Wire_T>& Range<Wire_T>::operator=(Range<Wire_T>&& move)
{
  if(!this->remapped) { free(this->wires); }

  this->length = move.length;
  this->wires = move.wires;
  this->newRange = move.newRange;
  this->remapped = move.remapped;
  this->canGrow = move.canGrow;

  move.wires = nullptr;
  return *this;
}

template<typename Wire_T>
Range<Wire_T>::~Range()
{
  if(!this->remapped) { free(this->wires); }
  this->wires = nullptr;
}

template<typename Wire_T>
size_t Scope<Wire_T>::findRange(wire_idx idx) const
{
  log_assert(this->offsets.size() == this->ranges.size());

  size_t l = 0;
  size_t h = this->offsets.size() - 1;

  while (l != h)
  {
    size_t const mid = l + ((1 + h - l) / 2);
    if(this->offsets[mid] > idx) { h = mid - 1; }
    else { l = mid; }
  }

  log_assert(l == 0
      || (l == this->offsets.size() - 1 && idx >= this->offsets[l])
      || (l < this->offsets.size() - 1
        && idx >= this->offsets[l] && idx < this->offsets[l + 1]));
  return l;
}

template<typename Wire_T>
Wire_T* Scope<Wire_T>::newRange(
    wire_idx const first, wire_idx const last, ScopeError* const err)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("new range: %lu, %lu", first, last);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  // technically we need this safety check for 32-bit systems
  if(!canConvertWireIdxToSize(1 + last - first))
  {
    *err = ScopeError::outOfMem;
    return nullptr;
  }

  size_t const length = 1 + last - first;
  *err = ScopeError::success;

  if(this->offsets.size() == 0)
  {
    // No existing ranges, so make the first one
    Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * length);
    if(wires == nullptr)
    {
      *err = ScopeError::outOfMem;
      return nullptr;
    }

    this->offsets.push_back(first);
    this->ranges.emplace_back(length, wires, true);

    return wires;
  }
  else
  {
    if(UNLIKELY(UNLIKELY(first < this->firstLocal)
          || UNLIKELY(this->assigned.has(first, last))))
    {
      *err = ScopeError::alreadyExists;
      return nullptr;
    }

    size_t const idx = this->findRange(first);

    if(idx == 0 && first < this->offsets[0])
    {
      // Put a range before the first one.
      if(UNLIKELY(UNLIKELY(last >= this->offsets[0])
            && this->ranges[0].newRange))
      {
        *err = ScopeError::alreadyExists;
        return nullptr;
      }

      Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * length);
      if(wires == nullptr) { *err = ScopeError::outOfMem; return nullptr; }

      this->offsets.insert(this->offsets.begin(), first);
      this->ranges.emplace(this->ranges.begin(), length, wires, true);

      return wires;
    }
    else if(idx < this->ranges.size() - 1)
    {
      // Somewhere in the middle of the ranges.
      if(UNLIKELY(UNLIKELY(last >= this->offsets[idx + 1]
              && this->ranges[idx + 1].newRange)
          || UNLIKELY(first <= this->offsets[idx] + this->ranges[idx].length - 1
            && this->ranges[idx].newRange)))
      {
        *err = ScopeError::alreadyExists;
        return nullptr;
      }

      Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * length);
      if(wires == nullptr) { *err = ScopeError::outOfMem; return nullptr; }

      this->offsets.insert(iter_offset(this->offsets.begin(), idx + 1), first);
      this->ranges.emplace(
          iter_offset(this->ranges.begin(), idx + 1), length, wires, true);

      // Check for splitting range
      wire_idx prev_last = this->offsets[idx] + this->ranges[idx].length - 1;
      if(last <= prev_last)
      {
        Wire_T* const old_wires = this->ranges[idx].wires;
        size_t const old_offset = (size_t) this->offsets[idx];
        ScopeError split_err = ScopeError::success;

        size_t split_idx = idx + 2;

        this->active.forRange(last + 1, prev_last, [&](wire_idx f, wire_idx l)
            {
              size_t const new_len = (size_t) (1 + l - f);
              Wire_T* const new_wires =
                (Wire_T*) malloc(sizeof(Wire_T) * new_len);
              if(new_wires == nullptr) { split_err = ScopeError::outOfMem; }

              for(wire_idx i = f; i <= l; i++)
              {
                new(new_wires + i - f) Wire_T(
                    std::move(old_wires[i - old_offset]));
              }

              this->offsets.insert(
                  iter_offset(this->offsets.begin(), split_idx), f);
              this->ranges.emplace(iter_offset(
                    this->ranges.begin(), split_idx), new_len, new_wires);
              split_idx++;
            });

        if(UNLIKELY(split_err != ScopeError::success))
        {
          *err = split_err;
          return nullptr;
        }
      }

      return wires;
    }
    else // if(idx == this->ranges.size() - 1)
    {
      if(UNLIKELY(first <= this->offsets[idx] + this->ranges[idx].length - 1
          && this->ranges[idx].newRange))
      {
        *err = ScopeError::alreadyExists;
        return nullptr;
      }

      Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * length);
      if(wires == nullptr) { *err = ScopeError::outOfMem; return nullptr; }

      this->offsets.push_back(first);
      this->ranges.emplace_back(length, wires, true);

      // Check for splitting range
      wire_idx prev_last = this->offsets[idx] + this->ranges[idx].length - 1;
      if(last <= prev_last)
      {
        Wire_T* const old_wires = this->ranges[idx].wires;
        size_t const old_offset = (size_t) this->offsets[idx];
        ScopeError split_err = ScopeError::success;

        size_t split_idx = idx + 2;

        this->active.forRange(last + 1, prev_last, [&](wire_idx f, wire_idx l)
            {
              size_t const new_len = (size_t) (1 + l - f);
              Wire_T* const new_wires =
                (Wire_T*) malloc(sizeof(Wire_T) * new_len);
              if(new_wires == nullptr) { split_err = ScopeError::outOfMem; }

              for(wire_idx i = f; i <= l; i++)
              {
                new(new_wires + i - f) Wire_T(
                    std::move(old_wires[i - old_offset]));
              }

              this->offsets.insert(
                  iter_offset(this->offsets.begin(), split_idx), f);
              this->ranges.emplace(iter_offset(
                    this->ranges.begin(), split_idx), new_len, new_wires);
              split_idx++;
            });

        if(UNLIKELY(split_err != ScopeError::success))
        {
          *err = split_err;
          return nullptr;
        }
      }

      return wires;
    }
  }
}

template<typename Wire_T>
ScopeError Scope<Wire_T>::deleteRange(
    wire_idx const first, const wire_idx last)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_assert(this->offsets.size() > 0);
  log_debug("delete range: %lu, %lu", first, last);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  if(UNLIKELY(first < this->firstLocal))
  {
    return ScopeError::cannotDeleteRemap;
  }

  if(UNLIKELY(!this->active.hasAll(first, last)))
  {
    if(this->assigned.hasAll(first, last))
    {
      return ScopeError::notAssigned;
    }

    return ScopeError::deleted;
  }

  size_t idx = this->findRange(first);
  log_assert(idx < this->ranges.size());
  wire_idx first_adj = first;
  wire_idx last_adj;

  do
  {
    wire_idx const range_end =
      this->offsets[idx] + this->ranges[idx].length - 1;

    if(this->ranges[idx].newRange)
    {
      if(first != this->offsets[idx] || last != range_end)
      {
        return ScopeError::unmatchedDelete;
      }

      last_adj = last;
    }
    else
    {
      if(idx < this->offsets.size() - 1)
      {
        wire_idx const next_offset = this->offsets[idx + 1] - 1;
        last_adj = next_offset > range_end
          ? range_end > last ? last : range_end
          : next_offset > last ? last : next_offset;
      }
      else
      {
        last_adj = range_end > last ? last : range_end;
      }
    }

    Wire_T* const range = this->ranges[idx].wires;
    for(wire_idx i = first_adj; i <= last_adj && i >= first_adj; i++)
    {
      // call destructors on the elements
      range[i - this->offsets[idx]].~Wire_T();
    }

    this->active.remove(first_adj, last_adj);
    bool do_delete = false;
    if(this->ranges[idx].newRange)
    {
      do_delete = true;
    }
    else
    {
      if(idx < this->offsets.size() - 1 && range_end > this->offsets[idx + 1])
      {
        do_delete =
          !this->active.has(this->offsets[idx], this->offsets[idx + 1] - 1);
      }
      else
      {
        do_delete = !this->active.has(this->offsets[idx], range_end);
      }

      this->ranges[idx].canGrow = false;
    }

    if(do_delete)
    {
      this->offsets.erase(iter_offset(this->offsets.begin(), idx));
      this->ranges.erase(iter_offset(this->ranges.begin(), idx));
    }
    else
    {
      idx++;
    }

    first_adj = last_adj + 1;
  }
  while(last_adj < last);

  return ScopeError::success;
}

template<typename Wire_T>
Wire_T const* Scope<Wire_T>::retrieve(
    wire_idx const wire, ScopeError* const err) const
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("retrieve: %lu", wire);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  if(UNLIKELY(!this->active.has(wire)))
  {
    if(this->assigned.has(wire))
    {
      *err = ScopeError::deleted;
    }

    *err = ScopeError::notAssigned;
    return nullptr;
  }

  *err = ScopeError::success;
  size_t idx = this->findRange(wire);

  log_assert(wire - this->offsets[idx] < this->ranges[idx].length);

  return this->ranges[idx].wires + (size_t) (wire - this->offsets[idx]);
}

// must be > 1, else the growth factor 1.5 fails.
size_t constexpr RANGE_DEFAULT_SIZE = 4;

template<typename Wire_T>
Wire_T* Scope<Wire_T>::assign(
    wire_idx const wire, ScopeError* const err)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("assign: %lu", wire);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  if(UNLIKELY(this->assigned.has(wire)))
  {
    *err = ScopeError::alreadyExists;
    return nullptr;
  }

  if(UNLIKELY(this->offsets.size() == 0))
  {
    Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * RANGE_DEFAULT_SIZE);
    if(wires == nullptr)
    {
      *err = ScopeError::outOfMem;
      return nullptr;
    }

    this->offsets.push_back(wire);
    if(UINT64_MAX - RANGE_DEFAULT_SIZE < wire)
    {
      this->ranges.emplace_back(1, wires);
      this->ranges.back().canGrow = false;
    }
    else
    {
      this->ranges.emplace_back(RANGE_DEFAULT_SIZE, wires);
    }

    new(wires) Wire_T();
    this->assigned.insert(wire);
    this->active.insert(wire);
    return wires;
  }

  size_t idx = this->findRange(wire);

  if(LIKELY(LIKELY(wire >= this->offsets[idx]) && LIKELY(
          wire <=
          (wire_idx) (this->offsets[idx] + this->ranges[idx].length - 1))))
  {
    Wire_T* ret =
      this->ranges[idx].wires + (size_t) (wire - this->offsets[idx]);
    new(ret) Wire_T();
    this->assigned.insert(wire);
    this->active.insert(wire);
    return ret;
  }
  else if(idx == 0 && wire < this->offsets[0])
  {
    Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * RANGE_DEFAULT_SIZE);
    if(wires == nullptr)
    {
      *err = ScopeError::outOfMem;
      return nullptr;
    }

    this->offsets.insert(this->offsets.begin(), wire);
    if(UINT64_MAX - RANGE_DEFAULT_SIZE < wire)
    {
      this->ranges.emplace(this->ranges.begin(), 1, wires);
      this->ranges.begin()->canGrow = false;
    }
    else
    {
      this->ranges.emplace(this->ranges.begin(), RANGE_DEFAULT_SIZE, wires);
    }

    new(wires) Wire_T();
    this->assigned.insert(wire);
    this->active.insert(wire);
    return wires;
  }
  else
  {
    wire_idx const first = this->offsets[idx];
    wire_idx const last = this->offsets[idx] + this->ranges[idx].length - 1;
    wire_idx const growth_amt =
      this->ranges[idx].length + (this->ranges[idx].length >> 1);
    wire_idx const growth_last =
      (UINT64_MAX - growth_amt > first) ? first + growth_amt : UINT64_MAX;

    if(this->ranges[idx].canGrow && wire < growth_last
        && canConvertWireIdxToSize(growth_last - first))
    {
      wire_idx const adj_growth =
        (idx < this->ranges.size() - 1 && growth_last >= this->offsets[idx + 1])
        ? this->offsets[idx + 1] - 1
        : growth_last;

      size_t const new_size = (size_t) adj_growth - first + 1;
      Wire_T* new_wires = (Wire_T*) malloc(sizeof(Wire_T) * new_size);
      if(new_wires == nullptr)
      {
        *err = ScopeError::outOfMem;
        return nullptr;
      }

      this->active.forRange(first, last, [&](wire_idx f, wire_idx l)
          {
            for(wire_idx i = f; i <= l; i++)
            {
              new(new_wires + i - first) Wire_T(std::move(
                    this->ranges[idx].wires[i - first]));
              this->ranges[idx].wires[i - first].~Wire_T();
            }
          });

      free(this->ranges[idx].wires);
      this->ranges[idx].wires = new_wires;
      this->ranges[idx].length = new_size;

      new(new_wires + wire - first) Wire_T();
      this->assigned.insert(wire);
      this->active.insert(wire);
      return new_wires + wire - first;
    }
    else
    {
      Wire_T* wires = (Wire_T*) malloc(sizeof(Wire_T) * RANGE_DEFAULT_SIZE);
      if(wires == nullptr)
      {
        *err = ScopeError::outOfMem;
        return nullptr;
      }

      this->offsets.insert(iter_offset(this->offsets.begin(), idx + 1), wire);
      if(UINT64_MAX - RANGE_DEFAULT_SIZE < wire)
      {
        this->ranges.emplace(iter_offset(this->ranges.begin(), idx + 1),
            1, wires);
        iter_offset(this->ranges.begin(), idx + 1)->canGrow = false;
      }
      else
      {
        this->ranges.emplace(iter_offset(this->ranges.begin(), idx + 1),
            RANGE_DEFAULT_SIZE, wires);
      }

      new(wires) Wire_T();
      this->assigned.insert(wire);
      this->active.insert(wire);
      return wires;
    }
  }
}

template<typename Wire_T>
Wire_T* Scope<Wire_T>::findOutputs(
    wire_idx const first, wire_idx const last, ScopeError* const err)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("find outputs: %lu ... %lu", first, last);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  if(UNLIKELY(this->assigned.has(first, last)))
  {
    *err = ScopeError::alreadyExists;
    return nullptr;
  }

  if(this->ranges.size() == 0)
  {
    return this->newRange(first, last, err);
  }

  *err = ScopeError::success;
  size_t const idx = this->findRange(first);

  wire_idx const range_first = this->offsets[idx];
  wire_idx const range_last = range_first + this->ranges[idx].length - 1;
  if((this->ranges[idx].newRange || this->ranges[idx].remapped) &&
      LIKELY(LIKELY(range_first <= first) && LIKELY(range_last >= last)))
  {
    return this->ranges[idx].wires + first - range_first;
  }
  else
  {
    return this->newRange(first, last, err);
  }
}

template<typename Wire_T>
Wire_T* Scope<Wire_T>::findInputs(
    wire_idx const first, wire_idx const last, ScopeError* const err)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("find inputs: %lu ... %lu", first, last);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  if(UNLIKELY(!this->active.hasAll(first, last)))
  {
    if(this->assigned.hasAll(first, last))
    {
      *err = ScopeError::deleted;
      return nullptr;
    }

    *err = ScopeError::notAssigned;
    return nullptr;
  }

  *err = ScopeError::success;
  size_t const idx = this->findRange(first);

  wire_idx const range_first = this->offsets[idx];
  wire_idx const range_last = range_first + this->ranges[idx].length - 1;
  if(LIKELY(LIKELY(this->ranges[idx].newRange || this->ranges[idx].remapped
        || first == last) &&
      LIKELY(LIKELY(range_first <= first) && LIKELY(range_last >= last
          || /* range overflow */ (last >= first
            && UINT64_MAX - this->ranges[idx].length <= range_first)))))
  {
    return this->ranges[idx].wires + first - range_first;
  }
  else
  {
    *err = ScopeError::discontiguous;
    return nullptr;
  }
}

template<typename Wire_T>
void Scope<Wire_T>::mapOutputs(size_t const length, Wire_T* const wires)
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("map outputs: %zu", length);
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  this->offsets.push_back(this->firstLocal);
  this->ranges.emplace_back(length, wires, false);

  this->firstLocal += length;
}

template<typename Wire_T>
void Scope<Wire_T>::mapInputs(size_t const length, Wire_T* const wires)
{
  log_debug("map inputs: %zu", length);

  wire_idx first = this->firstLocal;
  this->mapOutputs(length, wires);
  wire_idx last = this->firstLocal - 1;

  this->assigned.insert(first, last);
  this->active.insert(first, last);
}

template<typename Wire_T>
std::string Scope<Wire_T>::toString() const
{
  std::string ret;

  for(size_t i = 0; i < this->offsets.size(); i++)
  {
    ret += "[" + wtk::utils::dec(this->offsets[i]) + " ... ";
    ret += wtk::utils::dec(this->offsets[i] + this->ranges[i].length - 1);
    if(this->ranges[i].newRange)      { ret += " new range], "; }
    else if(this->ranges[i].remapped) { ret += " remapped], "; }
    else if(this->ranges[i].canGrow)  { ret += " growable] "; }
    else                              { ret += " nongrowable], "; }
  }

  return ret;
}

template<typename Wire_T>
Scope<Wire_T>::~Scope()
{
  log_assert(this->offsets.size() == this->ranges.size());
  log_debug("destruct");
  log_debug("\nassigned: %s\n  active: %s\n  ranges: %s\n",
      this->assigned.toString().c_str(), this->active.toString().c_str(),
      this->toString().c_str());

  size_t idx = 0;
  this->active.forEach([&](wire_idx f, wire_idx l)
      {
        wire_idx first;
        wire_idx last;

        do
        {
          first = this->offsets[idx];
          last = this->offsets[idx] + this->ranges[idx].length - 1;

          if(this->ranges[idx].remapped)
          {
            idx++;
            continue;
          }

          if(idx < this->offsets.size() - 1 && last >= this->offsets[idx + 1])
          {
            last = this->offsets[idx + 1] - 1;
          }

          wire_idx const offset = this->offsets[idx];
          Wire_T* const wires = this->ranges[idx].wires;

          if(first < f) { first = f; }
          if(last > l) { last = l; } else { idx++; }

          if(last >= first)
          {
            for(wire_idx i = first; i <= last; i++)
            {
              wires[(size_t) (i - offset)].~Wire_T();
            }
          }

          first = last + 1;
        } while(first <= l && idx < this->offsets.size());
      });
}

} } // namespace wtk::nails
