/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PARSER_
#define WTK_PARSER_

#include <cstddef>
#include <string>

#include <wtk/circuit/Parser.h>

namespace wtk {

/**
 * An enumeration to indicate the which IR resource is to be parsed.
 */
enum class ResourceType
{
  translation,
  circuit,
  public_in,
  private_in,
  configuration // CCC
};

template<typename Number_T>
class TranslationParser;

template<typename Number_T>
class InputStream;

template<typename Number_T>
class ConfigurationParser;

/**
 * A common interface for IR parsers. The interface will largely delegate to
 * TranslationParser, CircuitParser, and InputStream.
 */
template<typename Number_T>
class Parser
{
public:

  /**
   * Version information in major.minor.patch-extra form.
   * Assigned by parseHeader().
   */
  struct {
    size_t major = 0;
    size_t minor = 0;
    size_t patch = 0;
    std::string extra;
  } version;

  /**
   * An indicator for the type of resource. This is set by parseHeader().
   */
  ResourceType type;

  /**
   * Parses the IR header to get the version and type.
   * returns false on parse failure.
   */
  virtual bool parseHeader() = 0;

  /**
   * Gets a delegate parser for a Translation resource.
   * Valid only when this->type == ResourceType::translation.
   * Repeated calls will return the same object.
   */
  virtual TranslationParser<Number_T>* translation() = 0;

  /**
   * Gets a delegate parser for a Circuit resource.
   * Valid only when this->type == ResourceType::circuit.
   * Repeated calls will return the same object.
   */
  virtual wtk::circuit::Parser<Number_T>* circuit() = 0;

  /**
   * Gets a delegate parser for a public input resource.
   * Valid only when this->type == ResourceType::public_in.
   * Repeated calls will return the same object.
   */
  virtual InputStream<Number_T>* publicIn() = 0;

  /**
   * Gets a delegate parser for a private input resource.
   * Valid only when this->type == ResourceType::private_in.
   * Repeated calls will return the same object.
   */
  virtual InputStream<Number_T>* privateIn() = 0;

  /**
   * Gets a delagate parser for a configuration resource.
   * Valid only when this->type == ResourceType::configuration.
   * Repeated calls will return the same object.
   */
  virtual ConfigurationParser<Number_T>* configuration() = 0;

  Parser() = default;
  Parser(Parser const& copy) = delete;
  Parser(Parser&& /* move */) = default;
  Parser& operator=(Parser const& copy) = delete;
  Parser& operator=(Parser&& move) = default;
  virtual ~Parser() = default;
};

/**
 * A common interface for Translation IR parsers.
 */
template<typename Number_T>
class TranslationParser
{
public:
  // virtual Translation<Number_T> const* parse() = 0;

  virtual ~TranslationParser() = default;
};

/**
 * Enum that indicates the status of the InputStream class which follows
 * shortly.
 */
enum class StreamStatus
{
  success, // successfully retrieved the next item of the stream.
  end,     // reached the end of the stream.
  error    // A parsing error occurred.
};

/**
 * A common interface for parsing public and private input streams.
 */
template<typename Number_T>
class InputStream
{
public:

  // The element type for this Input Stream.
  std::unique_ptr<wtk::circuit::TypeSpec<Number_T>> type;

  /**
   * Parse the input stream's header. This must be called exactly once before
   * any calls to next().
   */
  virtual bool parseStreamHeader() = 0;

  /**
   * Parses the next item in the stream and returns it by pointer.
   * Behavior is undefined if num is an invalid pointer.
   */
  virtual StreamStatus next(Number_T* num) = 0;

  /**
   * Optional method to return the line number of the previously
   * read stream value. Default return is 0.
   *
   * If the stream is at its end, this should return the line of
   * the end token. (or zero, if not supported.)
   */
  virtual size_t lineNum() { return 0; }

  virtual ~InputStream() = default;
};

/**
 * A common interface for parsing the Circuit Configuration Communication.
 */
template<typename Number_T>
class ConfigurationParser
{
public:
  /* TODO */
};

} // namespace wtk

#endif//WTK_CIRCUIT_PARSER_
