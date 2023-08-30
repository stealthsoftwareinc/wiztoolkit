/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_CIRCUIT_PARSER_
#define WTK_CIRCUIT_PARSER_

#include <cstddef>
#include <vector>
#include <string>

#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Data.h>

namespace wtk {
namespace circuit {


/**
 * A common interface for circuit abstraction parsers.
 */
template<typename Number_T>
class Parser
{
public:
  // List of declared plugin names
  std::vector<std::string> plugins;

  // List of declared types
  std::vector<TypeSpec<Number_T>> types;

  // List of declared conversion specifications
  std::vector<ConversionSpec> conversions;

  /**
   * Parses the circuit header, emplacing information into this->plugins,
   * this->types, and this->conversions.
   *
   * Returns false on failure.
   */
  virtual bool parseCircuitHeader() = 0;

  /**
   * Parsing a circuit uses a callback interface for handling each gate.
   *
   * Returns false on failure.
   */
  virtual bool parse(Handler<Number_T>* const handler) = 0;

  virtual ~Parser() = default;
};

} } // namespace wtk::circuit

#endif//WTK_CIRCUIT_PARSER_
