/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

/**
 * Knowing a few assumptions about this SkipList's usage I've decided to
 * implement it backwards. that is, higher numbers at the "front" and
 * lower numbers at the back.
 *
 * The first reason is that the expected use case should have almost
 * exclusively search misses. The second is that insertions are likely
 * to be somewhat sequential and increasing.
 */

template<typename Number_T>
struct Range
{
  Number_T last;
  Number_T first;

  std::unique_ptr<Range<Number_T>> next = nullptr;

  Range(Number_T l, Number_T f) : last(l), first(f) { }
  Range(Number_T l) : last(l), first(l) { }
};

template<typename Number_T>
SkipList<Number_T>::SkipList(SkipList<Number_T> const& copy)
{
  if(copy.list == nullptr) { return; }
  else
  {
    Range<Number_T>* copy_range = copy.list.get();
    this->list = std::unique_ptr<Range<Number_T>>(
        new Range<Number_T>(copy_range->last, copy_range->first));

    Range<Number_T>* new_range = this->list.get();
    copy_range = copy_range->next.get();
    while(copy_range != nullptr)
    {
      new_range->next = std::unique_ptr<Range<Number_T>>(
          new Range<Number_T>(copy_range->last, copy_range->first));

      new_range = new_range->next.get();
      copy_range = copy_range->next.get();
    }
  }
}

template<typename Number_T>
SkipList<Number_T>& SkipList<Number_T>::operator=(
    SkipList<Number_T> const& copy)
{
  this->~SkipList();
  new(this)SkipList<Number_T>(copy);
}

template<typename Number_T>
bool SkipList<Number_T>::integrityCheck() const
{
  Range<Number_T> const* place = this->list.get();

  while(place != nullptr)
  {
    if(place->last < place->first) { return false; }

    Range<Number_T> const* next = place->next.get();
    if(next != nullptr && place->first <= next->last + 1)
    {
      return false;
    }

    place = next;
  }

  return true;
}

template<typename Number_T>
std::string SkipList<Number_T>::toString() const
{
  Range<Number_T> const* place = this->list.get();
  std::string ret;

  while(place != nullptr)
  {
    ret += "[" + dec(place->last) + ", " + dec(place->first) + "] ";
    place = place->next.get();
  }

  return ret;
}

template<typename Number_T>
void SkipList<Number_T>::print() const
{
  Range<Number_T> const* place = this->list.get();

  printf("%s\n", this->toString().c_str());
}

template<typename Number_T>
bool SkipList<Number_T>::has(Number_T const n) const
{
  Range<Number_T> const* place = this->list.get();

  while(place != nullptr)
  {
    if(n > place->last) { return false; }
    else if(n >= place->first) { return true; }
    else { place = place->next.get(); }
  }

  return false;
}

template<typename Number_T>
bool SkipList<Number_T>::has(Number_T const first, Number_T const last) const
{
  Range<Number_T> const* place = this->list.get();

  while(place != nullptr)
  {
    if((place->last >= first && place->first <= first)
        || (place->last >= last && place->first <= last)
        || (place->last < last && place->first > first))
    {
      return true;
    }
    else if(first > place->last) { return false; }
    else { place = place->next.get(); }
  }

  return false;
}

template<typename Number_T>
bool SkipList<Number_T>::hasAll(
    Number_T const first, Number_T const last) const
{
  if(first > last) { return false; }

  Range<Number_T> const* place = this->list.get();

  while(place != nullptr)
  {
    if(place->last >= last && place->first <= first)
    {
      return true;
    }
    else if(first > place->last) { return false; }
    else { place = place->next.get(); }
  }

  return false;
}

