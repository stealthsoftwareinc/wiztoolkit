/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_ARITHMETIC_FLATBUFFER_STREAM_PRINTER_H_
#define WTK_FLATBUFFER_ARITHMETIC_FLATBUFFER_STREAM_PRINTER_H_

#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <string>

#include <wtk/IRParameters.h>
#include <wtk/ArithmeticStreamHandler.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/flatbuffer/FlatBufferPrinter.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>
#include <wtk/flatbuffer/sieve_ir_generated.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template <typename Number_T>
class ArithmeticFlatBufferStreamPrinter
  : public ArithmeticStreamHandler<Number_T>,
    public FlatBufferPrinter<Number_T>
{
  std::vector<flatbuffers::Offset<Directive>> directivesList;

  void flushBuffer();
  void checkBuffer();

public:
  ArithmeticFlatBufferStreamPrinter(FILE* ofile);

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

} } // namespace wtk::flatbuffer

#include <wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.t.h>

#endif//WTK_FLATBUFFER_ARITHMETIC_FLATBUFFER_STREAM_PRINTER_H_
