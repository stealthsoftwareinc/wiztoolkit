/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_PARSER_H_
#define WTK_ANTLR_PARSER_H_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>

#include <antlr4-runtime.h>
#include <wtk/antlr/SIEVEIRParser.h>
#include <wtk/antlr/SIEVEIRLexer.h>

#include <wtk/index.h>
#include <wtk/Parser.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/QueueInputStream.h>

#include <wtk/antlr/to_uint.t.h>
#include <wtk/antlr/AntlrIRTree.t.h>
#include <wtk/antlr/GateUtils.t.h>
#include <wtk/antlr/TreeParser.t.h>
#include <wtk/antlr/ErrorListener.t.h>

#define LOG_IDENTIFIER "wtk::antlr"
#include <stealth_logging.h>

namespace wtk {
namespace antlr {

using namespace wtk_gen_antlr;

template<typename Number_T>
class ArithmeticParser : public wtk::ArithmeticParser<Number_T>
{
  SIEVEIRParser& parser;

  wtk::GateSet const* gateSet = nullptr;
  wtk::FeatureToggles const* featureToggles = nullptr;

  // sub-parser helpers
  std::unique_ptr<TreeParser<Number_T>> treeParser = nullptr;
  std::unique_ptr<wtk::QueueInputStream<Number_T>> inputStream = nullptr;

  // file name for error printing.
  char const* const fname;

public:
  ArithmeticParser(SIEVEIRParser& p,
      wtk::GateSet const* g, wtk::FeatureToggles const* t, char const* f)
    : parser(p), gateSet(g), featureToggles(t), fname(f) { }

  bool parseStream(wtk::ArithmeticStreamHandler<Number_T>* handler) override;

  AntlrIRTree<Number_T>* parseTree() override;

  wtk::QueueInputStream<Number_T>* instance() override;
  wtk::QueueInputStream<Number_T>* shortWitness() override;
};

class BooleanParser : public wtk::BooleanParser
{
  SIEVEIRParser& parser;

  wtk::GateSet const* gateSet = nullptr;
  wtk::FeatureToggles const* featureToggles = nullptr;

  // sub-parser helpers
  std::unique_ptr<TreeParser<uint8_t>> treeParser = nullptr;
  std::unique_ptr<wtk::QueueInputStream<uint8_t>> inputStream = nullptr;

  // file name for error printing.
  char const* const fname;

public:
  BooleanParser(SIEVEIRParser& p,
      wtk::GateSet const* g, wtk::FeatureToggles const* t, char const* f)
    : parser(p), gateSet(g), featureToggles(t), fname(f) { }

  bool parseStream(wtk::BooleanStreamHandler* handler) override;

  AntlrIRTree<uint8_t>* parseTree() override;

  wtk::QueueInputStream<uint8_t>* instance() override;
  wtk::QueueInputStream<uint8_t>* shortWitness() override;
};

template<typename Number_T>
class Parser : public wtk::Parser<Number_T>
{
  std::ifstream instream;
  antlr4::ANTLRInputStream antlrStream;
  SIEVEIRLexer lexer;
  antlr4::CommonTokenStream tknStream;
  SIEVEIRParser parser;

  ErrorListener errorListener;

public:

  Parser(std::string& f_name);

  bool parseHeader() override;
  bool parseResource() override;
  bool parseParameters() override;

  ArithmeticParser<Number_T>* arithmetic() override;
  ArithmeticParser<uint64_t>* arithmetic64() override;
  ArithmeticParser<uint32_t>* arithmetic32() override;

  BooleanParser* boolean() override;

private:
  std::unique_ptr<ArithmeticParser<Number_T>> arithmeticNumParser = nullptr;
  std::unique_ptr<ArithmeticParser<uint64_t>> arithmetic64Parser = nullptr;
  std::unique_ptr<ArithmeticParser<uint32_t>> arithmetic32Parser = nullptr;

  std::unique_ptr<BooleanParser> booleanParser = nullptr;
};

} } // namespace antlr

#include <wtk/antlr/Parser.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif // WTK_ANTLR_PARSER_H_