template<typename Number_T>
bool SkipList<Number_T>::insert(Number_T const n)
{
  if(this->list == nullptr) // its the first range
  {
    this->list = std::unique_ptr<Range<Number_T>>(new Range<Number_T>(n));
    return true;
  }
  // its before the first range
  if(n > this->list->last + 1 && this->list->last + 1 > this->list->last)
  {
    std::unique_ptr<Range<Number_T>> next = std::move(this->list);
    this->list = std::unique_ptr<Range<Number_T>>(new Range<Number_T>(n));
    this->list->next = std::move(next);
    return true;
  }
  else // traverse the existing ranges
  {
    Range<Number_T>* place = this->list.get();
    Range<Number_T>* prev = place;

    while(place != nullptr)
    {
      if(n == place->last + 1 && n > place->last) // extend the last.
      {
        place->last = n;
        return true;
      }
      else if(n == place->first - 1) // extend the first.
      {
        place->first = n;

        if(place->next != nullptr
            && place->next->last == place->first - 1) // also join the next.
        {
          place->first = place->next->first;
          std::unique_ptr<Range<Number_T>> next(std::move(place->next));
          place->next = std::move(next->next);
        }
        return true;
      }
      else if(n < place->first
          && place->next != nullptr
          && n > place->next->last + 1) // between this range and the next.
      {
        std::unique_ptr<Range<Number_T>> next(std::move(place->next));
        place->next = std::unique_ptr<Range<Number_T>>(new Range<Number_T>(n));
        place->next->next = std::move(next);
        return true;
      }
      else if(n <= place->last && n >= place->first) // already inserted.
      {
        return false;
      }
      else // in a later range.
      {
        prev = place;
        place = place->next.get();
      }
    }

    // New last range.

    // Static analyzer comes up with a false positive here due to difficulties
    // recognizing null checks around std::unique_ptr.

    // NOLINTNEXTLINE
    prev->next = std::unique_ptr<Range<Number_T>>(new Range<Number_T>(n));

    return true;
  }
}

template<typename Number_T>
bool SkipList<Number_T>::insert(Number_T const first, Number_T const last)
{
  // invalid input range
  if(first > last) { return false; }

  if(this->list == nullptr) // the first range
  {
    this->list = std::unique_ptr<Range<Number_T>>(
        new Range<Number_T>(last, first));
    return true;
  }
  // before the first range
  if(first > this->list->last + 1 && this->list->last + 1 > this->list->last)
  {
    std::unique_ptr<Range<Number_T>> next = std::move(this->list);
    this->list = std::unique_ptr<Range<Number_T>>(
        new Range<Number_T>(last, first));
    this->list->next = std::move(next);
    return true;
  }
  else // traverse the existing ranges
  {
    Range<Number_T>* place = this->list.get();
    Range<Number_T>* prev = place;

    while(place != nullptr)
    {
      if(first == place->last + 1 && first > place->last) // extend the last.
      {
        place->last = last;
        return true;
      }
      else if(last == place->first - 1) // extend the first.
      {
        if(place->next != nullptr
            && first <= place->next->last) // fail if it would overlap the next
        {
          return false;
        }
        else if(place->next != nullptr
            && place->next->last == first - 1) // join the next
        {
          place->first = place->next->first;
          std::unique_ptr<Range<Number_T>> next(std::move(place->next));
          place->next = std::move(next->next);
        }
        else // it just extends
        {
          place->first = first;
        }
        return true;
      }
      else if(last < place->first
          && place->next != nullptr
          && first > place->next->last + 1) // between this range and the next.
      {
        std::unique_ptr<Range<Number_T>> next(std::move(place->next));
        place->next = std::unique_ptr<Range<Number_T>>(
            new Range<Number_T>(last, first));
        place->next->next = std::move(next);
        return true;
      }
      else if((place->last >= first && place->first <= first)
        || (place->last >= last && place->first <= last)
        || (place->last < last && place->first > first)) // already inserted.
      {
        return false;
      }
      else // in a later range.
      {
        prev = place;
        place = place->next.get();
      }
    }

    // New last range.

    // Static analyzer comes up with a false positive here due to difficulties
    // recognizing null checks around std::unique_ptr.

    // NOLINTNEXTLINE
    prev->next = std::unique_ptr<Range<Number_T>>(
        new Range<Number_T>(last, first));

    return true;
  }
}

