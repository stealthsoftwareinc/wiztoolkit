/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IR_PARAMETERS_H_
#define WTK_IR_PARAMETERS_H_

namespace wtk {

/**
 * An enum describing which resource type is presented.
 */
enum class Resource
{
  relation,
  instance,
  shortWitness,
  invalid
};

/**
 * The GateSet describes which gates are allowed to be used within the
 * following relation.
 */
struct GateSet
{
  /**
   * enumeration of what variety the gateset takes.
   */
  enum
  {
    arithmetic,
    boolean,
    invalid
  } gateSet = invalid;

  /**
   * Toggles for individual gate types.
   */
  union
  {
    struct
    {
      bool enableAdd;
      bool enableAddC;
      bool enableMul;
      bool enableMulC;
    };
    struct
    {
      bool enableXor;
      bool enableAnd;
      bool enableNot;
    };
  };

  /**
   * Indicates if the gateset is one of the cannonical gatesets.
   */
  bool cannonical() const;
  GateSet();

};

/**
 * The feature toggles indicate which structural features are enabled
 * within the following relation.
 */
struct FeatureToggles
{
  bool functionToggle = false;
  bool forLoopToggle = false;
  bool switchCaseToggle = false;

  /**
   * A simple FeatureToggles has no additional features.
   */
  bool simple() const;

  /**
   * A complete FeatureToggles has all features enabled.
   */
  bool complete() const;
};

} // namespace wtk

#endif//WTK_IR_PARAMETERS_H_
