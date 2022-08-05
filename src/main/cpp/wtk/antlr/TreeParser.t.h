/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_TREE_PARSER_H_
#define WTK_ANTLR_TREE_PARSER_H_

#include <cstddef>
#include <cstring>
#include <vector>
#include <string>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/IRParameters.h>
#include <wtk/utils/Pool.h>

#include <wtk/irregular/TextIRTree.t.h>

#include <antlr4-runtime.h>
#include <wtk/antlr/SIEVEIRParser.h>

#include <wtk/antlr/AntlrIRTree.t.h>
#include <wtk/antlr/GateUtils.t.h>

#define LOG_IDENTIFIER "wtk::antlr"
#include <stealth_logging.h>

namespace wtk {
namespace antlr {

using namespace wtk_gen_antlr;

template<typename Number_T>
struct TreeParser
{
  wtk::GateSet const* gateSet = nullptr;
  wtk::FeatureToggles const* featureToggles = nullptr;

  AntlrIRTree<Number_T> tree;

  wtk::utils::Pool<AntlrDirectiveList<Number_T>> directiveListPool;
  wtk::utils::Pool<AntlrFunctionDeclare<Number_T>> functionDeclarePool;
  wtk::utils::Pool<AntlrBinaryGate> binaryGatePool;
  wtk::utils::Pool<AntlrBinaryConstGate<Number_T>> binaryConstGatePool;
  wtk::utils::Pool<AntlrUnaryGate> unaryGatePool;
  wtk::utils::Pool<AntlrInput> inputPool;
  wtk::utils::Pool<AntlrAssign<Number_T>> assignPool;
  wtk::utils::Pool<AntlrTerminal> terminalPool;
  wtk::utils::Pool<AntlrWireRange> wireRangePool;
  wtk::utils::Pool<AntlrWireList> wireListPool;
  wtk::utils::Pool<char> labelPool;
  wtk::utils::Pool<AntlrFunctionInvoke> functionInvokePool;
  wtk::utils::Pool<AntlrAnonFunction<Number_T>> anonFunctionPool;
  wtk::utils::Pool<AntlrIterExpr> iterExprPool;
  wtk::utils::Pool<AntlrIterExprWireRange> iterExprWireRangePool;
  wtk::utils::Pool<AntlrIterExprWireList> iterExprWireListPool;
  wtk::utils::Pool<AntlrIterExprFunctionInvoke> iterExprFunctionInvokePool;
  wtk::utils::Pool<
    AntlrIterExprAnonFunction<Number_T>> iterExprAnonFunctionPool;
  wtk::utils::Pool<AntlrForLoop<Number_T>> forLoopPool;
  wtk::utils::Pool<AntlrCaseFunctionInvoke> caseFunctionInvokePool;
  wtk::utils::Pool<AntlrCaseAnonFunction<Number_T>> caseAnonFunctionPool;
  wtk::utils::Pool<AntlrCaseBlock<Number_T>> caseBlockPool;
  wtk::utils::Pool<AntlrSwitchStatement<Number_T>> switchStatementPool;

  // file name for error printing.
  char const* const fname;

  TreeParser(
      wtk::GateSet const* g, wtk::FeatureToggles const* f, char const* fn)
    : gateSet(g), featureToggles(f), fname(fn) { }

