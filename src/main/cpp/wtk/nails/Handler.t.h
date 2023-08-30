/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T>
bool Handler<Number_T>::addGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  log_debug("%zu: add gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->addGate(out, left, right, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->addGate(out, left, right, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::mulGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  log_debug("%zu: mul gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->mulGate(out, left, right, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->mulGate(out, left, right, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::addcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  log_debug("%zu: addc gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->addcGate(out, left, std::move(right), type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->addcGate(out, left, std::move(right), type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::mulcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  log_debug("%zu: mulc gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->mulcGate(out, left, std::move(right), type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->mulcGate(out, left, std::move(right), type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::copy(
    wire_idx const out, wire_idx const left, type_idx const type)
{
  log_debug("%zu: copy gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->copy(out, left, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->copy(out, left, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::copyMulti(wtk::circuit::CopyMulti* multi)
{
  log_debug("%zu: copy gate", this->lineNum);

  if(UNLIKELY((size_t) multi->type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum,
        (unsigned int) multi->type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->copyMulti(multi);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->copyMulti(multi);
  }
}

template<typename Number_T>
bool Handler<Number_T>::assign(wire_idx const out,
    Number_T&& left, type_idx const type)
{
  log_debug("%zu: assign gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->assign(out, std::move(left), type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->assign(out, std::move(left), type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::assertZero(
    wire_idx const left, type_idx const type)
{
  log_debug("%zu: assert zero gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->assertZero(left, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->assertZero(left, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::publicIn(wire_idx const out, type_idx const type)
{
  log_debug("%zu: instance gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->publicIn(out, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->publicIn(out, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::publicInMulti(
    wtk::circuit::Range* out, type_idx const type)
{
  log_debug("%zu: instance gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->publicInMulti(out, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->publicInMulti(out, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::privateIn(wire_idx const out, type_idx const type)
{
  log_debug("%zu: witness gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->privateIn(out, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->privateIn(out, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::privateInMulti(
    wtk::circuit::Range* out, type_idx const type)
{
  log_debug("%zu: witness gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->privateInMulti(out, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->privateInMulti(out, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::convert(
    wire_idx const first_out, wire_idx const last_out,
    type_idx const out_type,
    wire_idx const first_in, wire_idx const last_in,
    type_idx const in_type, bool modulus)
{
  log_debug("%zu: convert gate", this->lineNum);

  if(UNLIKELY((size_t) out_type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) out_type);
    return false;
  }

  if(UNLIKELY((size_t) in_type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) in_type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->convert(
        first_out, last_out, out_type, first_in, last_in, in_type, modulus);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->convert(
        first_out, last_out, out_type, first_in, last_in, in_type, modulus);
  }
}

template<typename Number_T>
bool Handler<Number_T>::newRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  log_debug("%zu: new gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->newRange(first, last, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->newRange(first, last, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::deleteRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  log_debug("%zu: delete gate", this->lineNum);

  if(UNLIKELY((size_t) type >= this->interpreter->interpreters.size()))
  {
    log_error("%s:%zu: Type index %u is not defined.",
        this->interpreter->fileName, this->lineNum, (unsigned int) type);
    return false;
  }

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->deleteRange(first, last, type);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->deleteRange(first, last, type);
  }
}

template<typename Number_T>
bool Handler<Number_T>::startFunction(
    wtk::circuit::FunctionSignature&& signature)
{
  log_debug("%zu: start function %s", this->lineNum, signature.name.c_str());

  if(UNLIKELY(this->interpreter->functions.find(signature.name.c_str())
        != this->interpreter->functions.end()))
  {
    log_error("%s:%zu: Function \'%s\' is already defined.",
        this->interpreter->fileName, this->lineNum, signature.name.c_str());
    return false;
  }

  for(size_t i = 0; i < signature.outputs.size(); i++)
  {
    if(UNLIKELY((size_t) signature.outputs[i].type
        > this->interpreter->interpreters.size()))
    {
      log_error("%s:%zu: Type index %u is not defined.",
          this->interpreter->fileName, this->lineNum,
          (unsigned int) signature.outputs[i].type);
      return false;
    }
  }

  for(size_t i = 0; i < signature.inputs.size(); i++)
  {
    if(UNLIKELY((size_t) signature.inputs[i].type
        > this->interpreter->interpreters.size()))
    {
      log_error("%s:%zu: Type index %u is not defined.",
          this->interpreter->fileName, this->lineNum,
          (unsigned int) signature.inputs[i].type);
      return false;
    }
  }

  this->sigConstruction = std::move(signature);

  return true;
}

template<typename Number_T>
bool Handler<Number_T>::regularFunction()
{
  this->funcConstruction =
    this->functionFactory->createFunction(std::move(this->sigConstruction));
  return true;
}

template<typename Number_T>
bool Handler<Number_T>::endFunction()
{
  if(UNLIKELY(!this->funcConstruction->typeCheck(this->interpreter)))
  {
    return false;
  }

  this->interpreter->functions[this->funcConstruction->signature.name.c_str()]
    = this->funcConstruction;
  this->funcConstruction = nullptr;

  return true;
}

template<typename Number_T>
bool Handler<Number_T>::pluginFunction(
    wtk::circuit::PluginBinding<Number_T>&& binding)
{
  PluginFunction<Number_T>* const pf = this->pluginPool.allocate(
      1, std::move(this->sigConstruction), std::move(binding));

  pf->operation = this->pluginsManager->create(&pf->signature, &pf->binding);
  if(pf->operation == nullptr)
  {
    return false;
  }
  else
  {
    this->interpreter->functions[pf->signature.name.c_str()] = pf;
    return true;
  }
}

template<typename Number_T>
bool Handler<Number_T>::invoke(wtk::circuit::FunctionCall* const call)
{
  log_debug("%zu: invoke, %s",
      this->lineNum, call->name.c_str());

  if(this->funcConstruction == nullptr)
  {
    this->interpreter->lineNum = this->lineNum;
    return this->interpreter->invoke(call);
  }
  else
  {
    this->funcConstruction->lineNum = this->lineNum;
    return this->funcConstruction->invoke(std::move(*call));
  }
}

} } // namespace wtk::nails
