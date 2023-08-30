/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T, typename OutWire_T, typename InWire_T>
bool LeadConverter<Number_T, OutWire_T, InWire_T>::convert(
    wire_idx const first_out, wire_idx const last_out,
    TypeInterpreter<Number_T>* const interpreter_out,
    wire_idx const first_in, wire_idx const last_in,
    TypeInterpreter<Number_T>* const interpreter_in, bool modulus)
{
  Scope<OutWire_T>* const out_scope =
    static_cast<LeadTypeInterpreter<Number_T, OutWire_T>* const>(
        interpreter_out)->top();
  Scope<InWire_T>* const in_scope =
    static_cast<LeadTypeInterpreter<Number_T, InWire_T>* const>(
        interpreter_in)->top();

  ScopeError err = ScopeError::success;

  InWire_T const* const in_wires =
    in_scope->findInputs(first_in, last_in, &err);
  if(in_wires == nullptr)
  {
    log_error("%s:%zu: (input range $%" PRIu64 " ... $%" PRIu64 ") %s",
        this->fileName, this->lineNum, first_in, last_in,
        scopeErrorString(err));
    return false;
  }

  OutWire_T* const out_wires =
    out_scope->findOutputs(first_out, last_out, &err);
  if(out_wires == nullptr)
  {
    log_error("%s:%zu: (output range $%" PRIu64 " ... $%" PRIu64 ") %s",
        this->fileName, this->lineNum, first_out, last_out,
        scopeErrorString(err));
    return false;
  }

  for(size_t i = 0; i < this->converter->outLength; i++)
  {
    new(out_wires + i) OutWire_T();
  }

  log_assert(this->converter->outLength == 1 + last_out - first_out);
  log_assert(this->converter->inLength == 1 + last_in - first_in);

  this->converter->lineNum = this->lineNum;
  this->converter->convert(out_wires, in_wires, modulus);
  out_scope->assigned.insert(first_out, last_out);
  out_scope->active.insert(first_out, last_out);
  return true;
}

} } // namespace wtk::nails
