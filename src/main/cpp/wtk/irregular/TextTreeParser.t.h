/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_TEXT_IR1_PARSER_H_
#define WTK_IRREGULAR_TEXT_IR1_PARSER_H_

#include <cstddef>
#include <memory>
#include <vector>

#include <wtk/IRParameters.h>
#include <wtk/utils/Pool.h>

#include <wtk/irregular/Automatas.h>
#include <wtk/irregular/AutomataCtx.h>

#include <wtk/irregular/TextIRTree.t.h>
#include<wtk/irregular/checkMacros.t.h>

#include <stealth_logging.h>

namespace wtk {
namespace irregular {

template<typename Number_T>
struct TextTreeParser
{
  AutomataCtx& ctx;
  wtk::GateSet* gateSet;
  wtk::FeatureToggles* features;

  TextTreeParser(AutomataCtx& c, wtk::GateSet* gs, wtk::FeatureToggles* ft)
    : ctx(c), gateSet(gs), features(ft) { }

  // The top level IR Tree
  TextIRTree<Number_T> irTree;

  // Pool allocation for lower level tree nodes.
  wtk::utils::Pool<TextBinaryGate> binaryGatePool;
  wtk::utils::Pool<TextBinaryConstGate<Number_T>> binaryConstGatePool;
  wtk::utils::Pool<TextUnaryGate> unaryGatePool;
  wtk::utils::Pool<TextInput> inputPool;
  wtk::utils::Pool<TextAssign<Number_T>> assignPool;
  wtk::utils::Pool<TextTerminal> terminalPool;

  wtk::utils::Pool<TextWireRange> wireRangePool;
  wtk::utils::Pool<TextWireList> wireListPool;
  wtk::utils::Pool<char> identifierPool;

  wtk::utils::Pool<TextFunctionDeclare<Number_T>> functionDeclarePool;
  wtk::utils::Pool<TextFunctionInvoke> functionInvokePool;
  wtk::utils::Pool<TextAnonFunction<Number_T>> anonFunctionPool;
  wtk::utils::Pool<TextForLoop<Number_T>> forLoopPool;

  wtk::utils::Pool<TextIterExpr> iterExprPool;
  wtk::utils::Pool<TextIterExprWireRange> iterExprWireRangePool;
  wtk::utils::Pool<TextIterExprWireList> iterExprWireListPool;
  wtk::utils::Pool<TextIterExprFunctionInvoke> iterExprFunctionInvokePool;
  wtk::utils::Pool<TextIterExprAnonFunction<Number_T>>
    iterExprAnonFunctionPool;

  wtk::utils::Pool<TextCaseFunctionInvoke> caseFunctionInvokePool;
  wtk::utils::Pool<TextCaseAnonFunction<Number_T>> caseAnonFunctionPool;
  wtk::utils::Pool<TextCaseBlock<Number_T>> caseBlockPool;
  wtk::utils::Pool<TextSwitchStatement<Number_T>> switchStatementPool;

  wtk::utils::Pool<TextDirectiveList<Number_T>> directiveListPool;

  /**
   * Parses a function invoke directive. Input is the output list. 
   *
   * Expects the "@call" to be parsed, but not whitespace.
   */
  TextFunctionInvoke* parseFunctionInvoke(TextWireList* out_list)
  {
    BEGIN WSPACE_AROUND(leftParen) END_NULL;
    BEGIN CHECK(identifier); END_NULL_SEQ;

    size_t len = this->ctx.place - this->ctx.mark;
    char* name = this->identifierPool.allocate(len + 1);
    strncpy(name, &this->ctx.buffer[this->ctx.mark], len);
    name[len] = '\0';

    TextWireList* in_list = this->wireListPool.allocate();

    BEGIN WSPACE END_NULL_SEQ;
    WireListSeparators sep = wireListSeparatorsC(&this->ctx);

    while(sep != WireListSeparators::end)
    {
      BEGIN
        CHECK_EXPR(sep != WireListSeparators::invalid)
        WSPACE
        CHECK(wireNumberBegin)
      END_NULL;

      wtk::index_t first_wire;
      NUMBER_NULL(first_wire);

      BEGIN WSPACE END_NULL_SEQ;
      sep = wireListSeparatorsD(&this->ctx);
      switch(sep)
      {
      case WireListSeparators::list: /* fallthrough */
      case WireListSeparators::end:
      {
        in_list->types.push_back(wtk::WireList::SINGLE);
        in_list->elements.emplace_back();
        in_list->elements.back().single = first_wire;
        break;
      }
      case WireListSeparators::range:
      {
        BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;

        wtk::index_t last_wire;
        NUMBER_NULL(last_wire);

        TextWireRange* range = this->wireRangePool.allocate();
        range->firstIdx = first_wire;
        range->lastIdx = last_wire;

        in_list->types.push_back(wtk::WireList::RANGE);
        in_list->elements.emplace_back();
        in_list->elements.back().range = range;

        BEGIN WSPACE END_NULL_SEQ;
        sep = wireListSeparatorsC(&this->ctx);
        break;
      }
      case WireListSeparators::invalid:
      {
        return nullptr;
      }
      }
    }

    TextFunctionInvoke* invoke = this->functionInvokePool.allocate();
    invoke->name_ = name;
    invoke->outList = out_list;
    invoke->inList = in_list;

    return invoke;
  }

