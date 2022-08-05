/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T> 
Wire_T* WireRef<Wire_T, Number_T>::deref() const
{
  if(this->index == UINT64_MAX) { return this->direct; }
  else { return this->indirect->deref(this->index); }
}

template<typename Wire_T, typename Number_T> 
void WireRef<Wire_T, Number_T>::setDirect(Wire_T* const d)
{
  this->index = UINT64_MAX;
  this->direct = d;
}

template<typename Wire_T, typename Number_T> 
void WireRef<Wire_T, Number_T>::setIndirect(
    WireRange<Wire_T, Number_T>* const i, wtk::index_t const idx)
{
  this->index = idx;
  this->indirect = i;
}

template<typename Wire_T, typename Number_T> 
void WireRange<Wire_T, Number_T>::setRange(
    wtk::index_t const f, wtk::index_t const l)
{
  this->first = f;
  this->last = l;
}

template<typename Wire_T, typename Number_T>
void WireRange<Wire_T, Number_T>::update(std::vector<wtk::index_t>& stack)
{
  (void) stack;
}

template<typename Wire_T, typename Number_T>
LocalWireRange<Wire_T, Number_T>::LocalWireRange(
    wtk::index_t const f, wtk::index_t const l)
  : wires(1 + (size_t) (l - f))
{
  log_assert(f <= l);
  this->setRange(f, l);
}

template<typename Wire_T, typename Number_T> 
void LocalWireRange<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  ref->setDirect(&this->wires[(size_t) (idx - this->first)]);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* LocalWireRange<Wire_T, Number_T>::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;

  DirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->directWireRangeRef.allocate(
        1, &this->wires[(size_t) (first - this->first)]);

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* LocalWireRange<Wire_T, Number_T>::deref(wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  return &this->wires[(size_t) (idx - this->first)];
}

template<typename Wire_T, typename Number_T> 
void DirectWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->direct != nullptr);

  ref->setDirect(this->direct + (idx - this->first));
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* DirectWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  log_assert(this->direct != nullptr);

  DirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->directWireRangeRef.allocate(1, this->direct + (first - this->first));

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* DirectWireRangeRef<Wire_T, Number_T>::deref(wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->direct != nullptr);

  return this->direct + (idx - this->first);
}

template<typename Wire_T, typename Number_T> 
void IndirectWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->indirect != nullptr);
  log_assert(
      this->indirect->first + this->offset + this->last - this->first
      <= this->indirect->last);

  ref->setIndirect(this->indirect,
      this->indirect->first + this->offset + idx - this->first);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* IndirectWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  log_assert(this->indirect != nullptr);
  log_assert(
      this->indirect->first + this->offset + this->last - this->first
      <= this->indirect->last);

  IndirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->indirectWireRangeRef.allocate();
  ref->indirect = this->indirect;
  ref->offset = this->indirect->first + this->offset + first - this->first;

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* IndirectWireRangeRef<Wire_T, Number_T>::deref(wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->indirect != nullptr);
  log_assert(
      this->indirect->first + this->offset + this->last - this->first
      <= this->indirect->last);

  return this->indirect->deref(
      this->indirect->first + this->offset + idx - this->first);
}

template<typename Wire_T, typename Number_T>
size_t WireSet<Wire_T, Number_T>::findRangeIdx(wtk::index_t key) const
{
  // Assume the key is always going to be found, because all wires should
  // be pre-allocated before scope processing.
  log_assert(this->disableChecks() || key >= this->ranges[0]->first);
  log_assert(this->disableChecks() || key <= this->ranges.back()->last);

  // TODO determine if short-circuit here is effective.
  // if(this->ranges.size() == 1) { return 0; }

  size_t l = 0;
  size_t h = this->ranges.size() - 1;

  while(l != h)
  {
    size_t const mid = l + ((1 + h - l) / 2);
    if(this->ranges[mid]->first > key) { h = mid - 1; }
    else { l = mid; }
  }

  log_assert(this->disableChecks() || l < this->ranges.size());
  log_assert(this->disableChecks()
      || (key >= this->ranges[l]->first && key <= this->ranges[l]->last));

  return l;
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* WireSet<Wire_T, Number_T>::findRange(
    wtk::index_t key) const
{
  return this->ranges[this->findRangeIdx(key)];
}

template<typename Wire_T, typename Number_T>
void WireSetWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  // this->wires might be nullptr until evaluation, so I'll skip checks

  ref->setIndirect(this, idx);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* WireSetWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  // this->wires might be nullptr until evaluation, so I'll skip checks

  IndirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->indirectWireRangeRef.allocate();
  ref->indirect = this;
  ref->offset = first - this->first;

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* WireSetWireRangeRef<Wire_T, Number_T>::deref(wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->wires != nullptr);
  log_assert(this->wires->ranges.size() > 0);
  log_assert(this->wires->ranges[0]->first == this->first);
  log_assert(this->wires->ranges.back()->last == this->last);

  wtk::index_t idx_adj = idx - this->first;
  return this->wires->findRange(idx_adj)->deref(idx_adj);
}

