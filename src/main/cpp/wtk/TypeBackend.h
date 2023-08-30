/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_TYPE_BACKEND_H_
#define WTK_TYPE_BACKEND_H_

#include <cstddef>

#include <wtk/indexes.h>
#include <wtk/circuit/Data.h>

namespace wtk {

/**
 * A type erasing parent class for all TypeBackends. The Wire_T template
 * gets erased, and all actions non-sepcific to the Wire_T may be performed
 * upon the type-erased parent.
 */
template<typename Number_T>
class TypeBackendEraser
{
public:
  /**
   * The lineNum attribute will be set by the callbacks' callers before
   * each call. This allows errors to be reported with the line number.
   *
   * (note that some parsers may report the line number as 0)
   */
  size_t lineNum = 0;

  /**
   * This backend's type.
   */
  wtk::circuit::TypeSpec<Number_T> const* const type;

  TypeBackendEraser(wtk::circuit::TypeSpec<Number_T> const* const t)
    : type(t) { }

  /**
   * Because evaluation failures are cached until the end (hiding which
   * gate caused the failure) the check method must indicate if a failure
   * occurred.
   *
   * The "main()" function should call check(). E.g. WizToolKit will not
   * call it for you.
   *
   * It should return false on failure and true otherwise.
   */
  virtual bool check() = 0;

  /**
   * Optional helper method for cleanup and finalization tasks.
   *
   * The "main()" program may call this function, if necessary for your
   * proof system.
   */
  virtual void finish() { }

  virtual ~TypeBackendEraser() = default;
};

/**
 * The TypeBackend must be implemented by a ZK backend for each type/field
 * it will implement.
 *
 * The Number_T template represents a plain-text value/field-element and
 * should be set to the same type as parameterizing the parser.
 *
 * The Wire_T template represents a ZK value, or wire, as it passes from
 * one gate (callback) to another. It may be any necesary type or class,
 * so long as appropriate constructors/destructors are present.
 *
 * It has callbacks to handle all the various gates.
 */
template<typename Number_T, typename Wire_T>
class TypeBackend : public TypeBackendEraser<Number_T>
{
public:

  TypeBackend(wtk::circuit::TypeSpec<Number_T> const* const t)
    : TypeBackendEraser<Number_T>(t) { }

  /**
   * The supportsGates flag indicates if this type supports gate operations
   * (e.g. calling the following callbacks)
   *
   * Most types except for those associated with certain plugins should
   * always report true.
   */
  virtual bool supportsGates() const { return true; }

  /**
   * Callback for assigning a circuit constant to a wire.
   */
  virtual void assign(Wire_T* wire, Number_T&& value) = 0;

  /**
   * Callback for copying one wire's value to another.
   */
  virtual void copy(Wire_T* wire, Wire_T const* value) = 0;

  /**
   * Callback for adding two wires.
   */
  virtual void addGate(
      Wire_T* out, Wire_T const* left, Wire_T const* right) = 0;

  /**
   * Callback for multiplying two wires.
   */
  virtual void mulGate(
      Wire_T* out, Wire_T const* left, Wire_T const* right) = 0;

  /**
   * Callback for adding a wire and a circuit constant.
   */
  virtual void addcGate(Wire_T* out, Wire_T const* left, Number_T&& right) = 0;

  /**
   * Callback for multiplying a wire and a circuit constant.
   */
  virtual void mulcGate(Wire_T* out, Wire_T const* left, Number_T&& right) = 0;

  /**
   * Callback for asserting that a wire carries the value 0.
   *
   * Failing asserts must be cached until check() is called at the end.
   */
  virtual void assertZero(Wire_T const* left) = 0;

  /**
   * Callback for assigning a public stream input.
   */
  virtual void publicIn(Wire_T* wire, Number_T&& value) = 0;

  /**
   * Callback for assigning a private stream input.
   */
  virtual void privateIn(Wire_T* wire, Number_T&& value) = 0;

  /**
   * Indicates whether or not extended witness retrieval is supported.
   *
   * If this backend belongs to a verifier, and extended witness retrieval
   * is supported by the corresponding prover, then it should return true.
   */
  virtual bool supportsExtendedWitness() { return false; }

  /**
   * If supported, this will return the corresponding value of the extended
   * witness. In other terms, it converts a wire to a plaintext number.
   *
   * It should return 0 if not supported, or if this backend belongs to a
   * verifier (e.g. no access to the witness).
   */
  virtual Number_T getExtendedWitness(Wire_T const* wire)
  {
    (void) wire;
    return Number_T(0);
  }

  /**
   * If supported, this will return the corresponding value of the extended
   * witness as the wire_idx type rather than the Number_T. Naturally, it
   * will wraparound at 2**64.
   *
   * It should return 0 if not supported, or if this backend belongs to a
   * verifier (e.g. no access to the witness).
   */
  virtual wtk::wire_idx getExtendedWitnessIdx(Wire_T const* wire)
  {
    (void) wire;
    return 0;
  }

  virtual ~TypeBackend() = default;
};

} // namespace wtk

#endif//WTK_TYPE_BACKEND_H_
