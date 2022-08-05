/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRINTERS_ARITHMETIC_TEXT_STREAM_PRINTER_H_
#define WTK_PRINTERS_ARITHMETIC_TEXT_STREAM_PRINTER_H_

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cinttypes>

#include <wtk/IRParameters.h>
#include <wtk/ArithmeticStreamHandler.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace printers {

template <typename Number_T>
class ArithmeticTextStreamPrinter : public ArithmeticStreamHandler<Number_T> {
  FILE* const outFile;

public:
  ArithmeticTextStreamPrinter(FILE* ofile);

  void handleInstance(wtk::index_t const idx) override;

  void handleShortWitness(wtk::index_t const idx) override;

  void handleAdd(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  void handleMul(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  void handleAddC(wtk::index_t const out, wtk::index_t const left,
      Number_T const right) override;

  void handleMulC(wtk::index_t const out, wtk::index_t const left,
      Number_T const right) override;

  void handleAssign(wtk::index_t const out, Number_T const val) override;

  void handleCopy(wtk::index_t const out, wtk::index_t const in) override;

  void handleAssertZero(wtk::index_t const in) override;

  void handleDeleteSingle(wtk::index_t const in) override;

  void handleDeleteRange(wtk::index_t const first,
      wtk::index_t const last) override;

  void handleEnd() override;
};

} } // namespace wtk::printers

#include <wtk/printers/ArithmeticTextStreamPrinter.t.h>

#endif // WTK_PRINTERS_ARITHMETIC_TEXT_STREAM_PRINTER_H_