template<typename Wire_T, typename Number_T>
ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>
  ::ShortcutLoopWireSetWireRangeRef(
    WireSet<Wire_T, Number_T>* const w, Expr&& e)
  : wires(w), offsetExpr(std::move(e)) { }

template<typename Wire_T, typename Number_T>
void ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->wires != nullptr);

  ref->setIndirect(this, idx);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>
  ::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  log_assert(this->wires != nullptr);

  IndirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->indirectWireRangeRef.allocate();
  ref->indirect = this;
  ref->offset = first - this->first;

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>::deref(
    wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->wires != nullptr);

  wtk::index_t idx_adj = this->mappingOffset + idx - this->first;
  return this->wires->findRange(idx_adj)->deref(idx_adj);
}

template<typename Wire_T, typename Number_T>
void ShortcutLoopWireSetWireRangeRef<Wire_T, Number_T>::update(
    std::vector<wtk::index_t>& stack)
{
  this->mappingOffset = this->offsetExpr.eval(stack);
}

template<typename Wire_T, typename Number_T>
ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>
  ::ShortcutLoopIndirectWireRangeRef(
    WireRange<Wire_T, Number_T>* const i, Expr&& e)
  : indirect(i), offsetExpr(std::move(e)) { }

template<typename Wire_T, typename Number_T>
void ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->indirect != nullptr);

  ref->setIndirect(this, idx);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>
  ::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  log_assert(this->indirect != nullptr);

  IndirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->indirectWireRangeRef.allocate();
  ref->indirect = this;
  ref->offset = first - this->first;

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>::deref(
    wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->indirect != nullptr);

  wtk::index_t idx_adj = this->mappingOffset + idx - this->first;
  return this->indirect->deref(idx_adj);
}

template<typename Wire_T, typename Number_T>
void ShortcutLoopIndirectWireRangeRef<Wire_T, Number_T>::update(
    std::vector<wtk::index_t>& stack)
{
  this->mappingOffset = this->offsetExpr.eval(stack);
}

template<typename Wire_T, typename Number_T>
SoftLoopWireRangeRef<Wire_T, Number_T>::SoftLoopWireRangeRef(
    WireSet<Wire_T, Number_T> const* const w,
    size_t const n_mappings)
  : wires(w)
{
  this->translations.reserve(n_mappings);
  this->mappings.reserve(n_mappings);
}


template<typename Wire_T, typename Number_T>
void SoftLoopWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->wires != nullptr);

  ref->setIndirect(this, idx);
}

template<typename Wire_T, typename Number_T>
WireRange<Wire_T, Number_T>* SoftLoopWireRangeRef<Wire_T, Number_T>::ref(
    wtk::index_t const first, wtk::index_t const last,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  log_assert(first >= this->first && last <= this->last); (void) last;
  log_assert(this->wires != nullptr);

  IndirectWireRangeRef<Wire_T, Number_T>* ref =
    pool->indirectWireRangeRef.allocate();
  ref->indirect = this;
  ref->offset = first - this->first;

  return ref;
}

template<typename Wire_T, typename Number_T>
Wire_T* SoftLoopWireRangeRef<Wire_T, Number_T>::deref(wtk::index_t const idx)
{
  log_assert(idx >= this->first && idx <= this->last);
  log_assert(this->wires != nullptr);
  log_assert(this->wires->findRange(idx) != nullptr);

  // TODO determine if short-circuit here is effective.
  // if(this->ranges.size() == 1) { return 0; }

  size_t l = 0;
  size_t h = this->translations.size() - 1;

  while(l != h)
  {
    size_t const mid = (1 + l + h) / 2;
    if(this->translations[mid].first > idx) { h = mid - 1; }
    else { l = mid; }
  }

  log_assert(l < this->translations.size());
  log_assert(idx >= this->translations[l].first
      && idx <= this->translations[l].last);

  wtk::index_t idx_adj =
    this->translations[l].offset + idx - this->translations[l].first;
  return this->wires->findRange(idx_adj)->deref(idx_adj);
}

template<typename Wire_T, typename Number_T>
void SoftLoopWireRangeRef<Wire_T, Number_T>::update(
    std::vector<wtk::index_t>& stack)
{
  this->translations.clear();

  wtk::index_t place = 0;
  for(size_t i = 0; i < this->mappings.size(); i++)
  {
    wtk::index_t const f = this->mappings[i].first.eval(stack);
    wtk::index_t const l = this->mappings[i].last.eval(stack);
    wtk::index_t const span = f - l;

    this->translations.emplace_back(place, place + span, f);

    place += span + 1;
  }
}

} } // namespace wtk::bolt
