/**
 * Copyright (C) 2022 Stealth Software Technolgies, Inc.
 */

#ifndef WTK_BOLT_BOOLEAN_PLASMASNOOZE_HANDLER_H_
#define WTK_BOLT_BOOLEAN_PLASMASNOOZE_HANDLER_H_

#include <cstddef>
#include <cstdint>

#include <wtk/index.h>
#include <wtk/BooleanStreamHandler.h>
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
template<typename Wire_T>
class BooleanPLASMASnoozeHandler : public wtk::BooleanStreamHandler
{
private:

  // the backend to use
  Backend<Wire_T, uint8_t>* const backend;

  // Membership for the wireSet (which itself has heuristic membership)
  wtk::utils::SkipList<wtk::index_t> assigned;

  // A global scope wire-set.
  PLASMASnoozeWireSet<Wire_T, uint8_t> wireSet;

  // The instance and witness streams
  wtk::InputStream<uint8_t>* const instanceStream;
  wtk::InputStream<uint8_t>* const witnessStream;

  // Indicates if there is a failure, and to stop further processing.
  PLASMASnoozeStatus status = PLASMASnoozeStatus::well_formed;
  bool failure() { return this->status != PLASMASnoozeStatus::well_formed; }

public:

  BooleanPLASMASnoozeHandler(
      Backend<Wire_T, uint8_t>* const b,
      wtk::InputStream<uint8_t>* const ins,
      wtk::InputStream<uint8_t>* const wit)
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

    uint8_t n(0);
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

    uint8_t n(0);
    if(this->witnessStream != nullptr && UNLIKELY(UNLIKELY(
            this->witnessStream->next(&n) != wtk::StreamStatus::success)
          || UNLIKELY(n >= this->backend->prime)))
    {
      log_error("bad witness");
      this->status = PLASMASnoozeStatus::bad_stream;
      return;
    }

    this->backend->witness(wire, n);
  }

  void handleXor(wtk::index_t const out, wtk::index_t const left,
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

    this->backend->xorGate(out_wire, left_wire, right_wire);
  }

  void handleAnd(wtk::index_t const out, wtk::index_t const left,
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

    this->backend->andGate(out_wire, left_wire, right_wire);
  }

  void handleNot(wtk::index_t const out, wtk::index_t const left) override
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

    this->backend->notGate(out_wire, left_wire);
  }

  void handleAssign(wtk::index_t const out, uint8_t const right) override
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

