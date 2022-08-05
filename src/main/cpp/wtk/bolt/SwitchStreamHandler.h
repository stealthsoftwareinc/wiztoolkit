/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_SWITCH_STREAM_HANDLER_H_
#define WTK_BOLT_SWITCH_STREAM_HANDLER_H_

#include <cstddef>

namespace wtk {
namespace bolt {

/**
 * This holds and "dispenses" witness values within a switch-statement.
 * The outer-most switch reads from the the wtk::InputStream into a buffer.
 * Inner switch-statements carry pointers into the outer's Handler.
 */
template<typename Wire_T>
struct SwitchStreamHandler
{
  Wire_T* values = nullptr;
  size_t place = 0;

  // total number of stream values used by this stream.
  size_t const total;

  SwitchStreamHandler(size_t const t);

  // Produces the next n many values, advancing by n (default 1).
  Wire_T* next(size_t const n = 1);

  // Resets the place to 0.
  void reset();
};

} } // namespace wtk::bolt

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

#include <wtk/bolt/SwitchStreamHandler.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_BOLT_SWITCH_STREAM_HANDLER_H_
