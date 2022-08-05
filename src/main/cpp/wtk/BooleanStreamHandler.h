/**
 * Copyright (C) 2020-2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ABSTRACT_BOOLEAN_STREAM_HANDLER_H_
#define WTK_ABSTRACT_BOOLEAN_STREAM_HANDLER_H_

#include <cstddef>
#include <cstdint>

#include <wtk/index.h>

namespace wtk {

/**
 * This is a streaming API for boolean relations. In order to work
 * with this API, the relation must have the simple feature set ("IR0").
 *
 * TA2 should implement this abstract class, and a parser will invoke
 * methods of the implementation.
 */
class BooleanStreamHandler {
public:
  virtual ~BooleanStreamHandler() = default;

  /**
   * Optional method for recording line numbers.
   */
  virtual void setLineNum(size_t const line);

  /**
   * Declares the index of an instance wire.
   */
  virtual void handleInstance(wtk::index_t const idx);

  /**
   * Declares the index of a short witness wire.
   */
  virtual void handleShortWitness(wtk::index_t const idx);

  /**
   * Declares the index of the output wire, and index of left and right
   * input wires of an XOR gate.
   */
  virtual void handleXor(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right);

  /**
   * Declares the index of the output wire, and index of left and right
   * input wires of a AND gate.
   */
  virtual void handleAnd(wtk::index_t const out, wtk::index_t const left,
      wtk::index_t const right);

  /**
   * Declares the index of the output wire, and the index of the input
   * wire for a NOT gate.
   */
  virtual void handleNot(wtk::index_t const out, wtk::index_t const in);

  /**
   * Declares that an output wire is assigned a given value.
   */
  virtual void handleAssign(wtk::index_t const out, uint8_t const val);

  /**
   * Declares that an output wire's value is a copy of the input value.
   */
  virtual void handleCopy(wtk::index_t const out, wtk::index_t const in);

  /**
   * Declares that the wire index should have a value of zero.
   */
  virtual void handleAssertZero(wtk::index_t const in);

  /**
   * Delete a wire or a range of wires.
   */
  virtual void handleDeleteSingle(wtk::index_t const in);
  virtual void handleDeleteRange(wtk::index_t const first,
      wtk::index_t const last);

  /**
   * Indicates that the end of the resource has been reached.
   */
  virtual void handleEnd();
};

} // namespace wtk

#endif//WTK_ABSTRACT_BOOLEAN_STREAM_HANDLER_H_
