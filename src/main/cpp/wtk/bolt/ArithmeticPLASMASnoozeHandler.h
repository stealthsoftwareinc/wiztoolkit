/**
 * Copyright (C) 2022 Stealth Software Technolgies, Inc.
 */

#ifndef WTK_BOLT_ARITHMETIC_PLASMASNOOZE_HANDLER_H_
#define WTK_BOLT_ARITHMETIC_PLASMASNOOZE_HANDLER_H_

#include <cstddef>
#include <cstdint>

#include <wtk/index.h>
#include <wtk/ArithmeticStreamHandler.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/SkipList.h>

#include <wtk/bolt/PLASMASnooze.h>
#include <wtk/bolt/Backend.h>

#define LOG_IDENTIFIER "PLASMASnooze"
#include <stealth_logging.h>

namespace wtk {
namespace bolt {

/**
 * This is an implementation of the WTK Streaming API which passes off
 * to the Backend API for ZK or something.
 */
template<typename Wire_T, typename Number_T>
class ArithmeticPLASMASnoozeHandler
  : public wtk::ArithmeticStreamHandler<Number_T>
{
private:

  // the backend to use
  Backend<Wire_T, Number_T>* const backend;

  // Membership for the wireSet (which itself has heuristic membership)
  wtk::utils::SkipList<wtk::index_t> assigned;

  // A global scope wire-set.
  PLASMASnoozeWireSet<Wire_T, Number_T> wireSet;

  // The instance and witness streams
  wtk::InputStream<Number_T>* const instanceStream;
  wtk::InputStream<Number_T>* const witnessStream;

  // Indicates if there is a failure, and to stop further processing.
  PLASMASnoozeStatus status = PLASMASnoozeStatus::well_formed;
  bool failure() { return this->status != PLASMASnoozeStatus::well_formed; }

public:

  ArithmeticPLASMASnoozeHandler(
      Backend<Wire_T, Number_T>* const b,
      wtk::InputStream<Number_T>* const ins,
      wtk::InputStream<Number_T>* const wit)
    : backend(b), instanceStream(ins), witnessStream(wit) { }

  /**
   * Returns true on success/false on failure.
   */
  PLASMASnoozeStatus check()
  {
    return this->status;
  }

  void handleInstance(wtk::index_t const idx) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(!this->assigned.insert(idx)))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* wire = this->wireSet.insert(idx);
    log_assert(wire != nullptr);

    Number_T n(0);
    if(UNLIKELY(UNLIKELY(
            this->instanceStream->next(&n) != wtk::StreamStatus::success)
          || UNLIKELY(n >= this->backend->prime)))
    {
      log_error("bad instance");
      this->status = PLASMASnoozeStatus::bad_stream;
      return;
    }

    this->backend->instance(wire, n);
  }

  void handleShortWitness(wtk::index_t const idx) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(!this->assigned.insert(idx)))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* wire = this->wireSet.insert(idx);
    log_assert(wire != nullptr);

    Number_T n(0);
    if(this->witnessStream != nullptr)
    {
      if(UNLIKELY(UNLIKELY(
              this->witnessStream->next(&n) != wtk::StreamStatus::success)
            || UNLIKELY(n >= this->backend->prime)))
      {
        log_error("bad witness");
        this->status = PLASMASnoozeStatus::bad_stream;
        return;
      }

      this->backend->witness(wire, n);
    }
    else
    {
      Number_T zero(0);
      this->backend->witness(wire, zero);
    }
  }

  void handleAdd(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(!this->assigned.has(left))
          || UNLIKELY(!this->assigned.has(right))
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);
    Wire_T const* left_wire = this->wireSet.retrieve(left);
    Wire_T const* right_wire = this->wireSet.retrieve(right);

    log_assert(left_wire != nullptr
        && right_wire != nullptr
        && out_wire != nullptr);

    this->backend->addGate(out_wire, left_wire, right_wire);
  }

  void handleMul(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(!this->assigned.has(left))
          || UNLIKELY(!this->assigned.has(right))
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);
    Wire_T const* left_wire = this->wireSet.retrieve(left);
    Wire_T const* right_wire = this->wireSet.retrieve(right);

    log_assert(left_wire != nullptr
        && right_wire != nullptr
        && out_wire != nullptr);

    this->backend->mulGate(out_wire, left_wire, right_wire);
  }

  void handleAddC(wtk::index_t const out, wtk::index_t const left,
      Number_T const right) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(!this->assigned.has(left))
          || UNLIKELY(right >= this->backend->prime)
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);
    Wire_T const* left_wire = this->wireSet.retrieve(left);

    log_assert(out_wire != nullptr && left_wire != nullptr);

    this->backend->addcGate(out_wire, left_wire, right);
  }

  void handleMulC(wtk::index_t const out, wtk::index_t const left,
      Number_T const right) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(!this->assigned.has(left))
          || UNLIKELY(right >= this->backend->prime)
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);
    Wire_T const* left_wire = this->wireSet.retrieve(left);

    log_assert(out_wire != nullptr && left_wire != nullptr);

    this->backend->mulcGate(out_wire, left_wire, right);
  }

  void handleAssign(wtk::index_t const out, Number_T const right) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(right >= this->backend->prime)
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);

    log_assert(out_wire != nullptr);

    this->backend->assign(out_wire, right);
  }

  void handleCopy(wtk::index_t const out, wtk::index_t const left) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(!this->assigned.has(left))
          || UNLIKELY(!this->assigned.insert(out))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T* out_wire = this->wireSet.insert(out);
    Wire_T const* left_wire = this->wireSet.retrieve(left);

    log_assert(out_wire != nullptr && left_wire != nullptr);

    this->backend->copy(out_wire, left_wire);
  }

  void handleAssertZero(wtk::index_t const in) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(!this->assigned.has(in)))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }

    Wire_T const* wire = this->wireSet.retrieve(in);

    log_assert(wire != nullptr);

    this->backend->assertZero(wire);
  }

  void handleDeleteSingle(wtk::index_t const in) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(!this->assigned.remove(in)))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }
  }

  void handleDeleteRange(
      wtk::index_t const first, wtk::index_t const last) override
  {
    if(UNLIKELY(this->failure())) { return; }
    if(UNLIKELY(UNLIKELY(first > last)
        || UNLIKELY(!this->assigned.remove(first, last))))
    {
      log_error("Bad form");
      this->status = PLASMASnoozeStatus::bad_relation;
      return;
    }
  }
};

} } // namespace wtk::bolt

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_ARITHMETIC_PLASMASNOOZE_HANDLER_H_
