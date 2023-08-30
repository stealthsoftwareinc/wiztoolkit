/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRESS_NOTHING_PRINTER_H_
#define WTK_PRESS_NOTHING_PRINTER_H_

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>

#include <wtk/indexes.h>

#include <wtk/Parser.h>
#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Data.h>

#include <wtk/press/Printer.h>

namespace wtk {
namespace press {

template<typename Number_T>
class NothingPrinter : public wtk::press::Printer<Number_T>
{

public:

  bool printHeader(
      size_t const major, size_t const minor, size_t const patch,
      char const* const extra,
      wtk::ResourceType const resource) override
  {
    (void) major;
    (void) minor;
    (void) patch;
    (void) extra;
    (void) resource;
    return true;
  }

  bool printPluginDecl(char const* const plugin_name) override
  {
    (void) plugin_name;
    return true;
  }

  bool printFieldType(Number_T const prime) override
  {
    (void) prime;
    return true;
  }

  bool printRingType(size_t const bit_width) override
  {
    (void) bit_width;
    return true;
  }

  bool printPluginType(
      wtk::circuit::PluginBinding<Number_T> const* const plugin) override
  {
    (void) plugin;
    return true;
  }

  bool printConversionSpec(
      wtk::circuit::ConversionSpec const* const conversion) override
  {
    (void) conversion;
    return true;
  }

  bool printBeginKw() override
  {
    return true;
  }

  bool printEndKw() override
  {
    return true;
  }

  bool printStreamValue(Number_T const& val) override
  {
    (void) val;
    return true;
  }

  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) right;
    (void) type;
    return true;
  }

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) right;
    (void) type;
    return true;
  }

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) right;
    (void) type;
    return true;
  }

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) right;
    (void) type;
    return true;
  }

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) type;
    return true;
  }

  bool copyMulti(wtk::circuit::CopyMulti* copy_multi) override
  {
    (void) copy_multi;
    return true;
  }

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) override
  {
    (void) out;
    (void) left;
    (void) type;
    return true;
  }

  virtual bool assertZero(wire_idx const left, type_idx const type) override
  {
    (void) left;
    (void) type;
    return true;
  }

  bool publicIn(wire_idx const out, type_idx const type) override
  {
    (void) out;
    (void) type;
    return true;
  }

  bool publicInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    (void) out;
    (void) type;
    return true;
  }

  bool privateIn(wire_idx const out, type_idx const type) override
  {
    (void) out;
    (void) type;
    return true;
  }

  bool privateInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    (void) out;
    (void) type;
    return true;
  }

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) override
  {
    (void) first_out;
    (void) last_out;
    (void) out_type;
    (void) first_in;
    (void) last_in;
    (void) in_type;
    (void) modulus;
    return true;
  }

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    (void) first;
    (void) last;
    (void) type;
    return true;
  }

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    (void) first;
    (void) last;
    (void) type;
    return true;
  }

  bool startFunction(wtk::circuit::FunctionSignature&& signature) override
  {
    (void) signature;
    return true;
  }

  bool regularFunction() override { return true; }

  bool endFunction() override { return true; }

  bool pluginFunction(wtk::circuit::PluginBinding<Number_T>&& binding) override
  {
    (void) binding;
    return true;
  }

  bool invoke(wtk::circuit::FunctionCall* const call) override
  {
    (void) call;
    return true;
  }
};

#undef PRINT_HELPER
#undef PRINT
#undef PRINTLN

} } // namespace wtk::press

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PRESS_TEXT_PRINTER_H_
