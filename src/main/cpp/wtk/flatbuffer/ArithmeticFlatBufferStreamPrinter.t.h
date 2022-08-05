/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

template<typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::flushBuffer()
{
  std::vector<flatbuffers::Offset<Function>> functions;
  this->writeRelation(&this->directivesList, &functions);
  this->directivesList.clear();
}

template<typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::checkBuffer()
{
  if(this->builder.GetSize() + this->directivesList.size() * sizeof(uint32_t)
      > FLATBUFFERS_MAX_BUFFER_SIZE - (2048 + this->characteristicLen))
  {
    this->flushBuffer();
  }
}

template <typename Number_T>
ArithmeticFlatBufferStreamPrinter<Number_T>::ArithmeticFlatBufferStreamPrinter(
    FILE* ofile) : FlatBufferPrinter<Number_T>(ofile) { }

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleInstance(
    wtk::index_t const idx)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateInstance, CreateGateInstance(
          this->builder, CreateWire(this->builder, idx)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleShortWitness(
    wtk::index_t const idx)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateWitness, CreateGateWitness(
          this->builder, CreateWire(this->builder, idx)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleAdd(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateAdd, CreateGateAdd(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          CreateWire(this->builder, right)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleMul(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateMul, CreateGateMul(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          CreateWire(this->builder, right)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleAddC(
    wtk::index_t const out, wtk::index_t const left, Number_T const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateAddConstant,
        CreateGateAddConstant(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          flattenNumber(&this->builder, right)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleMulC(
    wtk::index_t const out, wtk::index_t const left, Number_T const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateMulConstant,
        CreateGateMulConstant(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          flattenNumber(&this->builder, right)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleCopy(
    wtk::index_t const out, wtk::index_t const left)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateCopy, CreateGateCopy(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleAssign(
    wtk::index_t const out, Number_T const left)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateConstant,
        CreateGateConstant(this->builder,
          CreateWire(this->builder, out),
          flattenNumber(&this->builder, left)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleAssertZero(
    wtk::index_t const in)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateAssertZero, CreateGateAssertZero(
          this->builder, CreateWire(this->builder, in)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleDeleteSingle(
    wtk::index_t const in)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, in),
          // I think this next line should create a "null" wire index
          flatbuffers::Offset<Wire>()).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleDeleteRange(
    wtk::index_t const first, wtk::index_t const last)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, first),
          CreateWire(this->builder, last)).Union()));
}

template <typename Number_T>
void ArithmeticFlatBufferStreamPrinter<Number_T>::handleEnd()
{
  this->flushBuffer();
}

} } // namespace wtk::flatbuffer
