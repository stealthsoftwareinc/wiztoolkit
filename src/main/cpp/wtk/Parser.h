/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ABSTRACT_PARSER_H_
#define WTK_ABSTRACT_PARSER_H_

#include <cstdint>
#include <cstddef>
#include <memory>

#include <wtk/ArithmeticStreamHandler.h>
#include <wtk/BooleanStreamHandler.h>
#include <wtk/IRTree.h>
#include <wtk/IRParameters.h>

/**
 * This header defines a few abstract types for parsing the SIEVE IR.
 *  - Parser: Parses the header and can produces parsers for different
 *  profiles.
 *  - BooleanParser: Parses the IR body using boolean types.
 *  - ArithmeticParser: Parses the IR body using arithmetic types.
 *
 * For a simple FeatureToggles setting the BooleanParser and ArithmeticParsers
 * can use the handlers API and direct IR Directives through a handler
 * (wtk/ArithmeticStreamHandler and wtk/BooleanStreamHandler), forgoing the
 * production of a syntax tree. This means that the parser could be made O(1)
 * in memory, although IRRegular is the only one which actually is O(1).
 *
 * For non-simple FeatureToggle settings, a syntax tree must be constructed
 * instead, to handle nesting/scoping directives.
 *
 * For Instances and Short Witnesses, this file has an InputStream type
 * which can retrieve values of the instance/witness one at a time.
 *
 * In all classes a Number_T parameter is referred to. This is the numeric
 * type used to represent field elements. To represent wire numbers
 * an alias from uint64_t to wtk::index_t is used.
 */

namespace wtk {

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
 * Encapsulates a stream of instance or short-witness values.
 */
template<typename Number_T>
struct InputStream
{
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
  virtual size_t lineNum();

  virtual ~InputStream() = default;
};

/**
 * Parser for Arithmetic types.
 */
template<typename Number_T>
struct ArithmeticParser
{
  /**
   * Parse the body of a simple arithmetic IR relation.
   * Results are fed directly to the handler, which should be nonnull.
   * Returns false on a parse error.
   * This should be aware of both the GateSet and FeatureToggles, it will
   * error if either is violated.
   */
  virtual bool parseStream(ArithmeticStreamHandler<Number_T>* handler) = 0;

  /**
   * Returns the Syntax Tree of the IR relation.
   * May return nullptr on a parse error.
   * This should be aware of both the GateSet and FeatureToggles, it will
   * error if either is violated.
   */
  virtual IRTree<Number_T>* parseTree() = 0;

  /**
   * Return an InputStream for an instance.
   * Will not return nullptr, if there is a parse error, the returned
   * object should return an error upon consumption.
   */
  virtual InputStream<Number_T>* instance() = 0;

  /**
   * Return an InputStream for a short witness.
   * Will not return nullptr, if there is a parse error, the returned
   * object should return an error upon consumption.
   */
  virtual InputStream<Number_T>* shortWitness() = 0;

  virtual ~ArithmeticParser() = default;
};

/**
 * Parser for Boolean types.
 */
struct BooleanParser
{
  /**
   * Parse the body of a simple boolean IR relation.
   * Results are fed directly to the handler, which should be nonnull.
   * Returns false on a parse error.
   * This should be aware of both the GateSet and FeatureToggles, it will
   * error if either is violated.
   */
  virtual bool parseStream(BooleanStreamHandler* handler) = 0;

  /**
   * Returns the Syntax Tree of the IR relation.
   * May return nullptr on a parse error.
   * This should be aware of both the GateSet and FeatureToggles, it will
   * error if either is violated.
   */
  virtual IRTree<uint8_t>* parseTree() = 0;

  /**
   * Return an InputStream for a boolean instance.
   * Will not return nullptr, if there is a parse error, the returned
   * object should return an error upon consumption.
   */
  virtual InputStream<uint8_t>* instance() = 0;

  /**
   * Return an InputStream for a boolean short witness.
   * Will not return nullptr, if there is a parse error, the returned
   * object should return an error upon consumption.
   */
  virtual InputStream<uint8_t>* shortWitness() = 0;

  virtual ~BooleanParser() = default;
};

template<typename Number_T>
struct Parser
{
  /* Header Elements */
  struct {
    size_t major = 0;
    size_t minor = 0;
    size_t patch = 0;
  } version;

  Number_T characteristic = Number_T(0);
  size_t degree = 0;

  /**
   * Parser for the header
   *  - version
   *  - field (characteristic, degree)
   *
   * Results are stored as struct attributes.
   */
  virtual bool parseHeader() = 0;

  Resource resource = Resource::invalid;

  /**
   * Parse the resource type. The result is stored in this->resource.
   */
  virtual bool parseResource() = 0;

  GateSet gateSet;
  FeatureToggles featureToggles;

  /**
   * Parse the gate set and feature toggle parameters.
   */
  virtual bool parseParameters() = 0;

  /**
   * Parse the header, resource and parameters (if resource is relation).
   */
  virtual bool parseHdrResParams();

  /**
   * Parse the body of a boolean IR relation.
   * Returns nullptr on failure.
   */
  virtual BooleanParser* boolean() = 0;

  /**
   * Parse the body of an arithmetic IR relation.
   * Returns nullptr on failure.
   */
  virtual ArithmeticParser<Number_T>* arithmetic() = 0;
  virtual ArithmeticParser<uint64_t>* arithmetic64() = 0;
  virtual ArithmeticParser<uint32_t>* arithmetic32() = 0;

  virtual ~Parser() = default;
};

} // namespace wtk

#include <wtk/Parser.t.h>

#endif // WTK_ABSTRACT_PARSER_H_
