/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cinttypes>
#include <string>
#include <vector>

#include <wtk/Parser.h>
#include <wtk/IRTree.h>
#include <wtk/utils/hints.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>
#include <wtk/flatbuffer/FBIRTree.t.h>
#include <wtk/flatbuffer/TreeParser.t.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

#define LOG_IDENTIFIER "wtk::flatbuffer"
#include <stealth_logging.h>

#ifndef WTK_FLATBUFFER_PARSER_H_
#define WTK_FLATBUFFER_PARSER_H_

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
struct InstanceInputStream : public wtk::InputStream<Number_T>
{
private:
  // vector of flatbuffers
  std::vector<Instance const*> buffers;
  // index of the flatbuffer
  size_t buffer_idx = 0;
  // index of the instance inside the flatbuffer
  size_t instance_idx = 0;

public:
  bool load(std::vector<Root const*>& roots);

  StreamStatus next(Number_T* num) override;
};

template<typename Number_T>
struct WitnessInputStream : public wtk::InputStream<Number_T>
{
private:
  // vector of flatbuffers
  std::vector<Witness const*> buffers;
  // index of the flatbuffer
  size_t buffer_idx = 0;
  // index of the instance inside the flatbuffer
  size_t witness_idx = 0;

public:
  bool load(std::vector<Root const*>& roots);

  StreamStatus next(Number_T* num) override;
};

template<typename Number_T>
struct ArithmeticParser : public wtk::ArithmeticParser<Number_T>
{
private:
  std::vector<Root const*>& roots;
  GateSet* gateSet;
  FeatureToggles* toggles;

  std::unique_ptr<InstanceInputStream<Number_T>> instanceStream = nullptr;
  std::unique_ptr<WitnessInputStream<Number_T>> witnessStream = nullptr;

  std::unique_ptr<TreeParser<Number_T>> treeParser = nullptr;

public:
  ArithmeticParser(
      std::vector<Root const*>& roots, GateSet* gs, FeatureToggles* ft);

  bool parseStream(wtk::ArithmeticStreamHandler<Number_T>* handler) override;

  FBIRTree<Number_T>* parseTree() override;

  InstanceInputStream<Number_T>* instance() override;

  WitnessInputStream<Number_T>* shortWitness() override;
};

struct BooleanParser : public wtk::BooleanParser
{
private:
  std::vector<Root const*>& roots;
  GateSet* gateSet;
  FeatureToggles* toggles;

  std::unique_ptr<InstanceInputStream<uint8_t>> instanceStream = nullptr;
  std::unique_ptr<WitnessInputStream<uint8_t>> witnessStream = nullptr;

  std::unique_ptr<TreeParser<uint8_t>> treeParser = nullptr;

public:
  BooleanParser(
      std::vector<Root const*>& roots, GateSet* gs, FeatureToggles* ft);

  bool parseStream(wtk::BooleanStreamHandler* handler) override;

  FBIRTree<uint8_t>* parseTree() override;

  InstanceInputStream<uint8_t>* instance() override;

  WitnessInputStream<uint8_t>* shortWitness() override;
};

template<typename Number_T>
struct Parser : public wtk::Parser<Number_T>
{
private:
  // Name of the file
  std::string fileName;

  // File and buffer for the flatbuffer.
  FILE* file = nullptr;
  size_t bufferSize = 0;
  uint8_t* buffer = nullptr;

  // Indicates if file reading failed. Causes API to return failures.
  bool failure = false;

  // list of flatbuffer root pointers and sizes.
  // Note, the spec defines a flatbuffer size using 32-bits because flatbuffer
  // defines its size as 32-bits, so sizes here are 32-bits.
  std::vector<uint32_t> sizes;
  std::vector<Root const*> roots;

  // parsers for each IR variety
  std::unique_ptr<BooleanParser> boolParser;
  std::unique_ptr<ArithmeticParser<Number_T>> arithParser;
  std::unique_ptr<ArithmeticParser<uint64_t>> arith64Parser;
  std::unique_ptr<ArithmeticParser<uint32_t>> arith32Parser;

public:
  Parser(std::string& f_name);
  ~Parser();

  bool parseHeader() override;

  bool parseResource() override;

  bool parseParameters() override;

  BooleanParser* boolean() override;

  ArithmeticParser<Number_T>* arithmetic() override;
  ArithmeticParser<uint64_t>* arithmetic64() override;
  ArithmeticParser<uint32_t>* arithmetic32() override;
};

} } // namespace wtk::flatbuffer

#include <wtk/flatbuffer/Parser.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_PARSER_H_
