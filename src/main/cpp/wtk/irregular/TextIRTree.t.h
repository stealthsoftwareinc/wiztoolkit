/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_TEXT_IR_TREE_H_
#define WTK_IRREGULAR_TEXT_IR_TREE_H_

#include <cstddef>
#include <memory>
#include <vector>

#include <wtk/IRTree.h>

namespace wtk {
namespace irregular {

/**
 * These are the smallest implementations of the IRTree I can come up with.
 * Its all raw pointers, no thinking about destruction. Instead, pool allocate
 * everything.
 */

struct TextBinaryGate : public BinaryGate
{
  wtk::index_t out = 0;
  wtk::index_t left = 0;
  wtk::index_t right = 0;

  Calculation calc;

  Calculation calculation() override { return this->calc; }

  wtk::index_t outputWire() override { return this->out; }
  wtk::index_t leftWire() override { return this->left; }
  wtk::index_t rightWire() override { return this->right; }

  virtual ~TextBinaryGate() = default;
};

struct TextUnaryGate : public UnaryGate
{
  wtk::index_t out = 0;
  wtk::index_t in = 0;

  Calculation calc;

  Calculation calculation() override { return this->calc; }

  wtk::index_t outputWire() override { return this->out; }
  wtk::index_t inputWire() override { return this->in; }

  virtual ~TextUnaryGate() = default;
};

template<typename Number_T>
struct TextBinaryConstGate : public BinaryConstGate<Number_T>
{
  wtk::index_t out = 0;
  wtk::index_t left = 0;
  Number_T right = Number_T(0);

  typename BinaryConstGate<Number_T>::Calculation calc;

  typename BinaryConstGate<Number_T>::Calculation calculation() override
  {
    return this->calc;
  }

  wtk::index_t outputWire() override { return this->out; }
  wtk::index_t leftWire() override { return this->left; }
  Number_T rightValue() override { return this->right; }

  virtual ~TextBinaryConstGate() = default;
};

struct TextInput : public Input
{
  wtk::index_t out = 0;
  Stream stream_;

  Stream stream() override { return this->stream_; }

  wtk::index_t outputWire() override { return this->out; }

  virtual ~TextInput() = default;
};

template<typename Number_T>
struct TextAssign : public Assign<Number_T>
{
  wtk::index_t out = 0;
  Number_T value = Number_T(0);

  wtk::index_t outputWire() override { return this->out; }
  Number_T constValue() override { return this->value; }

  virtual ~TextAssign() = default;
};

struct TextTerminal : public Terminal
{
  wtk::index_t wire_ = 0;

  wtk::index_t wire() override { return this->wire_; }

  virtual ~TextTerminal() = default;
};

struct TextWireRange : public WireRange
{
  wtk::index_t firstIdx = 0;
  wtk::index_t lastIdx = 0;

  wtk::index_t first() override { return this->firstIdx; }
  wtk::index_t last() override { return this->lastIdx; }

  virtual ~TextWireRange() = default;
};

struct TextWireList : public WireList
{
  union Element
  {
    wtk::index_t single = 0;
    TextWireRange* range;
  };

  std::vector<Type> types;
  std::vector<Element> elements;

  size_t size() override { return this->types.size(); }

  Type type(size_t n) override { return this->types[n]; }

  wtk::index_t single(size_t n) override { return this->elements[n].single; }

  TextWireRange* range(size_t n) override { return this->elements[n].range; }

  virtual ~TextWireList() = default;
};

struct TextFunctionInvoke;

template<typename Number_T>
struct TextAnonFunction;

template<typename Number_T>
struct TextForLoop;

template<typename Number_T>
struct TextSwitchStatement;

template<typename Number_T>
struct TextDirectiveList : public DirectiveList<Number_T>
{
  union Element {
    TextBinaryGate* binaryGate = nullptr;
    TextBinaryConstGate<Number_T>* binaryConstGate;
    TextUnaryGate* unaryGate;
    TextInput* input;
    TextAssign<Number_T>* assign;
    TextTerminal* assertZero;
    TextTerminal* deleteSingle;
    TextWireRange* deleteRange;
    TextFunctionInvoke* functionInvoke;
    TextAnonFunction<Number_T>* anonFunction;
    TextForLoop<Number_T>* forLoop;
    TextSwitchStatement<Number_T>* switchStatement;
  };

  std::vector<typename DirectiveList<Number_T>::Type> types;
  std::vector<Element> elements;

  size_t size() override { return this->types.size(); }

  typename DirectiveList<Number_T>::Type type(size_t n) override
  {
    return this->types[n];
  }

  TextBinaryGate* binaryGate(size_t n) override
  {
    return this->elements[n].binaryGate;
  }

  TextBinaryConstGate<Number_T>* binaryConstGate(size_t n) override
  {
    return this->elements[n].binaryConstGate;
  }

