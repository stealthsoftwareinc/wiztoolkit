/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <cstring>

#include <wtk/utils/IRTreeUtils.h>

#define LOG_IDENTIFIER "utils"
#include <stealth_logging.h>

namespace wtk {
namespace utils {

wtk::index_t countWireList(wtk::WireList* list)
{
  wtk::index_t num_wires = 0;
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::WireList::SINGLE:
    {
      num_wires++;
      break;
    }
    case wtk::WireList::RANGE:
    {
      // undefined behavior what happens if range.first > range.last.
      if(list->range(i)->first() > list->range(i)->last())
      {
        log_warn("Range element where first > last is undefined behavior");
      }

      num_wires += 1 + list->range(i)->last() - list->range(i)->first();
    }
    }
  }

  return num_wires;
}

wtk::index_t iterExprEval(
    wtk::IterExpr* expr, std::unordered_map<std::string, wtk::index_t>& iters)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    return expr->literal();
  }
  case IterExpr::ITERATOR:
  {
    std::string n(expr->name());
    return iters[n];
  }
  case IterExpr::ADD:
  {
    index_t left = iterExprEval(expr->lhs(), iters);
    index_t right = iterExprEval(expr->rhs(), iters);
    return left + right;
  }
  case IterExpr::SUB:
  {
    index_t left = iterExprEval(expr->lhs(), iters);
    index_t right = iterExprEval(expr->rhs(), iters);
    return left - right;
  }
  case IterExpr::MUL:
  {
    index_t left = iterExprEval(expr->lhs(), iters);
    index_t right = iterExprEval(expr->rhs(), iters);
    return left * right;
  }
  case IterExpr::DIV:
  {
    index_t left = iterExprEval(expr->lhs(), iters);
    index_t right = expr->literal();
    return left / right;
  }
  }

  return 0;
}

bool iterExprEvalOverflow(wtk::index_t* ret,
    wtk::IterExpr* expr, std::unordered_map<std::string, wtk::index_t>& iters)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    *ret = expr->literal();
    return false;
  }
  case IterExpr::ITERATOR:
  {
    std::string n(expr->name());
    *ret = iters[n];
    return false;
  }
  case IterExpr::ADD:
  {
    index_t left;
    bool overflow = iterExprEvalOverflow(&left, expr->lhs(), iters);
    index_t right;
    overflow = iterExprEvalOverflow(&right, expr->rhs(), iters) && overflow;

    index_t tmp1 = (left & UINT32_MAX) + (right & UINT32_MAX);
    index_t tmp2 = (left >> 32) + (right >> 32) + (tmp1 >> 32);
    *ret = (tmp1 & UINT32_MAX) + ((tmp2 & UINT32_MAX) << 32);

    return 0 != (tmp2 >> 32) || overflow;
  }
  case IterExpr::SUB:
  {
    index_t left;
    bool overflow = iterExprEvalOverflow(&left, expr->lhs(), iters);
    index_t right;
    overflow = iterExprEvalOverflow(&right, expr->rhs(), iters) && overflow;

    index_t tmp1 = (left & UINT32_MAX) + (~right & UINT32_MAX) + 1;
    index_t tmp2 = (left >> 32) + (~right >> 32) + (tmp1 >> 32);
    *ret = (tmp1 & UINT32_MAX) + ((tmp2 & UINT32_MAX) << 32);

    return 0 == (tmp2 >> 32) || overflow;
  }
  case IterExpr::MUL:
  {
    index_t left;
    bool overflow = iterExprEvalOverflow(&left, expr->lhs(), iters);
    index_t right;
    overflow = iterExprEvalOverflow(&right, expr->rhs(), iters) && overflow;

    index_t tmp0 = (left & UINT32_MAX) * (right & UINT32_MAX);
    index_t tmp1 = (left & UINT32_MAX) * (right >> 32);
    index_t tmp2 = (left >> 32) * (right & UINT32_MAX);
    index_t tmp3 = (left >> 32) * (right >> 32);

    *ret = tmp0 + ((tmp1 & UINT32_MAX) << 32) + ((tmp2 & UINT32_MAX) << 32);

    return 0 != ((tmp1 >> 32) + (tmp2 >> 32) + tmp3) || overflow;
  }
  case IterExpr::DIV:
  {
    index_t left;
    bool overflow = iterExprEvalOverflow(&left, expr->lhs(), iters);
    index_t right = expr->literal();
    *ret = left / right;
    // Pretty sure its impossible for unsigned division to over or underflow
    return overflow;
  }
  }

  return false;
}

bool iterExprConstant(wtk::IterExpr* expr)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    return true;
  }
  case IterExpr::ITERATOR:
  {
    return false;
  }
  case IterExpr::ADD: /* Fallthrough */
  case IterExpr::SUB: /* Fallthrough */
  case IterExpr::MUL:
  {
    return iterExprConstant(expr->lhs()) && iterExprConstant(expr->rhs());
  }
  case IterExpr::DIV:
  {
    return iterExprConstant(expr->lhs());
  }
  }

  log_error("missing case");
  return false;
}

bool iterExprLinear(wtk::IterExpr* expr)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL: /* Fallthrough */
  case IterExpr::ITERATOR:
  {
    return true;
  }
  case IterExpr::ADD: /* Fallthrough */
  case IterExpr::SUB:
  {
    return iterExprLinear(expr->lhs()) && iterExprLinear(expr->rhs());
  }
  case IterExpr::MUL:
  {
    return (iterExprLinear(expr->lhs()) && iterExprConstant(expr->rhs()))
      || (iterExprConstant(expr->lhs()) && iterExprLinear(expr->rhs()));
  }
  case IterExpr::DIV:
  {
    return false;
  }
  }

  log_error("missing case");
  return false;
}

bool iterExprSoleDependence(wtk::IterExpr* expr, char const* iter)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    return true;
  }
  case IterExpr::ITERATOR:
  {
    return strcmp(expr->name(), iter) == 0;
  }
  case IterExpr::ADD: /* Fallthrough */
  case IterExpr::SUB: /* Fallthrough */
  case IterExpr::MUL:
  {
    return iterExprSoleDependence(expr->lhs(), iter)
      && iterExprSoleDependence(expr->rhs(), iter);
  }
  case IterExpr::DIV:
  {
    return iterExprSoleDependence(expr->lhs(), iter);
  }
  }

  log_error("missing case");
  return false;
}

} } // namespace wtk::utils
