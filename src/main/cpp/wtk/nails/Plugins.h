/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_PLUGINS_H_
#define WTK_NAILS_PLUGINS_H_

#include <cstddef>
#include <vector>
#include <string>

#include <wtk/indexes.h>
#include <wtk/nails/Functions.h>

#include <wtk/circuit/Data.h>
#include <wtk/plugins/Plugin.h>

namespace wtk {
namespace nails {

/**
 * The PluginFunction implements the Function interface and backs itself
 * with a plugin operation.
 */
template<typename Number_T>
struct PluginFunction : public Function<Number_T>
{
  wtk::circuit::PluginBinding<Number_T> const binding;

  PluginFunction(wtk::circuit::FunctionSignature&& sig,
      wtk::circuit::PluginBinding<Number_T>&& b)
    : Function<Number_T>(std::move(sig)), binding(std::move(b)) { }

  wtk::plugins::Operation<Number_T>* operation = nullptr;

  bool evaluate(Interpreter<Number_T>* const interpreter) final;
};


} } // namespace wtk::nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Plugins.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_PLUGINS_H_
