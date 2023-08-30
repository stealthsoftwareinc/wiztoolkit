/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_SIMPLE_PLUGIN_H_
#define WTK_PLUGINS_SIMPLE_PLUGIN_H_

#include <cstddef>
#include <vector>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/circuit/Data.h>
#include <wtk/TypeBackend.h>
#include <wtk/utils/CharMap.h>

#include <wtk/plugins/Plugin.h>

namespace wtk {
namespace plugins {

/**
 * The Simple interface is a collection of helpful super classes for plugins.
 * A requirement of the Simple interface is that an operation may be reused
 * for multiple plugin-functions.
 *
 * The SimplePlugin maintains, for each IR type, a map of names to
 * SimpleOperation objects.
 */

template<typename Number_T, typename Wire_T>
struct SimpleOperation : public Operation<Number_T>
{
  // Reference and index for the corresponding type's backend
  wtk::TypeBackend<Number_T, Wire_T>* const backend;
  wtk::type_idx const type;

  SimpleOperation(
      wtk::type_idx const t, wtk::TypeBackend<Number_T, Wire_T>* const be)
    : backend(be), type(t) { }

  /**
   * Checks if the function-signature and plugin-binding can be handled by
   * the operation.
   *
   * checkSignature(...) may return false on failure. If the check succeeds,
   * evaluate(...) must not fail, except by means of @assert_zero.
   */
  virtual bool checkSignature(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) = 0;

  /**
   * Two versions of the evaluate(...) function. Either may be implemented,
   * otherwise the Operation would be a no-op. If both are implemented the
   * 4-arg version takes precedence.
   */
  void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) override;

  virtual void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs);

  virtual ~SimpleOperation() { }
};

template<typename Number_T, typename Wire_T>
struct SimplePlugin : public Plugin<Number_T, Wire_T>
{
  bool addBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) override;

  Operation<Number_T>* create(wtk::type_idx const type,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) override;

  /**
   * Build a map of operations for the given type and backend.
   *
   * May return false on failure.
   */
  virtual bool buildBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend,
      wtk::utils::CharMap<std::unique_ptr<
        SimpleOperation<Number_T, Wire_T>>>* const operations) = 0;

  // For each type, map the operation name to a single/reusable operation
  std::vector<wtk::utils::CharMap<std::unique_ptr<
    SimpleOperation<Number_T, Wire_T>>>> types;

  // Map type indexes to indexes in the types map.
  std::vector<size_t> reverseTypes;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "wtk::plugins"
#include <stealth_logging.h>

#include <wtk/plugins/SimplePlugin.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_SIMPLE_PLUGIN_H_
