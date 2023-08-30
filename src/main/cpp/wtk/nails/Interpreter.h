/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_INTERPRETER_H_
#define WTK_NAILS_INTERPRETER_H_

#include <cstddef>
#include <vector>
#include <memory>
#include <unordered_map>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/CharMap.h>

#include <wtk/nails/TypeInterpreter.h>
#include <wtk/nails/Converter.h>
#include <wtk/nails/Functions.h>

#ifdef WTK_NAILS_ENABLE_TRACES
#include <wtk/utils/Indent.h>
#endif//WTK_NAILS_ENABLE_TRACES

namespace wtk {
namespace nails {

/**
 * NAILS: Naive Amenity for Interpreting Long Streams
 *
 * The Interpreter distributes gates by thier type to the correct
 * FieldInterpreter.
 */

template<typename Number_T>
struct Interpreter
{
  // A field interpreter for each field. Populated by caller.
  std::vector<std::unique_ptr<TypeInterpreter<Number_T>>> interpreters;

  // A converter for each of the requisite conversions. Populated by caller.
  std::unordered_map<
    wtk::circuit::ConversionSpec, std::unique_ptr<Converter<Number_T>>>
    converters;

  // A map of names to declared functions.
  wtk::utils::CharMap<Function<Number_T>*> functions;

  // file name for error reporting.
  char const* const fileName = "<relation>";

  // Handler will set this for error reporting.
  size_t lineNum = 0;

  /**
   * Construct an Interpreter, given the file-name for error reporting.
   */
  Interpreter(char const* const fn) : fileName(fn) { }

#ifdef WTK_NAILS_ENABLE_TRACES
  bool trace = false;
  bool traceDetail = false;
  wtk::utils::Indent indent;

  void enableTrace();
  void enableTraceDetail();
#endif//WTK_NAILS_ENABLE_TRACES

  /**
   * Add a type to the interpreter.
   *
   * The order of invocation defines the type's type index.
   */
  template<typename Wire_T>
  void addType(wtk::TypeBackend<Number_T, Wire_T>* const tb,
      wtk::InputStream<Number_T>* const public_in,
      wtk::InputStream<Number_T>* const private_in);

  /**
   * Add a conversion to the interpreter.
   *
   * Returns false if the conversion is a duplicate.
   */
  template<typename Out_T, typename In_T>
  bool addConversion(
      wtk::circuit::ConversionSpec const* const spec,
      wtk::Converter<Out_T, In_T>* conv);

  // The Handler and Functions will call these as necessary.
  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type);

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type);

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type);

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type);

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type);

  bool copyMulti(wtk::circuit::CopyMulti const* const multi);

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type);

  bool assertZero(wire_idx const left, type_idx const type);

  bool publicIn(wire_idx const out, type_idx const type);

  bool publicInMulti(
      wtk::circuit::Range const* const out, type_idx const type);

  bool privateIn(wire_idx const out, type_idx const type);

  bool privateInMulti(
      wtk::circuit::Range const* const out, type_idx const type);

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus);

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type);

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type);

  bool invoke(wtk::circuit::FunctionCall const* const call);

  ~Interpreter() = default;
};

} } // namespace wtk::nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Interpreter.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_INTERPRTER_H_
