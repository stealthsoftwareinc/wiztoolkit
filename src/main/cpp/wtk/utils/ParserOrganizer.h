/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_PARSER_ORGANIZER_H_
#define WTK_UTILS_PARSER_ORGANIZER_H_

#include <cstddef>
#include <vector>
#include <cstring>

#include <wtk/Parser.h>
#include <wtk/circuit/Parser.h>
#include <wtk/circuit/Data.h>
#include <wtk/versions.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace utils {

// A helper for stringifying a version.
std::string stringify_version(
    size_t const major, size_t const minor, size_t const patch,
    char const* const extra);

/**
 * This enumeration indicates in what setting the IR process will work.
 * This is determined by the presence or absence of public and private
 * input streams.
 */
enum class Setting
{
  failure,    /** Could not recognize the setting. */
  preprocess, /** No access to public or private inputs. */
  verifier,   /** Access to public inputs, but not private. */
  prover      /** Access to both public and private inputs. */
};

/**
 * A helper for recognizing the resource type of an input file and organizing
 * it for easy use.
 *
 * Start by opening all the parsers, then call organize().
 */
template<typename Parser_T, typename Number_T>
struct ParserOrganizer
{
private:
  std::vector<Parser_T> parsers;
  std::vector<char const*> fileNames;
  std::vector<bool> parserUsed;

public:
  /**
   * Open the named file and create a parser for it.
   * Note that the fileName is stored and may be used later, so it may need
   * a lifetime longer than the call to open().
   *
   * Returns false on failure.
   */
  bool open(char const* const fileName);

  /**
   * Once all the resources are opened, organize them and return the Setting
   * detected.
   *
   * Returns the detected Setting, or else Setting::failure.
   */
  Setting organize();

  /**
   * Once organize() has returned successfully the following
   * Setting-appropriate attributes will be set.
   */

  /** The top-level parser for the circuit */
  wtk::Parser<Number_T>* circuitParser = nullptr;
  /** The circuit-specific parser */
  wtk::circuit::Parser<Number_T>* circuitBodyParser = nullptr;
  /** The circuit's file name. */
  char const* circuitName = nullptr;

  struct InputStreamPair
  {
    /** The top-level parser for the public_input */
    wtk::Parser<Number_T>* publicParser = nullptr;
    /** The stream parser for the public_input */
    wtk::InputStream<Number_T>* publicStream = nullptr;
    /** the public_input's file name */
    char const* publicName = nullptr;

    /** The top-level parser for the private_input */
    wtk::Parser<Number_T>* privateParser = nullptr;
    /** The stream parser for the private_input */
    wtk::InputStream<Number_T>* privateStream = nullptr;
    /** The private_input's file name */
    char const* privateName = nullptr;
  };

  /** A list of input streams, co-ordered with circuitBodyParser.types */
  std::vector<InputStreamPair> circuitStreams;
};

template<typename Number_T>
std::string type_str(wtk::circuit::TypeSpec<Number_T> const* const type);

} } // namespace wtk::utils

#define LOG_IDENTIFIER "wtk::utils::ParserOrganizer"
#include <stealth_logging.h>

#include <wtk/utils/ParserOrganizer.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_UTILS_PARSER_ORGANIZER_H_
