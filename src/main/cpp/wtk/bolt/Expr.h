/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

#ifndef WTK_BOLT_POSTFIX_EXPR_H_
#define WTK_BOLT_POSTFIX_EXPR_H_

#include <cstddef>
#include <vector>
#include <unordered_map>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/utils/hints.h>
#include <wtk/utils/IRTreeUtils.h>

namespace wtk {
namespace bolt {

/**
 * This version of an expression is converted to postfix and iterator
 * name strings are replaced by iterator value pointers.
 */
struct Term
{
  wtk::index_t literal = 0;
  wtk::index_t const* iterator = nullptr;

  enum
  {
    LITERAL,
    ITERATOR,
    ADD,
    OFFSET,
    SUB,
    MUL,
    MULTIPLE,
    DIVC
  } type;
};

struct Expr
{
  std::vector<Term> terms;

  wtk::index_t eval(std::vector<wtk::index_t>& stack) const;
};

/**
 * The ExprBuilder is the simplest thing that is capable of building an Expr.
 * It's used by PLASMASnooze.
 */
struct ExprBuilder
{
  std::vector<std::string> names;   // name of iterators
  std::vector<wtk::index_t*> iterators; // pointers to "current" iteration

  // Find the index of the iterator.
  // when the iterator is not found, the return is greater than size.
  size_t findIterIdx(char const* const str) const;

  /**
   * Builds an expression. True is success.
   */
  bool build(wtk::IterExpr* const syn_expr, Expr* const tx_expr) const;

  // Push and pop iterators. Push can fail if the iterator is a duplicate.
  bool push(char const* const name, wtk::index_t* const iterator);
  virtual void pop();
};

/**
 * The IterBounds are responsible for managing the bounds of outer-scope
 * loop bounds and consequently for checking and building expressions in BOLT.
 */
struct IterBounds : public ExprBuilder
{
  // All of These are co-indexed with names and iterators
  std::vector<wtk::index_t> firsts; // first bounds
  std::vector<wtk::index_t> lasts;  // last bounds

  // Indicates which iterators are used since the last reset.
  // These are set by checkIterUsage(), and reset to false by reset().
  // They will be used during soft-unrolling.
  std::vector<bool> iterUsage;

  // used during shortcut processing to evaluate at first, second, last.
  std::unordered_map<std::string, wtk::index_t> shortcutTable;

  // Reset the soft unroll bits.
  // Also sets the iterators to their firsts, so first/second/last can be
  // evaluated in shortcut mode.
  void reset();

  // Add a new iterator
  bool push(char const* const name,
      wtk::index_t const first, wtk::index_t const last,
      wtk::index_t* const iterator);

  // Remove the top iterator (after processing its body).
  void pop() override;

  // Checks which iterators are used by an expression. Sets the iterUsage
  // flags.
  void checkIterUsage(wtk::IterExpr* const expr);

  // Evaluate the expression range at all bounds and check for overflow.
  // Also checks that each evaluation has an expected span.
  // Returns false if overflow occurs, or if the span is "negative", or
  // the span does not match the expected span.
  // Also sets a min and max value (guaranteed correct IFF expr is linear).
  bool evalBounds(wtk::IterExpr* const first, wtk::IterExpr* const last,
      wtk::index_t* const min, wtk::index_t* const max,
      wtk::index_t const expected_span, size_t const place = 0);
};

} } // namespace wtk::bolt

#endif//WTK_BOLT_POSTFIX_EXPR_H_
