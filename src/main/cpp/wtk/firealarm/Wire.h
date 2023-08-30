/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_WIRE_
#define WTK_FIREALARM_WIRE_

#include <cstddef>

#include <wtk/firealarm/Counters.h>

namespace wtk {
namespace firealarm {

// A wrapper for a value and a counter. At destruction, counters must
// be updated
template<typename Element_T>
struct Wire
{
  Element_T value = Element_T(0);

  TypeCounter* counter = nullptr;

  Wire() = default;
  
  Wire(Wire const& copy) = delete;
  Wire(Wire&& move);
  Wire& operator=(Wire const& copy) = delete;
  Wire& operator=(Wire&& move);

  ~Wire();
};

} } // namespace wtk::firealarm

#include <wtk/firealarm/Wire.t.h>

#endif//WTK_FIREALARM_WIRE_
