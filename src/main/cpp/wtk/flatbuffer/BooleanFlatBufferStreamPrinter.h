/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_BOOLEAN_FLATBUFFER_STREAM_PRINTER_H_
#define WTK_FLATBUFFER_BOOLEAN_FLATBUFFER_STREAM_PRINTER_H_

#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <string>

#include <wtk/IRParameters.h>
#include <wtk/BooleanStreamHandler.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/flatbuffer/FlatBufferPrinter.h>
#include <wtk/flatbuffer/sieve_ir_generated.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

class BooleanFlatBufferStreamPrinter
  : public BooleanStreamHandler, public FlatBufferPrinter<uint8_t>
{
  std::vector<flatbuffers::Offset<Directive>> directivesList;

  void flushBuffer();
  void checkBuffer();

public:
  BooleanFlatBufferStreamPrinter(FILE* ofile);

  void handleInstance(wtk::index_t const idx) override;

  void handleShortWitness(wtk::index_t const idx) override;

  void handleAnd(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  void handleXor(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  void handleNot(wtk::index_t const out, wtk::index_t const left) override;

  void handleAssign(wtk::index_t const out, uint8_t const val) override;

  void handleCopy(wtk::index_t const out, wtk::index_t const in) override;

  void handleAssertZero(wtk::index_t const in) override;

  void handleDeleteSingle(wtk::index_t const in) override;

  void handleDeleteRange(wtk::index_t const first,
      wtk::index_t const last) override;

  void handleEnd() override;
};

} } // namespace wtk::flatbuffer

#endif//WTK_FLATBUFFER_BOOLEAN_FLATBUFFER_STREAM_PRINTER_H_
