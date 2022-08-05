/**
 * Copyright 2021, Stealth Software Technologies, Inc.
 */

#include <cstdlib>
#include <cstring>
#include <random>
#include <chrono>
#include <unordered_map>

#include <gtest/gtest.h>

#include <wtk/utils/IRTreeUtils.h>
#include <wtk/irregular/TextIRTree.t.h>

TEST(IRTreeUtils, iterExprOverflow_Add)
{
  auto rand = std::bind(
      std::uniform_int_distribution<wtk::index_t>(0, (1ULL << 63) - 1),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));

  wtk::irregular::TextIterExpr expr, expr_lhs, expr_rhs;
  expr.type_ = wtk::IterExpr::ADD;
  expr.expr.leftHand = &expr_lhs;
  expr.expr.rightHand = &expr_rhs;
  expr_lhs.type_ = wtk::IterExpr::ITERATOR;
  expr_lhs.name_ = strdup("iter_a");
  expr_rhs.type_ = wtk::IterExpr::ITERATOR;
  expr_rhs.name_ = strdup("iter_b");

  std::unordered_map<std::string, wtk::index_t> table;
  std::string iter_a_str("iter_a");
  std::string iter_b_str("iter_b");

  for(size_t i = 0; i < 1000; i++)
  {
    wtk::index_t const rand_a = rand();
    wtk::index_t const rand_b = rand();

    table[iter_a_str] = rand_a;
    table[iter_b_str] = rand_b;

    wtk::index_t actual_no_overflow;
    EXPECT_FALSE(
        wtk::utils::iterExprEvalOverflow(&actual_no_overflow, &expr, table));

    EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_no_overflow);

    table[iter_a_str] = rand_a + (1ULL << 63);
    table[iter_b_str] = rand_b + (1ULL << 63);

    wtk::index_t actual_yes_overflow;
    EXPECT_TRUE(
        wtk::utils::iterExprEvalOverflow(&actual_yes_overflow, &expr, table));

    EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_yes_overflow);
  }

  free(expr_lhs.name_);
  free(expr_rhs.name_);
}

TEST(IRTreeUtils, iterExprOverflow_Sub)
{
  auto rand = std::bind(
      std::uniform_int_distribution<wtk::index_t>(0, (1ULL << 63) - 1),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));

  wtk::irregular::TextIterExpr expr, expr_lhs, expr_rhs;
  expr.type_ = wtk::IterExpr::SUB;
  expr.expr.leftHand = &expr_lhs;
  expr.expr.rightHand = &expr_rhs;
  expr_lhs.type_ = wtk::IterExpr::ITERATOR;
  expr_lhs.name_ = strdup("iter_a");
  expr_rhs.type_ = wtk::IterExpr::ITERATOR;
  expr_rhs.name_ = strdup("iter_b");

  std::unordered_map<std::string, wtk::index_t> table;
  std::string iter_a_str("iter_a");
  std::string iter_b_str("iter_b");

  for(size_t i = 0; i < 2000; i++)
  {
    wtk::index_t const rand_a = rand();
    wtk::index_t const rand_b = rand();

    table[iter_a_str] = rand_a;
    table[iter_b_str] = rand_b;

    if(rand_a >= rand_b)
    {
      wtk::index_t actual_no_overflow;
      EXPECT_FALSE(wtk::utils::iterExprEvalOverflow(
            &actual_no_overflow, &expr, table));

      EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_no_overflow);
    }
    else
    {
      wtk::index_t actual_yes_overflow;
      EXPECT_TRUE(wtk::utils::iterExprEvalOverflow(
            &actual_yes_overflow, &expr, table));

      EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_yes_overflow);
    }
  }

  free(expr_lhs.name_);
  free(expr_rhs.name_);
}

TEST(IRTreeUtils, iterExprOverflow_Mul)
{
  auto rand = std::bind(
      std::uniform_int_distribution<wtk::index_t>(0, (1ULL << 32) - 1),
      std::default_random_engine(std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now())));

  wtk::irregular::TextIterExpr expr, expr_lhs, expr_rhs;
  expr.type_ = wtk::IterExpr::MUL;
  expr.expr.leftHand = &expr_lhs;
  expr.expr.rightHand = &expr_rhs;
  expr_lhs.type_ = wtk::IterExpr::ITERATOR;
  expr_lhs.name_ = strdup("iter_a");
  expr_rhs.type_ = wtk::IterExpr::ITERATOR;
  expr_rhs.name_ = strdup("iter_b");

  std::unordered_map<std::string, wtk::index_t> table;
  std::string iter_a_str("iter_a");
  std::string iter_b_str("iter_b");

  for(size_t i = 0; i < 1000; i++)
  {
    wtk::index_t const rand_a = rand();
    wtk::index_t const rand_b = rand();

    table[iter_a_str] = rand_a;
    table[iter_b_str] = rand_b;

    wtk::index_t actual_no_overflow;
    EXPECT_FALSE(
        wtk::utils::iterExprEvalOverflow(&actual_no_overflow, &expr, table));

    EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_no_overflow);

    table[iter_a_str] = rand_a + (1ULL << 32);
    table[iter_b_str] = rand_b + (1ULL << 32);

    wtk::index_t actual_yes_overflow;
    EXPECT_TRUE(
        wtk::utils::iterExprEvalOverflow(&actual_yes_overflow, &expr, table));

    EXPECT_EQ(wtk::utils::iterExprEval(&expr, table), actual_yes_overflow);
  }

  free(expr_lhs.name_);
  free(expr_rhs.name_);
}
