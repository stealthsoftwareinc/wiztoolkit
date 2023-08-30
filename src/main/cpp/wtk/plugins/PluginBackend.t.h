/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::assign(Wire_T* wire, Number_T&& value)
{
  (void) wire;
  (void) value;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::copy(Wire_T* wire, Wire_T const* value)
{
  (void) wire;
  (void) value;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::addGate(
    Wire_T* out, Wire_T const* left, Wire_T const* right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::mulGate(
    Wire_T* out, Wire_T const* left, Wire_T const* right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::addcGate(
    Wire_T* out, Wire_T const* left, Number_T&& right)
{
  (void) out;
  (void) left;
  (void) right;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::mulcGate(
    Wire_T* out, Wire_T const* left, Number_T&& right)
{
  (void) out;
  (void) left;
  (void) right;
}


template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::assertZero(
    Wire_T const* left)
{
  (void) left;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::publicIn(
    Wire_T* out, Number_T&& right)
{
  (void) out;
  (void) right;
}

template<typename Number_T, typename Wire_T>
void PluginBackend<Number_T, Wire_T>::privateIn(
    Wire_T* out, Number_T&& right)
{
  (void) out;
  (void) right;
}

} } // namespace wtk::plugins