  AntlrIRTree<Number_T>* parseTree(SIEVEIRParser::RelationBodyContext* ctx)
  {
    std::vector<SIEVEIRParser::FunctionDeclareContext*> funcs =
      ctx->functionDeclare();
    this->tree.functionDeclares.reserve(funcs.size());

    for(SIEVEIRParser::FunctionDeclareContext* i_func : funcs)
    {
      AntlrFunctionDeclare<Number_T>* c_func =
        this->functionDeclarePool.allocate();
      c_func->line = i_func->getStart()->getLine();

      c_func->name_ = this->parseLabel(i_func->LABEL());
      num_to_uint(i_func->NUMERIC_LITERAL(0)->getText(), c_func->outCount);
      num_to_uint(i_func->NUMERIC_LITERAL(1)->getText(), c_func->inCount);
      num_to_uint(i_func->NUMERIC_LITERAL(2)->getText(), c_func->insCount);
      num_to_uint(i_func->NUMERIC_LITERAL(3)->getText(), c_func->witCount);

      AntlrDirectiveList<Number_T>* dir_list =
        this->parseDirectiveList(i_func->directiveList());
      if(dir_list == nullptr) { return nullptr; }

      c_func->directives = dir_list;
      this->tree.functionDeclares.push_back(c_func);
    }

    AntlrDirectiveList<Number_T>* dir_list =
      this->parseDirectiveList(ctx->directiveList());
    if(dir_list == nullptr) { return nullptr; }

    this->tree.directives = dir_list;
    this->tree.line = ctx->getStart()->getLine();

    return &this->tree;
  }