  /**
   * Parses an anon function directive. Input is the output list. 
   *
   * Expects the "@anon_call" to be parsed, but not whitespace.
   */
  TextAnonFunction<Number_T>* parseAnonFunction(TextWireList* out_list)
  {
    BEGIN WSPACE_AROUND(leftParen) END_NULL_SEQ;

    TextWireList* in_list = this->wireListPool.allocate();

    WireNumberOrInstance num_or_ins = wireNumberOrInstance(&this->ctx);

    while(num_or_ins != WireNumberOrInstance::instance)
    {
      if(num_or_ins == WireNumberOrInstance::wireNumber)
      {
        this->ctx.updateMark();
        wtk::index_t first_wire;
        NUMBER_NULL(first_wire);

        BEGIN WSPACE END_NULL_SEQ;
        WireListSeparators sep = wireListSeparatorsE(&this->ctx);

        switch(sep)
        {
        case WireListSeparators::list:
        {
          in_list->types.push_back(wtk::WireList::SINGLE);
          in_list->elements.emplace_back();
          in_list->elements.back().single = first_wire;
          break;
        }
        case WireListSeparators::range:
        {
          BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;

          wtk::index_t last_wire;
          NUMBER_NULL(last_wire);

          TextWireRange* range = this->wireRangePool.allocate();
          range->firstIdx = first_wire;
          range->lastIdx = last_wire;

          in_list->types.push_back(wtk::WireList::RANGE);
          in_list->elements.emplace_back();
          in_list->elements.back().range = range;

          BEGIN WSPACE CHECK(comma) END_NULL_SEQ;
          break;
        }
        default: { return nullptr; }
        }

        BEGIN WSPACE END_NULL_SEQ;
        num_or_ins = wireNumberOrInstance(&this->ctx);
      }
      else // invalid
      {
        return nullptr;
      }
    }

    BEGIN WSPACE_AROUND(colon) END_NULL;

    wtk::index_t num_ins;
    NUMBER_NULL(num_ins);

    BEGIN
      WSPACE_AROUND(comma)
      WSPACE_AFTER(shortWitnessKw)
      WSPACE_AFTER(colon)
    END_NULL;

    wtk::index_t num_wit;
    NUMBER_NULL(num_wit);

    BEGIN WSPACE_AROUND(rightParen) END_NULL_SEQ;
    TextDirectiveList<Number_T>* body = this->parseDirectiveList();

    TextAnonFunction<Number_T>* anon = this->anonFunctionPool.allocate();
    anon->outList = out_list;
    anon->inList = in_list;
    anon->insCount = num_ins;
    anon->witCount = num_wit;
    anon->directives = body;

    return anon;
  }

  /**
   * Assume no leading whitespace. Trailing whitespace is not consumed. parse
   * and return an index expression return nullptr if none found.
   */
  TextIterExpr* parseIterExpr()
  {
    TextIterExpr* ret = this->iterExprPool.allocate();
    IterExprBase base = iterExprBase(&this->ctx);

    if(base == IterExprBase::expr)
    {
      BEGIN WSPACE END_NULL;
      TextIterExpr* lhs = this->parseIterExpr();

      BEGIN WSPACE END_NULL_SEQ;
      IterExprOp op = iterExprOp(&this->ctx);

      switch(op)
      {
      case IterExprOp::add: ret->type_ = wtk::IterExpr::ADD; break;
      case IterExprOp::sub: ret->type_ = wtk::IterExpr::SUB; break;
      case IterExprOp::mul: ret->type_ = wtk::IterExpr::MUL; break;
      case IterExprOp::div: ret->type_ = wtk::IterExpr::DIV; break;
      default:
        return nullptr;
      }

      if(op == IterExprOp::div)
      {
        BEGIN WSPACE END_NULL;
        wtk::index_t denom;
        NUMBER_NULL(denom);

        ret->div.leftHand = lhs;
        ret->div.literalValue = denom;
      }
      else
      {
        BEGIN WSPACE END_NULL;
        TextIterExpr* rhs = this->parseIterExpr();

        ret->expr.leftHand = lhs;
        ret->expr.rightHand = rhs;
      }

      BEGIN WSPACE CHECK(rightParen) END_NULL;
    }
    else if(base == IterExprBase::name)
    {
      size_t len = this->ctx.place - this->ctx.mark;
      ret->name_ = this->identifierPool.allocate(len + 1);
      strncpy(ret->name_, &this->ctx.buffer[this->ctx.mark], len);
      ret->name_[len] = '\0';
      ret->type_ = wtk::IterExpr::ITERATOR;
    }
    else // numeric literal.
    {
      BEGIN CHECK_EXPR(to_uint(base, ret->literalValue, &this->ctx)) END_NULL;
      ret->type_ = wtk::IterExpr::LITERAL;
    }

    return ret;
  }