template<typename Number_T>
bool SkipList<Number_T>::remove(Number_T const n)
{
  Range<Number_T>* place = this->list.get();

  while(place != nullptr)
  {
    if(n == place->last)
    {
      place->last--;
      return true;
    }
    else if(n == place->first)
    {
      place->first++;
      return true;
    }
    else if(n < place->last && n > place->first)
    {
      std::unique_ptr<Range<Number_T>> newRange(
          new Range<Number_T>(n - 1, place->first));
      newRange->next = std::move(place->next);
      place->first = n + 1;
      place->next = std::move(newRange);
      return true;
    }
    else if(n > place->last) { return false; }
    else
    {
      place = place->next.get();
    }
  }

  return false;
}

template<typename Number_T>
bool SkipList<Number_T>::remove(Number_T const first, Number_T const last)
{
  Range<Number_T>* place = this->list.get();
  std::unique_ptr<Range<Number_T>>* prev = &this->list;

  while(place != nullptr)
  {
    if(last == place->last && first == place->first)
    {
      *prev = std::move(place->next);
      return true;
    }
    else if(last == place->last && first > place->first)
    {
      place->last = first - 1;
      return true;
    }
    else if(first == place->first && last < place->last)
    {
      place->first = last + 1;
      return true;
    }
    else if(last < place->last && first > place->first)
    {
      std::unique_ptr<Range<Number_T>> newRange(
          new Range<Number_T>(first - 1, place->first));
      newRange->next = std::move(place->next);
      place->first = last + 1;
      place->next = std::move(newRange);
      return true;
    }
    else if(first > place->last) { return false; }
    else
    {
      prev = &(*prev)->next;
      place = place->next.get();
    }
  }

  return false;
}

// SkipLists are normally reverse stored, but for forEach we want forward
// traversal, So this unreverses them.
template<typename Number_T>
void for_each_helper(std::function<void(Number_T, Number_T)>& func,
    Range<Number_T> const* place)
{
  if(place != nullptr)
  {
    for_each_helper(func, place->next.get());
    func(place->first, place->last);
  }
}

template<typename Number_T>
void SkipList<Number_T>::forEach(
    std::function<void(Number_T, Number_T)> func) const
{
  for_each_helper(func, this->list.get());
}

// Recursive helper for forward traversal in forRange
template<typename Number_T>
void for_range_helper(Number_T const rf, Number_T const rl,
    std::function<void(Number_T, Number_T)>& func,
    Range<Number_T> const* place)
{
  if(place != nullptr)
  {
    if(rf >= place->first && rl <= place->last)
    {
      for_range_helper(rf, rl, func, place->next.get());
      func(rf, rl);
    }
    else if(rf <= place->first && rl >= place->last)
    {
      for_range_helper(rf, rl, func, place->next.get());
      func(place->first, place->last);
    }
    else if(rf <= place->last && rf >= place->first && rl >= place->last)
    {
      for_range_helper(rf, rl, func, place->next.get());
      func(rf, place->last);
    }
    else if(rf < place->first && rl >= place->first && rl <= place->last)
    {
      for_range_helper(rf, rl, func, place->next.get());
      func(place->first, rl);
    }
    else if(rl < place->last)
    {
      for_range_helper(rf, rl, func, place->next.get());
    }
    // else if(rl < place->last) { return; }
  }
}

template<typename Number_T>
void SkipList<Number_T>::forRange(
    Number_T const range_first, Number_T const range_last,
    std::function<void(Number_T, Number_T)> func) const
{
  for_range_helper(range_first, range_last, func, this->list.get());
}

template<typename Number_T>
bool SkipList<Number_T>::equivalent(
    SkipList<Number_T>const* const a, SkipList<Number_T> const* const b)
{
  Range<Number_T> const* a_place = a->list.get();
  Range<Number_T> const* b_place = b->list.get();

  // check that all elements have the same value.
  while(a_place != nullptr && b_place != nullptr)
  {
    if(a_place->last == b_place->last && a_place->first == b_place->first)
    {
      a_place = a_place->next.get();
      b_place = b_place->next.get();
    }
    else
    {
      return false;
    }
  }

  // check they have the same length
  return a_place == b_place;
}

template<typename Number_T>
void SkipList<Number_T>::clear()
{
  // should recurse on the range's destructor until all are deleted.
  this->list = nullptr;
}

} } // namespace wtk::utils
