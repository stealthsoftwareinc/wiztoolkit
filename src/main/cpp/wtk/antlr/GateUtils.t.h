/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_GATE_UTILS_H_
#define WTK_ANTLR_GATE_UTILS_H_

#include <wtk/IRParameters.h>
#include <wtk/utils/hints.h>

#include <wtk/antlr/SIEVEIRParser.h>

namespace wtk {
namespace antlr {

using namespace wtk_gen_antlr;

ALWAYS_INLINE static inline bool checkBinaryGate(
    SIEVEIRParser::BinaryGateTypeContext* ctx, GateSet const* gateSet)
{
  switch(gateSet->gateSet)
  {
  case GateSet::arithmetic:
  {
    return (ctx->ADD() != nullptr && gateSet->enableAdd)
      || (ctx->MUL() != nullptr && gateSet->enableMul);
  }
  case GateSet::boolean:
  {
    return (ctx->XOR() != nullptr && gateSet->enableXor)
      || (ctx->AND() != nullptr && gateSet->enableAnd);
  }
  case GateSet::invalid:
  {
    return false;
  }
  }

  return false;
}

ALWAYS_INLINE static inline bool checkBinaryConstGate(
    SIEVEIRParser::BinaryConstGateTypeContext* ctx, GateSet const* gateSet)
{
  switch(gateSet->gateSet)
  {
  case GateSet::arithmetic:
  {
    return (ctx->ADDC() != nullptr && gateSet->enableAddC)
      || (ctx->MULC() != nullptr && gateSet->enableMulC);
  }
  case GateSet::boolean:
  {
    return false;
  }
  case GateSet::invalid:
  {
    return false;
  }
  }

  return false;
}

ALWAYS_INLINE static inline bool checkUnaryGate(
    SIEVEIRParser::UnaryGateTypeContext* ctx, GateSet const* gateSet)
{
  switch(gateSet->gateSet)
  {
  case GateSet::arithmetic:
  {
    return false;
  }
  case GateSet::boolean:
  {
    return (ctx->NOT() != nullptr && gateSet->enableNot);
  }
  case GateSet::invalid:
  {
    return false;
  }
  }

  return false;
}

} } // namespace wtk::antlr

#endif//WTK_ANTLR_GATE_UTILS_H_
