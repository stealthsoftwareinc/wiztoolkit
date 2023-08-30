/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_FUNCTIONS_H_
#define WTK_NAILS_FUNCTIONS_H_

#include <cstddef>
#include <vector>
#include <string>

#include <wtk/indexes.h>
#include <wtk/utils/Pool.h>
#include <wtk/utils/SkipList.h>

#include <wtk/circuit/Data.h>

namespace wtk {
namespace nails {

template<typename Number_T>
struct Interpreter;

/**
 * The function is a top-level abstraction for all functions (regular or
 * plugin), which establishes polymorphism for identifying and evaluating
 * functions.
 */
template<typename Number_T>
struct Function
{
  /**
   * This function's signature.
   */
  wtk::circuit::FunctionSignature const signature;

  /**
   * Line number, set by the caller during building.
   */
  size_t lineNum = 0;

  /**
   * Invoke/evaluate callback.
   */
  virtual bool evaluate(Interpreter<Number_T>* const interpreter) = 0;

  Function(wtk::circuit::FunctionSignature&& sig)
    : signature(std::move(sig)) { }

  virtual ~Function() = default;
};

/**
 * The RegularFunction is an abstraction for functions defined by a list of
 * gates (the regular way). Each callback must retain enough information to
 * replay the list of gates when the function is "evaluate()"ed.
 */
template<typename Number_T>
struct RegularFunction : public Function<Number_T>
{
  RegularFunction(wtk::circuit::FunctionSignature&& sig)
    : Function<Number_T>(std::move(sig)) { }

  /**
   * Callbacks for building the function.
   *
   * They mirror those of wtk::CircuitHandler
   */
  virtual bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) = 0;

  virtual bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) = 0;

  virtual bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) = 0;

  virtual bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) = 0;

  virtual bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) = 0;

  virtual bool copyMulti(wtk::circuit::CopyMulti* multi) = 0;

  virtual bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) = 0;

  virtual bool assertZero(wire_idx const left, type_idx const type) = 0;

  virtual bool publicIn(wire_idx const out, type_idx const type) = 0;

  virtual bool publicInMulti(
      wtk::circuit::Range* out, type_idx const type) = 0;

  virtual bool privateIn(wire_idx const out, type_idx const type) = 0;

  virtual bool privateInMulti(
      wtk::circuit::Range* out, type_idx const type) = 0;

  virtual bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) = 0;

  virtual bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) = 0;

  virtual bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) = 0;

  virtual bool invoke(wtk::circuit::FunctionCall&& invocation) = 0;

  /**
   * Type-check the function once when it is created.
   */
  virtual bool typeCheck(Interpreter<Number_T> const* const interpreter) = 0;

  virtual ~RegularFunction() = default;
};

// A factory for defining functions on demand
// The default function implementation traverses a gate-array

/**
 * As the circuit progresses, new functions may be declared. As they are
 * declared, each must be created by a function factory, based mainly on its
 * signature.
 *
 * Each created function should take on the Factory's lifetime.
 */
template<typename Number_T>
struct FunctionFactory
{
  virtual RegularFunction<Number_T>* createFunction(
      wtk::circuit::FunctionSignature&& sig) = 0;

  virtual ~FunctionFactory() = default;
};

/**
 * The GatesFunction the default Function implementation is based on recording
 * the sequence of gates, and repeating them when invoked.
 *
 * The Gate struct is largely internal to the GatesFunction.
 */
template<typename Number_T>
struct Gate
{
  enum Operation
  {
    uninitialized,
    // Normal
    add,
    mul,
    copy,
    assertZero,
    publicIn,
    privateIn,
    // Constant
    addc,
    mulc,
    assign,
    // Convert
    convert_,
    // Memory
    newRange,
    deleteRange,
    // Function Call
    call_,
    // Multi Wire Copy
    copyMulti,
    // Multi Wire Input
    publicInMulti,
    privateInMulti
  };

  Operation operation = uninitialized;

  struct NormalGate
  {
    wire_idx out;
    wire_idx left;
    wire_idx right;

    type_idx type;

    NormalGate(
        wire_idx const o, wire_idx const l, wire_idx const r, type_idx const t)
      : out(o), left(l), right(r), type(t) { }
  };

  struct ConstantGate
  {
    wire_idx out;
    wire_idx left;

    Number_T right;

    type_idx type;

    ConstantGate(
        wire_idx const o, wire_idx const l, Number_T&& r, type_idx const t)
      : out(o), left(l), right(r), type(t) { }
  };

