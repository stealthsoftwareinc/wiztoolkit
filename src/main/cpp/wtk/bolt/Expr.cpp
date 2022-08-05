/**
 * Copyright (C) 2021-2022 Stealth Software Technologies, Inc.
 */

#include <cstring>

#include <wtk/bolt/Expr.h>

#define LOG_IDENTIFIER "wtk::bolt"
#include <stealth_logging.h>

namespace wtk {
namespace bolt {

wtk::index_t Expr::eval(std::vector<wtk::index_t>& stack) const
{
  wtk::index_t first;
  switch(this->terms[0].type)
  {
  case Term::LITERAL:
  {
    first = this->terms[0].literal;
    break;
  }
  case Term::ITERATOR:
  {
    first = *this->terms[0].iterator;
    break;
  }
  case Term::OFFSET:
  {
    first = this->terms[0].literal + *this->terms[0].iterator;
    break;
  }
  case Term::MULTIPLE:
  {
    first = this->terms[0].literal * *this->terms[0].iterator;
    break;
  }
  default:
  {
    // The first term may not pop, because there's 0 items to pop
    // and we've enumerated all the push types.
    log_fatal("unreachable");
  }
  }

  // short circuit the whole stack machine if there was just 1 term.
  if(this->terms.size() == 1)
  {
    return first;
  }
  else
  {
    if(UNLIKELY(this->terms.size() > stack.size()))
    {
      stack.resize(this->terms.size());
    }

    stack[0] = first;
    size_t place = 1;

    for(size_t i = 1; i < this->terms.size(); i++)
    {
      switch(this->terms[i].type)
      {
      case Term::LITERAL:
      {
        stack[place] = this->terms[i].literal;
        place++;

        break;
      }
      case Term::ITERATOR:
      {
        stack[place] = *this->terms[i].iterator;
        place++;

        break;
      }
      case Term::ADD:
      {
        place--;
        wtk::index_t b = stack[place];
        wtk::index_t a = stack[place - 1];

        stack[place - 1] = a + b;
        break;
      }
      case Term::OFFSET:
      {
        stack[place] = this->terms[i].literal + *this->terms[i].iterator;
        place++;

        break;
      }
      case Term::SUB:
      {
        place--;
        wtk::index_t b = stack[place];
        wtk::index_t a = stack[place - 1];

        stack[place - 1] = a - b;
        break;
      }
      case Term::MUL:
      {
        place--;
        wtk::index_t b = stack[place];
        wtk::index_t a = stack[place - 1];

        stack[place - 1] = a * b;
        break;
      }
      case Term::MULTIPLE:
      {
        stack[place] = this->terms[i].literal * *this->terms[i].iterator;
        place++;

        break;
      }
      case Term::DIVC:
      {
        wtk::index_t b = this->terms[i].literal;
        wtk::index_t a = stack[place - 1];

        stack[place - 1] = a / b;
        break;
      }
      }
    }

    wtk::index_t ret = stack[0];
    log_assert(place == 1);
    return ret;
  }
}

size_t ExprBuilder::findIterIdx(char const* const str) const
{
  for(size_t i = this->names.size(); i > 0; i--)
  {
    if(strcmp(str, this->names[i - 1].c_str()) == 0) { return i - 1; }
  }

  return this->names.size() + 1;
}

bool ExprBuilder::build(
    wtk::IterExpr* const syn_expr, Expr* const tx_expr) const
{
  switch(syn_expr->type())
  {
  case wtk::IterExpr::LITERAL:
  {
    tx_expr->terms.emplace_back();
    tx_expr->terms.back().type = Term::LITERAL;
    tx_expr->terms.back().literal = syn_expr->literal();
    return true;
  }
  case wtk::IterExpr::ITERATOR:
  {
    size_t idx = this->findIterIdx(syn_expr->name());
    if(idx >= this->names.size())
    {
      log_error("no such iterator: %s", syn_expr->name());
      return false;
    }

    tx_expr->terms.emplace_back();
    tx_expr->terms.back().type = Term::ITERATOR;
    tx_expr->terms.back().iterator = this->iterators[idx];
    return true;
  }
  case wtk::IterExpr::ADD:
  {
    if(syn_expr->lhs()->type() == wtk::IterExpr::LITERAL
        && syn_expr->rhs()->type() == wtk::IterExpr::ITERATOR)
    {
      size_t idx = this->findIterIdx(syn_expr->rhs()->name());
      if(idx >= this->names.size())
      {
        log_error("no such iterator: %s", syn_expr->rhs()->name());
        return false;
      }

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::OFFSET;
      tx_expr->terms.back().iterator = this->iterators[idx];
      tx_expr->terms.back().literal = syn_expr->lhs()->literal();

      return true;
    }
    else if(syn_expr->lhs()->type() == wtk::IterExpr::ITERATOR
        && syn_expr->rhs()->type() == wtk::IterExpr::LITERAL)
    {
      size_t idx = this->findIterIdx(syn_expr->lhs()->name());
      if(idx >= this->names.size())
      {
        log_error("no such iterator: %s", syn_expr->lhs()->name());
        return false;
      }

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::OFFSET;
      tx_expr->terms.back().iterator = this->iterators[idx];
      tx_expr->terms.back().literal = syn_expr->rhs()->literal();

      return true;
    }
    else
    {
      bool success = this->build(syn_expr->lhs(), tx_expr)
        && this->build(syn_expr->rhs(), tx_expr);

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::ADD;

      return success;
    }
  }
  case wtk::IterExpr::SUB:
  {
    bool success = this->build(syn_expr->lhs(), tx_expr)
      && this->build(syn_expr->rhs(), tx_expr);

    tx_expr->terms.emplace_back();
    tx_expr->terms.back().type = Term::SUB;

    return success;
  }
  case wtk::IterExpr::MUL:
  {
    if(syn_expr->lhs()->type() == wtk::IterExpr::LITERAL
        && syn_expr->rhs()->type() == wtk::IterExpr::ITERATOR)
    {
      size_t idx = this->findIterIdx(syn_expr->rhs()->name());
      if(idx >= this->names.size())
      {
        log_error("no such iterator: %s", syn_expr->rhs()->name());
        return false;
      }

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::MULTIPLE;
      tx_expr->terms.back().iterator = this->iterators[idx];
      tx_expr->terms.back().literal = syn_expr->lhs()->literal();

      return true;
    }
    else if(syn_expr->lhs()->type() == wtk::IterExpr::ITERATOR
        && syn_expr->rhs()->type() == wtk::IterExpr::LITERAL)
    {
      size_t idx = this->findIterIdx(syn_expr->lhs()->name());
      if(idx >= this->names.size())
      {
        log_error("no such iterator: %s", syn_expr->lhs()->name());
        return false;
      }

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::MULTIPLE;
      tx_expr->terms.back().iterator = this->iterators[idx];
      tx_expr->terms.back().literal = syn_expr->rhs()->literal();

      return true;
    }
    else
    {
      bool success = this->build(syn_expr->lhs(), tx_expr)
        && this->build(syn_expr->rhs(), tx_expr);

      tx_expr->terms.emplace_back();
      tx_expr->terms.back().type = Term::MUL;

      return success;
    }
  }
  case wtk::IterExpr::DIV:
  {
    bool success = this->build(syn_expr->lhs(), tx_expr);

    tx_expr->terms.emplace_back();
    tx_expr->terms.back().type = Term::DIVC;
    tx_expr->terms.back().literal = syn_expr->literal();

    return success;
  }
  }

  log_error("unreachable case");
  return false;
}

bool ExprBuilder::push(
    char const* const name, wtk::index_t* const iterator)
{
  size_t idx = this->findIterIdx(name);
  if(idx < this->names.size())
  {
    log_error("duplicate iterator: %s", name);
    return false;
  }

  this->names.emplace_back(name);
  this->iterators.push_back(iterator);

  return true;
}

void ExprBuilder::pop()
{
  this->names.pop_back();
  this->iterators.pop_back();
}

void IterBounds::reset()
{
  for(size_t i = 0; i < this->iterUsage.size(); i++)
  {
    this->iterUsage[i] = false;
    *this->iterators[i] = this->firsts[i];
  }
}

bool IterBounds::push(char const* const name,
    wtk::index_t const first, wtk::index_t const last,
    wtk::index_t* const iterator)
{
  if(!this->ExprBuilder::push(name, iterator)) { return false; }

  this->firsts.push_back(first);
  this->lasts.push_back(last);
  this->iterUsage.push_back(false);

  return true;
}

void IterBounds::pop()
{
  this->ExprBuilder::pop();
  this->firsts.pop_back();
  this->lasts.pop_back();
  this->iterUsage.pop_back();
}

void IterBounds::checkIterUsage(wtk::IterExpr* const expr)
{
  log_assert(expr != nullptr);

  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    return;
  }
  case IterExpr::ITERATOR:
  {
    size_t idx = this->findIterIdx(expr->name());
    if(idx < this->names.size())
    {
      this->iterUsage[idx] = true;
    }
    return;
  }
  case IterExpr::ADD: /* fallthrough */
  case IterExpr::SUB: /* fallthrough */
  case IterExpr::MUL:
  {
    this->checkIterUsage(expr->lhs());
    this->checkIterUsage(expr->rhs());
    return;
  }
  case IterExpr::DIV:
  {
    this->checkIterUsage(expr->lhs());
    return;
  }
  }
}

bool IterBounds::evalBounds(
    wtk::IterExpr* const first, wtk::IterExpr* const last,
    wtk::index_t* const min, wtk::index_t* const max,
    wtk::index_t const expected_span, size_t const place)
{
  if(place == this->names.size())
  {
    wtk::index_t first_val;
    bool ret = !wtk::utils::iterExprEvalOverflow(
        &first_val, first, this->shortcutTable);
    wtk::index_t last_val;
    ret = !wtk::utils::iterExprEvalOverflow(
        &last_val, last, this->shortcutTable) && ret;

    ret = first_val <= last_val && ret;
    ret = 1 + last_val - first_val  == expected_span && ret;

    if(first_val < *min) { *min = first_val; }
    if(last_val > *max) { *max = last_val; }

    return ret;
  }
  else
  {
    this->shortcutTable[this->names[place]] = this->firsts[place];
    bool ret =
      this->evalBounds(first, last, min, max, expected_span, place + 1);
    this->shortcutTable[this->names[place]] = this->lasts[place];
    return
      this->evalBounds(first, last, min, max, expected_span, place + 1) && ret;
  }
}

} } // namespace wtk::bolt
