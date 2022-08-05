/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_PARSER_H_
#define WTK_IRREGULAR_PARSER_H_

#include <cstddef>
#include <memory>

#include <wtk/Parser.h>
#include <wtk/ArithmeticStreamHandler.h>
#include <wtk/BooleanStreamHandler.h>
#include <wtk/IRTree.h>
#include <wtk/IRParameters.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/irregular/Automatas.h>
#include <wtk/irregular/AutomataCtx.h>

#include <wtk/irregular/TextIRTree.t.h>
#include <wtk/irregular/TextTreeParser.t.h>

namespace wtk {
namespace irregular {

template<typename Number_T>
class TextInputStream : public InputStream<Number_T>
{
  AutomataCtx& ctx;

public:
  TextInputStream(AutomataCtx& c) : ctx(c) { }

  wtk::StreamStatus next(Number_T* num) override;
};

template<typename Number_T>
class ArithmeticParser : public wtk::ArithmeticParser<Number_T>
{
  AutomataCtx& ctx;

  // helper attributes for holding sub-parsers.
  std::unique_ptr<TextTreeParser<Number_T>> treeParser = nullptr;
  std::unique_ptr<TextInputStream<Number_T>> inputStream = nullptr;

  // class member avoids alloc/deallocing unbounded integer types.
  Number_T constValue = Number_T(0);

  // Pointers to gate set/feature toggles
  wtk::GateSet* gateSet;
  wtk::FeatureToggles* featureToggles;

public:
  ArithmeticParser(AutomataCtx& c, wtk::GateSet* gs, wtk::FeatureToggles* ft)
    : ctx(c), gateSet(gs), featureToggles(ft) { }

  bool parseStream(wtk::ArithmeticStreamHandler<Number_T>* handler) override;

  TextIRTree<Number_T>* parseTree() override;

  TextInputStream<Number_T>* instance() override;

  TextInputStream<Number_T>* shortWitness() override;
};

class BooleanParser : public wtk::BooleanParser
{
  AutomataCtx& ctx;

  // helper attributes for holding sub-parsers.
  std::unique_ptr<TextTreeParser<uint8_t>> treeParser = nullptr;
  std::unique_ptr<TextInputStream<uint8_t>> inputStream = nullptr;

  // Pointers to gate set/feature toggles
  wtk::GateSet* gateSet;
  wtk::FeatureToggles* featureToggles;

public:
  BooleanParser(AutomataCtx& c, wtk::GateSet* gs, wtk::FeatureToggles* ft)
    : ctx(c), gateSet(gs), featureToggles(ft) { }

  bool parseStream(wtk::BooleanStreamHandler* handler) override;

  TextIRTree<uint8_t>* parseTree() override;

  TextInputStream<uint8_t>* instance() override;

  TextInputStream<uint8_t>* shortWitness() override;
};

template<typename Number_T>
class Parser : public wtk::Parser<Number_T>
{
  AutomataCtx ctx;

public:

  Parser(FILE* f) : ctx(f) { }
  Parser(std::string& f_name) : ctx(fopen(f_name.c_str(), "r")) { }

  // Overridden parsing functions
  bool parseHeader() override;
  bool parseResource() override;
  bool parseParameters() override;

  ArithmeticParser<Number_T>* arithmetic() override;
  ArithmeticParser<uint64_t>* arithmetic64() override;
  ArithmeticParser<uint32_t>* arithmetic32() override;

  BooleanParser* boolean() override;

  virtual ~Parser() = default;

private:
  std::unique_ptr<ArithmeticParser<Number_T>> arithmeticNumParser = nullptr;
  std::unique_ptr<ArithmeticParser<uint64_t>> arithmetic64Parser = nullptr;
  std::unique_ptr<ArithmeticParser<uint32_t>> arithmetic32Parser = nullptr;

  std::unique_ptr<BooleanParser> booleanParser = nullptr;
};

} } // namespace wtk::irregular

#define LOG_IDENTIFIER "wtk::irregular"
#include <stealth_logging.h>

#include <wtk/irregular/Parser.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_IRREGULAR_TEXT_PARSER_H_