  /**
   * input is a TextForLoop with a valid output range, everything else
   * is unassigned.
   * the '@for' just finished, this will eat whitespace, the loop controls,
   * the body, and then finish.
   */
  TextForLoop<Number_T>* parseForLoop(TextWireList* out_list)
  {
    BEGIN WSPACE END_NULL;
    BEGIN CHECK(identifier) END_NULL_SEQ;

    size_t len = this->ctx.place - this->ctx.mark;
    char* iter_name = this->identifierPool.allocate(len + 1);
    strncpy(iter_name, &this->ctx.buffer[this->ctx.mark], len);
    iter_name[len] = '\0';

    BEGIN WSPACE_AROUND(firstKw) END_NULL;
    wtk::index_t bound_first;
    NUMBER_NULL(bound_first);

    BEGIN WSPACE_AROUND(lastKw) END_NULL;
    wtk::index_t bound_last;
    NUMBER_NULL(bound_last);

    BEGIN WSPACE END_NULL_SEQ;
    WireNumberOrFunction num_or_func = wireNumberOrFunction(&this->ctx);
    FunctionType func_type;
    TextIterExprWireList* iter_out_list =
      this->iterExprWireListPool.allocate();

    switch(num_or_func)
    {
    case WireNumberOrFunction::wireNumber:
    {
      WireListSeparators sep = WireListSeparators::list;

      bool first = true;
      while(sep != WireListSeparators::end)
      {
        if(!first) { BEGIN WSPACE CHECK(wireNumberBegin) END_NULL; }
        else { first = false; }

        BEGIN WSPACE END_NULL;
        TextIterExpr* first_iter = this->parseIterExpr();

        BEGIN CHECK_EXPR(first_iter != nullptr) WSPACE END_NULL_SEQ;

        sep = wireListSeparatorsA(&this->ctx);

        switch(sep)
        {
        case WireListSeparators::list: /* fallthrough */
        case WireListSeparators::end:
        {
          iter_out_list->types.push_back(wtk::IterExprWireList::SINGLE);
          iter_out_list->elements.emplace_back();
          iter_out_list->elements.back().single = first_iter;
          break;
        }
        case WireListSeparators::range:
        {
          BEGIN WSPACE_AROUND(wireNumberBegin) END_NULL;
          TextIterExpr* second_iter = this->parseIterExpr();

          BEGIN WSPACE END_NULL_SEQ;
          sep = wireListSeparatorsB(&this->ctx);

          TextIterExprWireRange* range =
            this->iterExprWireRangePool.allocate();
          range->first_ = first_iter;
          range->last_ = second_iter;

          iter_out_list->types.push_back(wtk::IterExprWireList::RANGE);
          iter_out_list->elements.emplace_back();
          iter_out_list->elements.back().range = range;

          break;
        }
        case WireListSeparators::invalid:
        {
          return nullptr;
        }
        }
      }

      BEGIN WSPACE END_NULL_SEQ;
      func_type = functionType(&this->ctx);
      break;
    }
    case WireNumberOrFunction::call:
    {
      func_type = FunctionType::call;
      break;
    }
    case WireNumberOrFunction::anonCall:
    {
      func_type = FunctionType::anonCall;
      break;
    }
    case WireNumberOrFunction::invalid:
    {
      return nullptr;
    }
    }

    TextForLoop<Number_T>* ret = this->forLoopPool.allocate();
    ret->outList = out_list;
    ret->iterator = iter_name;
    ret->first_ = bound_first;
    ret->last_ = bound_last;

    switch(func_type)
    {
    case FunctionType::call:
    {
      BEGIN WSPACE_AROUND(leftParen) END_NULL;
      BEGIN CHECK(identifier); END_NULL_SEQ;

      size_t len = this->ctx.place - this->ctx.mark;
      char* name = this->identifierPool.allocate(len + 1);
      strncpy(name, &this->ctx.buffer[this->ctx.mark], len);
      name[len] = '\0';

      TextIterExprWireList* iter_in_list =
        this->iterExprWireListPool.allocate();

      BEGIN WSPACE END_NULL_SEQ;
      WireListSeparators sep = wireListSeparatorsC(&this->ctx);

      while(sep != WireListSeparators::end)
      {
        BEGIN
          CHECK_EXPR(sep != WireListSeparators::invalid)
          WSPACE_AROUND(wireNumberBegin)
        END_NULL;

        TextIterExpr* first_wire = this->parseIterExpr();
        BEGIN CHECK_EXPR(first_wire != nullptr) WSPACE END_NULL_SEQ;

        sep = wireListSeparatorsD(&this->ctx);
        switch(sep)
        {
        case WireListSeparators::list: /* fallthrough */
        case WireListSeparators::end:
        {
          iter_in_list->types.push_back(wtk::IterExprWireList::SINGLE);
          iter_in_list->elements.emplace_back();
          iter_in_list->elements.back().single = first_wire;
          break;
        }
        case WireListSeparators::range:
        {
          BEGIN CHECK(wireNumberBegin) WSPACE END_NULL;

          TextIterExpr* last_wire = this->parseIterExpr();
          BEGIN CHECK_EXPR(last_wire != nullptr) WSPACE END_NULL_SEQ;

          TextIterExprWireRange* range =
            this->iterExprWireRangePool.allocate();
          range->first_ = first_wire;
          range->last_ = last_wire;

          iter_in_list->types.push_back(wtk::IterExprWireList::RANGE);
          iter_in_list->elements.emplace_back();
          iter_in_list->elements.back().range = range;

          sep = wireListSeparatorsC(&this->ctx);
          break;
        }
        case WireListSeparators::invalid:
        {
          return nullptr;
        }
        }
      }

      TextIterExprFunctionInvoke* invoke =
        this->iterExprFunctionInvokePool.allocate();
      invoke->name_ = name;
      invoke->outList = iter_out_list;
      invoke->inList = iter_in_list;
      ret->invoke = invoke;
      ret->bodyType_ = wtk::ForLoop<Number_T>::INVOKE;

      BEGIN WSPACE CHECK(semiColon) END_NULL_SEQ;
      break;
    }
    case FunctionType::anonCall:
    {
      BEGIN WSPACE_AROUND(leftParen) END_NULL_SEQ;

      TextIterExprWireList* iter_in_list =
        this->iterExprWireListPool.allocate();

      WireNumberOrInstance num_or_ins = wireNumberOrInstance(&this->ctx);

      while(num_or_ins != WireNumberOrInstance::instance)
      {
        if(num_or_ins == WireNumberOrInstance::wireNumber)
        {
          BEGIN WSPACE END_NULL;
          TextIterExpr* first_wire = this->parseIterExpr();
          BEGIN CHECK_EXPR(first_wire != nullptr) WSPACE END_NULL_SEQ;

          WireListSeparators sep = wireListSeparatorsE(&this->ctx);

          switch(sep)
          {
          case WireListSeparators::list:
          {
            iter_in_list->types.push_back(wtk::IterExprWireList::SINGLE);
            iter_in_list->elements.emplace_back();
            iter_in_list->elements.back().single = first_wire;
            break;
          }
          case WireListSeparators::range:
          {
            BEGIN WSPACE_AROUND(wireNumberBegin) END_NULL;

            TextIterExpr* last_wire = this->parseIterExpr();
            BEGIN
              CHECK_EXPR(last_wire != nullptr)
              WSPACE
              CHECK(comma)
            END_NULL_SEQ;

            TextIterExprWireRange* range =
              this->iterExprWireRangePool.allocate();
            range->first_ = first_wire;
            range->last_ = last_wire;

            iter_in_list->types.push_back(wtk::IterExprWireList::RANGE);
            iter_in_list->elements.emplace_back();
            iter_in_list->elements.back().range = range;

            break;
          }
          default: { return nullptr; }
          }

          BEGIN WSPACE END_NULL_SEQ;
          num_or_ins = wireNumberOrInstance(&this->ctx);
        }
        else // invalid
        {
          return nullptr;
        }
      }

      BEGIN WSPACE_AROUND(colon) END_NULL;

      wtk::index_t num_ins;
      NUMBER_NULL(num_ins);

      BEGIN
        WSPACE_AROUND(comma)
        WSPACE_AFTER(shortWitnessKw)
        WSPACE_AFTER(colon)
      END_NULL;

      wtk::index_t num_wit;
      NUMBER_NULL(num_wit);

      BEGIN WSPACE_AROUND(rightParen) END_NULL_SEQ;
      TextDirectiveList<Number_T>* body = this->parseDirectiveList();

      TextIterExprAnonFunction<Number_T>* anon =
        this->iterExprAnonFunctionPool.allocate();
      anon->outList = iter_out_list;
      anon->inList = iter_in_list;
      anon->insCount = num_ins;
      anon->witCount = num_wit;
      anon->directives = body;
      ret->anonymous = anon;
      ret->bodyType_ = wtk::ForLoop<Number_T>::ANONYMOUS;

      break;
    }
    case FunctionType::invalid:
    {
      return nullptr;
    }
    }

    BEGIN WSPACE_AROUND(endKw) END_NULL_SEQ;
    return ret;
  }

