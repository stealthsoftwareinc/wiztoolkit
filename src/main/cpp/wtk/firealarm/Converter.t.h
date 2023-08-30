/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Number_T, typename OutWire_T, typename InWire_T>
void Converter<Number_T, OutWire_T, InWire_T>::convert(
    Wire<OutWire_T>* const out_wires, Wire<InWire_T> const* const in_wires,
    bool modulus)
{
  (*this->count)++;
  Number_T val(0);
  for(size_t i = 0; i < this->inLength; i++)
  {
    val = (val * this->inPrime) + static_cast<Number_T>(in_wires[i].value);
  }

  this->outTypeCounter->converts += this->outLength;
  this->outTypeCounter->increment(this->outLength);
  for(size_t i = 0; i < this->outLength; i++)
  {
    out_wires[this->outLength - 1 - i].value =
      static_cast<OutWire_T>(val % this->outPrime);
    out_wires[this->outLength - 1 - i].counter = this->outTypeCounter;
    val /= this->outPrime;
  }

  if(!modulus && val != 0)
  {
    log_error(
        "%s:%zu: Conversion would overflow in @no_modulus mode (%s overflow)",
        this->fileName, this->lineNum, wtk::utils::dec(val).c_str());
    this->ok = false;
  }
}

template<typename Number_T, typename OutWire_T, typename InWire_T>
bool Converter<Number_T, OutWire_T, InWire_T>::check() { return this->ok; }

} } // namespace wtk::firealarm
