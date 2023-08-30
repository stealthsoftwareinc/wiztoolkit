/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRESS_PRINTER_H_
#define WTK_PRESS_PRINTER_H_

#include <cstddef>

#include <wtk/indexes.h>

#include <wtk/Parser.h>
#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Data.h>

namespace wtk {
namespace press {

/**
 * Abstract class for printing out an IR resource.
 *
 * It extends from Handler, and passes on those pure-virtual methods.
 */
template<typename Number_T>
class Printer : public wtk::circuit::Handler<Number_T>
{
public:

  virtual bool printHeader(
      size_t const major, size_t const minor, size_t const patch,
      char const* const extra,
      wtk::ResourceType const resource) = 0;

  virtual bool printPluginDecl(char const* const plugin_name) = 0;

  virtual bool printFieldType(Number_T const prime) = 0;

  virtual bool printRingType(size_t const bit_width) = 0;

  virtual bool printPluginType(
      wtk::circuit::PluginBinding<Number_T> const* const plugin) = 0;

  virtual bool printConversionSpec(
      wtk::circuit::ConversionSpec const* const conversion) = 0;

  virtual bool printBeginKw() = 0;

  virtual bool printEndKw() = 0;

  virtual bool printStreamValue(Number_T const& val) = 0;

  virtual ~Printer() { }
};

} } // namespace wtk::press

#endif//WTK_PRESS_PRINTER_H_
