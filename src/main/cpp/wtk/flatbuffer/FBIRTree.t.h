/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_IR_TREE_H_
#define WTK_FLATBUFFER_IR_TREE_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include <wtk/index.h>
#include <wtk/IRTree.h>

#include <wtk/flatbuffer/FlatNumberHelper.t.h>
#include <wtk/flatbuffer/sieve_ir_generated.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
struct FBAssign : public wtk::Assign<Number_T>
{
  GateConstant const* gateConstant = nullptr;

  index_t outputWire() override
  {
    return this->gateConstant->output()->id();
  }

  Number_T constValue() override
  {
    return flatToNumber<Number_T>(this->gateConstant->constant());
  }
};

struct FBTerminal : public wtk::Terminal
{
  bool isDelete = false;
  union
  {
    GateAssertZero const* assertZero;
    GateFree const* gateFree;
  };

  index_t wire() override
  {
    if(this->isDelete)
    {
      return this->gateFree->first()->id();
    }
    else
    {
      return this->assertZero->input()->id();
    }
  }
};

struct FBWireRange : public wtk::WireRange
{
  bool isDelete = false;
  union
  {
    wtk_gen_flatbuffer::WireRange const* range;
    GateFree const* gateFree;
  };

  index_t first() override
  {
    if(this->isDelete)
    {
      return this->gateFree->first()->id();
    }
    else
    {
      return this->range->first()->id();
    }
  }

  index_t last() override
  {
    if(this->isDelete)
    {
      return this->gateFree->last()->id();
    }
    else
    {
      return this->range->last()->id();
    }
  }
};

struct FBUnaryGate : public wtk::UnaryGate
{
  wtk::UnaryGate::Calculation calc;

  union
  {
    GateCopy const* gateCopy;
    GateNot const* gateNot;
  };

  wtk::UnaryGate::Calculation calculation() override
  {
    return this->calc;
  }

