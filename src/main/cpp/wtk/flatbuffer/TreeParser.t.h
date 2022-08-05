/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_TREE_PARSER_H_
#define WTK_FLATBUFFER_TREE_PARSER_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include <wtk/utils/Pool.h>
#include <wtk/utils/hints.h>
#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/IRParameters.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>

#include <wtk/flatbuffer/FBIRTree.t.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

#define LOG_IDENTIFIER "wtk::flatbuffer"
#include <stealth_logging.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

#define NONULL(ptr) do { if(UNLIKELY((ptr) == nullptr)) \
  { \
    log_error("null is illegal"); \
    return nullptr; \
  } } while(false)

#define NONULLB(ptr) do { if(UNLIKELY((ptr) == nullptr)) \
  { \
    log_error("null is illegal"); \
    return false; \
  } } while(false)

template<typename Number_T>
struct TreeParser
{
  FBIRTree<Number_T> tree;

  GateSet const* gateSet = nullptr;
  FeatureToggles const* features = nullptr;

  TreeParser(GateSet const* gs, FeatureToggles const* ft)
    : gateSet(gs), features(ft) { }

  FBIRTree<Number_T>* parseIRTree(std::vector<Root const*>& roots)
  {
    bool seen_top_level = false;

    for(size_t i = 0; i < roots.size(); i++)
    {
      NONULL(roots[i]);
      if(roots[i]->message_type() != Message_Relation)
      {
        log_error("not a relation");
        return nullptr;
      }

      NONULL(roots[i]->message_as_Relation());
      Relation const* relation = roots[i]->message_as_Relation();

      NONULL(relation->functions());
      if(relation->functions()->size() != 0)
      {
        if(!this->features->functionToggle)
        {
          log_error("functions disabled");
          return nullptr;
        }
        if(seen_top_level)
        {
          log_error("functions out of place in sequence");
          return nullptr;
        }

        for(size_t j = 0; j < relation->functions()->size(); j++)
        {
          Function const* function = relation->functions()->Get(j);
          NONULL(function->name());
          NONULL(function->name()->c_str());

          this->tree.functions.emplace_back(function);
          FBFunctionDeclare<Number_T>* func_decl =
            &this->tree.functions.back();

          if(!this->parseDirectiveList(function->body(), &func_decl->bodyList))
          {
            return nullptr;
          }
        }
      }

      NONULL(relation->directives());
      if(relation->directives()->size() > 0)
      {
        seen_top_level = true;

        if(!this->parseDirectiveList(
              relation->directives(), &this->tree.bodyList))
        {
          return nullptr;
        }
      }
    }

    return &this->tree;
  }

  wtk::utils::Pool<FBAssign<Number_T>> assignPool;
  wtk::utils::Pool<FBTerminal> terminalPool;
  wtk::utils::Pool<FBWireRange> wireRangePool;
  wtk::utils::Pool<FBUnaryGate> unaryGatePool;
  wtk::utils::Pool<FBBinaryGate> binaryGatePool;
  wtk::utils::Pool<FBBinaryConstGate<Number_T>> binaryConstGatePool;
  wtk::utils::Pool<FBInput> inputPool;
  wtk::utils::Pool<FBFunctionInvoke> functionInvokePool;
  wtk::utils::Pool<FBAnonFunction<Number_T>> anonFunctionPool;
  wtk::utils::Pool<FBDirectiveList<Number_T>> directiveListPool;
  wtk::utils::Pool<FBSwitchStatement<Number_T>> switchStatementPool;
  wtk::utils::Pool<FBCaseFunctionInvoke> caseFunctionInvokePool;
  wtk::utils::Pool<FBCaseAnonFunction<Number_T>> caseAnonFunctionPool;
  wtk::utils::Pool<FBIterExpr> iterExprPool;
  wtk::utils::Pool<FBIterExprWireRange> iterExprWireRangePool;
  wtk::utils::Pool<FBIterExprFunctionInvoke> iterExprFunctionInvokePool;
  wtk::utils::Pool<FBIterExprAnonFunction<Number_T>> iterExprAnonFunctionPool;
  wtk::utils::Pool<FBForLoop<Number_T>> forLoopPool;