  AntlrDirectiveList<Number_T>* parseDirectiveList(
      SIEVEIRParser::DirectiveListContext* ctx)
  {
    AntlrDirectiveList<Number_T>* dir_list =
      this->directiveListPool.allocate();
    dir_list->line = ctx->getStart()->getLine();

    std::vector<SIEVEIRParser::DirectiveContext*> i_dirs = ctx->directive();

    dir_list->types.reserve(i_dirs.size());
    dir_list->elements.reserve(i_dirs.size());

    for(SIEVEIRParser::DirectiveContext* i_dir : i_dirs)
    {
      if(i_dir->binaryGate() != nullptr)
      {
        SIEVEIRParser::BinaryGateContext* i_gate = i_dir->binaryGate();
        AntlrBinaryGate* c_gate = this->binaryGatePool.allocate();
        c_gate->line = i_gate->getStart()->getLine();

        wire_to_uint(i_gate->WIRE_NUM(0)->getText(), c_gate->out);
        wire_to_uint(i_gate->WIRE_NUM(1)->getText(), c_gate->left);
        wire_to_uint(i_gate->WIRE_NUM(2)->getText(), c_gate->right);

        if(!checkBinaryGate(i_gate->binaryGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.",
              this->fname, i_gate->binaryGateType()->getStart()->getLine(),
              i_gate->binaryGateType()->getText().c_str());
          return nullptr;
        }

        if(i_gate->binaryGateType()->AND() != nullptr)
        {
          c_gate->calc = BinaryGate::AND;
        }
        else if(i_gate->binaryGateType()->XOR() != nullptr)
        {
          c_gate->calc = BinaryGate::XOR;
        }
        else if(i_gate->binaryGateType()->ADD() != nullptr)
        {
          c_gate->calc = BinaryGate::ADD;
        }
        else if(i_gate->binaryGateType()->MUL() != nullptr)
        {
          c_gate->calc = BinaryGate::MUL;
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().binaryGate = c_gate;
        dir_list->types.push_back(DirectiveList<Number_T>::BINARY_GATE);
      }
      else if(i_dir->binaryConstGate() != nullptr)
      {
        SIEVEIRParser::BinaryConstGateContext* i_gate =
          i_dir->binaryConstGate();
        AntlrBinaryConstGate<Number_T>* c_gate =
          this->binaryConstGatePool.allocate();
        c_gate->line = i_gate->getStart()->getLine();

        wire_to_uint(i_gate->WIRE_NUM(0)->getText(), c_gate->out);
        wire_to_uint(i_gate->WIRE_NUM(1)->getText(), c_gate->left);
        num_to_uint( i_gate->fieldLiteral()->NUMERIC_LITERAL()->getText(),
            c_gate->right);

        if(!checkBinaryConstGate(i_gate->binaryConstGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.",
              this->fname,
              i_gate->binaryConstGateType()->getStart()->getLine(),
              i_gate->binaryConstGateType()->getText().c_str());
          return nullptr;
        }

        else if(i_gate->binaryConstGateType()->ADDC() != nullptr)
        {
          c_gate->calc = BinaryConstGate<Number_T>::ADDC;
        }
        else if(i_gate->binaryConstGateType()->MULC() != nullptr)
        {
          c_gate->calc = BinaryConstGate<Number_T>::MULC;
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().binaryConstGate = c_gate;
        dir_list->types.push_back(DirectiveList<Number_T>::BINARY_CONST_GATE);
      }
      else if(i_dir->unaryGate() != nullptr)
      {
        SIEVEIRParser::UnaryGateContext* i_gate = i_dir->unaryGate();
        AntlrUnaryGate* c_gate = this->unaryGatePool.allocate();
        c_gate->line = i_gate->getStart()->getLine();

        wire_to_uint(i_gate->WIRE_NUM(0)->getText(), c_gate->out);
        wire_to_uint(i_gate->WIRE_NUM(1)->getText(), c_gate->in);

        if(!checkUnaryGate(i_gate->unaryGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.",
              this->fname, i_gate->unaryGateType()->getStart()->getLine(),
              i_gate->unaryGateType()->getText().c_str());
          return nullptr;
        }

        if(i_gate->unaryGateType()->NOT() != nullptr)
        {
          c_gate->calc = UnaryGate::NOT;
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().unaryGate = c_gate;
        dir_list->types.push_back(DirectiveList<Number_T>::UNARY_GATE);
      }
      else if(i_dir->input() != nullptr)
      {
        SIEVEIRParser::InputContext* i_in = i_dir->input();
        AntlrInput* c_in = this->inputPool.allocate();
        c_in->line = i_in->getStart()->getLine();

        wire_to_uint(i_in->WIRE_NUM()->getText(), c_in->out);

        if(i_in->INSTANCE() != nullptr)
        {
          c_in->stream_ = Input::INSTANCE;
        }
        else if(i_in->SHORT_WITNESS() != nullptr)
        {
          c_in->stream_ = Input::SHORT_WITNESS;
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().input = c_in;
        dir_list->types.push_back(DirectiveList<Number_T>::INPUT);
      }
      else if(i_dir->copy() != nullptr)
      {
        SIEVEIRParser::CopyContext* i_copy = i_dir->copy();
        AntlrUnaryGate* c_copy = this->unaryGatePool.allocate();
        c_copy->line = i_copy->getStart()->getLine();

        wire_to_uint(i_copy->WIRE_NUM(0)->getText(), c_copy->out);
        wire_to_uint(i_copy->WIRE_NUM(1)->getText(), c_copy->in);

        c_copy->calc = UnaryGate::COPY;

        dir_list->elements.emplace_back();
        dir_list->elements.back().unaryGate = c_copy;
        dir_list->types.push_back(DirectiveList<Number_T>::UNARY_GATE);
      }
      else if(i_dir->assign() != nullptr)
      {
        SIEVEIRParser::AssignContext* i_assign = i_dir->assign();
        AntlrAssign<Number_T>* c_assign = this->assignPool.allocate();
        c_assign->line = i_assign->getStart()->getLine();

        wire_to_uint(i_assign->WIRE_NUM()->getText(), c_assign->out);
        num_to_uint( i_assign->fieldLiteral()->NUMERIC_LITERAL()->getText(),
            c_assign->value);

        dir_list->elements.emplace_back();
        dir_list->elements.back().assign = c_assign;
        dir_list->types.push_back(DirectiveList<Number_T>::ASSIGN);
      }
      else if(i_dir->assertZero() != nullptr)
      {
        SIEVEIRParser::AssertZeroContext* i_assertZero = i_dir->assertZero();
        AntlrTerminal* c_assertZero = this->terminalPool.allocate();
        c_assertZero->line = i_assertZero->getStart()->getLine();

        wire_to_uint(i_assertZero->WIRE_NUM()->getText(), c_assertZero->wire_);

        dir_list->elements.emplace_back();
        dir_list->elements.back().assertZero = c_assertZero;
        dir_list->types.push_back(DirectiveList<Number_T>::ASSERT_ZERO);
      }
      else if(i_dir->deleteSingle() != nullptr)
      {
        SIEVEIRParser::DeleteSingleContext* i_delete = i_dir->deleteSingle();
        AntlrTerminal* c_delete = this->terminalPool.allocate();
        c_delete->line = i_delete->getStart()->getLine();

        wire_to_uint(i_delete->WIRE_NUM()->getText(), c_delete->wire_);

        dir_list->elements.emplace_back();
        dir_list->elements.back().deleteSingle = c_delete;
        dir_list->types.push_back(DirectiveList<Number_T>::DELETE_SINGLE);
      }
      else if(i_dir->deleteRange() != nullptr)
      {
        SIEVEIRParser::DeleteRangeContext* i_delete = i_dir->deleteRange();
        AntlrWireRange* c_delete = this->wireRangePool.allocate();
        c_delete->line = i_delete->getStart()->getLine();

        wire_to_uint(i_delete->WIRE_NUM(0)->getText(), c_delete->firstIdx);
        wire_to_uint(i_delete->WIRE_NUM(1)->getText(), c_delete->lastIdx);

        dir_list->elements.emplace_back();
        dir_list->elements.back().deleteRange = c_delete;
        dir_list->types.push_back(DirectiveList<Number_T>::DELETE_RANGE);
      }
      else if(i_dir->functionInvoke() != nullptr)
      {
        if(!this->featureToggles->functionToggle)
        {
          log_error("%s:%zu: Function Invocation forbidden by feature toggle.",
              this->fname, i_dir->functionInvoke()->getStart()->getLine());
          return nullptr;
        }

        SIEVEIRParser::FunctionInvokeContext* i_invoke =
          i_dir->functionInvoke();
        AntlrFunctionInvoke* c_invoke = this->functionInvokePool.allocate();
        c_invoke->line = i_invoke->getStart()->getLine();

        c_invoke->outList = this->parseWireList(i_invoke->outputs);
        c_invoke->name_ = this->parseLabel(i_invoke->LABEL());
        c_invoke->inList = this->parseWireList(i_invoke->inputs);

        dir_list->elements.emplace_back();
        dir_list->elements.back().functionInvoke = c_invoke;
        dir_list->types.push_back(DirectiveList<Number_T>::FUNCTION_INVOKE);
      }
      else if(i_dir->anonFunction() != nullptr)
      {
        if(!this->featureToggles->functionToggle)
        {
          log_error("%s:%zu: Anonymous Function forbidden by feature toggle.",
              this->fname, i_dir->anonFunction()->getStart()->getLine());
          return nullptr;
        }

        SIEVEIRParser::AnonFunctionContext* i_anon = i_dir->anonFunction();
        AntlrAnonFunction<Number_T>* c_anon =
          this->anonFunctionPool.allocate();
        c_anon->line = i_anon->getStart()->getLine();

        c_anon->outList = this->parseWireList(i_anon->outputs);
        c_anon->inList = this->parseWireList(i_anon->inputs);
        c_anon->directives = this->parseDirectiveList(i_anon->directiveList());

        num_to_uint(i_anon->NUMERIC_LITERAL(0)->getText(), c_anon->insCount);
        num_to_uint(i_anon->NUMERIC_LITERAL(1)->getText(), c_anon->witCount);

        dir_list->elements.emplace_back();
        dir_list->elements.back().anonFunction = c_anon;
        dir_list->types.push_back(DirectiveList<Number_T>::ANON_FUNCTION);
      }
      else if(i_dir->forLoop() != nullptr)
      {
        if(!this->featureToggles->forLoopToggle)
        {
          log_error("%s:%zu: For Loops forbidden by feature toggle.",
              this->fname, i_dir->forLoop()->getStart()->getLine());
          return nullptr;
        }

        SIEVEIRParser::ForLoopContext* i_loop = i_dir->forLoop();
        AntlrForLoop<Number_T>* c_loop = this->forLoopPool.allocate();
        c_loop->line = i_loop->getStart()->getLine();

        c_loop->outList = this->parseWireList(i_loop->wireList());

        c_loop->iterator = this->parseLabel(i_loop->LABEL());

        num_to_uint(i_loop->NUMERIC_LITERAL(0)->getText(), c_loop->first_);
        num_to_uint(i_loop->NUMERIC_LITERAL(1)->getText(), c_loop->last_);

        if(i_loop->iterExprFunctionInvoke() != nullptr)
        {
          SIEVEIRParser::IterExprFunctionInvokeContext* i_invoke =
            i_loop->iterExprFunctionInvoke();
          AntlrIterExprFunctionInvoke* c_invoke =
            this->iterExprFunctionInvokePool.allocate();
          c_invoke->line = i_invoke->getStart()->getLine();

          c_invoke->outList = this->parseIterExprWireList(i_invoke->outputs);
          c_invoke->name_ = this->parseLabel(i_invoke->LABEL());
          c_invoke->inList = this->parseIterExprWireList(i_invoke->inputs);

          c_loop->bodyType_ = ForLoop<Number_T>::INVOKE;
          c_loop->invoke = c_invoke;
        }
        else
        {
          SIEVEIRParser::IterExprAnonFunctionContext* i_anon =
            i_loop->iterExprAnonFunction();
          AntlrIterExprAnonFunction<Number_T>* c_anon =
            this->iterExprAnonFunctionPool.allocate();
          c_anon->line = i_anon->getStart()->getLine();

          c_anon->outList = this->parseIterExprWireList(i_anon->outputs);
          c_anon->inList = this->parseIterExprWireList(i_anon->inputs);

          num_to_uint(i_anon->NUMERIC_LITERAL(0)->getText(), c_anon->insCount);
          num_to_uint(i_anon->NUMERIC_LITERAL(1)->getText(), c_anon->witCount);

          AntlrDirectiveList<Number_T>* dlist =
            this->parseDirectiveList(i_anon->directiveList());

          if(dlist == nullptr) { return nullptr; }
          c_anon->directives = dlist;

          c_loop->bodyType_ = ForLoop<Number_T>::ANONYMOUS;
          c_loop->anonymous  = c_anon;
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().forLoop  = c_loop;
        dir_list->types.push_back(DirectiveList<Number_T>::FOR_LOOP);
      }
      else if(i_dir->switchStatement() != nullptr)
      {
        if(!this->featureToggles->switchCaseToggle)
        {
          log_error("%s:%zu: Switch Statements forbidden by feature toggle.",
              this->fname, i_dir->switchStatement()->getStart()->getLine());
          return nullptr;
        }

        SIEVEIRParser::SwitchStatementContext* i_switch =
          i_dir->switchStatement();
        AntlrSwitchStatement<Number_T>* c_switch =
          this->switchStatementPool.allocate();
        c_switch->line = i_switch->getStart()->getLine();
 
        c_switch->outList = this->parseWireList(i_switch->wireList());
        wire_to_uint(i_switch->WIRE_NUM()->getText(), c_switch->cond);

        std::vector<SIEVEIRParser::FieldLiteralContext*> i_sels =
          i_switch->fieldLiteral();
        std::vector<SIEVEIRParser::CaseFunctionContext*> i_cases =
          i_switch->caseFunction();

        c_switch->cases.reserve(i_sels.size());

        for(size_t i = 0; i < i_sels.size(); i++)
        {
          AntlrCaseBlock<Number_T>* c_case = this->caseBlockPool.allocate();
          c_case->line = i_sels[i]->getStart()->getLine();

          num_to_uint(i_sels[i]->NUMERIC_LITERAL()->getText(), c_case->match_);

          if(i_cases[i]->caseFunctionInvoke() != nullptr)
          {
            SIEVEIRParser::CaseFunctionInvokeContext* i_invoke =
              i_cases[i]->caseFunctionInvoke();
            AntlrCaseFunctionInvoke* c_invoke =
              this->caseFunctionInvokePool.allocate();
            c_invoke->line = i_invoke->getStart()->getLine();

            c_invoke->name_ = this->parseLabel(i_invoke->LABEL());
            c_invoke->inList = this->parseWireList(i_invoke->wireList());

            c_case->bodyType_ = CaseBlock<Number_T>::INVOKE;
            c_case->invoke = c_invoke;
          }
          else
          {
            SIEVEIRParser::CaseAnonFunctionContext* i_anon =
              i_cases[i]->caseAnonFunction();
            AntlrCaseAnonFunction<Number_T>* c_anon =
              this->caseAnonFunctionPool.allocate();
            c_anon->line = i_anon->getStart()->getLine();

            c_anon->inList = this->parseWireList(i_anon->wireList());
            num_to_uint(
                i_anon->NUMERIC_LITERAL(0)->getText(), c_anon->insCount);
            num_to_uint(
                i_anon->NUMERIC_LITERAL(1)->getText(), c_anon->witCount);

            AntlrDirectiveList<Number_T>* dlist =
              this->parseDirectiveList(i_anon->directiveList());

            if(dlist == nullptr) { return nullptr; }

            c_anon->directives = dlist;

            c_case->bodyType_ = CaseBlock<Number_T>::ANONYMOUS;
            c_case->anonymous = c_anon;
          }

          c_switch->cases.push_back(c_case);
        }

        dir_list->elements.emplace_back();
        dir_list->elements.back().switchStatement = c_switch;
        dir_list->types.push_back(DirectiveList<Number_T>::SWITCH_STATEMENT);
      }
      else
      {
        log_error("%s:%zu: Unrecognized directive '%s'.", this->fname,
            i_dir->getStart()->getLine(), i_dir->getText().c_str());
      }
    }

    return dir_list;
  }

  /**
   * This should return a valid value regardless, even if its empty.
   */
  AntlrWireList* parseWireList(SIEVEIRParser::WireListContext* ctx)
  {
    AntlrWireList* list = this->wireListPool.allocate();

    if(ctx == nullptr) { return list; }

    list->line = ctx->getStart()->getLine();

    std::vector<SIEVEIRParser::WireListElementContext*> i_elts =
      ctx->wireListElement();

    list->types.reserve(i_elts.size());
    list->elements.reserve(i_elts.size());

    for(SIEVEIRParser::WireListElementContext* i_elt : i_elts)
    {
      if(i_elt->wireListSingle() != nullptr)
      {
        list->elements.emplace_back();
        wire_to_uint(i_elt->wireListSingle()->WIRE_NUM()->getText(),
            list->elements.back().single);
        list->types.push_back(WireList::SINGLE);
      }
      else
      {
        AntlrWireRange* range = this->wireRangePool.allocate();
        range->line = i_elt->wireListRange()->getStart()->getLine();

        wire_to_uint(i_elt->wireListRange()->WIRE_NUM(0)->getText(),
            range->firstIdx);
        wire_to_uint(i_elt->wireListRange()->WIRE_NUM(1)->getText(),
            range->lastIdx);

        list->elements.emplace_back();
        list->elements.back().range = range;
        list->types.push_back(WireList::RANGE);
      }
    }

    return list;
  }

  char* parseLabel(antlr4::tree::TerminalNode* label)
  {
    std::string str = label->getText();
    char* ret = this->labelPool.allocate(str.size() + 1);
    ret[str.size()] = '\0';
    strncpy(ret, str.c_str(), str.size());

    return ret;
  }

  AntlrIterExpr* parseIterExpr(SIEVEIRParser::IterExprContext* ctx)
  {
    AntlrIterExpr* expr = this->iterExprPool.allocate();
    expr->line = ctx->getStart()->getLine();

    if(ctx->OP_DIV() != nullptr)
    {
      expr->div.leftHand = this->parseIterExpr(ctx->iterExpr(0));
      num_to_uint(ctx->NUMERIC_LITERAL()->getText(), expr->div.literalValue);
      expr->type_ = IterExpr::DIV;
    }
    else if(ctx->LABEL() != nullptr)
    {
      expr->name_ = this->parseLabel(ctx->LABEL());
      expr->type_ = IterExpr::ITERATOR;
    }
    else if(ctx->NUMERIC_LITERAL() != nullptr)
    {
      num_to_uint(ctx->NUMERIC_LITERAL()->getText(), expr->literalValue);
      expr->type_ = IterExpr::LITERAL;
    }
    else
    {
      expr->expr.leftHand = this->parseIterExpr(ctx->iterExpr(0));
      expr->expr.rightHand = this->parseIterExpr(ctx->iterExpr(1));

      if(ctx->OP_ADD() != nullptr)      { expr->type_ = IterExpr::ADD; }
      else if(ctx->OP_SUB() != nullptr) { expr->type_ = IterExpr::SUB; }
      else                              { expr->type_ = IterExpr::MUL; }
    }

    return expr;
  }

  /**
   * This should return a valid value regardless, even if its empty.
   */
  AntlrIterExprWireList* parseIterExprWireList(
      SIEVEIRParser::IterExprWireListContext* ctx)
  {
    AntlrIterExprWireList* list = this->iterExprWireListPool.allocate();

    if(ctx == nullptr)
    {
      return list;
    }

    list->line = ctx->getStart()->getLine();

    std::vector<SIEVEIRParser::IterExprWireListElementContext*> i_elems =
      ctx->iterExprWireListElement();

    list->elements.reserve(i_elems.size());
    list->types.reserve(i_elems.size());
    for(SIEVEIRParser::IterExprWireListElementContext* i_elem : i_elems)
    {
      if(i_elem->iterExprWireListSingle() != nullptr)
      {
        SIEVEIRParser::IterExprWireNumContext* i_num =
          i_elem->iterExprWireListSingle()->iterExprWireNum();

        AntlrIterExpr* c_elem;

        if(i_num->WIRE_NUM() != nullptr)
        {
          c_elem = this->iterExprPool.allocate();
          wire_to_uint(i_num->WIRE_NUM()->getText(), c_elem->literalValue);
          c_elem->type_ = IterExpr::LITERAL;
          c_elem->line = i_num->getStart()->getLine();
        }
        else
        {
          c_elem = this->parseIterExpr(i_num->iterExpr());
        }

        list->elements.emplace_back();
        list->elements.back().single = c_elem;
        list->types.push_back(IterExprWireList::SINGLE);
      }
      else
      {
        SIEVEIRParser::IterExprWireListRangeContext* i_range =
          i_elem->iterExprWireListRange();
        AntlrIterExprWireRange* c_range =
          this->iterExprWireRangePool.allocate();
        c_range->line = i_range->getStart()->getLine();

        SIEVEIRParser::IterExprWireNumContext* i_num_f =
          i_range->iterExprWireNum(0);
        SIEVEIRParser::IterExprWireNumContext* i_num_l =
          i_range->iterExprWireNum(1);

        if(i_num_f->WIRE_NUM() != nullptr)
        {
          AntlrIterExpr* i_expr = this->iterExprPool.allocate();
          wire_to_uint(i_num_f->WIRE_NUM()->getText(), i_expr->literalValue);
          i_expr->type_ = IterExpr::LITERAL;
          i_expr->line = i_num_f->getStart()->getLine();

          c_range->first_ = i_expr;
        }
        else
        {
          c_range->first_ = this->parseIterExpr(i_num_f->iterExpr());
        }

        if(i_num_l->WIRE_NUM() != nullptr)
        {
          AntlrIterExpr* i_expr = this->iterExprPool.allocate();
          wire_to_uint( i_num_l->WIRE_NUM()->getText(), i_expr->literalValue);
          i_expr->type_ = IterExpr::LITERAL;
          i_expr->line = i_num_l->getStart()->getLine();

          c_range->last_ = i_expr;
        }
        else
        {
          c_range->last_ = this->parseIterExpr(i_num_l->iterExpr());
        }

        list->elements.emplace_back();
        list->elements.back().range = c_range;
        list->types.push_back(IterExprWireList::RANGE);
      }
    }

    return list;
  }
};

} } // namespace wtk::antlr

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_ANTLR_TREE_PARSER_H_
