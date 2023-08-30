/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_PARSER_
#define WTK_FLATBUFFER_PARSER_

#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <memory>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <wtk/indexes.h>
#include <wtk/Parser.h>
#include <wtk/utils/hints.h>

#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Parser.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>
#include <wtk/irregular/AutomataCtx.h>

#include <flatbuffers/flatbuffers.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
class TranslationParser;

template<typename Number_T>
class CircuitParser;

template<typename Number_T>
class PublicInputStream;

template<typename Number_T>
class PrivateInputStream;

template<typename Number_T>
class ConfigurationParser;

struct FlatbufferCtx
{
  char const* fileName = nullptr;

  std::vector<uint32_t> sizes;
  std::vector<Root const*> roots;
};

template<typename Number_T>
class Parser : public wtk::Parser<Number_T>
{
  FILE* file = nullptr;
  int fileDescriptor = -1;
  size_t fileSize = 0;

  uint8_t* buffer = nullptr;

  FlatbufferCtx ctx;

  std::unique_ptr<TranslationParser<Number_T>> translationParser;
  std::unique_ptr<CircuitParser<Number_T>> circuitParser;
  std::unique_ptr<PublicInputStream<Number_T>> publicInputStream;
  std::unique_ptr<PrivateInputStream<Number_T>> privateInputStream;
  std::unique_ptr<ConfigurationParser<Number_T>> configurationParser;

public:

  /**
   * Open the parser using the given filename.
   */
  bool open(char const* const fname);

  /**
   * Open the parser with an existing FILE*, and use the optional file name
   * for error reporting.
   */
  bool open(FILE* const file, char const* const fname = "<FILE*>");

private:
  bool openHelper();

public:

  bool parseHeader() final;

  TranslationParser<Number_T>* translation() final;

  CircuitParser<Number_T>* circuit() final;

  InputStream<Number_T>* publicIn() final;

  InputStream<Number_T>* privateIn() final;

  ConfigurationParser<Number_T>* configuration() final;

  Parser() = default;
  Parser(Parser const& copy) = delete;
  Parser(Parser&& move);
  Parser& operator=(Parser const& copy) = delete;
  Parser& operator=(Parser&& move);
  ~Parser();
};

template<typename Number_T>
class TranslationParser : public wtk::TranslationParser<Number_T>
{
public:

  TranslationParser() { }
};

template<typename Number_T>
class CircuitParser : public wtk::circuit::Parser<Number_T>
{
  FlatbufferCtx* const ctx;

  bool parseGate(
      Gate const* const, wtk::circuit::Handler<Number_T>* const handler);

public:
  CircuitParser(FlatbufferCtx* const c) : ctx(c) { }

  bool parseCircuitHeader() final;

  bool parse(wtk::circuit::Handler<Number_T>* const handler) final;

  ~CircuitParser() = default;
};

template<typename Number_T>
class PublicInputStream : public wtk::InputStream<Number_T>
{
  FlatbufferCtx* const ctx;

  size_t bufferIdx = 0;
  flatbuffers::uoffset_t streamIdx = 0;

public:

  bool parseStreamHeader() final;

  PublicInputStream(FlatbufferCtx* const c) : ctx(c) { }

  wtk::StreamStatus next(Number_T* num) final;
};

template<typename Number_T>
class PrivateInputStream : public wtk::InputStream<Number_T>
{
  FlatbufferCtx* const ctx;

  size_t bufferIdx = 0;
  flatbuffers::uoffset_t streamIdx = 0;

public:

  bool parseStreamHeader() final;

  PrivateInputStream(FlatbufferCtx* const c) : ctx(c) { }

  wtk::StreamStatus next(Number_T* num) final;
};

template<typename Number_T>
class ConfigurationParser : public wtk::ConfigurationParser<Number_T>
{
public:

  ConfigurationParser() { }
};

} } // namespace wtk::flatbuffer

#define LOG_IDENTIFIER "wtk::flatbuffer"
#include <stealth_logging.h>

#include <wtk/flatbuffer/Parser.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_PARSER_
