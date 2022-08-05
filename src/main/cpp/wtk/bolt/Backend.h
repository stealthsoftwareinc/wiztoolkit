/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_BACKEND_H_
#define WTK_BOLT_BACKEND_H_

namespace wtk {
namespace bolt {

/**
 * An abstract class defining actions commonly taken by ZK backends.
 *
 * It  (along with all of BOLT) is templatized on the Wire_T, the backend's
 * notion of a wire object, and on the Number_T, the parser's notion of a
 * field-element constant.
 *
 * A backend can implement this interface for BOLT to use during evaluation.
 *
 * Each funciton is a handler for a different simple gate. In comparison to
 * the parser's StreamHandler APIs, wires are preallocated and reported as
 * pointers rather than indexes. BOLT does ensure that
 * out pointer != in pointer, but multiple in pointers could be equal.
 *
 * Do be aware that BOLT may reuse an out pointer without de/reconstructing
 * the Wire_T object.
 *
 * Handler functions return void and must cache any failures to be returned
 * when `check()` is called at the main level.
 */
template<typename Wire_T, typename Number_T>
class Backend
{
public:
  // The characteristic/prime
  Number_T const prime;
  // Indicates if the circuit is a boolean circuit
  bool const isBoolean;

  /**
   * P is the field's characteristic.
   *
   * is_boolean indicates if the circuit is a boolean circuit
   * (Otherwise consider it to be an arithmetic).
   */
  Backend(Number_T const p, bool const ib) : prime(p), isBoolean(ib) { }

  virtual void addGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) = 0;

  virtual void mulGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) = 0;

  virtual void addcGate(Wire_T* const out,
      Wire_T const* const left, Number_T const right) = 0;

  virtual void mulcGate(Wire_T* const out,
      Wire_T const* const left, Number_T const right) = 0;

  virtual void xorGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) = 0;

  virtual void andGate(Wire_T* const out,
      Wire_T const* const left, Wire_T const* const right) = 0;

  virtual void notGate(Wire_T* const out, Wire_T const* const left) = 0;

  virtual void copy(Wire_T* const out, Wire_T const* const left) = 0;

  virtual void assign(Wire_T* const out, Number_T const left) = 0;

  virtual void instance(Wire_T* const out, Number_T const left) = 0;

  virtual void witness(Wire_T* const out, Number_T const left) = 0;

  virtual void assertZero(Wire_T const* const wire) = 0;

  /**
   * Helper function for exponentiation in the default implementation of
   * arithmetic caseSelect(...).
   */
  void exponentiate(Wire_T* const out, Wire_T const* const base, Number_T exp);

  /**
   * Indicates if a wire is selected by setting selected_bit to either 0 or
   * 1 if the case_number and select_wire are non-equal or equal, respectively.
   */
  virtual void caseSelect(Wire_T* const selected_bit,
      Number_T const case_number, Wire_T const* const select_wire);

  /**
   * Implements a multiplexer by summing a column of dummy wires conditionally
   * on each selector bit. This is repeated for each column of dummies.
   * Unfortunately, this method must expose some of BOLT's internals.
   *
   * out: output wire (sum)
   * dummies: rectangular matrix, only one column should be used
   * selector_bits: vector of condition bits, guaranteed to have the same
   * length as the column.
   * dummy_place: the column to be summed
   *
   * Here is a pseudo-ish code this function should implement:
   *
   *   out := sum(i, (*dummies)[i].deref(dummy_place) * (*selector_bits)[i] );
   */
  virtual void multiplexHelper(Wire_T* const out,
      std::vector<LocalWireRange<Wire_T, Number_T>*>* const dummies,
      std::vector<Wire_T> const* const selector_bits,
      wtk::index_t const dummy_place);

  /**
   * For the switch-case to be valid, exactly 1 of the selection bits must
   * be true, unless it's within a disabled case. This checks that
   * enabled_bit * (1 - sum(bits)) is zero.
   */
  void checkSelectorBits(
      std::vector<Wire_T> const* const bits, Wire_T const* const enabled_bit);

  /**
   * To be called by the caller (rather than the BOLT Evaluator or Backend)
   * if the witnessed-statement is well-formed.
   * After the proof this function should indicate if the proof is legitimate
   * or not.
   */
  virtual bool check() = 0;

  /**
   * To be called by the caller after the proof finishes to cleanup any
   * resources, etc.
   */
  virtual void finish() { }

  virtual ~Backend() = default;
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

#include <wtk/bolt/Backend.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_BACKEND_H_
