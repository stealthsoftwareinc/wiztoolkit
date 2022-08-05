/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
FlatBufferTreePrinter<Number_T>::FlatBufferTreePrinter(FILE* ofile)
  : FlatBufferPrinter<Number_T>(ofile) { }

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printBinaryGate(wtk::BinaryGate* gate,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  switch(gate->calculation())
  {
  case wtk::BinaryGate::ADD:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateAdd, CreateGateAdd(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            CreateWire(this->builder, gate->rightWire())).Union()));
    break;
  }
  case wtk::BinaryGate::MUL:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateMul, CreateGateMul(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            CreateWire(this->builder, gate->rightWire())).Union()));
    break;
  }
  case wtk::BinaryGate::AND:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateAnd, CreateGateAnd(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            CreateWire(this->builder, gate->rightWire())).Union()));
    break;
  }
  case wtk::BinaryGate::XOR:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateXor, CreateGateXor(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            CreateWire(this->builder, gate->rightWire())).Union()));
    break;
  }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printBinaryConstGate(
    wtk::BinaryConstGate<Number_T>* gate,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  switch(gate->calculation())
  {
  case wtk::BinaryConstGate<Number_T>::ADDC:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateAddConstant,
          CreateGateAddConstant(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            flattenNumber(&this->builder, gate->rightValue())).Union()));
    break;
  }
  case wtk::BinaryConstGate<Number_T>::MULC:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateMulConstant,
          CreateGateMulConstant(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->leftWire()),
            flattenNumber(&this->builder, gate->rightValue())).Union()));
    break;
  }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printUnaryGate(wtk::UnaryGate* gate,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  switch(gate->calculation())
  {
  case wtk::UnaryGate::NOT:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateNot, CreateGateNot(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->inputWire())).Union()));
    break;
  }
  case wtk::UnaryGate::COPY:
  {
    directives->emplace_back(CreateDirective(
          this->builder, DirectiveSet_GateCopy, CreateGateCopy(this->builder,
            CreateWire(this->builder, gate->outputWire()),
            CreateWire(this->builder, gate->inputWire())).Union()));
    break;
  }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printInput(wtk::Input* input,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  switch(input->stream())
  {
  case wtk::Input::INSTANCE:
  {
    directives->emplace_back(CreateDirective( this->builder,
          DirectiveSet_GateInstance, CreateGateInstance(this->builder,
            CreateWire(this->builder, input->outputWire())).Union()));
    break;
  }
  case wtk::Input::SHORT_WITNESS:
  {
    directives->emplace_back(CreateDirective( this->builder,
          DirectiveSet_GateWitness, CreateGateWitness(this->builder,
            CreateWire(this->builder, input->outputWire())).Union()));
    break;
  }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printAssign(
    wtk::Assign<Number_T>* assign,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  directives->emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateConstant,
        CreateGateConstant(this->builder,
          CreateWire(this->builder, assign->outputWire()),
          flattenNumber(&this->builder, assign->constValue())).Union()));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printAssertZero(
    wtk::Terminal* assertZero,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  directives->emplace_back(CreateDirective(this->builder,
        DirectiveSet_GateAssertZero, CreateGateAssertZero(this->builder,
          CreateWire(this->builder, assertZero->wire())).Union()));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printDeleteSingle(wtk::Terminal* del,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  directives->emplace_back(CreateDirective(this->builder,
        DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, del->wire()),
          // I think this next line should create a "null" wire index
          flatbuffers::Offset<Wire>()).Union()));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printDeleteRange(wtk::WireRange* del,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  directives->emplace_back(CreateDirective(
        this->builder, DirectiveSet_GateFree, CreateGateFree(this->builder,
          CreateWire(this->builder, del->first()),
          CreateWire(this->builder, del->last())).Union()));
}

template<typename Number_T>
flatbuffers::Offset<wtk_gen_flatbuffer::WireList>
FlatBufferTreePrinter<Number_T>::printWireList(
    wtk::WireList* list)
{
  std::vector<flatbuffers::Offset<WireListElement>> elements;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::WireList::SINGLE:
    {
      elements.emplace_back(CreateWireListElement(this->builder,
            WireListElementU_Wire, CreateWire(
              this->builder, list->single(i)).Union()));
      break;
    }
    case wtk::WireList::RANGE:
    {
      wtk::WireRange* range = list->range(i);
      elements.emplace_back(CreateWireListElement(this->builder,
            WireListElementU_WireRange, CreateWireRange(this->builder,
              CreateWire(this->builder, range->first()),
              CreateWire(this->builder, range->last())).Union()));
      break;
    }
    }
  }

  return CreateWireList(this->builder, this->builder.CreateVector(elements));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printFunctionInvoke(
    wtk::FunctionInvoke* invoke,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  directives->emplace_back(CreateDirective(this->builder,
        DirectiveSet_GateCall, CreateGateCall(this->builder,
          this->builder.CreateString(invoke->name()),
          this->printWireList(invoke->outputList()),
          this->printWireList(invoke->inputList())).Union()));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printAnonFunction(
    wtk::AnonFunction<Number_T>* anon,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  std::vector<flatbuffers::Offset<Directive>> sub_directives;
  this->printDirectiveList(anon->body(), &sub_directives);

  directives->emplace_back(CreateDirective(this->builder,
        DirectiveSet_GateAnonCall, CreateGateAnonCall(this->builder,
          this->printWireList(anon->outputList()),
          CreateAbstractAnonCall(this->builder,
            this->printWireList(anon->inputList()),
            anon->instanceCount(), anon->shortWitnessCount(),
            this->builder.CreateVector(sub_directives))).Union()));
}

template<typename Number_T>
flatbuffers::Offset<IterExprWireNumber>
FlatBufferTreePrinter<Number_T>::printIterExpr(wtk::IterExpr* expr)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprConst,
        CreateIterExprConst(this->builder, expr->literal()).Union());
    break;
  }
  case IterExpr::ITERATOR:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprName,
        CreateIterExprName(this->builder,
          this->builder.CreateString(expr->name())).Union());
    break;
  }
  case IterExpr::ADD:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprAdd,
        CreateIterExprAdd(this->builder,
          this->printIterExpr(expr->lhs()),
          this->printIterExpr(expr->rhs())).Union());
    break;
  }
  case IterExpr::SUB:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprSub,
        CreateIterExprSub(this->builder,
          this->printIterExpr(expr->lhs()),
          this->printIterExpr(expr->rhs())).Union());
    break;
  }
  case IterExpr::MUL:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprMul,
        CreateIterExprMul(this->builder,
          this->printIterExpr(expr->lhs()),
          this->printIterExpr(expr->rhs())).Union());
    break;
  }
  case IterExpr::DIV:
  {
    return CreateIterExprWireNumber(this->builder, IterExpr_IterExprDivConst,
        CreateIterExprDivConst(this->builder,
          this->printIterExpr(expr->lhs()),
          expr->literal()).Union());
    break;
  }
  }

  // should be unreachable
  return flatbuffers::Offset<IterExprWireNumber>();
}

