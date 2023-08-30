/**
 * Copyright (C) 2022-2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_PLUGIN_H_
#define WTK_PLUGINS_PLUGIN_H_

#include <cstddef>
#include <cinttypes>
#include <vector>
#include <tuple>
#include <memory>
#include <utility>
#include <type_traits>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/CharMap.h>

namespace wtk {
namespace plugins {

/**
 * The WiresRefEraser performs type erasure over the WiresRef type.
 */
struct WiresRefEraser
{
  // The length of wires being referenced
  size_t const size;

protected:
  // Reference to the wires
  void* const wires;

public:
  // The type
  type_idx const type;

  virtual ~WiresRefEraser() { }

protected:
  WiresRefEraser(size_t const s, void* const ws, type_idx const t)
    : size(s), wires(ws), type(t) { }
};

/**
 * The WiresRef is a wrapper for a size and pointer of wires.
 */
template<typename Wire_T>
struct WiresRef : public WiresRefEraser
{
  WiresRef(size_t const s, Wire_T* const ws, type_idx const t)
    : WiresRefEraser(s, static_cast<void*>(ws), t) { }

  Wire_T* get() { return static_cast<Wire_T*>(this->wires); }
};


/**
 * The SingleOperation (and SingleOperationEraser?) is a super class for the
 * implementation of all plugin operations with a single wire type.
 *
 * The interface is supposed to be agnostic to IR layer, so everything
 * is based on WiresRefs.
 */
template<typename Number_T>
struct Operation
{
  /**
   * Evaluate a plugin operation on some inputs and outputs.
   * Must assign all outputs.
   */
  virtual void evaluate(
      std::vector<WiresRefEraser>& outputs,
      std::vector<WiresRefEraser>& inputs,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) = 0;

  virtual ~Operation() { }
};

/**
 * Each plugin manages Operations for a plugin within a particular Wire_T.
 * In the case that a requested Operation's signature uses multiple fields,
 * first the inputs are scanned for a type supported by the plugin, then the
 * outputs are scanned.
 */
template<typename Number_T, typename Wire_T>
struct Plugin
{
  /**
   * Setup the plugin by adding a backend to it.
   *
   * May return false on failure.
   */
  virtual bool addBackend(wtk::type_idx const type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend) = 0;

  /**
   * create (or return a pre-existing) operation from this plugin. May return
   * nulltpr to indicate failure.
   */
  virtual Operation<Number_T>* create(type_idx const type,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) = 0;

  virtual ~Plugin() { }
};

// Type eraser for the later PluginsManager
template<typename Number_T>
struct PluginsManagerEraser
{
  virtual Operation<Number_T>* create(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) = 0;

  virtual ~PluginsManagerEraser() { }
};

/**
 * The plugins manager just distributes plugin function signatures and
 * bindings to the correct plugin.
 */
template<typename Number_T, typename... Wire_Ts>
struct PluginsManager : public PluginsManagerEraser<Number_T>
{
  /**
   * Add a plugin to this plugin manager. This will allow WTK machinery
   * access to the plugin to create operations. It will also add subsequently
   * added backends to the plugin.
   *
   * Returns false if a duplicate plugin/Wire_T is provided.
   */
  template<typename Wire_T>
  bool addPlugin(char const* const name,
      std::unique_ptr<Plugin<Number_T, Wire_T>>&& plugin);

  /**
   * Adds a backend to each plugin already known to the manager.
   *
   * Returns false if rejected by a plugin.
   */
  template<typename Wire_T>
  bool addBackend(wtk::type_idx type,
      wtk::TypeBackend<Number_T, Wire_T>* const backend);

  /**
   * Request that a plugin creates an operation matching the given signature
   * and binding.
   */
  Operation<Number_T>* create(
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding) override;

private:
  // Template meta helpers for getting dynamic wire-indexes in create(...)
  template<size_t I>
  typename std::enable_if<I == sizeof...(Wire_Ts), Operation<Number_T>*>::type
  createHelper(size_t tpl_idx, wtk::type_idx const type,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding,
      std::tuple<std::unique_ptr<Plugin<Number_T, Wire_Ts>>...>& tpl);

  template<size_t I>
  typename std::enable_if<I < sizeof...(Wire_Ts), Operation<Number_T>*>::type
  createHelper(size_t tpl_idx, wtk::type_idx const type,
      wtk::circuit::FunctionSignature const* const signature,
      wtk::circuit::PluginBinding<Number_T> const* const binding,
      std::tuple<std::unique_ptr<Plugin<Number_T, Wire_Ts>>...>& tpl);

public:

  // Helper struct for locating the correct plugin given just a name and type.
  struct PluginBundle
  {
    std::tuple<std::unique_ptr<Plugin<Number_T, Wire_Ts>>...> pluginBundle;

    // reverseTypeIdxs[type] is the index in pluginBundle
    std::vector<size_t> reverseTypeIdxs;
  };

  wtk::utils::CharMap<PluginBundle> plugins;
};

} } // namespace wtk::plugins

#define LOG_IDENTIFIER "wtk::plugins"
#include <stealth_logging.h>

#include <wtk/plugins/Plugin.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PLUGINS_PLUGIN_H_