  /**
   * input the switch statement's output list. The '@switch' just finished
   * parsing. This function will parse the condition, the body, and then
   * finish.
   */
  TextSwitchStatement<Number_T>* parseSwitchStatement(TextWireList* out_list)
  {
    BEGIN
      WSPACE_AROUND(leftParen)
      WSPACE
      CHECK(wireNumberBegin)
    END_NULL;

    TextSwitchStatement<Number_T>* stmt = this->switchStatementPool.allocate();
    stmt->outList = out_list;
    NUMBER_NULL(stmt->cond);

    BEGIN
      WSPACE_AROUND(rightParen)
      WSPACE_AFTER(caseKw)
    END_NULL_SEQ;

    bool cont = true;
    do
    {
      BEGIN
        WSPACE_AFTER(fieldLiteralBegin)
      END_NULL;

      TextCaseBlock<Number_T>* case_blk = this->caseBlockPool.allocate();
      stmt->cases.push_back(case_blk);
      NUMBER_NULL(case_blk->match_);

      BEGIN
        WSPACE_AROUND(fieldLiteralEnd)
        WSPACE_AFTER(colon)
      END_NULL_SEQ;

      FunctionType func_type = functionType(&this->ctx);
      BEGIN WSPACE END_NULL_SEQ;
      switch(func_type)
      {
      case FunctionType::call:
      {
        BEGIN WSPACE_AROUND(leftParen) END_NULL;
        BEGIN CHECK(identifier); END_NULL_SEQ;

        size_t len = this->ctx.place - this->ctx.mark;
        char* name = this->identifierPool.allocate(len + 1);
        strncpy(name, &this->ctx.buffer[this->ctx.mark], len);
        name[len] = '\0';

        TextWireList* in_list = this->wireListPool.allocate();

        BEGIN WSPACE END_NULL_SEQ;
        WireListSeparators sep = wireListSeparatorsC(&this->ctx);

        while(sep != WireListSeparators::end)
        {
          BEGIN
            CHECK_EXPR(sep != WireListSeparators::invalid)
            WSPACE
            CHECK(wireNumberBegin)
          END_NULL;

          wtk::index_t first_wire;
          NUMBER_NULL(first_wire);

          BEGIN WSPACE END_NULL_SEQ;
          sep = wireListSeparatorsD(&this->ctx);
          switch(sep)
          {
          case WireListSeparators::list: /* fallthrough */
          case WireListSeparators::end:
          {
            in_list->types.push_back(wtk::WireList::SINGLE);
            in_list->elements.emplace_back();
            in_list->elements.back().single = first_wire;
            break;
          }
          case WireListSeparators::range:
          {
            BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;

            wtk::index_t last_wire;
            NUMBER_NULL(last_wire);

            TextWireRange* range = this->wireRangePool.allocate();
            range->firstIdx = first_wire;
            range->lastIdx = last_wire;

            in_list->types.push_back(wtk::WireList::RANGE);
            in_list->elements.emplace_back();
            in_list->elements.back().range = range;

            BEGIN WSPACE END_NULL_SEQ;
            sep = wireListSeparatorsC(&this->ctx);
            break;
          }
          case WireListSeparators::invalid:
          {
            return nullptr;
          }
          }
        }

        TextCaseFunctionInvoke* invoke =
          this->caseFunctionInvokePool.allocate();
        invoke->name_ = name;
        invoke->inList = in_list;
        case_blk->bodyType_ = wtk::CaseBlock<Number_T>::INVOKE;
        case_blk->invoke = invoke;
        
        BEGIN WSPACE_AROUND(semiColon) END_NULL_SEQ;
        break;
      }
      case FunctionType::anonCall:
      {
        BEGIN WSPACE_AROUND(leftParen) END_NULL_SEQ;

        TextWireList* in_list = this->wireListPool.allocate();

        WireNumberOrInstance num_or_ins = wireNumberOrInstance(&this->ctx);

        while(num_or_ins != WireNumberOrInstance::instance)
        {
          if(num_or_ins == WireNumberOrInstance::wireNumber)
          {
            this->ctx.updateMark();
            wtk::index_t first_wire;
            NUMBER_NULL(first_wire);

            BEGIN WSPACE END_NULL_SEQ;
            WireListSeparators sep = wireListSeparatorsE(&this->ctx);

            switch(sep)
            {
            case WireListSeparators::list:
            {
              in_list->types.push_back(wtk::WireList::SINGLE);
              in_list->elements.emplace_back();
              in_list->elements.back().single = first_wire;
              break;
            }
            case WireListSeparators::range:
            {
              BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;

              wtk::index_t last_wire;
              NUMBER_NULL(last_wire);

              TextWireRange* range = this->wireRangePool.allocate();
              range->firstIdx = first_wire;
              range->lastIdx = last_wire;

              in_list->types.push_back(wtk::WireList::RANGE);
              in_list->elements.emplace_back();
              in_list->elements.back().range = range;

              BEGIN WSPACE CHECK(comma) END_NULL_SEQ;
              break;
            }
            default: { return nullptr; }
            }

            BEGIN WSPACE END_NULL_SEQ;
            num_or_ins = wireNumberOrInstance(&this->ctx);
          }
          else // invalid
          {
            return nullptr;
          }
        }

        BEGIN WSPACE_AROUND(colon) END_NULL;

        wtk::index_t num_ins;
        NUMBER_NULL(num_ins);

        BEGIN
          WSPACE_AROUND(comma)
          WSPACE_AFTER(shortWitnessKw)
          WSPACE_AFTER(colon)
        END_NULL;

        wtk::index_t num_wit;
        NUMBER_NULL(num_wit);

        BEGIN WSPACE_AROUND(rightParen) END_NULL_SEQ;
        TextDirectiveList<Number_T>* body = this->parseDirectiveList();

        TextCaseAnonFunction<Number_T>* anon =
          this->caseAnonFunctionPool.allocate();
        anon->inList = in_list;
        anon->insCount = num_ins;
        anon->witCount = num_wit;
        anon->directives = body;
        case_blk->bodyType_ = wtk::CaseBlock<Number_T>::ANONYMOUS;
        case_blk->anonymous = anon;
        
        BEGIN WSPACE END_NULL_SEQ;
        break;
      }
      case FunctionType::invalid:
      {
        return nullptr;
      }
      }

      CaseOrEnd case_or_end = caseOrEnd(&this->ctx);
      if(case_or_end == CaseOrEnd::invalid) { return nullptr; }
      else if(case_or_end == CaseOrEnd::end) { cont = false; }
      BEGIN WSPACE END_NULL_SEQ;
    } while(cont);

    return stmt;
  }