  TextUnaryGate* unaryGate(size_t n) override
  {
    return this->elements[n].unaryGate;
  }

  TextInput* input(size_t n) override
  {
    return this->elements[n].input;
  }

  TextAssign<Number_T>* assign(size_t n) override
  {
    return this->elements[n].assign;
  }

  TextTerminal* assertZero(size_t n) override
  {
    return this->elements[n].assertZero;
  }

  TextTerminal* deleteSingle(size_t n) override
  {
    return this->elements[n].deleteSingle;
  }

  TextWireRange* deleteRange(size_t n) override
  {
    return this->elements[n].deleteRange;
  }

  TextFunctionInvoke* functionInvoke(size_t n) override
  {
    return this->elements[n].functionInvoke;
  }

  TextAnonFunction<Number_T>* anonFunction(size_t n) override
  {
    return this->elements[n].anonFunction;
  }

  TextForLoop<Number_T>* forLoop(size_t n) override
  {
    return this->elements[n].forLoop;
  }

  TextSwitchStatement<Number_T>* switchStatement(size_t n) override
  {
    return this->elements[n].switchStatement;
  }
};

template<typename Number_T>
struct TextFunctionDeclare : public FunctionDeclare<Number_T>
{
  char const* name_ = nullptr;

  wtk::index_t outCount = 0;
  wtk::index_t inCount = 0;
  wtk::index_t insCount = 0;
  wtk::index_t witCount = 0;

  TextDirectiveList<Number_T>* directives;

  char const* name() override { return this->name_; }

  wtk::index_t outputCount() override { return this->outCount; }
  wtk::index_t inputCount() override { return this->inCount; }
  wtk::index_t instanceCount() override { return this->insCount; }
  wtk::index_t shortWitnessCount() override { return this->witCount; }

  TextDirectiveList<Number_T>* body() override { return this->directives; }

  virtual ~TextFunctionDeclare() = default;
};

struct TextFunctionInvoke : public FunctionInvoke
{
  char const* name_ = nullptr;

  TextWireList* outList = 0;
  TextWireList* inList = 0;

  char const* name() override { return this->name_; }

  TextWireList* outputList() override { return this->outList; }
  TextWireList* inputList() override { return this->inList; }

  virtual ~TextFunctionInvoke() = default;
};

template<typename Number_T>
struct TextAnonFunction : public AnonFunction<Number_T>
{
  TextWireList* outList = nullptr;
  TextWireList* inList = nullptr;

  wtk::index_t insCount = 0;
  wtk::index_t witCount = 0;

  TextDirectiveList<Number_T>* directives = nullptr;

  TextWireList* outputList() override { return this->outList; }
  TextWireList* inputList() override { return this->inList; }

  wtk::index_t instanceCount() override { return this->insCount; }
  wtk::index_t shortWitnessCount() override { return this->witCount; }

  TextDirectiveList<Number_T>* body() override { return this->directives; }

  virtual ~TextAnonFunction() = default;
};

struct TextIterExpr : public IterExpr
{
  union
  {
    wtk::index_t literalValue;
    char* name_;
    struct
    {
      TextIterExpr* leftHand;
      TextIterExpr* rightHand;
    } expr;
    struct
    {
      TextIterExpr* leftHand = nullptr;
      wtk::index_t literalValue = 0;
    } div;
  };

  Type type_;

  TextIterExpr() : type_()
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

  TextIterExpr* lhs() override
  {
    if(this->type_ == Type::DIV) { return this->div.leftHand; }
    return this->expr.leftHand;
  }

  TextIterExpr* rhs() override { return this->expr.rightHand; }

  virtual ~TextIterExpr() = default;
};

struct TextIterExprWireRange : public IterExprWireRange
{
  TextIterExpr* first_ = nullptr;
  TextIterExpr* last_  = nullptr;

  TextIterExpr* first() override { return this->first_; }
  TextIterExpr* last() override { return this->last_; }

  virtual ~TextIterExprWireRange() = default;
};

struct TextIterExprWireList : public IterExprWireList
{
  union Element
  {
    TextIterExpr* single = nullptr;
    TextIterExprWireRange* range;
  };

  std::vector<Type> types;
  std::vector<Element> elements;

  size_t size() override { return this->types.size(); }

  Type type(size_t n) override { return this->types[n]; }

  TextIterExpr* single(size_t n) override { return this->elements[n].single; }

  TextIterExprWireRange* range(size_t n) override
  {
    return this->elements[n].range;
  }

  virtual ~TextIterExprWireList() = default;
};

struct TextIterExprFunctionInvoke : public IterExprFunctionInvoke
{
  char const* name_ = nullptr;

  TextIterExprWireList* outList = nullptr;
  TextIterExprWireList* inList = nullptr;

