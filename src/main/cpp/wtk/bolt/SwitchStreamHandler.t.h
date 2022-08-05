/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace bolt {

template<typename Wire_T>
SwitchStreamHandler<Wire_T>::SwitchStreamHandler(size_t const t)
  : total(t) { }

template<typename Wire_T>
Wire_T* SwitchStreamHandler<Wire_T>::next(size_t const n)
{
  log_assert(this->place + n <= this->total);
  this->place += n; return &this->values[this->place - n];
}

template<typename Wire_T>
void SwitchStreamHandler<Wire_T>::reset()
{
  this->place = 0;
}

} } // namespace wtk::bolt
