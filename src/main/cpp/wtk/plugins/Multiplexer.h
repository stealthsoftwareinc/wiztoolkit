/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_MULTIPLEXERS_H_
#define WTK_PLUGINS_MULTIPLEXERS_H_

#include <cstddef>
#include <vector>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/utils/CharMap.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/circuit/Data.h>

#include <wtk/plugins/SimplePlugin.h>

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
struct MuxOperation : public SimpleOperation<Number_T, Wire_T>
{

  MuxOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : SimpleOperation<Number_T, Wire_T>(type, backend) { }


  bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  void evaluate(std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs) final;

  /**
   * Virtual function for evaluating the multiplexer operation.
   *
   * sizes, outputs, and inputs[i] are all coindexed.
   * outputs[j] and inputs[i][j] are pointers to output and input ranges,
   * each of length size[j].
   *
   * Selector should be constrained to the range [0, i), and indicates which
   * bundle of inputs is selected by the multiplexer.
   * 
   * the last input, selector_bits is only not equal to 1 in the boolean case.
   */
  virtual void evaluateMux(std::vector<size_t> const& sizes,
      std::vector<Wire_T*>& outputs,
      Wire_T const* const selector,
      std::vector<std::vector<Wire_T const*>>& inputs,
      size_t const selector_bits = 1) = 0;
};

template<typename Number_T, typename Wire_T>
struct MultiplexerPlugin : public SimplePlugin<Number_T, Wire_T>
{
  bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend,
      wtk::utils::CharMap<std::unique_ptr<
        SimpleOperation<Number_T, Wire_T>>>* const operations) final;

  virtual SimpleOperation<Number_T, Wire_T>* buildStrictMultiplexer(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  virtual SimpleOperation<Number_T, Wire_T>* buildPermissiveMultiplexer(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;
};

template<typename Number_T, typename Wire_T>
struct FallbackMultiplexerPlugin : public MultiplexerPlugin<Number_T, Wire_T>
{
  SimpleOperation<Number_T, Wire_T>* buildStrictMultiplexer(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  SimpleOperation<Number_T, Wire_T>* buildPermissiveMultiplexer(
      wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;
};

/**
 * Default implementation of a strict multiplexer using
 * Fermat's Little Theorem.
 */
template<typename Number_T, typename Wire_T>
struct StrictFLTMuxOperation : MuxOperation<Number_T, Wire_T>
{
  StrictFLTMuxOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : MuxOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateMux(std::vector<size_t> const& sizes,
      std::vector<Wire_T*>& outputs,
      Wire_T const* const selector,
      std::vector<std::vector<Wire_T const*>>& inputs,
      size_t const selector_bits = 1) override;
};

/**
 * Default implementation of a permissive multiplexer using
 * Fermat's Little Theorem.
 */
template<typename Number_T, typename Wire_T>
struct PermissiveFLTMuxOperation : MuxOperation<Number_T, Wire_T>
{
  PermissiveFLTMuxOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : MuxOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateMux(std::vector<size_t> const& sizes,
      std::vector<Wire_T*>& outputs,
      Wire_T const* const selector,
      std::vector<std::vector<Wire_T const*>>& inputs,
      size_t const selector_bits = 1) override;
};

/**
 * Default implementation of a strict multiplexer using
 * Treed booleans.
 */
template<typename Number_T, typename Wire_T>
struct StrictTreedBooleanMuxOperation : MuxOperation<Number_T, Wire_T>
{
  StrictTreedBooleanMuxOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : MuxOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateMux(std::vector<size_t> const& sizes,
      std::vector<Wire_T*>& outputs,
      Wire_T const* const selector,
      std::vector<std::vector<Wire_T const*>>& inputs,
      size_t const selector_bits) override;
};

/**
 * Default implementation of a permissive multiplexer using
 * Treed booleans.
 */
template<typename Number_T, typename Wire_T>
struct PermissiveTreedBooleanMuxOperation : MuxOperation<Number_T, Wire_T>
{
  PermissiveTreedBooleanMuxOperation(type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend)
    : MuxOperation<Number_T, Wire_T>(type, backend) { }

  void evaluateMux(std::vector<size_t> const& sizes,
      std::vector<Wire_T*>& outputs,
      Wire_T const* const selector,
      std::vector<std::vector<Wire_T const*>>& inputs,
      size_t const selector_bits) override;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "mux_v0"
#include <stealth_logging.h>

#include <wtk/plugins/Multiplexer.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_MULTIPLEXERS_H_

