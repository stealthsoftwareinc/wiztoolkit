/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_FLATBUFFER_INPUT_STREAM_PRINTER_H_
#define WTK_FLATBUFFER_FLATBUFFER_INPUT_STREAM_PRINTER_H_

#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <string>
#include <vector>

#include <wtk/Parser.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
class FlatBufferInputStreamPrinter : public FlatBufferPrinter<Number_T>
{
public:
  FlatBufferInputStreamPrinter(FILE* ofile);

  /**
   * Prints out the input stream as an instance flatbuffer.
   */
  bool printInstance(wtk::InputStream<Number_T>* stream);

  /**
   * Prints out the input stream as a witness flatbuffer.
   */
  bool printShortWitness(wtk::InputStream<Number_T>* stream);

private:
  std::vector<flatbuffers::Offset<Value>> values;
  bool isWitness;

  bool printStream(wtk::InputStream<Number_T>* stream);

  void checkFlush();
  void flush();
};

} } // namespace wtk::flatbuffer

#include <wtk/flatbuffer/FlatBufferInputStreamPrinter.t.h>

#endif//WTK_FLATBUFFER_FLATBUFFER_INPUT_STREAM_PRINTER_H_