  struct ConvertGate
  {
    wire_idx firstOut;
    wire_idx lastOut;

    wire_idx firstIn;
    wire_idx lastIn;

    type_idx outType;
    type_idx inType;

    bool modulus;

    ConvertGate(wire_idx const fo, wire_idx const lo, type_idx const ot,
        wire_idx const fi, wire_idx const li, type_idx const it, bool const m)
      : firstOut(fo), lastOut(lo), firstIn(fi), lastIn(li),
         outType(ot), inType(it), modulus(m) { }
  };

  struct MemoryGate
  {
    wire_idx first;
    wire_idx last;

    type_idx type;

    MemoryGate(wire_idx const f, wire_idx const l, type_idx const t)
      : first(f), last(l), type(t) { }
  };

  struct MultiInput
  {
    wtk::circuit::Range outputs;
    type_idx type;

    MultiInput(wtk::circuit::Range&& o, type_idx const t)
      : outputs(std::move(o)), type(t) { }
  };

  // This union is tagged by the gate's type
  union
  {
    NormalGate normal;
    ConstantGate constant;
    ConvertGate convert;
    MemoryGate memory;
    wtk::circuit::FunctionCall call;
    wtk::circuit::CopyMulti multiCopy;
    MultiInput multiInput;
  };

  // Records each gate's line number (for error reporting).
  size_t lineNum;

  // normal constructor
  Gate(Operation const op,
      wire_idx const o, wire_idx const l, wire_idx const r,
      type_idx const t, size_t const ln);

  // constant constructor
  Gate(Operation const op,
      wire_idx const o, wire_idx const l, Number_T&& r,
      type_idx const t, size_t const ln, bool);

  // convert constructor
  Gate(wire_idx const fo, wire_idx const lo, type_idx const ot,
      wire_idx const fi, wire_idx const li, type_idx const it,
      bool m, size_t const ln);

  // memory constructor
  Gate(Operation const op,
      wire_idx const f, wire_idx const l, type_idx const t, size_t const ln);

  // invoke constructor
  Gate(wtk::circuit::FunctionCall&& c);

  // multi copy constructor
  Gate(wtk::circuit::CopyMulti&& m, size_t const ln);

  // multi wire input gate constructor
  Gate(Operation const op,
      wtk::circuit::Range&& r, type_idx const t, size_t const ln);

  // No copying
  Gate(Gate const&) = delete;
  Gate& operator=(Gate const&) = delete;

  // Only move construction
  Gate(Gate&& move);
  Gate& operator=(Gate&&) = delete;

  ~Gate();
};

/**
 * The GatesFunction is the default function implementation.
 *
 * It records a list of gates during build, then repeats the list on each
 * invoke.
 */
template<typename Number_T>
struct GatesFunction : public RegularFunction<Number_T>
{
  // This is the list of gates
  std::vector<Gate<Number_T>> gates;

  GatesFunction(wtk::circuit::FunctionSignature&& sig)
    : RegularFunction<Number_T>(std::move(sig)) { }

  // Record gates
  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override;

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override;

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override;

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override;

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) override;

  bool copyMulti(wtk::circuit::CopyMulti* multi) override;

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) override;

  bool assertZero(wire_idx const left, type_idx const type) override;

  bool publicIn(wire_idx const out, type_idx const type) override;

  bool publicInMulti(wtk::circuit::Range* out, type_idx const type) override;

  bool privateIn(wire_idx const out, type_idx const type) override;

  bool privateInMulti(wtk::circuit::Range* out, type_idx const type) override;

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) override;

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) override;

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) override;

  bool invoke(wtk::circuit::FunctionCall&& call) override;

  bool typeCheck(Interpreter<Number_T> const* const interpreter) override;

  // Evaluate the function
  bool evaluate(Interpreter<Number_T>* const interpreter) override;
};

/**
 * This is the default implementation for a function factory.
 */
template<typename Number_T>
struct GatesFunctionFactory : public FunctionFactory<Number_T>
{
  wtk::utils::Pool<GatesFunction<Number_T>> gatesFunctionPool;

  RegularFunction<Number_T>* createFunction(
      wtk::circuit::FunctionSignature&& sig) override;

  virtual ~GatesFunctionFactory() = default;
};

} } // namespace wtk::nails

#define LOG_IDENTIFER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Functions.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_FUNCTIONS_H_
