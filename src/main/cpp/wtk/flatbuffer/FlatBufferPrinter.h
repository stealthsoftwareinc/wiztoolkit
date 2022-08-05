/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_FLATBUFFER_PRINTER_H_
#define WTK_FLATBUFFER_FLATBUFFER_PRINTER_H_

#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <string>
#include <vector>

#include <wtk/IRParameters.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
class FlatBufferPrinter
{
  FILE* const outFile;

  // things that go into the header
  std::string version;

  Number_T characteristic;
  size_t degree;

  // additional "front matter" for the relation
  std::string gateSet;
  std::string features;

  void writeBuffer();

protected:
  // sub-classes should use this to build flatbuffers
  flatbuffers::FlatBufferBuilder builder;

  // approx. log_256(characteristic).
  // Should be the maximum conceivable size for a flattened number
  size_t characteristicLen;

  FlatBufferPrinter(FILE* ofile);

  // can be called multiple times for a super-flatbuffer
  void writeRelation(std::vector<flatbuffers::Offset<Directive>>* body,
      std::vector<flatbuffers::Offset<Function>>* functions);
  void writeInstance(std::vector<flatbuffers::Offset<Value>>* body);
  void writeWitness(std::vector<flatbuffers::Offset<Value>>* body);

public:
  // These set up the header... Must be called together and before any others.
  void setVersion(size_t const major, size_t const minor, size_t const patch);
  void setField(Number_T const characteristic, size_t const degree);

  // Only used by relation
  void setGateSet(wtk::GateSet const* const gate_set);
  void setFeatureToggles(wtk::FeatureToggles const* const toggles);
};

} } // namespace wtk::flatbuffer

#define LOG_IDENTIFIER "wtk::flatbuffer"
#include <stealth_logging.h>

#include <wtk/flatbuffer/FlatBufferPrinter.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_FLATBUFFER_PRINTER_H_
