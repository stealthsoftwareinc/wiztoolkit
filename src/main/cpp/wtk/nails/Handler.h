/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_HANDLER_H_
#define WTK_NAILS_HANDLER_H_

#include <cstddef>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/utils/CharMap.h>
#include <wtk/utils/Pool.h>
#include <wtk/circuit/Handler.h>

#include <wtk/nails/Interpreter.h>
#include <wtk/nails/Functions.h>
#include <wtk/nails/Plugins.h>

namespace wtk {
namespace nails {

/**
 * NAILS: Naive Amenity for Interpreting Long Streams
 *
 * The Handler is the interface between the parser and the backend. It will
 * direct gates either, in the top-level scope, to the interpreter, or in
 * a function definition to a function object to be remembered for later.
 */
template<typename Number_T>
class Handler : public wtk::circuit::Handler<Number_T>
{
public:
  Interpreter<Number_T>* const interpreter;

  FunctionFactory<Number_T>* const functionFactory;

  // under construction function stuff
  wtk::circuit::FunctionSignature sigConstruction;
  RegularFunction<Number_T>* funcConstruction = nullptr;

  //plugins stuff
  wtk::utils::Pool<PluginFunction<Number_T>> pluginPool;
  wtk::plugins::PluginsManagerEraser<Number_T>* const pluginsManager;

  Handler(Interpreter<Number_T>* const i, FunctionFactory<Number_T>* const ff,
      wtk::plugins::PluginsManagerEraser<Number_T>* const pm)
    : interpreter(i), functionFactory(ff), pluginsManager(pm) { }

  // Gate handler functions
  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) final;

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) final;

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) final;

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) final;

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) final;

  bool copyMulti(wtk::circuit::CopyMulti* copy_multi) final;

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) final;

  bool assertZero(wire_idx const left, type_idx const type) final;

  bool publicIn(wire_idx const out, type_idx const type) final;

  bool publicInMulti(wtk::circuit::Range* outs, type_idx const type) final;

  bool privateIn(wire_idx const out, type_idx const type) final;

  bool privateInMulti(wtk::circuit::Range* outs, type_idx const type) final;

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) final;

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) final;

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) final;

  bool startFunction(wtk::circuit::FunctionSignature&& signature) final;

  bool regularFunction() final;

  bool endFunction() final;

  bool pluginFunction(wtk::circuit::PluginBinding<Number_T>&& binding) final;

  bool invoke(wtk::circuit::FunctionCall* const call) final;
};

} } // namespace wtk::nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/Handler.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_HANDLER_H_
