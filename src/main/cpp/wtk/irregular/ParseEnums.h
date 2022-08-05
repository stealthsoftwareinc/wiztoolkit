/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_PARSE_ENUMS_H_
#define WTK_IRREGULAR_PARSE_ENUMS_H_

namespace wtk {
namespace irregular {

/*****************
 * Common Tokens *
 *****************/

enum class NumericBase
{
  dec,
  hex,
  bin,
  oct,
  invalid
};

/***********************
 * Header/Front Matter *
 ***********************/

enum class GateSetFirst
{
  arithmetic = 0,
  boolean,
  add = 16,
  addc = 17,
  mul = 18,
  mulc = 19,
  and_ = 32,
  xor_ = 33,
  not_ = 34,
  invalid
};

enum class CommaOrSemiColon
{
  comma,
  semiColon,
  invalid
};

enum class GateSetArith
{
  add = 16, // same numbering as in GateSetFirst
  addc = 17,
  mul = 18,
  mulc = 19,
  invalid
};

enum class GateSetBool
{
  and_ = 32, // same numbering as in GateSetFirst
  xor_ = 33,
  not_ = 34,
  invalid
};

enum class FeatureFirst
{
  simple = 0,
  function = 1,
  for_loop = 2,
  switch_case = 3,
  invalid
};

enum class FeatureRest
{
  function = 1, // same numbering as FeatureFirst
  for_loop = 2,
  switch_case = 3,
  invalid
};

/***************************
 * Streaming API Specifics *
 ***************************/

enum class StreamingDirectiveTypes
{
  gate,
  assert_zero,
  delete_,
  end,
  invalid
};

enum class ArithStreamingDirectives
{
  add = 0,
  mul,
  addc,
  mulc,
  witness,
  instance,
  assign,
  copy,
  invalid
};

enum class BoolStreamingDirectives
{
  xor_,
  and_,
  not_,
  witness,
  instance,
  assign,
  copy,
  invalid
};

enum class CommaOrRightParen
{
  comma = 0,
  rightParen,
  invalid
};

enum class LiteralOrEnd
{
  literal,
  end,
  invalid
};

/**********************
 * Tree API Specifics *
 **********************/

enum class TreeDirectiveGlobalStart
{
  wireNumber,
  assertZero,
  delete_,
  call,
  anonCall,
  forLoop,
  switchStatement,
  end,
  function,
  invalid
};

enum class TreeDirectiveStart
{
  wireNumber, // same as TreeDirectiveGlobalStart, without "function"
  assertZero,
  delete_,
  call,
  anonCall,
  forLoop,
  switchStatement,
  end,
  invalid
};

enum class SingleOutputDirective
{
  add,
  mul,
  and_,
  xor_,
  addc,
  mulc,
  not_,
  witness,
  instance,
  assign,
  copy,
  call,
  anonCall,
  forLoop,
  switchStatement,
  invalid
};

enum class WireListSeparators
{
  end,
  list,
  range,
  invalid
};

enum class WireNumberOrInstance
{
  wireNumber,
  instance,
  invalid
};

enum class WireNumberOrFunction
{
  wireNumber,
  call,
  anonCall,
  invalid
};

enum class IterExprBase
{
  dec = 0,
  hex,
  bin,
  oct,
  name,
  expr,
  invalid
};

enum class IterExprOp
{
  add,
  sub,
  mul,
  div,
  invalid
};

enum class FunctionType
{
  call,
  anonCall,
  invalid
};

enum class MultiOutputDirective
{
  call,
  anonCall,
  forLoop,
  switchStatement,
  invalid
};

enum class CaseOrEnd
{
  case_,
  end,
  invalid
};

} } // namespace wtk::irregular

#endif // WTK_IRREGULAR_PARSE_ENUMS_H_
