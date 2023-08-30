/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PLUGINS_PLUGIN_BACKEND_H_
#define WTK_PLUGINS_PLUGIN_BACKEND_H_

#include <cstddef>

#include <wtk/TypeBackend.h>
#include <wtk/circuit/Data.h>

namespace wtk {
namespace plugins {

/**
 * A parent class for plugin types/backends which will not use ordinary
 * gates, and only function via plugin functions.
 *
 * Default implementations are provided for all callbacks.
 */
template<typename Number_T, typename Wire_T>
struct PluginBackend : public wtk::TypeBackend<Number_T, Wire_T>
{
  // Non-default constructor passes the Type through to TypeBackend.type
  PluginBackend(wtk::circuit::TypeSpec<Number_T> const* const type)
    : wtk::TypeBackend<Number_T, Wire_T>(type) { }

  // Indicate to the interpreter (such as NAILS) that gate calls should
  // be prohibited in favor of errors.
  bool supportsGates() const override { return false; }

  // Default no-op implementations of all callbacks
  void assign(Wire_T* wire, Number_T&& value) override;

  void copy(Wire_T* wire, Wire_T const* value) override;

  void addGate(
      Wire_T* out, Wire_T const* left, Wire_T const* right) override;

  void mulGate(
      Wire_T* out, Wire_T const* left, Wire_T const* right) override;

  void addcGate(Wire_T* out, Wire_T const* left, Number_T&& right) override;

  void mulcGate(Wire_T* out, Wire_T const* left, Number_T&& right) override;

  void assertZero(Wire_T const* left) override;

  void publicIn(Wire_T* wire, Number_T&& value) override;

  void privateIn(Wire_T* wire, Number_T&& value) override;
};

} } // namespace wtk::plugins

#include <wtk/plugins/PluginBackend.t.h>

#endif//WTK_PLUGINS_PLUGIN_BACKEND_H_
