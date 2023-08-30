/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_COUNTER_
#define WTK_FIREALARM_COUNTER_

#include <cstddef>

#define LOG_IDENTIFIER "counters"
#include <stealth_logging.h>

namespace wtk {
namespace firealarm {

// This is a counter for all the gate counts and directives that can be
// done within a single type.
//
// It also has a maximum active wire counter
struct TypeCounter
{
  // Counters for simple gates
  size_t add = 0;
  size_t mul = 0;

  size_t addc = 0;
  size_t mulc = 0;

  size_t copy = 0;
  size_t assign = 0;

  size_t assertZero = 0;

  size_t publicIn = 0;
  size_t privateIn = 0;

  // Counter for wires assigned by conversion gates
  size_t converts = 0;

  // Active wire counters for this field
  size_t currentActive = 0;
  size_t maximumActive = 0;

  // Active wire counters for all fields
  size_t* const totalCurrentActive;
  size_t* const totalMaximumActive;

  // Indicates if this type is a RAM type
  bool const ram;

  // counters for RAM
  size_t init = 0;
  size_t read = 0; // also used by non-ram for read assignments
  size_t write = 0;

  size_t totalAlloc = 0;
  size_t currentAlloc = 0;
  size_t maxAlloc = 0;

  TypeCounter(size_t* const tca, size_t* const tma, bool r = false)
    : totalCurrentActive(tca), totalMaximumActive(tma), ram(r) { }

  // Increment the maximum wire counters
  void increment(size_t const count = 1)
  {
    if(!this->ram)
    {
      this->currentActive += count;
      if(this->currentActive > this->maximumActive)
      {
        this->maximumActive = this->currentActive;
      }

      *this->totalCurrentActive += count;
      if(*this->totalCurrentActive > *this->totalMaximumActive)
      {
        *this->totalMaximumActive = *this->totalCurrentActive;
      }
    }
    else
    {
      this->totalAlloc += count;
      this->currentAlloc += count;
      if(this->currentAlloc > this->maxAlloc)
      {
        this->maxAlloc = this->currentAlloc;
      }
    }
  }

  void decrement(size_t const count = 1)
  {
    if(!this->ram)
    {
      this->currentActive -= count;
      *this->totalCurrentActive -= count;
    }
    else
    {
      this->currentAlloc -= count;
    }
  }

  void addTotal(TypeCounter const* const other)
  {
    if(!this->ram)
    {
      this->add += other->add;
      this->mul += other->mul;
      this->addc += other->addc;
      this->mulc += other->mulc;
      this->copy += other->copy;
      this->assign += other->assign;
      this->assertZero += other->assertZero;
      this->publicIn += other->publicIn;
      this->privateIn += other->privateIn;
      this->converts += other->converts;
      this->read += other->read;
    }
  }

  void print(bool const detail, char const* const name)
  {
    if(!this->ram)
    {
      log_info("Gate counts: %s", name);
      log_info("@add:            %zu", this->add);
      log_info("@mul:            %zu", this->mul);

      if(detail)
      {
        log_info("@addc:           %zu", this->addc);
        log_info("@mulc:           %zu", this->mulc);
        log_info("copy:            %zu", this->copy);
        log_info("assign:          %zu", this->assign);
        log_info("convert outputs: %zu", this->converts);
        log_info("@public_in:      %zu", this->publicIn);
        log_info("@private_in:     %zu", this->privateIn);
      }
      else
      {
        log_info("inputs:          %zu", this->publicIn + this->privateIn);
      }

      if(this->read != 0)
      {
        log_info("RAM Read:        %zu", this->read);
      }

      log_info("@assert_zero:    %zu", this->assertZero);
      log_info("total wires:     %zu", this->add + this->mul + this->addc
          + this->mulc + this->copy + this->assign + this->publicIn
          + this->privateIn + this->converts);

      if(detail)
      {
        log_info("maximum active wires: %zu\n", this->maximumActive);
      }
    }
    else
    {
      log_info("RAM Counts: %s", name);
      log_info("init:            %zu", this->init);
      log_info("read:            %zu", this->read);
      log_info("write:           %zu", this->write);

      log_info("total allocated: %zu", this->totalAlloc);
      log_info("max allocated:   %zu\n", this->maxAlloc);
    }
  }
};

} } // namespace wtk::firealarm

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_COUNTER_
