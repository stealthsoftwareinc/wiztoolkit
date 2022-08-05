/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_NO_ZK_BACKEND_H_
#define WTK_BOLT_NO_ZK_BACKEND_H_

#include <wtk/IRParameters.h>

#include <wtk/bolt/Backend.h>

#define LOG_IDENTIFIER "no ZK"
#include <stealth_logging.h>

namespace wtk {
namespace bolt {

template<typename Wire_T, typename Number_T>
class NoZKBackend : public Backend<Wire_T, Number_T>
{
  // bools to enforce gateset restrictions. ZK backends need not bother
  // as these are more for testing WizToolKit's implementation.
  bool canAdd;
  bool canMul;
  bool canAddC;
  bool canMulC;

  bool canXor;
  bool canAnd;
  bool canNot;

  // caches success or failure to be reported by check()
  bool success = true;

public:
  NoZKBackend(Number_T const p, wtk::GateSet* gs)
    : Backend<Wire_T, Number_T>(p, gs->gateSet == wtk::GateSet::boolean),
      canAdd(gs->gateSet == wtk::GateSet::arithmetic && gs->enableAdd),
      canMul(gs->gateSet == wtk::GateSet::arithmetic && gs->enableMul),
      canAddC(gs->gateSet == wtk::GateSet::arithmetic && gs->enableAddC),
      canMulC(gs->gateSet == wtk::GateSet::arithmetic && gs->enableMulC),
      canXor(gs->gateSet == wtk::GateSet::boolean && gs->enableXor),
      canAnd(gs->gateSet == wtk::GateSet::boolean && gs->enableAnd),
      canNot(gs->gateSet == wtk::GateSet::boolean && gs->enableNot) { }

  void addGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override
  {
    if(UNLIKELY(!this->canAdd))
    {
      log_error("add gate forbidden");
      this->success = false;
    }

    *out = (*left + *right) % (Wire_T) this->prime;
  }

  void mulGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override
  {
    if(UNLIKELY(!this->canMul))
    {
      log_error("mul gate forbidden");
      this->success = false;
    }

    *out = (*left * *right) % (Wire_T) this->prime;
  }

  void addcGate(Wire_T* const out,
      Wire_T const* const left, Number_T const right) override
  {
    if(UNLIKELY(!this->canAddC))
    {
      log_error("addc gate forbidden");
      this->success = false;
    }

    *out = (*left + (Wire_T) right) % (Wire_T) this->prime;
  }

  void mulcGate(Wire_T* const out,
      Wire_T const* const left, Number_T const right) override
  {
    if(UNLIKELY(!this->canMulC))
    {
      log_error("mulc gate forbidden");
      this->success = false;
    }

    *out = (*left * (Wire_T) right) % (Wire_T) this->prime;
  }

  void xorGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override
  {
    if(UNLIKELY(!this->canXor))
    {
      log_error("xor gate forbidden");
      this->success = false;
    }

    *out = (*left ^ *right) & 0x01;
  }

  void andGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) override
  {
    if(UNLIKELY(!this->canAnd))
    {
      log_error("and gate forbidden");
      this->success = false;
    }

    *out = (*left & *right) & 0x01;
  }

  void notGate(Wire_T* const out, Wire_T const* const left) override
  {
    if(UNLIKELY(!this->canNot))
    {
      log_error("not gate forbidden");
      this->success = false;
    }

    *out = (~(*left)) & 0x01;
  }

  void copy(Wire_T* const out, Wire_T const* const left) override
  {
    *out = *left;
  }

  void assign(Wire_T* const out, Number_T const left) override
  {
    *out = (Wire_T) left;
  }

  void instance(Wire_T* const out, Number_T const left) override
  {
    *out = (Wire_T) left;
  }

  void witness(Wire_T* const out, Number_T const left) override
  {
    *out = (Wire_T) left;
  }

  void assertZero(Wire_T const* const wire) override
  {
    this->success = (*wire == (Wire_T) 0) && this->success;
  }

  bool check() override
  {
    return this->success;
  }
};

} } // namespace wtk::bolt

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_NO_ZK_BACKEND_H_
