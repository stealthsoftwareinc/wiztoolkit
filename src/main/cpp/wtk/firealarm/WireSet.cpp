/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/firealarm/WireSet.h>

namespace wtk {
namespace firealarm {

char const* wireSetFailString(WireSetFail fail)
{
  switch(fail)
  {
  case WireSetFail::success:
    return "Success";
  case WireSetFail::deleted:
    return "Wire previously deleted";
  case WireSetFail::assigned:
    return "Wire previously assigned";
  case WireSetFail::not_assigned:
    return "Wire not yet assigned";
  case WireSetFail::null_deref:
    return "Operation would dereference null";
  case WireSetFail::cant_delete:
    return "Cannot delete input or output wires";
  case WireSetFail::invalid_range:
    return "Range is invalid";
  case WireSetFail::nonempty_subscope:
    return "Wires may only be mapped into an empty sub-scope";
  }

  return "Unknown";
}

} } // namespace wtk::firealarm;
