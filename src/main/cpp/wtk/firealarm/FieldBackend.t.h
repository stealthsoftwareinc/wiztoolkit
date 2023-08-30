/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::enableTrace(
    wtk::utils::Indent const* const idt)
{
  this->trace = true;
  this->indent = idt;
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::assign(
    Wire<Wire_T>* const element, Number_T&& value)
{
  this->counter->assign++;
  this->counter->increment();
  element->value = static_cast<Wire_T>(value);
  element->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(element->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::copy(
    Wire<Wire_T>* const element, Wire<Wire_T> const* value)
{
  this->counter->copy++;
  this->counter->increment();
  element->value = value->value;
  element->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(element->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::addGate(Wire<Wire_T>* const out,
    Wire<Wire_T> const* left, Wire<Wire_T> const* right)
{
  this->counter->add++;
  this->counter->increment();
  out->value =
    static_cast<Wire_T>((left->value + right->value) % this->primeWire);
  out->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(out->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::mulGate(Wire<Wire_T>* const out,
    Wire<Wire_T> const* left, Wire<Wire_T> const* right)
{
  this->counter->mul++;
  this->counter->increment();
  out->value =
    static_cast<Wire_T>((left->value * right->value) % this->primeWire);
  out->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(out->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::addcGate(
    Wire<Wire_T>* const out, Wire<Wire_T> const* left, Number_T&& right)
{
  this->counter->mul++;
  this->counter->increment();
  out->value = static_cast<Wire_T>(
      (left->value + static_cast<Wire_T>(right)) % this->primeWire);
  out->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(out->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::mulcGate(
    Wire<Wire_T>* const out, Wire<Wire_T> const* left, Number_T&& right)
{
  this->counter->mul++;
  this->counter->increment();
  out->value = static_cast<Wire_T>(
      (left->value * static_cast<Wire_T>(right)) % this->primeWire);
  out->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(out->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::assertZero(Wire<Wire_T> const* left)
{
  if(this->trace)
  {
    log_info("%s:%zu: %s<- %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(left->value).c_str());
  }

  this->counter->assertZero += 1;
  if(!this->suppressAsserts && left->value != 0)
  {
    log_error("%s:%zu: Assert zero failed at value %s",
        this->fileName, this->lineNum, wtk::utils::dec(left->value).c_str());
    this->fail = true;
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::publicIn(
    Wire<Wire_T>* const element, Number_T&& value)
{
  this->counter->privateIn++;
  this->counter->increment();
  element->value = static_cast<Wire_T>(value);
  element->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(element->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
void FieldBackend<Number_T, Wire_T>::privateIn(
    Wire<Wire_T>* const element, Number_T&& value)
{
  this->counter->privateIn++;
  this->counter->increment();
  element->value = static_cast<Wire_T>(value);
  element->counter = this->counter;

  if(this->trace)
  {
    log_info("%s:%zu: %s-> %s", this->fileName, this->lineNum,
        this->indent->get(), wtk::utils::dec(element->value).c_str());
  }
}

template<typename Number_T, typename Wire_T>
bool FieldBackend<Number_T, Wire_T>::check()
{
  return !this->fail;
}

template<typename Number_T, typename Wire_T>
Number_T FieldBackend<Number_T, Wire_T>::getExtendedWitness(
    Wire<Wire_T> const* wire)
{
  return wire->value;
}

template<typename Number_T, typename Wire_T>
wire_idx FieldBackend<Number_T, Wire_T>::getExtendedWitnessIdx(
    Wire<Wire_T> const* wire)
{
  return static_cast<wire_idx>(wire->value);
}

} } // namespace wtk::firealarm