  /**
   * Parses a sequence of directives. It expects no leading whitespace.
   * it will stop when an unmatched '@end' token is reached.
   *
   * first_dir_type is a pre-parsed type for the first type of directive.
   * this occurs at the global scope when reading function-declarations then
   * reading the global scope. If the first_dir_type is invalid (default)
   * then this function reads it itself
   */
  TextDirectiveList<Number_T>* parseDirectiveList(
      TreeDirectiveStart first_dir_start = TreeDirectiveStart::invalid)
  {
    TextDirectiveList<Number_T>* ret = this->directiveListPool.allocate();

    TreeDirectiveStart dir_start;
    if(first_dir_start == TreeDirectiveStart::invalid)
    {
      dir_start = treeDirectiveStart(&this->ctx);
    }
    else
    {
      dir_start = first_dir_start;
    }

    while(dir_start != TreeDirectiveStart::end)
    {
      switch(dir_start)
      {
      case TreeDirectiveStart::wireNumber:
      {
        this->ctx.updateMark();
        wtk::index_t first_output;
        NUMBER_NULL(first_output);
        BEGIN WSPACE END_NULL_SEQ;

        WireListSeparators sep = wireListSeparatorsA(&this->ctx);
        if(sep == WireListSeparators::end)
        {
          BEGIN WSPACE END_NULL_SEQ;
          SingleOutputDirective directive = singleOutputDirective(&this->ctx);

          switch(directive)
          {
          case SingleOutputDirective::add:  /* fallthrough */
          case SingleOutputDirective::mul:  /* fallthrough */
          case SingleOutputDirective::and_: /* fallthrough */
          case SingleOutputDirective::xor_:
          {
            BEGIN
              WSPACE_AROUND(leftParen)
              CHECK(wireNumberBegin)
            END_NULL;

            wtk::index_t left_input;
            NUMBER_NULL(left_input);
            BEGIN WSPACE_AROUND(comma) CHECK(wireNumberBegin) END_NULL;

            wtk::index_t right_input;
            NUMBER_NULL(right_input);

            TextBinaryGate* gate = this->binaryGatePool.allocate();
            gate->out = first_output;
            gate->left = left_input;
            gate->right = right_input;

            switch(directive)
            {
            case SingleOutputDirective::add:
            {
              if(this->gateSet->gateSet == wtk::GateSet::arithmetic
                  && !this->gateSet->enableAdd)
              {
                log_error("Gate '@add' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryGate::ADD;
              break;
            }
            case SingleOutputDirective::mul:
            {
              if(this->gateSet->gateSet == wtk::GateSet::arithmetic
                  && !this->gateSet->enableMul)
              {
                log_error("Gate '@mul' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryGate::MUL;
              break;
            }
            case SingleOutputDirective::and_:
            {
              if(this->gateSet->gateSet == wtk::GateSet::boolean
                  && !this->gateSet->enableAnd)
              {
                log_error("Gate '@and' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryGate::AND;
              break;
            }
            case SingleOutputDirective::xor_:
            {
              if(this->gateSet->gateSet == wtk::GateSet::boolean
                  && !this->gateSet->enableXor)
              {
                log_error("Gate '@xor' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryGate::XOR;
              break;
            }
            default: { log_fatal("unreachable"); }
            }

            ret->types.push_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
            ret->elements.emplace_back();
            ret->elements.back().binaryGate = gate;

            BEGIN
              WSPACE_AROUND(rightParen)
              WSPACE_AFTER(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::addc: /* fallthrough */
          case SingleOutputDirective::mulc:
          {
            BEGIN
              WSPACE_AROUND(leftParen)
              CHECK(wireNumberBegin)
            END_NULL;

            wtk::index_t left_input;
            NUMBER_NULL(left_input);

            BEGIN
              WSPACE_AROUND(comma)
              WSPACE_AFTER(fieldLiteralBegin)
            END_NULL;

            Number_T right_const;
            NUMBER_NULL(right_const);

            TextBinaryConstGate<Number_T>* gate =
              this->binaryConstGatePool.allocate();
            gate->out = first_output;
            gate->left = left_input;
            gate->right = right_const;

            switch(directive)
            {
            case SingleOutputDirective::addc:
            {
              if(this->gateSet->gateSet == wtk::GateSet::arithmetic
                  && !this->gateSet->enableAddC)
              {
                log_error("Gate '@addc' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryConstGate<Number_T>::ADDC;
              break;
            }
            case SingleOutputDirective::mulc:
            {
              if(this->gateSet->gateSet == wtk::GateSet::arithmetic
                  && !this->gateSet->enableMulC)
              {
                log_error("Gate '@mulc' is forbidden by gate set");
                return nullptr;
              }

              gate->calc = wtk::BinaryConstGate<Number_T>::MULC;
              break;
            }
            default: { log_fatal("unreachable"); }
            }

            ret->types.push_back(
                wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
            ret->elements.emplace_back();
            ret->elements.back().binaryConstGate = gate;

            BEGIN
              WSPACE_AROUND(fieldLiteralEnd)
              WSPACE_AFTER(rightParen)
              WSPACE_AFTER(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::not_:
          {
            BEGIN
              WSPACE_AROUND(leftParen)
              CHECK(wireNumberBegin)
            END_NULL;

            wtk::index_t input;
            NUMBER_NULL(input);

            TextUnaryGate* gate = this->unaryGatePool.allocate();
            gate->out = first_output;
            gate->in = input;

            if(this->gateSet->gateSet == wtk::GateSet::boolean
                && !this->gateSet->enableNot)
            {
              log_error("Gate '@not' is forbidden by gate set");
              return nullptr;
            }
            gate->calc = UnaryGate::NOT;

            ret->types.push_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
            ret->elements.emplace_back();
            ret->elements.back().unaryGate = gate;

            BEGIN
              WSPACE_AROUND(rightParen)
              WSPACE_AFTER(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::witness:
          {
            TextInput* input = this->inputPool.allocate();
            input->out = first_output;
            input->stream_ = wtk::Input::SHORT_WITNESS;

            ret->types.push_back(wtk::DirectiveList<Number_T>::INPUT);
            ret->elements.emplace_back();
            ret->elements.back().input = input;

            BEGIN
              WSPACE_AROUND(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::instance:
          {
            TextInput* input = this->inputPool.allocate();
            input->out = first_output;
            input->stream_ = wtk::Input::INSTANCE;

            ret->types.push_back(wtk::DirectiveList<Number_T>::INPUT);
            ret->elements.emplace_back();
            ret->elements.back().input = input;

            BEGIN
              WSPACE_AROUND(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::assign:
          {
            BEGIN WSPACE END_NULL;
            Number_T val;
            NUMBER_NULL(val);

            TextAssign<Number_T>* assign = this->assignPool.allocate();
            assign->out = first_output;
            assign->value = val;

            ret->types.push_back(wtk::DirectiveList<Number_T>::ASSIGN);
            ret->elements.emplace_back();
            ret->elements.back().assign = assign;

            BEGIN
              WSPACE_AROUND(fieldLiteralEnd)
              WSPACE_AFTER(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::copy:
          {
            this->ctx.updateMark();
            wtk::index_t input;
            NUMBER_NULL(input);

            TextUnaryGate* gate = this->unaryGatePool.allocate();
            gate->out = first_output;
            gate->in = input;

            gate->calc = UnaryGate::COPY;

            ret->types.push_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
            ret->elements.emplace_back();
            ret->elements.back().unaryGate = gate;

            BEGIN
              WSPACE_AROUND(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::call:
          {
            TextWireList* out_list = this->wireListPool.allocate();
            out_list->types.emplace_back(wtk::WireList::SINGLE);
            out_list->elements.emplace_back();
            out_list->elements.back().single = first_output;
            TextFunctionInvoke* invoke = this->parseFunctionInvoke(out_list);

            if(invoke == nullptr) { return nullptr; }

            ret->types.push_back(
                wtk::DirectiveList<Number_T>::FUNCTION_INVOKE);
            ret->elements.emplace_back();
            ret->elements.back().functionInvoke = invoke;

            BEGIN
              WSPACE_AROUND(semiColon)
            END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::anonCall:
          {
            TextWireList* out_list = this->wireListPool.allocate();
            out_list->types.emplace_back(wtk::WireList::SINGLE);
            out_list->elements.emplace_back();
            out_list->elements.back().single = first_output;
            TextAnonFunction<Number_T>* anon =
              this->parseAnonFunction(out_list);

            if(anon == nullptr) { return nullptr; }

            ret->types.push_back(wtk::DirectiveList<Number_T>::ANON_FUNCTION);
            ret->elements.emplace_back();
            ret->elements.back().anonFunction = anon;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::forLoop:
          {
            TextWireList* out_list = this->wireListPool.allocate();
            out_list->types.emplace_back(wtk::WireList::SINGLE);
            out_list->elements.emplace_back();
            out_list->elements.back().single = first_output;
            TextForLoop<Number_T>* loop = this->parseForLoop(out_list);

            if(loop == nullptr) { return nullptr; }

            ret->types.push_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
            ret->elements.emplace_back();
            ret->elements.back().forLoop = loop;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::switchStatement:
          {
            TextWireList* out_list = this->wireListPool.allocate();
            out_list->types.emplace_back(wtk::WireList::SINGLE);
            out_list->elements.emplace_back();
            out_list->elements.back().single = first_output;
            TextSwitchStatement<Number_T>* switch_stmt =
              this->parseSwitchStatement(out_list);

            if(switch_stmt == nullptr) { return nullptr; }

            ret->types.push_back(
                wtk::DirectiveList<Number_T>::SWITCH_STATEMENT);
            ret->elements.emplace_back();
            ret->elements.back().switchStatement = switch_stmt;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case SingleOutputDirective::invalid:
          {
            return nullptr;
          }
          }
        }
        else
        {
          /* Parse a wire list and then feed it to a feature directive */
          TextWireList* list = this->wireListPool.allocate();
          bool first = true;
          while(sep != WireListSeparators::end)
          {
            wtk::index_t first_wire;
            if(first)
            {
              first_wire = first_output;
              first = false;
            }
            else
            {
              BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;
              NUMBER_NULL(first_wire);

              BEGIN WSPACE END_NULL_SEQ;
              sep = wireListSeparatorsA(&this->ctx);
            }

            if(sep == WireListSeparators::list
                || sep == WireListSeparators::end)
            {
              list->types.push_back(wtk::WireList::SINGLE);
              list->elements.emplace_back();
              list->elements.back().single = first_wire;
            }
            else if(sep == WireListSeparators::range)
            {
              TextWireRange* range = this->wireRangePool.allocate();
              range->firstIdx = first_wire;
              BEGIN WSPACE CHECK(wireNumberBegin) END_NULL;
              NUMBER_NULL(range->lastIdx);

              list->types.push_back(wtk::WireList::RANGE);
              list->elements.emplace_back();
              list->elements.back().range = range;

              BEGIN WSPACE END_NULL_SEQ;
              sep = wireListSeparatorsB(&this->ctx);
            }
            else if(sep == WireListSeparators::invalid)
            {
              return nullptr;
            }
          }

          BEGIN WSPACE END_NULL_SEQ;
          MultiOutputDirective dir_type = multiOutputDirective(&this->ctx);
          switch(dir_type)
          {
          case MultiOutputDirective::call:
          {
            TextFunctionInvoke* invoke = this->parseFunctionInvoke(list);
            if(invoke == nullptr) { return nullptr; }

            ret->types.push_back(
                wtk::DirectiveList<Number_T>::FUNCTION_INVOKE);
            ret->elements.emplace_back();
            ret->elements.back().functionInvoke = invoke;

            BEGIN WSPACE_AROUND(semiColon) END_NULL_SEQ;
            break;
          }
          case MultiOutputDirective::anonCall:
          {
            TextAnonFunction<Number_T>* anon = this->parseAnonFunction(list);
            if(anon == nullptr) { return nullptr; }

            ret->types.push_back(wtk::DirectiveList<Number_T>::ANON_FUNCTION);
            ret->elements.emplace_back();
            ret->elements.back().anonFunction = anon;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case MultiOutputDirective::forLoop:
          {
            TextForLoop<Number_T>* loop = this->parseForLoop(list);
            if(loop == nullptr) { return nullptr; }

            ret->types.push_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
            ret->elements.emplace_back();
            ret->elements.back().forLoop = loop;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case MultiOutputDirective::switchStatement:
          {
            TextSwitchStatement<Number_T>* switch_stmt =
              this->parseSwitchStatement(list);
            if(switch_stmt == nullptr) { return nullptr; }

            ret->types.push_back(
                wtk::DirectiveList<Number_T>::SWITCH_STATEMENT);
            ret->elements.emplace_back();
            ret->elements.back().switchStatement = switch_stmt;

            BEGIN WSPACE END_NULL_SEQ;
            break;
          }
          case MultiOutputDirective::invalid:
          {
            return nullptr;
          }
          }
        }
        break;
      }
      case TreeDirectiveStart::assertZero:
      {
        BEGIN
          WSPACE_AROUND(leftParen)
          CHECK(wireNumberBegin)
        END_NULL;

        wtk::index_t wire;
        NUMBER_NULL(wire);

        TextTerminal* term = this->terminalPool.allocate();
        term->wire_ = wire;

        ret->types.push_back(wtk::DirectiveList<Number_T>::ASSERT_ZERO);
        ret->elements.emplace_back();
        ret->elements.back().assertZero = term;

        BEGIN
          WSPACE_AROUND(rightParen)
          WSPACE_AFTER(semiColon)
        END_NULL_SEQ;
        break;
      }
      case TreeDirectiveStart::delete_:
      {
        BEGIN
          WSPACE_AROUND(leftParen)
          CHECK(wireNumberBegin)
        END_NULL;

        wtk::index_t first;
        NUMBER_NULL(first);

        BEGIN WSPACE END_NULL_SEQ;

        CommaOrRightParen corp = commaOrRightParen(&this->ctx);

        switch(corp)
        {
        case CommaOrRightParen::comma:
        {
          BEGIN
            WSPACE
            CHECK(wireNumberBegin)
          END_NULL;

          wtk::index_t last;
          NUMBER_NULL(last);

          TextWireRange* range = this->wireRangePool.allocate();
          range->firstIdx = first;
          range->lastIdx = last;

          ret->types.push_back(wtk::DirectiveList<Number_T>::DELETE_RANGE);
          ret->elements.emplace_back();
          ret->elements.back().deleteRange = range;

          BEGIN
            WSPACE_AROUND(rightParen)
            WSPACE_AFTER(semiColon)
          END_NULL_SEQ;
          break;
        }
        case CommaOrRightParen::rightParen:
        {
          TextTerminal* term = this->terminalPool.allocate();
          term->wire_ = first;

          ret->types.push_back(wtk::DirectiveList<Number_T>::DELETE_SINGLE);
          ret->elements.emplace_back();
          ret->elements.back().deleteSingle = term;

          BEGIN WSPACE_AROUND(semiColon) END_NULL_SEQ;
          break;
        }
        case CommaOrRightParen::invalid:
        {
          return nullptr;
        }
        }
        break;
      }
      case TreeDirectiveStart::call:
      {
        TextWireList* out_list = this->wireListPool.allocate();
        TextFunctionInvoke* invoke = this->parseFunctionInvoke(out_list);

        if(invoke == nullptr) { return nullptr; }

        ret->types.push_back(wtk::DirectiveList<Number_T>::FUNCTION_INVOKE);
        ret->elements.emplace_back();
        ret->elements.back().functionInvoke = invoke;

        BEGIN WSPACE_AROUND(semiColon) END_NULL_SEQ;
        break;
      }
      case TreeDirectiveStart::anonCall:
      {
        TextWireList* out_list = this->wireListPool.allocate();
        TextAnonFunction<Number_T>* anon = this->parseAnonFunction(out_list);

        if(anon == nullptr) { return nullptr; }

        ret->types.push_back(wtk::DirectiveList<Number_T>::ANON_FUNCTION);
        ret->elements.emplace_back();
        ret->elements.back().anonFunction = anon;

        BEGIN WSPACE END_NULL_SEQ;
        break;
      }
      case TreeDirectiveStart::forLoop:
      {
        TextWireList* out_list = this->wireListPool.allocate();
        TextForLoop<Number_T>* loop = this->parseForLoop(out_list);

        if(loop == nullptr) { return nullptr; }

        ret->types.push_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
        ret->elements.emplace_back();
        ret->elements.back().forLoop = loop;

        BEGIN WSPACE END_NULL_SEQ;
        break;
      }
      case TreeDirectiveStart::switchStatement:
      {
        TextWireList* out_list = this->wireListPool.allocate();
        TextSwitchStatement<Number_T>* switch_stmt =
          this->parseSwitchStatement(out_list);

        if(switch_stmt == nullptr) { return nullptr; }

        ret->types.push_back(wtk::DirectiveList<Number_T>::SWITCH_STATEMENT);
        ret->elements.emplace_back();
        ret->elements.back().switchStatement = switch_stmt;

        BEGIN WSPACE END_NULL_SEQ;
        break;
      }
      // loop condition should make end unreachable, so fallthrough to invalid
      case TreeDirectiveStart::end:
      case TreeDirectiveStart::invalid:
      {
        return nullptr;
      }
      }

      dir_start = treeDirectiveStart(&this->ctx);
    }

    return ret;
  }

  TextIRTree<Number_T>* parseTree()
  {
    BEGIN WSPACE_AROUND(beginKw) END_NULL;
    TreeDirectiveGlobalStart dir_type = treeDirectiveGlobalStart(&this->ctx);

    while(dir_type == TreeDirectiveGlobalStart::function)
    {
      BEGIN WSPACE_AROUND(leftParen) END_NULL;
      BEGIN CHECK(identifier) END_NULL_SEQ;

      size_t len = this->ctx.place - this->ctx.mark;
      char* name = this->identifierPool.allocate(len + 1);
      strncpy(name, &this->ctx.buffer[this->ctx.mark], len);
      name[len] = '\0';

      BEGIN
        WSPACE_AROUND(comma)
        WSPACE_AFTER(outKw)
        WSPACE_AFTER(colon)
      END_NULL;

      wtk::index_t num_out;
      NUMBER_NULL(num_out);

      BEGIN
        WSPACE_AROUND(comma)
        WSPACE_AFTER(inKw)
        WSPACE_AFTER(colon)
      END_NULL;

      wtk::index_t num_in;
      NUMBER_NULL(num_in);

      BEGIN
        WSPACE_AROUND(comma)
        WSPACE_AFTER(instanceKw)
        WSPACE_AFTER(colon)
      END_NULL;

      wtk::index_t num_ins;
      NUMBER_NULL(num_ins);

      BEGIN
        WSPACE_AROUND(comma)
        WSPACE_AFTER(shortWitnessKw)
        WSPACE_AFTER(colon)
      END_NULL;

      wtk::index_t num_wit;
      NUMBER_NULL(num_wit);

      BEGIN
        WSPACE_AROUND(rightParen)
      END_NULL;

      TextDirectiveList<Number_T>* body = this->parseDirectiveList();

      if(body == nullptr) { return nullptr; }

      TextFunctionDeclare<Number_T>* decl =
        this->functionDeclarePool.allocate();
      decl->name_ = name;
      decl->outCount = num_out;
      decl->inCount = num_in;
      decl->insCount = num_ins;
      decl->witCount = num_wit;
      decl->directives = body;
      this->irTree.functionDeclares.push_back(decl);

      BEGIN WSPACE END_NULL_SEQ;
      dir_type = treeDirectiveGlobalStart(&this->ctx);
    }

    if(dir_type == TreeDirectiveGlobalStart::invalid)
    {
      log_error("invalid");
      return nullptr;
    }

    this->irTree.directives =
      this->parseDirectiveList((TreeDirectiveStart) dir_type);

    if(this->irTree.directives == nullptr) { return nullptr; }
    else { return &this->irTree; }
  }
};

} } // namespace wtk::irregular

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#define UNINCLUDE_CHECK_MACROS
#include <wtk/irregular/checkMacros.t.h>

#endif // WTK_IRREGULAR_TEXT_IR1_PARSER_H_
