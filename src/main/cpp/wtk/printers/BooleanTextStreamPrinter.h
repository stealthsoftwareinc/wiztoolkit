/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRINTERS_BOOLEAN_TEXT_STREAM_PRINTER_H_
#define WTK_PRINTERS_BOOLEAN_TEXT_STREAM_PRINTER_H_

#include <cstdio>
#include <cstddef>

#include <wtk/BooleanStreamHandler.h>

namespace wtk {
namespace printers {

class BooleanTextStreamPrinter : public BooleanStreamHandler {
  FILE* const outFile;

public:
  BooleanTextStreamPrinter(FILE* ofile);

  /**
   * Declares the value of an instance wire.
   */
  virtual void handleInstance(wtk::index_t const idx) override;

  /**
   * Declares the value of a short witness wire.
   */
  virtual void handleShortWitness(wtk::index_t const idx) override;

  /**
   * Declares the index of the output wire, and index of left and right
   * input wires of an XOR gate.
   */
  virtual void handleXor(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  /**
   * Declares the index of the output wire, and index of left and right
   * input wires of a AND gate.
   */
  virtual void handleAnd(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right) override;

  /**
   * Declares the index of the output wire, and the index of the input
   * wire for a NOT gate.
   */
  virtual void handleNot(wtk::index_t const out, wtk::index_t const in) override;

  /**
   * Declares that an output wire is assigned a given value.
   */
  virtual void handleAssign(wtk::index_t const out, uint8_t const val) override;

  /**
   * Declares that an output wire's value is a copy of the input value.
   */
  virtual void handleCopy(wtk::index_t const out, wtk::index_t const in) override;

  /**
   * Declares that the wire index should have a value of zero.
   */
  virtual void handleAssertZero(wtk::index_t const in) override;

  /**
   * Delete a wire or a range of wires.
   */
  void handleDeleteSingle(wtk::index_t const in) override;
  void handleDeleteRange(wtk::index_t const first, wtk::index_t const last) override;

  virtual void handleEnd() override;
};

} } // namespace wtk::printers

#endif//WTK_PRINTERS_BOOLEAN_TEXT_STREAM_PRINTER_H_
