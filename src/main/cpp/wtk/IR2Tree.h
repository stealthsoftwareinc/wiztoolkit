/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_IR2_TREE_
#define WTK_IR2_TREE_

#include <cstddef>
#include <vector>

#include <wtk/index.h>

namespace wtk {

/**
 * A parent class for AST nodes. It just has file/lineno for error reporting.
 */
struct Node
{
  char const* const fileName = nullptr;
  size_t const lineNum = 0;

  Node(char const* const fn, size_t const ln) : fileName(fn), lineNum(ln) { }

  virtual ~Node() = default;
};

template<typename Number_T>
struct IndexExpr;

/**
 * AST node for expressions
 */
template<typename Number_T>
struct Expr : public Node
{
  enum
  {
    // basics
    literal,       // literal
    identifier,    // identifer

    // public operands
    pub_add,       // binary
    pub_sub,       // binary
    pub_mul,       // binary
    pub_div,       // binary
    pub_mod,       // binary
    to_integer,    // unary
    to_size,       // unary
    index,         // index

    // private operands
    gate_add,      // binary
    gate_mul,      // binary
    gate_and,      // binary
    gate_xor,      // binary
    gate_not,      // binary
    gate_wire,     // unary

    // input gates
    gate_instance, // unary
    gate_witness   // unary
  } const operation;

  union
  {
    Number_T const literal;

    char const* const identifier;

    struct
    {
      Expr<Number_T> const* const lhs;
      Expr<Number_T> const* const rhs;
    } binary;

    Expr<Number_T> const* const unary;

    IndexExpr<Number_T> const* const index;
  } expr;
};

template<typename Number_T>
struct IndexExpr : public Node
{
  struct Fragment : public Node
  {
    enum
    {
      index,
      range,
      transform,
      transform_range
    } type;

    struct
    {
      Expr<Number_T> const* const first;
      Expr<Number_T> const* const last;
      index_t const transform;
    } expr;
  };

  std::vector<Fragment> const fragments;
};

template<typename Number_T>
struct Assignment : public Node
{
  char const* const identifier;

  Expr<Number_T> const* const expression;
};

template<typename Number_T>
struct Directive;

template<typename Number_T>
struct ForLoop : public Node
{
  struct InBlock : public Node
  {
    char const* const iterator;
    Expr<Number_T> const* const expression;
    char const* const assignment;
  };

  std::vector<InBlock> const outputs;

  std::vector<InBlock> const inputs;

  char const* const iterator;

  std::vector<Directive<Number_T> const*> const body;
};

template<typename Number_T>
struct FunctionDeclaration : public Node
{
  struct Parameter : public Node
  {
    enum
    {
      size,
      field,
      integer,
      identifier
    } type;

    char const* const identifier;

    std::vector<Expr<Number_T> const*> const tensor;
  };

  std::vector<Parameter> const outputs;

  std::vector<Parameter> const inputs;

  std::vector<Directive<Number_T> const*> const body;
};

template<typename Number_T>
struct FunctionInvocation : public Node
{
  std::vector<char const*> const outputs;

  std::vector<Expr<Number_T> const*> const inputs;
};

template<typename Number_T>
struct Directive : public Node
{
  enum
  {
    assignment,
    invoke,
    for_loop
  } type;

  union
  {
    Assignment<Number_T> const* const assignment;

    FunctionInvocation<Number_T> const* const invoke;

    ForLoop<Number_T> const* const forLoop;
  };
};

template<typename Number_T>
struct FieldDeclaration : public Node
{
  char const* const identifier;

  Number_T const prime;

  enum
  {
    arithmetic,
    boolean
  } gateSet;
}; 

struct ConversionDeclaration : public Node
{
  char const* const outField;
  index_t const outSize;

  char const* const inField;
  index_t const inSize;
}; 

template<typename Number_T>
struct GlobalDirective : public Node
{
  enum
  {
    assignment,
    invoke,
    for_loop,
    function,
    field,
    converison
  } type;

  union
  {
    Assignment<Number_T> const* const assignment;

    FunctionInvocation<Number_T> const* const invoke;

    ForLoop<Number_T> const* const forLoop;

    FunctionDeclaration<Number_T> const* const function;

    FieldDeclaration<Number_T> const* const field;

    ConversionDeclaration const* const conversion;
  };
};

struct Include : public Node
{
  char const* const path;

  char const* const identifier;
};

template<typename Number_T>
struct Translation
{
  std::vector<Include> const includes;

  std::vector<GlobalDirective<Number_T>> const globalScope;
};

} // namespace wtk

#endif//WTK_IR2_TREE_
