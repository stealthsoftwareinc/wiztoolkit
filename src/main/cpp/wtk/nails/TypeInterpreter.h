/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_NAILS_TYPE_INTERPRETER_H_
#define WTK_NAILS_TYPE_INTERPRETER_H_

#include <cstddef>
#include <vector>
#include <cstdint>
#include <cinttypes>
#include <memory>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/Parser.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/plugins/Plugin.h>

#include <wtk/nails/Scope.h>

namespace wtk {
namespace nails {

/**
 * The type interpreter interprets gates within a single field.
 *
 * It also does type-erasure of the Wire_T type.
 */

template<typename Number_T>
class TypeInterpreter
{
public:
  char const* const fileName;
  size_t lineNum = 0;

  TypeInterpreter(char const* const fn);

  // Stack operations (inter function)
  virtual void push() = 0;

  virtual bool mapOutput(wire_idx first, wire_idx last) = 0;

  virtual bool checkOutput(wire_idx first, wire_idx last, wire_idx* place) = 0;

  virtual bool mapInput(wire_idx first, wire_idx last) = 0;

  virtual void pop() = 0;

  // Gate callbacks
  virtual bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right) = 0;

  virtual bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right) = 0;

  virtual bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right) = 0;

  virtual bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right) = 0;

  virtual bool copy(wire_idx const out, wire_idx const left) = 0;

  virtual bool copyMulti(wtk::circuit::CopyMulti const* const copy) = 0;

  virtual bool assign(wire_idx const out, Number_T&& right) = 0;

  virtual bool assertZero(wire_idx const left) = 0;

  virtual bool publicIn(wire_idx const out) = 0;

  virtual bool publicInMulti(wtk::circuit::Range const* const outs) = 0;

  virtual bool privateIn(wire_idx const out) = 0;

  virtual bool privateInMulti(wtk::circuit::Range const* const outs) = 0;

  virtual bool newRange(wire_idx const first, wire_idx const last) = 0;

  virtual bool deleteRange(wire_idx const first, wire_idx const last) = 0;

  virtual bool checkNumber(Number_T const& value) = 0;

  virtual wtk::plugins::WiresRefEraser pluginOutput(
      type_idx const type, wire_idx const first, wire_idx const last) = 0;

  virtual wtk::plugins::WiresRefEraser pluginInput(
      type_idx const type, wire_idx const first, wire_idx const last) = 0;

  virtual void iterPluginHack(wire_idx const first, wire_idx const last) = 0;
  virtual Number_T getMaxValForIterPlugin() = 0;

  virtual ~TypeInterpreter() = default;
};

template<typename Number_T, typename Wire_T>
class LeadTypeInterpreter : public TypeInterpreter<Number_T>
{
public:
  TypeBackend<Number_T, Wire_T>* const backend;

  Number_T maxVal;

  InputStream<Number_T>* const publicInStream;

  InputStream<Number_T>* const privateInStream;

  std::vector<Scope<Wire_T>> stack;

  LeadTypeInterpreter(char const* const fn,
      TypeBackend<Number_T, Wire_T>* const f,
      InputStream<Number_T>* const ins, InputStream<Number_T>* const wit);

  // Operations on the stack of scopes.
  Scope<Wire_T>* top();

  void push() final;

  bool mapOutput(wire_idx first, wire_idx last) final;

  bool checkOutput(wire_idx first, wire_idx last, wire_idx* place) final;

  bool mapInput(wire_idx first, wire_idx last) final;

  void pop() final;

  // Gate callbacks.
  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right) final;

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right) final;

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right) final;

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right) final;

  bool copy(wire_idx const out, wire_idx const left) final;

  bool copyMulti(wtk::circuit::CopyMulti const* const copy) final;

  bool assign(wire_idx const out, Number_T&& right) final;

  bool assertZero(wire_idx const left) final;

  bool publicIn(wire_idx const out) final;

  bool publicInMulti(wtk::circuit::Range const* const outs) final;

  bool privateIn(wire_idx const out) final;

  bool privateInMulti(wtk::circuit::Range const* const outs) final;

  bool newRange(wire_idx const first, wire_idx const last) final;

  bool deleteRange(wire_idx const first, wire_idx const last) final;

  bool checkNumber(Number_T const& value) final;

  wtk::plugins::WiresRefEraser pluginOutput(
      type_idx const type, wire_idx const first, wire_idx const last) final;

  wtk::plugins::WiresRefEraser pluginInput(
      type_idx const type, wire_idx const first, wire_idx const last) final;

  void iterPluginHack(wire_idx const first, wire_idx const last) final;
  Number_T getMaxValForIterPlugin() final;
};

} } // namespace nails

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

#include <wtk/nails/TypeInterpreter.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_NAILS_TYPE_INTERPRETER_H_
