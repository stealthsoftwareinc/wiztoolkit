/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_FIELD_BACKEND_H_
#define WTK_FIREALARM_FIELD_BACKEND_H_

#include <cstddef>

#include <wtk/indexes.h>
#include <wtk/TypeBackend.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/firealarm/Wire.h>
#include <wtk/firealarm/Counters.h>

#include <wtk/utils/Indent.h>

#define LOG_IDENTIFIER "wtk::firealarm"
#include <stealth_logging.h>

namespace wtk {
namespace firealarm {

template<typename Number_T, typename Wire_T>
class FieldBackend : public wtk::TypeBackend<Number_T, Wire<Wire_T>>
{
public:
  char const* const fileName;

  Wire_T const primeWire;

  TypeCounter* const counter;

  bool suppressAsserts;
  bool trace = false;
  wtk::utils::Indent const* indent = nullptr;

  FieldBackend(
      char const* const fn, wtk::circuit::TypeSpec<Number_T> const* const t,
      TypeCounter* const c, bool sa)
    : wtk::TypeBackend<Number_T, Wire<Wire_T>>(t),
    fileName(fn), primeWire(static_cast<Wire_T>(t->prime)), counter(c),
    suppressAsserts(sa)
  {
    log_assert(t->variety == wtk::circuit::TypeSpec<Number_T>::field);
  }

  void enableTrace(wtk::utils::Indent const* const idt);

  void assign(Wire<Wire_T>* element, Number_T&& value) override;

  void copy(Wire<Wire_T>* element, Wire<Wire_T> const* value) override;

  void addGate(Wire<Wire_T>* out,
      Wire<Wire_T> const* left, Wire<Wire_T> const* right) override;

  void mulGate(Wire<Wire_T>* out,
      Wire<Wire_T> const* left, Wire<Wire_T> const* right) override;

  void addcGate(Wire<Wire_T>* out,
      Wire<Wire_T> const* left, Number_T&& right) override;

  void mulcGate(Wire<Wire_T>* out,
      Wire<Wire_T> const* left, Number_T&& right) override;

  void assertZero(Wire<Wire_T> const* left) override;

  void publicIn(Wire<Wire_T>* element, Number_T&& value) override;

  void privateIn(Wire<Wire_T>* element, Number_T&& value) override;

  bool check() override;

  bool supportsExtendedWitness() override { return true; }

  Number_T getExtendedWitness(Wire<Wire_T> const* wire) override;

  wire_idx getExtendedWitnessIdx(Wire<Wire_T> const* wire) override;

  // starts as false (no failure) and may be set to indicate a failure at end
  bool fail = false;
};

} } // namespace wtk::firealarm

#include <wtk/firealarm/FieldBackend.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_FIELD_BACKEND_H_