  index_t outputWire() override
  {
    switch(this->calc)
    {
    case wtk::UnaryGate::Calculation::COPY:
    {
      return this->gateCopy->output()->id();
    }
    case wtk::UnaryGate::Calculation::NOT:
    {
      return this->gateNot->output()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }

  index_t inputWire() override
  {
    switch(this->calc)
    {
    case wtk::UnaryGate::Calculation::COPY:
    {
      return this->gateCopy->input()->id();
    }
    case wtk::UnaryGate::Calculation::NOT:
    {
      return this->gateNot->input()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }
};

struct FBBinaryGate : public wtk::BinaryGate
{
  wtk::BinaryGate::Calculation calc;

  union
  {
    GateAdd const* gateAdd;
    GateMul const* gateMul;
    GateAnd const* gateAnd;
    GateXor const* gateXor;
  };

  wtk::BinaryGate::Calculation calculation() override
  {
    return this->calc;
  }

  index_t outputWire() override
  {
    switch(this->calc)
    {
    case wtk::BinaryGate::Calculation::ADD:
    {
      return this->gateAdd->output()->id();
    }
    case wtk::BinaryGate::Calculation::MUL:
    {
      return this->gateMul->output()->id();
    }
    case wtk::BinaryGate::Calculation::AND:
    {
      return this->gateAnd->output()->id();
    }
    case wtk::BinaryGate::Calculation::XOR:
    {
      return this->gateXor->output()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }

  index_t leftWire() override
  {
    switch(this->calc)
    {
    case wtk::BinaryGate::Calculation::ADD:
    {
      return this->gateAdd->left()->id();
    }
    case wtk::BinaryGate::Calculation::MUL:
    {
      return this->gateMul->left()->id();
    }
    case wtk::BinaryGate::Calculation::AND:
    {
      return this->gateAnd->left()->id();
    }
    case wtk::BinaryGate::Calculation::XOR:
    {
      return this->gateXor->left()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }

  index_t rightWire() override
  {
    switch(this->calc)
    {
    case wtk::BinaryGate::Calculation::ADD:
    {
      return this->gateAdd->right()->id();
    }
    case wtk::BinaryGate::Calculation::MUL:
    {
      return this->gateMul->right()->id();
    }
    case wtk::BinaryGate::Calculation::AND:
    {
      return this->gateAnd->right()->id();
    }
    case wtk::BinaryGate::Calculation::XOR:
    {
      return this->gateXor->right()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }
};

template<typename Number_T>
struct FBBinaryConstGate : public wtk::BinaryConstGate<Number_T>
{
  typename wtk::BinaryConstGate<Number_T>::Calculation calc;

  union
  {
    GateAddConstant const* gateAddC;
    GateMulConstant const* gateMulC;
  };

  typename wtk::BinaryConstGate<Number_T>::Calculation calculation() override
  {
    return this->calc;
  }

  index_t outputWire() override
  {
    switch(this->calc)
    {
    case wtk::BinaryConstGate<Number_T>::Calculation::ADDC:
    {
      return this->gateAddC->output()->id();
    }
    case wtk::BinaryConstGate<Number_T>::Calculation::MULC:
    {
      return this->gateMulC->output()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }

  index_t leftWire() override
  {
    switch(this->calc)
    {
    case wtk::BinaryConstGate<Number_T>::Calculation::ADDC:
    {
      return this->gateAddC->input()->id();
    }
    case wtk::BinaryConstGate<Number_T>::Calculation::MULC:
    {
      return this->gateMulC->input()->id();
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }

  Number_T rightValue() override
  {
    switch(this->calc)
    {
    case wtk::BinaryConstGate<Number_T>::Calculation::ADDC:
    {
      return flatToNumber<Number_T>(this->gateAddC->constant());
    }
    case wtk::BinaryConstGate<Number_T>::Calculation::MULC:
    {
      return flatToNumber<Number_T>(this->gateMulC->constant());
    }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }
};

struct FBInput : public wtk::Input
{
  wtk::Input::Stream type;

  union
  {
    GateInstance const* gateInstance;
    GateWitness const* gateWitness;
  };

  wtk::Input::Stream stream() override
  {
    return this->type;
  }

  index_t outputWire() override
  {
    switch(this->type)
    {
      case wtk::Input::INSTANCE:
      {
        return this->gateInstance->output()->id();
      }
      case wtk::Input::SHORT_WITNESS:
      {
        return this->gateWitness->output()->id();
      }
    }

    // GCC says this is reachable, but it shouldn't be
    return 0;
  }
};

struct FBWireList : public wtk::WireList
{
  union Element
  {
    Wire const* single = nullptr;
    FBWireRange* range;
  };

  std::vector<wtk::WireList::Type> types;
  std::vector<Element> elements;

  size_t size() override
  {
    return this->elements.size();
  }

  wtk::WireList::Type type(size_t n) override
  {
    return this->types[n];
  }

  index_t single(size_t n) override
  {
    return this->elements[n].single->id();
  }

  FBWireRange* range(size_t n) override
  {
    return this->elements[n].range;
  }
};

struct FBFunctionInvoke : public wtk::FunctionInvoke
{
  FBWireList outputs;
  FBWireList inputs;
  char const* name_;

  char const* name() override
  {
    return this->name_;
  }

  FBWireList* outputList() override
  {
    return &this->outputs;
  }

  FBWireList* inputList() override
  {
    return &this->inputs;
  }
};

template<typename Number_T>
struct FBDirectiveList;

template<typename Number_T>
struct FBAnonFunction : public wtk::AnonFunction<Number_T>
{
  FBWireList outputs;
  FBWireList inputs;
  FBDirectiveList<Number_T>* bodyList;
  GateAnonCall const* gateAnonCall = nullptr;

  FBWireList* outputList() override
  {
    return &this->outputs;
  }

  FBWireList* inputList() override
  {
    return &this->inputs;
  }

  index_t instanceCount() override
  {
    return this->gateAnonCall->inner()->instance_count();
  }

  index_t shortWitnessCount() override
  {
    return this->gateAnonCall->inner()->witness_count();
  }

  FBDirectiveList<Number_T>* body() override
  {
    return this->bodyList;
  }
};

struct FBCaseFunctionInvoke : public wtk::CaseFunctionInvoke
{
  FBWireList inputs;
  char const* name_;

  char const* name() override
  {
    return this->name_;
  }

  FBWireList* inputList() override
  {
    return &this->inputs;
  }
};

template<typename Number_T>
struct FBCaseAnonFunction : public wtk::CaseAnonFunction<Number_T>
{
  FBWireList inputs;
  FBDirectiveList<Number_T>* bodyList;
  AbstractAnonCall const* abstractAnonCall = nullptr;

  FBWireList* inputList() override
  {
    return &this->inputs;
  }

  index_t instanceCount() override
  {
    return this->abstractAnonCall->instance_count();
  }

  index_t shortWitnessCount() override
  {
    return this->abstractAnonCall->witness_count();
  }

  FBDirectiveList<Number_T>* body() override
  {
    return this->bodyList;
  }
};

template<typename Number_T>
struct FBCaseBlock : public wtk::CaseBlock<Number_T>
{
  Value const* value = nullptr;

  union
  {
    FBCaseFunctionInvoke* invokeFunction;
    FBCaseAnonFunction<Number_T>* anonFunction;
  };

  typename wtk::CaseBlock<Number_T>::BodyType type;

  Number_T match() override
  {
    return flatToNumber<Number_T>(this->value->value());
  }

  typename wtk::CaseBlock<Number_T>::BodyType bodyType() override
  {
    return this->type;
  }

  FBCaseFunctionInvoke* invokeBody() override
  {
    return this->invokeFunction;
  }

  FBCaseAnonFunction<Number_T>* anonymousBody() override
  {
    return this->anonFunction;
  }
};

template<typename Number_T>
struct FBSwitchStatement : public wtk::SwitchStatement<Number_T>
{
  Wire const* cond;
  FBWireList outputs;

  std::vector<FBCaseBlock<Number_T>> cases;

  FBWireList* outputList() override
  {
    return &this->outputs;
  }

  index_t condition() override
  {
    return this->cond->id();
  }

  size_t size() override
  {
    return this->cases.size();
  }

  FBCaseBlock<Number_T>* caseBlock(size_t n) override
  {
    return &this->cases[n];
  }
};

struct FBIterExpr : public IterExpr
{
  union
  {
    wtk::index_t literalValue;
    char const* name_;
    struct
    {
      FBIterExpr* leftHand;
      FBIterExpr* rightHand;
    } expr;
    struct
    {
      FBIterExpr* leftHand = nullptr;
      wtk::index_t literalValue = 0;
    } div;
  };

  Type type_;

  FBIterExpr() : type_()
  {
    this->div.leftHand = nullptr;
    this->div.literalValue = 0;
  }

  Type type() override { return this->type_; }

  wtk::index_t literal() override
  {
    if(this->type_ == Type::DIV) { return this->div.literalValue; }
    return this->literalValue;
  }

  char const* name() override { return this->name_; }

  FBIterExpr* lhs() override
  {
    if(this->type_ == Type::DIV) { return this->div.leftHand; }
    return this->expr.leftHand;
  }

  FBIterExpr* rhs() override { return this->expr.rightHand; }
};

struct FBIterExprWireRange : public IterExprWireRange
{
  FBIterExpr* first_ = nullptr;
  FBIterExpr* last_  = nullptr;

  FBIterExpr* first() override { return this->first_; }
  FBIterExpr* last() override { return this->last_; }
};

struct FBIterExprWireList : public IterExprWireList
{
  union Element
  {
    FBIterExpr* single = nullptr;
    FBIterExprWireRange* range;
  };

  std::vector<Type> types;
  std::vector<Element> elements;

  size_t size() override { return this->types.size(); }

  Type type(size_t n) override { return this->types[n]; }

  FBIterExpr* single(size_t n) override { return this->elements[n].single; }

  FBIterExprWireRange* range(size_t n) override
  {
    return this->elements[n].range;
  }
};

struct FBIterExprFunctionInvoke : public wtk::IterExprFunctionInvoke
{
  char const* name_ = nullptr;
  FBIterExprWireList outputs;
  FBIterExprWireList inputs;

  char const* name() { return this->name_; }

  FBIterExprWireList* outputList() { return &this->outputs; }

  FBIterExprWireList* inputList() { return &this->inputs; }
};

template<typename Number_T>
struct FBIterExprAnonFunction : public wtk::IterExprAnonFunction<Number_T>
{
  wtk_gen_flatbuffer::IterExprAnonFunction const* anonFunction = nullptr;

  FBIterExprWireList outputs;
  FBIterExprWireList inputs;

  FBDirectiveList<Number_T>* bodyList;

  FBIterExprWireList* outputList() { return &this->outputs; }

  FBIterExprWireList* inputList() { return &this->inputs; }

  index_t instanceCount() { return this->anonFunction->instance_count(); }

  index_t shortWitnessCount() { return this->anonFunction->witness_count(); }

  FBDirectiveList<Number_T>* body() { return this->bodyList; }
};

template<typename Number_T>
struct FBForLoop : public wtk::ForLoop<Number_T>
{
  GateFor const* gateFor = nullptr;
  FBWireList outputs;

  union
  {
    FBIterExprFunctionInvoke* functionInvokeBody = nullptr;
    FBIterExprAnonFunction<Number_T>* anonFunctionBody;
  };

  FBWireList* outputList() { return &this->outputs; }

  char const* iterName() { return this->gateFor->iterator()->c_str(); }

  index_t first() { return this->gateFor->first(); }

  index_t last() { return this->gateFor->last(); }

  typename wtk::ForLoop<Number_T>::BodyType bodyType()
  {
    switch(this->gateFor->body_type())
    {
    case ForLoopBody_IterExprFunctionInvoke:
    {
      return wtk::ForLoop<Number_T>::INVOKE;
    }
    case ForLoopBody_IterExprAnonFunction:
    {
      return wtk::ForLoop<Number_T>::ANONYMOUS;
    }
    default:
    {
      /* case shouldn't be reached because it's checked by the parser */
      return wtk::ForLoop<Number_T>::INVOKE;
    }
    }
  }

  FBIterExprFunctionInvoke* invokeBody()
  {
    return this->functionInvokeBody;
  }

  FBIterExprAnonFunction<Number_T>* anonymousBody()
  {
    return this->anonFunctionBody;
  }
};

template<typename Number_T>
struct FBDirectiveList : public wtk::DirectiveList<Number_T>
{
  union Element
  {
    FBAssign<Number_T>* assign = nullptr;
    FBTerminal* terminal;
    FBWireRange* wireRange;
    FBUnaryGate* unaryGate;
    FBBinaryGate* binaryGate;
    FBBinaryConstGate<Number_T>* binaryConstGate;
    FBInput* input;
    FBFunctionInvoke* functionInvoke;
    FBAnonFunction<Number_T>* anonFunction;
    FBSwitchStatement<Number_T>* switchStatement;
    FBForLoop<Number_T>* forLoop;
  };

  std::vector<typename wtk::DirectiveList<Number_T>::Type> types;
  std::vector<Element> elements;

  size_t size() override
  {
    return this->elements.size();
  }

  typename wtk::DirectiveList<Number_T>::Type type(size_t n) override
  {
    return this->types[n];
  }

  FBBinaryGate* binaryGate(size_t n) override
  {
    return this->elements[n].binaryGate;
  }

  FBBinaryConstGate<Number_T>* binaryConstGate(size_t n) override
  {
    return this->elements[n].binaryConstGate;
  }

  FBUnaryGate* unaryGate(size_t n) override
  {
    return this->elements[n].unaryGate;
  }

  FBInput* input(size_t n) override
  {
    return this->elements[n].input;
  }

  FBAssign<Number_T>* assign(size_t n) override
  {
    return this->elements[n].assign;
  }

  FBTerminal* assertZero(size_t n) override
  {
    return this->elements[n].terminal;
  }

  FBTerminal* deleteSingle(size_t n) override
  {
    return this->elements[n].terminal;
  }

  FBWireRange* deleteRange(size_t n) override
  {
    return this->elements[n].wireRange;
  }

  FBFunctionInvoke* functionInvoke(size_t n) override
  {
    return this->elements[n].functionInvoke;
  }

  FBAnonFunction<Number_T>* anonFunction(size_t n) override
  {
    return this->elements[n].anonFunction;
  }

  ForLoop<Number_T>* forLoop(size_t n) override
  {
    return this->elements[n].forLoop;
  }

  SwitchStatement<Number_T>* switchStatement(size_t n) override
  {
    return this->elements[n].switchStatement;
  }
};

template<typename Number_T>
struct FBFunctionDeclare : public wtk::FunctionDeclare<Number_T>
{
  Function const* function;
  FBDirectiveList<Number_T> bodyList;

  FBFunctionDeclare(Function const* f) : function(f) { }

  char const* name() override
  {
    return this->function->name()->c_str();
  }

  index_t outputCount() override
  {
    return this->function->output_count();
  }

  index_t inputCount() override
  {
    return this->function->input_count();
  }

  index_t instanceCount() override
  {
    return this->function->instance_count();
  }

  index_t shortWitnessCount() override
  {
    return this->function->witness_count();
  }

  FBDirectiveList<Number_T>* body() override
  {
    return &this->bodyList;
  }
};

template<typename Number_T>
struct FBIRTree : public wtk::IRTree<Number_T>
{
  std::vector<FBFunctionDeclare<Number_T>> functions;
  FBDirectiveList<Number_T> bodyList;

  size_t size() override
  {
    return functions.size();
  }

  FBFunctionDeclare<Number_T>* functionDeclare(size_t n) override
  {
    return &functions[n];
  }

  FBDirectiveList<Number_T>* body() override
  {
    return &this->bodyList;
  }
};

} } // namespace wtk::flatbuffer

#endif//WTK_FLATBUFFER_IR_TREE_H_
