/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#include <wtk/nails/Scope.h>

#define LOG_IDENTIFIER "wtk::nails"
#include <stealth_logging.h>

namespace wtk {
namespace nails {

char const* scopeErrorString(ScopeError err)
{
  switch(err)
  {
  case ScopeError::success:
    return "success.";
  case ScopeError:: outOfMem:
    return "Out of memory.";
  case ScopeError::alreadyExists:
    return "Wire already exists.";
  case ScopeError::cannotDeleteRemap:
    return "Cannot delete a remapped range.";
  case ScopeError::unmatchedDelete:
    return "Cannot delete a range which overlaps but does not equal a contiguous range";
  case ScopeError::notAssigned:
    return "Wire is not assigned.";
  case ScopeError::deleted:
    return "Wire was already deleted.";
  case ScopeError::discontiguous:
    return "Range is discontiguous.";
  }

  log_fatal("unreachable");
}

} } // namespace wtk::nails