  bool parseDirectiveList(
      flatbuffers::Vector<flatbuffers::Offset<Directive>> const* fb_directives,
      FBDirectiveList<Number_T>* directives)
  {
    NONULLB(fb_directives);
    for(size_t i = 0; i < fb_directives->size(); i++)
    {
      Directive const* directive = fb_directives->Get(i);
      switch(directive->directive_type())
      {
      case DirectiveSet_GateConstant:
      {
        GateConstant const* gate = directive->directive_as_GateConstant();
        NONULLB(gate->output());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        directives->types.push_back(wtk::DirectiveList<Number_T>::ASSIGN);
        directives->elements.emplace_back();
        directives->elements.back().assign = this->assignPool.allocate();
        directives->elements.back().assign->gateConstant = gate;
        break;
      }
      case DirectiveSet_GateAssertZero:
      {
        GateAssertZero const* gate = directive->directive_as_GateAssertZero();
        NONULLB(gate->input());

        directives->types.push_back(wtk::DirectiveList<Number_T>::ASSERT_ZERO);
        directives->elements.emplace_back();
        directives->elements.back().terminal = this->terminalPool.allocate();
        directives->elements.back().terminal->assertZero = gate;
        directives->elements.back().terminal->isDelete = false;
        break;
      }
      case DirectiveSet_GateCopy:
      {
        GateCopy const* gate = directive->directive_as_GateCopy();
        NONULLB(gate->input());
        NONULLB(gate->output());

        directives->types.push_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().unaryGate = this->unaryGatePool.allocate();
        directives->elements.back().unaryGate->gateCopy = gate;
        directives->elements.back().unaryGate->calc =
          wtk::UnaryGate::COPY;
        break;
      }
      case DirectiveSet_GateAdd:
      {
        if(this->gateSet->gateSet != GateSet::arithmetic
            || !this->gateSet->enableAdd)
        {
          log_error("add prohibited");
          return false;
        }

        GateAdd const* gate = directive->directive_as_GateAdd();
        NONULLB(gate->output());
        NONULLB(gate->left());
        NONULLB(gate->right());

        directives->types.push_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryGate =
          this->binaryGatePool.allocate();
        directives->elements.back().binaryGate->gateAdd = gate;
        directives->elements.back().binaryGate->calc =
          wtk::BinaryGate::ADD;
        break;
      }
      case DirectiveSet_GateMul:
      {
        if(this->gateSet->gateSet != GateSet::arithmetic
            || !this->gateSet->enableMul)
        {
          log_error("mul prohibited");
          return false;
        }

        GateMul const* gate = directive->directive_as_GateMul();
        NONULLB(gate->output());
        NONULLB(gate->left());
        NONULLB(gate->right());

        directives->types.push_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryGate =
          this->binaryGatePool.allocate();
        directives->elements.back().binaryGate->gateMul = gate;
        directives->elements.back().binaryGate->calc =
          wtk::BinaryGate::MUL;
        break;
      }
      case DirectiveSet_GateAddConstant:
      {
        if(this->gateSet->gateSet != GateSet::arithmetic
            || !this->gateSet->enableAddC)
        {
          log_error("addc prohibited");
          return false;
        }

        GateAddConstant const* gate =
          directive->directive_as_GateAddConstant();
        NONULLB(gate->output());
        NONULLB(gate->input());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        directives->types.push_back(
            wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryConstGate =
          this->binaryConstGatePool.allocate();
        directives->elements.back().binaryConstGate->gateAddC = gate;
        directives->elements.back().binaryConstGate->calc =
          wtk::BinaryConstGate<Number_T>::ADDC;
        break;
      }
      case DirectiveSet_GateMulConstant:
      {
        if(this->gateSet->gateSet != GateSet::arithmetic
            || !this->gateSet->enableMulC)
        {
          log_error("mulc prohibited");
          return false;
        }

        GateMulConstant const* gate =
          directive->directive_as_GateMulConstant();
        NONULLB(gate->output());
        NONULLB(gate->input());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        directives->types.push_back(
            wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryConstGate =
          this->binaryConstGatePool.allocate();
        directives->elements.back().binaryConstGate->gateMulC = gate;
        directives->elements.back().binaryConstGate->calc =
          wtk::BinaryConstGate<Number_T>::MULC;
        break;
      }
      case DirectiveSet_GateAnd:
      {
        if(this->gateSet->gateSet != GateSet::boolean
            || !this->gateSet->enableAnd)
        {
          log_error("and prohibited");
          return false;
        }

        GateAnd const* gate = directive->directive_as_GateAnd();
        NONULLB(gate->output());
        NONULLB(gate->left());
        NONULLB(gate->right());

        directives->types.push_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryGate =
          this->binaryGatePool.allocate();
        directives->elements.back().binaryGate->gateAnd = gate;
        directives->elements.back().binaryGate->calc =
          wtk::BinaryGate::AND;
        break;
      }
      case DirectiveSet_GateXor:
      {
        if(this->gateSet->gateSet != GateSet::boolean
            || !this->gateSet->enableXor)
        {
          log_error("xor prohibited");
          return false;
        }

        GateXor const* gate = directive->directive_as_GateXor();
        NONULLB(gate->output());
        NONULLB(gate->left());
        NONULLB(gate->right());

        directives->types.push_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().binaryGate =
          this->binaryGatePool.allocate();
        directives->elements.back().binaryGate->gateXor = gate;
        directives->elements.back().binaryGate->calc =
          wtk::BinaryGate::XOR;
        break;
      }
      case DirectiveSet_GateNot:
      {
        if(this->gateSet->gateSet != GateSet::boolean
            || !this->gateSet->enableNot)
        {
          log_error("not prohibited");
          return false;
        }

        GateNot const* gate = directive->directive_as_GateNot();
        NONULLB(gate->input());
        NONULLB(gate->output());

        directives->types.push_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
        directives->elements.emplace_back();
        directives->elements.back().unaryGate = this->unaryGatePool.allocate();
        directives->elements.back().unaryGate->gateNot = gate;
        directives->elements.back().unaryGate->calc =
          wtk::UnaryGate::NOT;
        break;
      }
      case DirectiveSet_GateInstance:
      {
        GateInstance const* gate = directive->directive_as_GateInstance();
        NONULLB(gate->output());

        directives->types.push_back(wtk::DirectiveList<Number_T>::INPUT);
        directives->elements.emplace_back();
        directives->elements.back().input = this->inputPool.allocate();
        directives->elements.back().input->gateInstance = gate;
        directives->elements.back().input->type = wtk::Input::INSTANCE;
        break;
      }
      case DirectiveSet_GateWitness:
      {
        GateWitness const* gate = directive->directive_as_GateWitness();
        NONULLB(gate->output());

        directives->types.push_back(wtk::DirectiveList<Number_T>::INPUT);
        directives->elements.emplace_back();
        directives->elements.back().input = this->inputPool.allocate();
        directives->elements.back().input->gateWitness = gate;
        directives->elements.back().input->type = wtk::Input::SHORT_WITNESS;
        break;
      }
      case DirectiveSet_GateFree:
      {
        GateFree const* gate = directive->directive_as_GateFree();
        NONULLB(gate->first());

        if(gate->last() == nullptr)
        {
          directives->types.push_back(
              wtk::DirectiveList<Number_T>::DELETE_SINGLE);
          directives->elements.emplace_back();
          directives->elements.back().terminal = this->terminalPool.allocate();
          directives->elements.back().terminal->gateFree = gate;
          directives->elements.back().terminal->isDelete = true;
        }
        else
        {
          directives->types.push_back(
              wtk::DirectiveList<Number_T>::DELETE_RANGE);
          directives->elements.emplace_back();
          directives->elements.back().wireRange =
            this->wireRangePool.allocate();
          directives->elements.back().wireRange->gateFree = gate;
          directives->elements.back().wireRange->isDelete = true;
        }
        break;
      }
      case DirectiveSet_GateCall:
      {
        if(!this->features->functionToggle)
        {
          log_error("function prohibited");
          return false;
        }

        GateCall const* gate = directive->directive_as_GateCall();
        NONULLB(gate->name());
        NONULLB(gate->name()->c_str());

        FBFunctionInvoke* invoke = this->functionInvokePool.allocate();
        invoke->name_ = gate->name()->c_str();
        if(!this->parseWireList(gate->output_wires(), &invoke->outputs))
        {
          return false;
        }
        if(!this->parseWireList(gate->input_wires(), &invoke->inputs))
        {
          return false;
        }

        directives->types.push_back(
            wtk::DirectiveList<Number_T>::FUNCTION_INVOKE);
        directives->elements.emplace_back();
        directives->elements.back().functionInvoke = invoke;
        break;
      }
      case DirectiveSet_GateAnonCall:
      {
        if(!this->features->functionToggle)
        {
          log_error("function prohibited");
          return false;
        }

        GateAnonCall const* gate = directive->directive_as_GateAnonCall();
        NONULLB(gate->inner());
        NONULLB(gate->inner()->subcircuit());

        FBAnonFunction<Number_T>* anon = this->anonFunctionPool.allocate();
        anon->gateAnonCall = gate;
        if(!this->parseWireList(gate->output_wires(), &anon->outputs))
        {
          return false;
        }
        if(!this->parseWireList(gate->inner()->input_wires(), &anon->inputs))
        {
          return false;
        }
        anon->bodyList = this->directiveListPool.allocate();
        if(!this->parseDirectiveList(
              gate->inner()->subcircuit(), anon->bodyList))
        {
          return false;
        }

        directives->types.push_back(
            wtk::DirectiveList<Number_T>::ANON_FUNCTION);
        directives->elements.emplace_back();
        directives->elements.back().anonFunction = anon;
        break;
      }
      case DirectiveSet_GateSwitch:
      {
        if(!this->features->switchCaseToggle)
        {
          log_error("switch prohibited");
          return false;
        }

        GateSwitch const* gate = directive->directive_as_GateSwitch();
        NONULLB(gate->condition());
        NONULLB(gate->cases());
        NONULLB(gate->branches());

        FBSwitchStatement<Number_T>* switch_stmt =
          this->switchStatementPool.allocate();
        switch_stmt->cond = gate->condition();
        if(!this->parseWireList(gate->output_wires(), &switch_stmt->outputs))
        {
          return false;
        }

        if(gate->cases()->size() != gate->branches()->size()
            || gate->cases()->size() == 0)
        {
          log_error("switch case size mismatch");
          return false;
        }
        else
        {
          for(size_t i = 0; i < gate->cases()->size(); i++)
          {
            switch_stmt->cases.emplace_back();
            NONULLB(gate->cases()->Get(i));
            if(!checkFlatNumber(gate->cases()->Get(i)->value()))
            {
              return false;
            }
            switch_stmt->cases.back().value = gate->cases()->Get(i);

            NONULLB(gate->branches()->Get(i));
            NONULLB(gate->branches()->Get(i)->invocation());
            switch(gate->branches()->Get(i)->invocation_type())
            {
            case CaseInvokeU_AbstractGateCall:
            {
              AbstractGateCall const* call =
                gate->branches()->Get(i)->invocation_as_AbstractGateCall();
              NONULLB(call->name());
              NONULLB(call->name()->c_str());

              FBCaseFunctionInvoke* invoke =
                this->caseFunctionInvokePool.allocate();
              invoke->name_ = call->name()->c_str();
              if(!this->parseWireList(call->input_wires(), &invoke->inputs))
              {
                return false;
              }

              switch_stmt->cases.back().invokeFunction = invoke;
              switch_stmt->cases.back().type =
                wtk::CaseBlock<Number_T>::INVOKE;
              break;
            }
            case CaseInvokeU_AbstractAnonCall:
            {
              /* TODO */
              AbstractAnonCall const* call =
                gate->branches()->Get(i)->invocation_as_AbstractAnonCall();
              NONULLB(call->subcircuit());

              FBCaseAnonFunction<Number_T>* anon =
                this->caseAnonFunctionPool.allocate();
              anon->abstractAnonCall = call;
              if(!this->parseWireList(call->input_wires(), &anon->inputs))
              {
                return false;
              }
              anon->bodyList = this->directiveListPool.allocate();
              if(!this->parseDirectiveList(call->subcircuit(), anon->bodyList))
              {
                return false;
              }

              switch_stmt->cases.back().anonFunction = anon;
              switch_stmt->cases.back().type =
                wtk::CaseBlock<Number_T>::ANONYMOUS;
              break;
            }
            default:
            {
              log_error("unrecognized case type");
              return false;
            }
            }
          }

        }

        directives->types.push_back(
            wtk::DirectiveList<Number_T>::SWITCH_STATEMENT);
        directives->elements.emplace_back();
        directives->elements.back().switchStatement = switch_stmt;
        break;
      }
      case DirectiveSet_GateFor:
      {
        if(!this->features->forLoopToggle)
        {
          log_error("loop prohibited");
          return false;
        }

        GateFor const* gate = directive->directive_as_GateFor();
        NONULLB(gate->iterator());
        NONULLB(gate->iterator()->c_str());
        NONULLB(gate->body());

        FBForLoop<Number_T>* for_loop = this->forLoopPool.allocate();
        for_loop->gateFor = gate;

        if(!this->parseWireList(gate->outputs(), &for_loop->outputs))
        {
          return false;
        }

        switch(gate->body_type())
        {
        case ForLoopBody_IterExprFunctionInvoke:
        {
          wtk_gen_flatbuffer::IterExprFunctionInvoke const* fb_invoke =
            gate->body_as_IterExprFunctionInvoke();

          NONULLB(fb_invoke);
          NONULLB(fb_invoke->name());
          NONULLB(fb_invoke->name()->c_str());

          FBIterExprFunctionInvoke* invoke =
            this->iterExprFunctionInvokePool.allocate();
          invoke->name_ =fb_invoke->name()->c_str();
          if(!this->parseIterExprWireList(
                fb_invoke->outputs(), &invoke->outputs))
          {
            return false;
          }
          if(!this->parseIterExprWireList(
                fb_invoke->inputs(), &invoke->inputs))
          {
            return false;
          }

          for_loop->functionInvokeBody = invoke;
          break;
        }
        case ForLoopBody_IterExprAnonFunction:
        {
          wtk_gen_flatbuffer::IterExprAnonFunction const* fb_anon =
            gate->body_as_IterExprAnonFunction();

          NONULLB(fb_anon);

          FBIterExprAnonFunction<Number_T>* anon =
            this->iterExprAnonFunctionPool.allocate();
          anon->anonFunction = fb_anon;
          if(!this->parseIterExprWireList(fb_anon->outputs(), &anon->outputs))
          {
            return false;
          }
          if(!this->parseIterExprWireList(fb_anon->inputs(), &anon->inputs))
          {
            return false;
          }

          NONULLB(fb_anon->body());
          anon->bodyList = this->directiveListPool.allocate();
          if(!this->parseDirectiveList(fb_anon->body(), anon->bodyList))
          {
            return false;
          }

          for_loop->anonFunctionBody = anon;
          break;
        }
        default:
        {
          log_error("unrecognized loop body");
          return false;
        }
        }

        directives->types.push_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
        directives->elements.emplace_back();
        directives->elements.back().forLoop = for_loop;
        break;
      }
      default:
      {
        log_error("unrecognized directive");
        return false;
      }
      }
    }

    return true;
  }

  bool parseWireList(
      wtk_gen_flatbuffer::WireList const* fb_list, FBWireList* list)
  {
    NONULLB(fb_list);
    NONULLB(fb_list->elements());
    for(size_t i = 0; i < fb_list->elements()->size(); i++)
    {
      WireListElement const* fb_element = fb_list->elements()->Get(i);
      NONULLB(fb_element);

      switch(fb_element->element_type())
      {
      case WireListElementU_Wire:
      {
        NONULLB(fb_element->element_as_Wire());
        list->elements.emplace_back();
        list->elements.back().single = fb_element->element_as_Wire();
        list->types.push_back(wtk::WireList::SINGLE);
        break;
      }
      case WireListElementU_WireRange:
      {
        NONULLB(fb_element->element_as_WireRange());
        NONULLB(fb_element->element_as_WireRange()->first());
        NONULLB(fb_element->element_as_WireRange()->last());

        list->elements.emplace_back();
        list->elements.back().range = this->wireRangePool.allocate();
        list->elements.back().range->isDelete = false;
        list->elements.back().range->range =
          fb_element->element_as_WireRange();
        list->types.push_back(wtk::WireList::RANGE);
        break;
      }
      default:
      {
        log_error("unrecognized list element");
        return false;
      }
      }
    }

    return true;
  }

  FBIterExpr* parseIterExpr(
      wtk_gen_flatbuffer::IterExprWireNumber const* fb_expr)
  {
    NONULL(fb_expr);
    NONULL(fb_expr->value());

    FBIterExpr* expr = this->iterExprPool.allocate();

    switch(fb_expr->value_type())
    {
    case IterExpr_IterExprConst:
    {
      expr->type_ = wtk::IterExpr::LITERAL;
      expr->literalValue = fb_expr->value_as_IterExprConst()->value();
      break;
    }
    case IterExpr_IterExprName:
    {
      expr->type_ = wtk::IterExpr::ITERATOR;
      NONULL(fb_expr->value_as_IterExprName()->name());
      NONULL(fb_expr->value_as_IterExprName()->name()->c_str());
      expr->name_ = fb_expr->value_as_IterExprName()->name()->c_str();
      break;
    }
    case IterExpr_IterExprAdd:
    {
      expr->type_ = wtk::IterExpr::ADD;
      FBIterExpr* l =
        this->parseIterExpr(fb_expr->value_as_IterExprAdd()->left());
      FBIterExpr* r =
        this->parseIterExpr(fb_expr->value_as_IterExprAdd()->right());

      NONULL(l);
      NONULL(r);

      expr->expr.leftHand = l;
      expr->expr.rightHand = r;
      break;
    }
    case IterExpr_IterExprSub:
    {
      expr->type_ = wtk::IterExpr::SUB;
      FBIterExpr* l =
        this->parseIterExpr(fb_expr->value_as_IterExprSub()->left());
      FBIterExpr* r =
        this->parseIterExpr(fb_expr->value_as_IterExprSub()->right());

      NONULL(l);
      NONULL(r);

      expr->expr.leftHand = l;
      expr->expr.rightHand = r;
      break;
    }
    case IterExpr_IterExprMul:
    {
      expr->type_ = wtk::IterExpr::MUL;
      FBIterExpr* l =
        this->parseIterExpr(fb_expr->value_as_IterExprMul()->left());
      FBIterExpr* r =
        this->parseIterExpr(fb_expr->value_as_IterExprMul()->right());

      NONULL(l);
      NONULL(r);

      expr->expr.leftHand = l;
      expr->expr.rightHand = r;
      break;
    }
    case IterExpr_IterExprDivConst:
    {
      expr->type_ = wtk::IterExpr::DIV;
      FBIterExpr* l =
        this->parseIterExpr(fb_expr->value_as_IterExprDivConst()->numer());

      NONULL(l);

      expr->div.leftHand = l;
      expr->div.literalValue = fb_expr->value_as_IterExprDivConst()->denom();
      break;
    }
    default:
    {
      log_error("unrecognized iterator expression");
      return nullptr;
    }
    }

    return expr;
  };

  bool parseIterExprWireList(
      wtk_gen_flatbuffer::IterExprWireList const* fb_list,
      FBIterExprWireList* list)
  {
    NONULLB(fb_list);
    NONULLB(fb_list->elements());
    for(size_t i = 0; i < fb_list->elements()->size(); i++)
    {
      IterExprWireListElement const* fb_element = fb_list->elements()->Get(i);
      NONULLB(fb_element);

      switch(fb_element->element_type())
      {
      case IterExprWireListElementU_IterExprWireNumber:
      {
        NONULLB(fb_element->element_as_IterExprWireNumber());
        list->elements.emplace_back();
        list->elements.back().single =
          this->parseIterExpr(fb_element->element_as_IterExprWireNumber());
        list->types.push_back(wtk::IterExprWireList::SINGLE);
        break;
      }
      case IterExprWireListElementU_IterExprWireRange:
      {
        NONULLB(fb_element->element_as_IterExprWireRange());
        NONULLB(fb_element->element_as_IterExprWireRange()->first());
        NONULLB(fb_element->element_as_IterExprWireRange()->last());

        list->elements.emplace_back();
        list->elements.back().range = this->iterExprWireRangePool.allocate();
        list->elements.back().range->first_ = this->parseIterExpr(
            fb_element->element_as_IterExprWireRange()->first());
        list->elements.back().range->last_ = this->parseIterExpr(
            fb_element->element_as_IterExprWireRange()->last());
        list->types.push_back(wtk::IterExprWireList::RANGE);

        NONULLB(list->elements.back().range->first_);
        NONULLB(list->elements.back().range->last_);
        break;
      }
      default:
      {
        log_error("unrecognized iterator expresion list element");
        return false;
      }
      }
    }

    return true;
  }
};

#undef NONULL
#undef NONULLB

} } // namespace wtk::flatbuffer

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_TREE_PARSER_H_
