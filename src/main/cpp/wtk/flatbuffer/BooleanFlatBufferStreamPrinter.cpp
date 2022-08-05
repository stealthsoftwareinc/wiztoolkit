/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/flatbuffer/BooleanFlatBufferStreamPrinter.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

namespace wtk {
namespace flatbuffer {

void BooleanFlatBufferStreamPrinter::flushBuffer()
{
  std::vector<flatbuffers::Offset<Function>> functions;
  this->writeRelation(&this->directivesList, &functions);
  this->directivesList.clear();
}

void BooleanFlatBufferStreamPrinter::checkBuffer()
{
  if(this->builder.GetSize() + this->directivesList.size() * sizeof(uint32_t)
      > FLATBUFFERS_MAX_BUFFER_SIZE - (2048 + this->characteristicLen))
  {
    this->flushBuffer();
  }
}

BooleanFlatBufferStreamPrinter::BooleanFlatBufferStreamPrinter(
    FILE* ofile) : FlatBufferPrinter<uint8_t>(ofile) { }

void BooleanFlatBufferStreamPrinter::handleInstance(
    wtk::index_t const idx)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateInstance, CreateGateInstance(
          this->builder, CreateWire(this->builder, idx)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleShortWitness(
    wtk::index_t const idx)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateWitness, CreateGateWitness(
          this->builder, CreateWire(this->builder, idx)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleAnd(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateAnd, CreateGateAnd(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          CreateWire(this->builder, right)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleXor(
    wtk::index_t const out, wtk::index_t const left, wtk::index_t const right)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateXor, CreateGateXor(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left),
          CreateWire(this->builder, right)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleNot(
    wtk::index_t const out, wtk::index_t const left)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateNot, CreateGateNot(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleCopy(
    wtk::index_t const out, wtk::index_t const left)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateCopy, CreateGateCopy(this->builder,
          CreateWire(this->builder, out),
          CreateWire(this->builder, left)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleAssign(
    wtk::index_t const out, uint8_t const left)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateConstant,
        CreateGateConstant(this->builder,
          CreateWire(this->builder, out),
          flattenNumber(&this->builder, left)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleAssertZero(
    wtk::index_t const in)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateAssertZero, CreateGateAssertZero(
          this->builder, CreateWire(this->builder, in)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleDeleteSingle(
    wtk::index_t const in)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, in),
          // I think this next line should create a "null" wire index
          flatbuffers::Offset<Wire>()).Union()));
}

void BooleanFlatBufferStreamPrinter::handleDeleteRange(
    wtk::index_t const first, wtk::index_t const last)
{
  this->checkBuffer();
  this->directivesList.emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, first),
          CreateWire(this->builder, last)).Union()));
}

void BooleanFlatBufferStreamPrinter::handleEnd()
{
  this->flushBuffer();
}

} } // namespace wtk::flatbuffer
