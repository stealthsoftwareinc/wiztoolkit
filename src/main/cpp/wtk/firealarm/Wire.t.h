/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Element_T>
Wire<Element_T>::Wire(Wire&& move)
  : value(std::move(move.value)), counter(move.counter)
{
  move.counter = nullptr;
}

template<typename Element_T>
Wire<Element_T>& Wire<Element_T>::operator=(Wire<Element_T>&& move)
{
  this->value = std::move(move.value);
  this->counter = move.counter;
  move.counter = nullptr;
}

template<typename Element_T>
Wire<Element_T>::~Wire()
{
  if(this->counter != nullptr) { this->counter->decrement(); }
}

} } // namespace wtk::firealarm
