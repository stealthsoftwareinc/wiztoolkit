/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_QUEUE_INPUT_STREAM_H_
#define WTK_QUEUE_INPUT_STREAM_H_

#include <cstddef>
#include <deque>

#include <wtk/index.h>
#include <wtk/Parser.h>

/**
 * This is an implementation of the InputStream from Parser.h which wraps
 * an in-memory queue. It maintains a queue of both value numbers and line
 * numbers.
 *
 * It can be constructied with QueueInputStream(true) to return parse errors
 * when next(...) is called. if after being constructed with an error,
 * numbers are subsequently inserted, the error will be cleared.
 */

namespace wtk {

template<typename Number_T>
class QueueInputStream : public wtk::InputStream<Number_T> 
{
  std::deque<Number_T> numbers;
  std::deque<size_t> lineNums;
  bool firstLine = true;
  bool parseError;

public:

  QueueInputStream(bool pe = false) : parseError(pe) { }

  wtk::StreamStatus next(Number_T* num) override;

  size_t lineNum() override;

  void insert(Number_T num, size_t line);

  /**
   * Although not required by the InputStream abstraction, this
   * input stream is capable of reporting the number of available
   * input values.
   */
  size_t size();
};

} // namespace wtk

#include <wtk/QueueInputStream.t.h>

#endif//WTK_QUEUE_INPUT_STREAM_H_
