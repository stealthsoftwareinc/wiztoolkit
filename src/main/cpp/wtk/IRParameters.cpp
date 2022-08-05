/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/IRParameters.h>

namespace wtk {

GateSet::GateSet()
{
  this->gateSet = GateSet::invalid;
  this->enableAdd = true;
  this->enableAddC = true;
  this->enableMul = true;
  this->enableMulC = true;
}

bool GateSet::cannonical() const
{
  if(this->gateSet == invalid) { return false; }
  else if(this->gateSet == arithmetic)
  {
    return this->enableAdd && this->enableAddC
      && this->enableMul && this->enableMulC;
  }
  else
  {
    return this->enableXor && this->enableAnd && this->enableNot;
  }
}

bool FeatureToggles::simple() const
{
  return !this->functionToggle && !this->forLoopToggle
    && !this->switchCaseToggle;
}

bool FeatureToggles::complete() const
{
  return this->functionToggle && this->forLoopToggle && this->switchCaseToggle;
}

} // namespace wtk
