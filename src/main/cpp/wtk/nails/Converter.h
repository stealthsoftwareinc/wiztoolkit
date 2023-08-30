/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_CONVERTER_H_
#define WTK_NAILS_CONVERTER_H_

#include <cstddef>
#include <cinttypes>

#include <wtk/indexes.h>
#include <wtk/Converter.h>

#include <wtk/nails/TypeInterpreter.h>

namespace wtk {
namespace nails {

/**
 * The converter converts wires from one field to another.
 *
 * It also does type-erasure for the Wire_T of each field.
 */
template<typename Number_T>
class Converter
{
public:

  // File name, for error reporting
  char const* const fileName;

  // Line number, set by caller for error reporting
  size_t lineNum = 0;

  // Constructor sets the file name.
  Converter(char const* const fn) : fileName(fn) { }

  // Callback for convert gates
  virtual bool convert(
      wire_idx const first_out, wire_idx const last_out,
      TypeInterpreter<Number_T>* const interpreter_out,
      wire_idx const first_in, wire_idx const last_in,
      TypeInterpreter<Number_T>* const interpreter_in, bool modulus) = 0;

  virtual ~Converter() = default;
};

/**
 * The LeadConverter is the non-type-eraseed Converter implementation.
 * It handles the IR semantics of conversion and uses a wtk::Converter for
 * the ZK conversion logic.
 */
template<typename Number_T, typename OutWire_T, typename InWire_T>
class LeadConverter : public Converter<Number_T>
{
  // ZK Conversion Logic
  wtk::Converter<OutWire_T, InWire_T>* const converter;
public:

  LeadConverter(
      char const* const fn, wtk::Converter<OutWire_T, InWire_T>* const c)
    : Converter<Number_T>(fn), converter(c) { }

  // Callback for convert gate semantics
  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      TypeInterpreter<Number_T>* const interpreter_out,
      wire_idx const first_in, wire_idx const last_in,
      TypeInterpreter<Number_T>* const interpreter_in, bool modulus) final;

  ~LeadConverter() = default;
};

} } // namespace wtk::nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Converter.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_CONVERTER_H_
