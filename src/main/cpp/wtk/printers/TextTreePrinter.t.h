/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace printers {

template<typename Number_T>
void TextTreePrinter<Number_T>::printBinaryGate(BinaryGate* gate)
{
  switch(gate->calculation())
  {
  case BinaryGate::ADD:
  {
    fprintf(
        this->outFile, "%s$%" PRIu64 "<-@add($%" PRIu64 ",$%" PRIu64 ");\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        gate->rightWire());
    break;
  }
  case BinaryGate::MUL:
  {
    fprintf(
        this->outFile, "%s$%" PRIu64 "<-@mul($%" PRIu64 ",$%" PRIu64 ");\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        gate->rightWire());
    break;
  }
  case BinaryGate::AND:
  {
    fprintf(
        this->outFile, "%s$%" PRIu64 "<-@and($%" PRIu64 ",$%" PRIu64 ");\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        gate->rightWire());
    break;
  }
  case BinaryGate::XOR:
  {
    fprintf(
        this->outFile, "%s$%" PRIu64 "<-@xor($%" PRIu64 ",$%" PRIu64 ");\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        gate->rightWire());
    break;
  }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printBinaryConstGate(
    BinaryConstGate<Number_T>* gate)
{
  switch(gate->calculation())
  {
  case BinaryConstGate<Number_T>::ADDC:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-@addc($%" PRIu64 ",<%s>);\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        wtk::utils::short_str(gate->rightValue()).c_str());
    break;
  }
  case BinaryConstGate<Number_T>::MULC:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-@mulc($%" PRIu64 ",<%s>);\n",
        this->indent.get(), gate->outputWire(), gate->leftWire(),
        wtk::utils::short_str(gate->rightValue()).c_str());
    break;
  }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printUnaryGate(UnaryGate* gate)
{
  switch(gate->calculation())
  {
  case UnaryGate::NOT:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-@not($%" PRIu64 ");\n",
        this->indent.get(), gate->outputWire(), gate->inputWire());
    break;
  }
  case UnaryGate::COPY:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-$%" PRIu64 ";\n",
        this->indent.get(), gate->outputWire(), gate->inputWire());
    break;
  }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printInput(Input* input)
{
  switch(input->stream())
  {
  case Input::INSTANCE:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-@instance;\n",
        this->indent.get(), input->outputWire());
    break;
  }
  case Input::SHORT_WITNESS:
  {
    fprintf(this->outFile, "%s$%" PRIu64 "<-@short_witness;\n",
        this->indent.get(), input->outputWire());
    break;
  }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printAssign(Assign<Number_T>* assign)
{
  fprintf(this->outFile, "%s$%" PRIu64 "<-<%s>;\n", this->indent.get(),
      assign->outputWire(),
      wtk::utils::short_str(assign->constValue()).c_str());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printAssertZero(Terminal* assertZero)
{
  fprintf(this->outFile, "%s@assert_zero($%" PRIu64 ");\n",
      this->indent.get(), assertZero->wire());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printDeleteSingle(Terminal* del)
{
  fprintf(this->outFile, "%s@delete($%" PRIu64 ");\n",
      this->indent.get(), del->wire());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printDeleteRange(WireRange* del)
{
  fprintf(this->outFile, "%s@delete($%" PRIu64 ",$%" PRIu64 ");\n",
      this->indent.get(), del->first(), del->last());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printWireList(WireList* list)
{
  bool first = true;
  for(size_t i = 0; i < list->size(); i++)
  {
    if(first) { first = false; }
    else { fputs(",", this->outFile); }
    switch(list->type(i))
    {
    case WireList::SINGLE:
    {
      fprintf(this->outFile, "$%" PRIu64, list->single(i));
      break;
    }
    case WireList::RANGE:
    {
      fprintf(this->outFile, "$%" PRIu64 "...$%" PRIu64,
          list->range(i)->first(), list->range(i)->last());
      break;
    }
    }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printFunctionInvoke(FunctionInvoke* invoke)
{
  this->indent.print(this->outFile);
  if(invoke->outputList()->size() != 0)
  {
    this->printWireList(invoke->outputList());
    fputs("<-", this->outFile);
  }

  fprintf(this->outFile, "@call(%s", invoke->name());

  if(invoke->inputList()->size() != 0)
  {
    fputs(",", this->outFile);
    this->printWireList(invoke->inputList());
  }

  fputs(");\n", this->outFile);
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printAnonFunction(AnonFunction<Number_T>* anon)
{
  this->indent.print(this->outFile);
  if(anon->outputList()->size() != 0)
  {
    this->printWireList(anon->outputList());
    fputs("<-", this->outFile);
  }

  fputs("@anon_call(", this->outFile);

  if(anon->inputList()->size() != 0)
  {
    this->printWireList(anon->inputList());
    fputs(",", this->outFile);
  }

  fprintf(this->outFile,
      "@instance:%" PRIu64 ",@short_witness:%" PRIu64 ")\n",
      anon->instanceCount(), anon->shortWitnessCount());

  this->printDirectiveList(anon->body());

  fprintf(this->outFile, "%s@end\n", this->indent.get());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printIterExpr(IterExpr* expr)
{
  switch(expr->type())
  {
  case IterExpr::LITERAL:
  {
    fprintf(this->outFile, "%" PRIu64, expr->literal());
    break;
  }
  case IterExpr::ITERATOR:
  {
    fputs(expr->name(), this->outFile);
    break;
  }
  case IterExpr::ADD:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" + ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::SUB:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" - ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::MUL:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fputs(" * ", this->outFile);
    this->printIterExpr(expr->rhs());
    fputs(")", this->outFile);
    break;
  }
  case IterExpr::DIV:
  {
    fputs("(", this->outFile);
    this->printIterExpr(expr->lhs());
    fprintf(this->outFile, " / %" PRIu64, expr->literal());
    break;
  }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printIterExprWireList(IterExprWireList* list)
{
  bool first = true;
  for(size_t i = 0; i < list->size(); i++)
  {
    if(first) { first = false; }
    else { fputs(",", this->outFile); }
    switch(list->type(i))
    {
    case IterExprWireList::SINGLE:
    {
      fputs("$", this->outFile);
      this->printIterExpr(list->single(i));
      break;
    }
    case IterExprWireList::RANGE:
    {
      fputs("$", this->outFile);
      this->printIterExpr(list->range(i)->first());
      fputs("...$", this->outFile);
      this->printIterExpr(list->range(i)->last());
      break;
    }
    }
  }
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printForLoop(ForLoop<Number_T>* loop)
{
  this->indent.print(this->outFile);
  if(loop->outputList()->size() != 0)
  {
    this->printWireList(loop->outputList());
    fputs("<-", this->outFile);
  }

  fprintf(this->outFile, "@for %s @first %" PRIu64 " @last %" PRIu64 "\n",
        loop->iterName(), loop->first(), loop->last());

  this->indent.inc();
  this->indent.print(this->outFile);

  switch(loop->bodyType())
  {
  case ForLoop<Number_T>::INVOKE:
  {
    IterExprFunctionInvoke* invoke = loop->invokeBody();

    if(invoke->outputList()->size() != 0)
    {
      this->printIterExprWireList(invoke->outputList());
      fputs("<-", this->outFile);
    }

    fprintf(this->outFile, "@call(%s", invoke->name());

    if(invoke->inputList()->size() != 0)
    {
      fputs(",", this->outFile);
      this->printIterExprWireList(invoke->inputList());
    }

    fputs(");\n", this->outFile);
    break;
  }
  case ForLoop<Number_T>::ANONYMOUS:
  {
    IterExprAnonFunction<Number_T>* anon = loop->anonymousBody();

    if(anon->outputList()->size() != 0)
    {
      this->printIterExprWireList(anon->outputList());
      fputs("<-", this->outFile);
    }

    fputs("@anon_call(", this->outFile);

    if(anon->inputList()->size() != 0)
    {
      this->printIterExprWireList(anon->inputList());
      fputs(",", this->outFile);
    }

    fprintf(this->outFile,
        "@instance:%" PRIu64 ",@short_witness:%" PRIu64 ")\n",
        anon->instanceCount(), anon->shortWitnessCount());

    this->printDirectiveList(anon->body());
    fprintf(this->outFile, "%s@end\n", this->indent.get());
    break;
  }
  }

  this->indent.dec();
  fprintf(this->outFile, "%s@end\n", this->indent.get());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printSwitchStatement(
    SwitchStatement<Number_T>* switch_stmt)
{
  this->indent.print(this->outFile);
  if(switch_stmt->outputList()->size() != 0)
  {
    this->printWireList(switch_stmt->outputList());
    fputs("<-", this->outFile);
  }

  fprintf(this->outFile, "@switch($%" PRIu64 ")\n",
      switch_stmt->condition());

  this->indent.inc();
  for(size_t i = 0; i < switch_stmt->size(); i++)
  {
    CaseBlock<Number_T>* case_blk = switch_stmt->caseBlock(i);

    fprintf(this->outFile, "%s@case<%s>:", this->indent.get(),
        wtk::utils::short_str(case_blk->match()).c_str());

    switch(case_blk->bodyType())
    {
    case CaseBlock<Number_T>::INVOKE:
    {
      CaseFunctionInvoke* invoke = case_blk->invokeBody();

      fprintf(this->outFile, "@call(%s", invoke->name());

      if(invoke->inputList()->size() != 0)
      {
        fputs(",", this->outFile);
        this->printWireList(invoke->inputList());
      }

      fputs(");\n", this->outFile);
      break;
    }
    case CaseBlock<Number_T>::ANONYMOUS:
    {
      CaseAnonFunction<Number_T>* anon = case_blk->anonymousBody();

      fputs("@anon_call(", this->outFile);

      if(anon->inputList()->size() != 0)
      {
        this->printWireList(anon->inputList());
        fputs(",", this->outFile);
      }

      fprintf(this->outFile,
          "@instance:%" PRIu64 ",@short_witness:%" PRIu64 ")\n",
          anon->instanceCount(), anon->shortWitnessCount());

      this->printDirectiveList(anon->body());
      fprintf(this->outFile, "%s@end\n", this->indent.get());
      break;
    }
    }
  }

  this->indent.dec();
  fprintf(this->outFile, "%s@end\n", this->indent.get());
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printDirectiveList(
    DirectiveList<Number_T>* list)
{
  this->indent.inc();

  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case DirectiveList<Number_T>::BINARY_GATE:
    {
      this->printBinaryGate(list->binaryGate(i));
      break;
    }
    case DirectiveList<Number_T>::BINARY_CONST_GATE:
    {
      this->printBinaryConstGate(list->binaryConstGate(i));
      break;
    }
    case DirectiveList<Number_T>::UNARY_GATE:
    {
      this->printUnaryGate(list->unaryGate(i));
      break;
    }
    case DirectiveList<Number_T>::INPUT:
    {
      this->printInput(list->input(i));
      break;
    }
    case DirectiveList<Number_T>::ASSIGN:
    {
      this->printAssign(list->assign(i));
      break;
    }
    case DirectiveList<Number_T>::ASSERT_ZERO:
    {
      this->printAssertZero(list->assertZero(i));
      break;
    }
    case DirectiveList<Number_T>::DELETE_SINGLE:
    {
      this->printDeleteSingle(list->deleteSingle(i));
      break;
    }
    case DirectiveList<Number_T>::DELETE_RANGE:
    {
      this->printDeleteRange(list->deleteRange(i));
      break;
    }
    case DirectiveList<Number_T>::FUNCTION_INVOKE:
    {
      this->printFunctionInvoke(list->functionInvoke(i));
      break;
    }
    case DirectiveList<Number_T>::ANON_FUNCTION:
    {
      this->printAnonFunction(list->anonFunction(i));
      break;
    }
    case DirectiveList<Number_T>::FOR_LOOP:
    {
      this->printForLoop(list->forLoop(i));
      break;
    }
    case DirectiveList<Number_T>::SWITCH_STATEMENT:
    {
      this->printSwitchStatement(list->switchStatement(i));
      break;
    }
    }
  }

  this->indent.dec();
}

template<typename Number_T>
void TextTreePrinter<Number_T>::printTree(IRTree<Number_T>* tree)
{
  fprintf(this->outFile, "@begin\n");
  for(size_t i = 0; i < tree->size(); i++)
  {
    FunctionDeclare<Number_T>* func = tree->functionDeclare(i);
    this->indent.inc();
    fprintf(this->outFile, "%s@function(%s, @out:%" PRIu64 ",@in:%" PRIu64
        ",@instance:%" PRIu64 ",@short_witness:%" PRIu64 ")\n",
        this->indent.get(), func->name(), func->outputCount(),
        func->inputCount(), func->instanceCount(), func->shortWitnessCount());

    this->printDirectiveList(func->body());

    fprintf(this->outFile, "%s@end\n", this->indent.get());
    this->indent.dec();
  }

  this->printDirectiveList(tree->body());
  fprintf(this->outFile, "@end\n");
}

} } // namespace wtk::printers