  char const* name() override { return this->name_; }

  TextIterExprWireList* outputList() override { return this->outList; }
  TextIterExprWireList* inputList() override { return this->inList; }

  virtual ~TextIterExprFunctionInvoke() = default;
};

template<typename Number_T>
struct TextIterExprAnonFunction : public IterExprAnonFunction<Number_T>
{
  TextIterExprWireList* outList = nullptr;
  TextIterExprWireList* inList = nullptr;

  wtk::index_t insCount = 0;
  wtk::index_t witCount = 0;

  TextDirectiveList<Number_T>* directives = nullptr;

  TextIterExprWireList* outputList() override { return this->outList; }
  TextIterExprWireList* inputList() override { return this->inList; }

  wtk::index_t instanceCount() override { return this->insCount; }
  wtk::index_t shortWitnessCount() override { return this->witCount; }

  TextDirectiveList<Number_T>* body() override { return this->directives; }

  virtual ~TextIterExprAnonFunction() = default;
};

template<typename Number_T>
struct TextForLoop : public ForLoop<Number_T>
{
  TextWireList* outList = nullptr;

  char const* iterator = nullptr;

  wtk::index_t first_ = 0;
  wtk::index_t last_ = 0;

  typename ForLoop<Number_T>::BodyType bodyType_;

  union
  {
    TextIterExprFunctionInvoke* invoke;
    TextIterExprAnonFunction<Number_T>* anonymous;
  };

  TextWireList* outputList() override { return this->outList; }

  char const* iterName() override { return this->iterator; }

  wtk::index_t first() override { return this->first_; }
  wtk::index_t last() override { return this->last_; }

  typename ForLoop<Number_T>::BodyType bodyType() override
  {
    return this->bodyType_;
  }

  virtual TextIterExprFunctionInvoke* invokeBody() override
  {
    return this->invoke;
  }

  virtual TextIterExprAnonFunction<Number_T>* anonymousBody() override
  {
    return this->anonymous;
  }

  virtual ~TextForLoop() = default;
};

struct TextCaseFunctionInvoke : public CaseFunctionInvoke
{
  char const* name_ = nullptr;

  TextWireList* inList = nullptr;

  char const* name() override { return this->name_; }

  TextWireList* inputList() override { return this->inList; }

  virtual ~TextCaseFunctionInvoke() = default;
};

template<typename Number_T>
struct TextCaseAnonFunction : public CaseAnonFunction<Number_T>
{
  TextWireList* inList = nullptr;

  wtk::index_t insCount = 0;
  wtk::index_t witCount = 0;

  TextDirectiveList<Number_T>* directives = nullptr;

  TextWireList* inputList() override { return this->inList; }

  wtk::index_t instanceCount() override { return this->insCount; }
  wtk::index_t shortWitnessCount() override { return this->witCount; }

  TextDirectiveList<Number_T>* body() override { return this->directives; }

  virtual ~TextCaseAnonFunction() = default;
};

template<typename Number_T>
struct TextCaseBlock : public CaseBlock<Number_T>
{
  Number_T match_ = Number_T(0);

  typename CaseBlock<Number_T>::BodyType bodyType_;

  union
  {
    TextCaseFunctionInvoke* invoke = nullptr;
    TextCaseAnonFunction<Number_T>* anonymous;
  };

  Number_T match() override { return this->match_; }

  typename CaseBlock<Number_T>::BodyType bodyType() override
  {
    return this->bodyType_;
  }

  TextCaseFunctionInvoke* invokeBody() override
  {
    return this->invoke;
  }

  TextCaseAnonFunction<Number_T>* anonymousBody() override
  {
    return this->anonymous;
  }

  virtual ~TextCaseBlock() = default;
};

template<typename Number_T>
struct TextSwitchStatement : public SwitchStatement<Number_T>
{
  TextWireList* outList = nullptr;
  wtk::index_t cond = 0;

  std::vector<TextCaseBlock<Number_T>*> cases;

  TextWireList* outputList() override { return this->outList; }

  wtk::index_t condition() override { return this->cond; }

  size_t size() override { return this->cases.size(); }

  TextCaseBlock<Number_T>* caseBlock(size_t n) override
  {
    return this->cases[n];
  }
};

template<typename Number_T>
struct TextIRTree : public IRTree<Number_T>
{
  std::vector<TextFunctionDeclare<Number_T>*> functionDeclares;

  TextDirectiveList<Number_T>* directives = nullptr;

  size_t size() override { return functionDeclares.size(); }

  TextFunctionDeclare<Number_T>* functionDeclare(size_t n) override
  {
    return this->functionDeclares[n];
  }

  TextDirectiveList<Number_T>* body() override { return this->directives; }
};

} } // namespace wtk::irregular

#endif // WTK_IRREGULAR_TEXT_IR_TREE_H_