template<typename Number_T>
flatbuffers::Offset<wtk_gen_flatbuffer::IterExprWireList> 
FlatBufferTreePrinter<Number_T>::printIterExprWireList(
    wtk::IterExprWireList* list)
{
  std::vector<flatbuffers::Offset<IterExprWireListElement>> elements;

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      elements.emplace_back(CreateIterExprWireListElement(this->builder,
            IterExprWireListElementU_IterExprWireNumber,
            this->printIterExpr(list->single(i)).Union()));
      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      wtk::IterExprWireRange* range = list->range(i);
      elements.emplace_back(CreateIterExprWireListElement(this->builder,
            IterExprWireListElementU_IterExprWireRange,
            CreateIterExprWireRange(this->builder,
              this->printIterExpr(range->first()),
              this->printIterExpr(range->last())).Union()));
      break;
    }
    }
  }

  return CreateIterExprWireList(
      this->builder, this->builder.CreateVector(elements));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printForLoop(
    wtk::ForLoop<Number_T>* loop,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  switch(loop->bodyType())
  {
  case ForLoop<Number_T>::INVOKE:
  {
    wtk::IterExprFunctionInvoke* invoke = loop->invokeBody();
    directives->emplace_back(CreateDirective(this->builder,
          DirectiveSet_GateFor, CreateGateFor(this->builder,
            this->printWireList(loop->outputList()),
            this->builder.CreateString(loop->iterName()),
            loop->first(), loop->last(), ForLoopBody_IterExprFunctionInvoke,
            CreateIterExprFunctionInvoke(
              this->builder, this->builder.CreateString(invoke->name()),
              this->printIterExprWireList(invoke->outputList()),
              this->printIterExprWireList(invoke->inputList())).Union())
          .Union()));
    break;
  }
  case ForLoop<Number_T>::ANONYMOUS:
  {
    wtk::IterExprAnonFunction<Number_T>* anon = loop->anonymousBody();
    std::vector<flatbuffers::Offset<Directive>> sub_directives;
    this->printDirectiveList(anon->body(), &sub_directives);

    directives->emplace_back(CreateDirective(this->builder,
          DirectiveSet_GateFor, CreateGateFor(this->builder,
            this->printWireList(loop->outputList()),
            this->builder.CreateString(loop->iterName()),
            loop->first(), loop->last(), ForLoopBody_IterExprAnonFunction,
            CreateIterExprAnonFunction(this->builder,
              this->printIterExprWireList(anon->outputList()),
              this->printIterExprWireList(anon->inputList()),
              anon->instanceCount(), anon->shortWitnessCount(),
              this->builder.CreateVector(sub_directives)).Union()).Union()));
    break;
  }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printSwitchStatement(
    wtk::SwitchStatement<Number_T>* switch_stmt,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  std::vector<flatbuffers::Offset<Value>> selectors;
  std::vector<flatbuffers::Offset<CaseInvoke>> cases;

  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    wtk::CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(i);
    selectors.emplace_back(CreateValue(this->builder,
          flattenNumber(&this->builder, case_blk->match())));

    switch(case_blk->bodyType())
    {
    case CaseBlock<Number_T>::INVOKE:
    {
      wtk::CaseFunctionInvoke* invoke = case_blk->invokeBody();
      cases.emplace_back(CreateCaseInvoke(this->builder,
            CaseInvokeU_AbstractGateCall, CreateAbstractGateCall(this->builder,
              this->builder.CreateString(invoke->name()),
              this->printWireList(invoke->inputList())).Union()));
      break;
    }
    case CaseBlock<Number_T>::ANONYMOUS:
    {
      wtk::CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();
      std::vector<flatbuffers::Offset<Directive>> sub_directives;
      this->printDirectiveList(anon->body(), &sub_directives);

      cases.emplace_back(CreateCaseInvoke(this->builder,
            CaseInvokeU_AbstractAnonCall, CreateAbstractAnonCall(this->builder,
              this->printWireList(anon->inputList()),
              anon->instanceCount(), anon->shortWitnessCount(),
              this->builder.CreateVector(sub_directives)).Union()));
      break;
    }
    }
  }

  directives->emplace_back(CreateDirective(this->builder,
        DirectiveSet_GateSwitch, CreateGateSwitch(this->builder,
          CreateWire(this->builder, switch_stmt->condition()),
          this->printWireList(switch_stmt->outputList()),
          this->builder.CreateVector(selectors),
          this->builder.CreateVector(cases)).Union()));
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printDirectiveList(
    wtk::DirectiveList<Number_T>* list,
    std::vector<flatbuffers::Offset<Directive>>* directives)
{
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case DirectiveList<Number_T>::BINARY_GATE:
    {
      this->printBinaryGate(list->binaryGate(i), directives);
      break;
    }
    case DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      this->printBinaryConstGate(list->binaryConstGate(i), directives);
      break;
    }
    case DirectiveList<Number_T>::UNARY_GATE:
    {
      this->printUnaryGate(list->unaryGate(i), directives);
      break;
    }
    case DirectiveList<Number_T>::INPUT:
    {
      this->printInput(list->input(i), directives);
      break;
    }
    case DirectiveList<Number_T>::ASSIGN:
    {
      this->printAssign(list->assign(i), directives);
      break;
    }
    case DirectiveList<Number_T>::ASSERT_ZERO:
    {
      this->printAssertZero(list->assertZero(i), directives);
      break;
    }
    case DirectiveList<Number_T>::DELETE_SINGLE:
    {
      this->printDeleteSingle(list->deleteSingle(i), directives);
      break;
    }
    case DirectiveList<Number_T>::DELETE_RANGE:
    {
      this->printDeleteRange(list->deleteRange(i), directives);
      break;
    }
    case DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      this->printFunctionInvoke(list->functionInvoke(i), directives);
      break;
    }
    case DirectiveList<Number_T>::ANON_FUNCTION:
    {
      this->printAnonFunction(list->anonFunction(i), directives);
      break;
    }
    case DirectiveList<Number_T>::FOR_LOOP:
    {
      this->printForLoop(list->forLoop(i), directives);
      break;
    }
    case DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      this->printSwitchStatement(list->switchStatement(i), directives);
      break;
    }
    }
  }
}

template<typename Number_T>
void FlatBufferTreePrinter<Number_T>::printTree(IRTree<Number_T>* tree)
{
  std::vector<flatbuffers::Offset<Function>> functions;
  for(size_t i = 0; i < tree->size(); i++)
  {
    wtk::FunctionDeclare<Number_T>* func = tree->functionDeclare(i);
    std::vector<flatbuffers::Offset<Directive>> directives;
    this->printDirectiveList(func->body(), &directives);

    functions.emplace_back(CreateFunction(this->builder,
          this->builder.CreateString(func->name()),
          func->outputCount(), func->inputCount(),
          func->instanceCount(),func->shortWitnessCount(),
          this->builder.CreateVector(directives)));
  }

  std::vector<flatbuffers::Offset<Directive>> directives;
  this->printDirectiveList(tree->body(), &directives);

  this->writeRelation(&directives, &functions);
}

} } // namespace wtk::flatbuffer
