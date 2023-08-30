/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_CIRCUIT_HANDLER_H_
#define WTK_CIRCUIT_HANDLER_H_

#include <cstddef>

#include <wtk/indexes.h>

#include <wtk/circuit/Data.h>

namespace wtk {
namespace circuit {

/**
 * This is a callback interface for the IR0 parser. When it reads a syntax
 * element, it calls the appropriate callback.
 *
 * In general, the callbacks should return true, but may return false to halt
 * processing of the circuit.
 */
template<typename Number_T>
class Handler
{
public:
  /**
   * Some parser implementations may update this before each directive's
   * callback.
   */
  size_t lineNum = 0;

  /**
   * Callback for @add
   */
  virtual bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) = 0;

  /**
   * Callback for @mul
   */
  virtual bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) = 0;

  /**
   * Callback for @addc
   */
  virtual bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) = 0;

  /**
   * Callback for @mulc
   */
  virtual bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) = 0;

  /**
   * Callback for copy directive
   */
  virtual bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) = 0;

  /**
   * Callback for copy multiple wires directive. Although the copy struct
   * is passed by pointer, it may (or may not) be std::move'd without issue.
   */
  virtual bool copyMulti(CopyMulti* copy_multi) = 0;

  /**
   * Callback for assignment directive
   */
  virtual bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) = 0;

  /**
   * Callback for @assert_zero
   */
  virtual bool assertZero(wire_idx const left, type_idx const type) = 0;

  /**
   * Callback for @public with a single wire output
   */
  virtual bool publicIn(wire_idx const out, type_idx const type) = 0;

  /**
   * Callback for @public with a wire range output. Although the range struct
   * is passed by pointer, it may (or may not) be std::move'd without issue.
   */
  virtual bool publicInMulti(Range* outs, type_idx const type) = 0;

  /**
   * Callback for @private with a single wire output
   */
  virtual bool privateIn(wire_idx const out, type_idx const type) = 0;

  /**
   * Callback for @private with a wire range output. Although the range struct
   * is passed by pointer, it may (or may not) be std::move'd without issue.
   */
  virtual bool privateInMulti(Range* outs, type_idx const type) = 0;

  /**
   * Callback for @convert
   */
  virtual bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) = 0;

  /**
   * Callback for @new
   */
  virtual bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) = 0;

  /**
   * Callback for @delete
   */
  virtual bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) = 0;

  /**
   * Callback for a function's signature. When the function is first
   * encountered, this callback will provide the signature.
   *
   * For regular functions, regularFunction() is called and then gate
   * callbacks will be called as needed until the end of the function, when
   * endFunction() is called.
   *
   * For plugin functions pluginFunction() will be called exactly once, and
   * neither regularFunction() nor endFunction() will be called.
   *
   */
  virtual bool startFunction(FunctionSignature&& signature) = 0;

  /**
   * Callback for the beginning of a regular (non-plugin) function.
   */
  virtual bool regularFunction() = 0;

  /**
   * Callback for the end of a function. Gate callbacks will be called until
   * the end of the function is reached, then endFunction() is called.
   */
  virtual bool endFunction() = 0;

  /**
   * Callback for when the body of a function is implemented with a plugin.
   * Neither, endFunction() nor any gate callbacks will be called for this
   * kind of function declaration.
   */
  virtual bool pluginFunction(PluginBinding<Number_T>&& binding) = 0;

  /**
   * Callback for a function invocation. The FunctionCall is passed by pointer
   * but may be std::moved or not.
   */
  virtual bool invoke(FunctionCall* const call) = 0;

  /**
   * virtual deleter
   */
  virtual ~Handler() = default;
};

} } // namespace wtk::circuit

#endif//WTK_CIRCUIT_HANDLER_H_
