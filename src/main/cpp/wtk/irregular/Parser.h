/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_PARSER_
#define WTK_IRREGULAR_PARSER_

#include <cstddef>
#include <cstdio>
#include <vector>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/Parser.h>

#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Parser.h>

#include <wtk/irregular/AutomataCtx.h>

namespace wtk {
namespace irregular {

template<typename Number_T>
class TranslationParser;

template<typename Number_T>
class CircuitParser;

template<typename Number_T>
class InputStream;

template<typename Number_T>
class ConfigurationParser;

template<typename Number_T>
class Parser : public wtk::Parser<Number_T>
{
  std::unique_ptr<AutomataCtx> ctx;

  std::unique_ptr<TranslationParser<Number_T>> translationParser;
  std::unique_ptr<CircuitParser<Number_T>> circuitParser;
  std::unique_ptr<InputStream<Number_T>> inputStream;
  std::unique_ptr<ConfigurationParser<Number_T>> configurationParser;

public:

  /**
   * Open the parser using the given filename.
   */
  bool open(char const* const fname);

  /**
   * Open the parser using a FILE* object.
   * The optional filename parameter is for error-reporting
   */
  bool open(FILE* const f, char const* const fname = "<FILE*>");

  bool parseHeader() final;

  TranslationParser<Number_T>* translation() final;

  CircuitParser<Number_T>* circuit() final;

  InputStream<Number_T>* publicIn() final;

  InputStream<Number_T>* privateIn() final;

  ConfigurationParser<Number_T>* configuration() final;

  Parser() = default;
  Parser(Parser const& copy) = delete;
  Parser(Parser&& /* move */) = default;
  Parser& operator=(Parser const& copy) = delete;
  Parser& operator=(Parser&& move) = default;
  ~Parser() = default;
};

template<typename Number_T>
class TranslationParser : public wtk::TranslationParser<Number_T>
{
  AutomataCtx* const ctx;
public:

  TranslationParser(AutomataCtx* const c) : ctx(c) { }
};

template<typename Number_T>
class CircuitParser : public wtk::circuit::Parser<Number_T>
{
  AutomataCtx* const ctx;
public:

  CircuitParser(AutomataCtx* const c) : ctx(c) { }

  bool parseCircuitHeader() final;

  bool parse(wtk::circuit::Handler<Number_T>* const handler) final;

  ~CircuitParser() = default;
};

template<typename Number_T>
class InputStream : public wtk::InputStream<Number_T>
{
  AutomataCtx* const ctx;

  size_t line = 0;
public:

  bool parseStreamHeader() final;

  InputStream(AutomataCtx* const c) : ctx(c) { }

  wtk::StreamStatus next(Number_T* num) final;

  size_t lineNum() final;
};

template<typename Number_T>
class ConfigurationParser : public wtk::ConfigurationParser<Number_T>
{
  AutomataCtx* const ctx;
public:

  ConfigurationParser(AutomataCtx* const c) : ctx(c) { }
};

} } // namespace wtk::irregular

#include <wtk/irregular/automatas.i.h>

#define LOG_IDENTIFIER "wtk::irregular"
#include <stealth_logging.h>

#include <wtk/irregular/Parser.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_IRREGULAR_PARSER_
