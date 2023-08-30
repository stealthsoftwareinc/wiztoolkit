/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <string>
#include <vector>
#include <memory>

#include <wtk/indexes.h>

#ifndef WTK_CIRCUIT_DATA_
#define WTK_CIRCUIT_DATA_

namespace wtk {
namespace circuit {

/**
 * Plugin Bindings specify the name and operation of the plugin, along with
 * a list of plugin parameters and optionally some input counts.
 */
template<typename Number_T>
struct PluginBinding
{
  // The plugin's name (should match one of wtk::circuit::Parser.plugins)
  std::string name;

  // The operation within the plugin
  std::string operation;

  struct Parameter
  {
    // An enum indicating what kind of plugin parameter.
    enum {
      textual,
      numeric
    } form;

    // Set when the parameter is textual
    std::string text = "";

    // Set when the parameter is numeric
    Number_T number = 0;

    Parameter(std::string&& txt) : form(textual), text(std::move(txt)) { }

    Parameter(Number_T&& n) : form(numeric), number(n) { }
  };

  // List of plugin parameters
  std::vector<Parameter> parameters;

  // Input stream consumption counts. vector index corresponds to a type_idx
  // and the value is that type's count.
  //
  // The vectors may have size 0 if no inputs are specified.
  std::vector<size_t> publicInputCount;
  std::vector<size_t> privateInputCount;
};

template<typename Number_T>
bool operator== (
    PluginBinding<Number_T> const& l, PluginBinding<Number_T> const& r);

template<typename Number_T>
bool operator!= (
    PluginBinding<Number_T> const& l, PluginBinding<Number_T> const& r);

/**
 * A type specification describes the attributes of a type.
 * It will correspond with a type_idx through the parser.types vector.
 */
template<typename Number_T>
struct TypeSpec
{
  // Enumeration indicating what kind of type is specified.
  enum
  {
    field,
    ring,
    plugin
  } const variety = field;

  // The prime for field types
  Number_T const prime = 0;

  // The bit-width for ring types
  size_t const bitWidth = 0;

  // The plugin binding for plugin types
  PluginBinding<Number_T> const binding;

  // Construct a field type
  TypeSpec(Number_T&& p) : variety(field), prime(std::move(p)) { }

  // Construct a ring type
  TypeSpec(size_t const bw) : variety(ring), bitWidth(bw) { }

  // Construct a plugin type
  TypeSpec(PluginBinding<Number_T>&& b)
    : variety(plugin), binding(std::move(b)) { }

  // Return the type's maximum value (e.g. field's prime or 2**bitWidth for
  // a ring, or 0 for a plugin).
  Number_T maxValue() const;

  // Return true if the type is the boolean field (GF(2))
  bool isBooleanField() const;
};

template<typename Number_T>
bool operator== (
    TypeSpec<Number_T> const& l, TypeSpec<Number_T> const& r);

template<typename Number_T>
bool operator!= (
    TypeSpec<Number_T> const& l, TypeSpec<Number_T> const& r);

/**
 * A struct to describe a conversion specification (predefinitions in the
 * circuit header).
 */
struct ConversionSpec
{
  // output and input types
  type_idx const outType;
  type_idx const inType;

  // output and input length
  size_t const outLength;
  size_t const inLength;

  ConversionSpec(
      type_idx const ot, size_t const ol, type_idx const it, size_t const il)
    : outType(ot), inType(it), outLength(ol), inLength(il) { }
};

inline bool operator==(ConversionSpec const& a, ConversionSpec const& b);

/**
 * Ranges in the IR use $first...$last notation (inclusive on both ends)
 */
struct Range
{
  // The first wire in the range
  wire_idx const first;
  // The last wire in the range
  wire_idx const last;

  Range(wire_idx const f, wire_idx const l) : first(f), last(l) { }
};

/**
 * A multiwire copy gate uses wire ranges to copy multiple wires.
 */
struct CopyMulti
{
  // There's one output range
  Range const outputs;

  // And multiple input ranges.
  std::vector<Range> inputs;

  // And the type index.
  type_idx const type;

  CopyMulti(wire_idx const fo, wire_idx const lo, type_idx const t)
    : outputs(fo, lo), type(t) { }
};

/**
 * A function signature is just the name and input/output parameters of a
 * function.
 */
struct FunctionSignature
{
  // The function's name
  std::string name;

  // A function parameter is the pair of a type and a count of wires
  struct Parameter
  {
    // The type of this parameter
    type_idx const type;

    // The count of wires necessary to satisfy the parameter
    size_t const length;

    Parameter(type_idx const t, size_t const l) : type(t), length(l) { }
  };

  // List of output parameters
  std::vector<Parameter> outputs;

  // List of input parameters
  std::vector<Parameter> inputs;

  /**
   * line number (if applicable).
   */
  size_t lineNum = 0;
};

/**
 * A function call has the name and the input/output wire bindings.
 */
struct FunctionCall
{
  // Function name
  std::string name;

  // List of output ranges
  std::vector<Range> outputs;

  // List of input ranges
  std::vector<Range> inputs;

  /**
   * line number (if applicable).
   */
  size_t lineNum = 0;
};

} } // namespace wtk::circuit

#include <wtk/circuit/Data.t.h>

#endif//WTK_CIRCUIT_DATA_
