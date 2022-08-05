/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
void Bolt<Wire_T, Number_T>::findWire(
    wtk::index_t const idx, WireRef<Wire_T, Number_T>* const ref)
{
  WireRange<Wire_T, Number_T>* range = this->wires.findRange(idx);
  range->ref(idx, ref);
}

template<typename Wire_T, typename Number_T>
void Bolt<Wire_T, Number_T>::findRanges(
    wtk::index_t const first, wtk::index_t const last,
    WireSet<Wire_T, Number_T>* const wires, wtk::index_t * const place,
    WireRangePool<Wire_T, Number_T>* const pool)
{
  size_t range_idx = this->wires.findRangeIdx(first);
  wtk::index_t adj_first = first;

  // The wire-range could be a sequence of "continuous-discontiguities",
  // so loop a bit.
  do
  {
    wtk::index_t adj_last = this->wires.ranges[range_idx]->last < last
      ? this->wires.ranges[range_idx]->last : last;

    WireRange<Wire_T, Number_T>* out_range =
      this->wires.ranges[range_idx]->ref(adj_first, adj_last, pool);
    out_range->setRange(*place, *place + adj_last - adj_first);
    *place += 1 + adj_last - adj_first;
    range_idx += 1;
    wires->ranges.push_back(out_range);
    adj_first = adj_last + 1;
  }
  while(adj_first <= last);
}

} } // namespace wtk::bolt
