/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace circuit {

template<typename Number_T>
bool operator== (
    PluginBinding<Number_T> const& l, PluginBinding<Number_T> const& r)
{
  if(l.name != r.name || l.operation != r.operation) { return false; }

  if(l.parameters.size() != r.parameters.size()) { return false; }

  for(size_t i = 0; i < l.parameters.size(); i++)
  {
    if(l.parameters[i].form != r.parameters[i].form) { return false; }
    switch(l.parameters[i].form)
    {
    case PluginBinding<Number_T>::Parameter::textual:
    {
      if(l.parameters[i].text != r.parameters[i].text) { return false; }
      break;
    }
    case PluginBinding<Number_T>::Parameter::numeric:
    {
      if(l.parameters[i].numeric != r.parameters[i].numeric) { return false; }
      break;
    }
    }
  }

  return true;
}

template<typename Number_T>
bool operator!= (
    PluginBinding<Number_T> const& l, PluginBinding<Number_T> const& r)
{
  return !(l == r);
}

template<typename Number_T>
Number_T TypeSpec<Number_T>::maxValue() const
{
  switch(this->variety)
  {
  case field:
  {
    return this->prime;
  }
  case ring:
  {
    return Number_T(1) << this->bitWidth;
  }
  case plugin:
  {
    return Number_T(0);
  }
  }

  return Number_T(0);
}

template<typename Number_T>
bool TypeSpec<Number_T>::isBooleanField() const
{
  return this->variety == field && this->prime == Number_T(2);
}

template<typename Number_T>
bool operator== (
    TypeSpec<Number_T> const& l, TypeSpec<Number_T> const& r)
{
  if(l.variety != r.variety) { return false; }

  switch(l.variety)
  {
  case TypeSpec<Number_T>::field:
  {
    if(l.prime != r.prime) { return false; }
    break;
  }
  case TypeSpec<Number_T>::ring:
  {
    if(l.bitWidth != r.bitWidth) { return false; }
    break;
  }
  case TypeSpec<Number_T>::plugin:
  {
    if(l.binding != r.binding) { return false; }
    break;
  }
  }

  return true;
}

template<typename Number_T>
bool operator!= (
    TypeSpec<Number_T> const& l, TypeSpec<Number_T> const& r)
{
  return !(l == r);
}

inline bool operator==(ConversionSpec const& a, ConversionSpec const& b)
{
  return a.outType == b.outType && a.inType == b.inType
    && a.outLength == b.outLength && a.inLength == b.inLength;
}

} } // namespace wtk::circuit

namespace std {

// Hash implementation for ConversionSpec, because an expected use-case
// is as the key to a hash-map.
//
// This hash was chosen from amongst a few naive candidates as it yeilded
// the best results with some expected field-conversion tests.
//
// It is based on accumulation with a prime, however it is modified to use
// XOR and uses different primes with each step.
//
// See here for more info:
//   https://gitlab.stealthsoftwareinc.com/stealth/wiztoolkit/-/snippets/33
template<>
struct hash<wtk::circuit::ConversionSpec>
{
  using result_type = size_t;

  result_type operator() (wtk::circuit::ConversionSpec const& c) const
  {
    return ((((size_t) c.outType * 113 ^ (size_t)c.inType)
          * 127 ^ c.outLength) * 131 ^ c.inLength) * 137;
  }
};

} // namespace std

