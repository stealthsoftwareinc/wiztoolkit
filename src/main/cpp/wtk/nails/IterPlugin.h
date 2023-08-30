/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_ITER_PLUGIN_H_
#define WTK_NAILS_ITER_PLUGIN_H_

#include <cstddef>
#include <vector>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/plugins/Plugin.h>
#include <wtk/nails/Interpreter.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace nails {

template<typename Number_T>
struct MapOperation;

template<typename Number_T, typename Wire_T>
struct IterPlugin : public wtk::plugins::Plugin<Number_T, Wire_T>
{
  MapOperation<Number_T>* operation;

  IterPlugin(MapOperation<Number_T>* op) : operation(op) { }

  bool addBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) final;

  wtk::plugins::Operation<Number_T>* create(type_idx const type,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;
};

template<typename Number_T>
struct MapOperation : public wtk::plugins::Operation<Number_T>
{
  Interpreter<Number_T>* const interpreter;

  MapOperation(Interpreter<Number_T>* const i) : interpreter(i) { }

  template<typename Wire_T>
  std::unique_ptr<wtk::plugins::Plugin<Number_T, Wire_T>> makePlugin();

  void evaluate(
      std::vector<wtk::plugins::WiresRefEraser>& outputs,
      std::vector<wtk::plugins::WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) final;

  bool typeCheck(wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding);
};

} } // namespace wtk::nails

#define LOG_IDENTIFIER "iter_v0"
#include <stealth_logging.h>

#include <wtk/nails/IterPlugin.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_ITER_PLUGIN_H_


