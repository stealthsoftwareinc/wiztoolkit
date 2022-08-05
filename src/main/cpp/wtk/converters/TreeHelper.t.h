/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace converters {

template<typename Number_T>
void TreePools<Number_T>::registerIter(char const* str)
{
  std::string new_iter("wtk::mux::iter");
  new_iter += wtk::utils::dec(this->iterRegisterNext++);

  this->iterRegister.emplace_back();
  this->iterRegister.back().first = str;
  this->iterRegister.back().second = this->strdup(new_iter.c_str());
}

template<typename Number_T>
void TreePools<Number_T>::unregisterIter()
{
  this->iterRegister.pop_back();
}

template<typename Number_T>
char* TreePools<Number_T>::findRegisteredIter(char const* str)
{
  if(strcmp("wtk::mux::i", str) == 0)
  {
    return this->strdup("wtk::mux::i");
  }

  for(size_t i = this->iterRegister.size(); i > 0; i--)
  {
    if(strcmp(str, this->iterRegister[i - 1].first) == 0)
    {
      return this->iterRegister[i - 1].second;
    }
  }
  return this->strdup("wtk::mux::unrecognized_iter");
}

template<typename Number_T>
char* TreePools<Number_T>::strdup(char const* str)
{
  size_t len = strlen(str);
  char* copy = this->identifiers.allocate(len + 1);
  strncpy(copy, str, len + 1);
  copy[len] = '\0';
  return copy;
}

template<typename Number_T>
WireListBuilder<Number_T>::WireListBuilder(TreePools<Number_T>* p)
  : pools(p), list(p->wireLists.allocate()) { }

template<typename Number_T>
void WireListBuilder<Number_T>::single(wtk::index_t wire)
{
  this->list->types.emplace_back(wtk::WireList::SINGLE);
  this->list->elements.emplace_back();
  this->list->elements.back().single = wire;
}

template<typename Number_T>
void WireListBuilder<Number_T>::range(wtk::index_t first, wtk::index_t last)
{
  wtk::irregular::TextWireRange* range = this->pools->wireRanges.allocate();
  range->firstIdx = first;
  range->lastIdx = last;

  this->list->types.emplace_back(wtk::WireList::RANGE);
  this->list->elements.emplace_back();
  this->list->elements.back().range = range;
}

template<typename Number_T>
void WireListBuilder<Number_T>::range(wtk::WireRange* range)
{
  this->range(range->first(), range->last());
}

template<typename Number_T>
void WireListBuilder<Number_T>::duplicate(wtk::WireList* list)
{
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::WireList::SINGLE:
    {
      this->single(list->single(i));
      break;
    }
    case wtk::WireList::RANGE:
    {
      this->range(list->range(i));
      break;
    }
    }
  }
}

template<typename Number_T>
IterExprBuilder<Number_T>::IterExprBuilder(TreePools<Number_T>* p)
  : pools(p) { }

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::literal(
    wtk::index_t l)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->literalValue = l;
  expr->type_ = wtk::IterExpr::LITERAL;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::name(char const* n)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->name_ = this->pools->findRegisteredIter(n);
  expr->type_ = wtk::IterExpr::ITERATOR;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::add(
    wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->expr.leftHand = l;
  expr->expr.rightHand = r;
  expr->type_ = wtk::IterExpr::ADD;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::mul(
    wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->expr.leftHand = l;
  expr->expr.rightHand = r;
  expr->type_ = wtk::IterExpr::MUL;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::sub(
    wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->expr.leftHand = l;
  expr->expr.rightHand = r;
  expr->type_ = wtk::IterExpr::SUB;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::div(
    wtk::irregular::TextIterExpr* l, wtk::index_t r)
{
  wtk::irregular::TextIterExpr* expr = this->pools->iterExprs.allocate();
  expr->div.leftHand = l;
  expr->div.literalValue = r;
  expr->type_ = wtk::IterExpr::DIV;

  return expr;
}

template<typename Number_T>
wtk::irregular::TextIterExpr* IterExprBuilder<Number_T>::duplicate(IterExpr* e)
{
  switch(e->type())
  {
  case wtk::IterExpr::LITERAL:
  {
    return this->literal(e->literal());
  }
  case wtk::IterExpr::ITERATOR:
  {
    return this->name(e->name());
  }
  case wtk::IterExpr::ADD:
  {
    return this->add(this->duplicate(e->lhs()), this->duplicate(e->rhs()));
  }
  case wtk::IterExpr::SUB:
  {
    return this->sub(this->duplicate(e->lhs()), this->duplicate(e->rhs()));
  }
  case wtk::IterExpr::MUL:
  {
    return this->mul(this->duplicate(e->lhs()), this->duplicate(e->rhs()));
  }
  case wtk::IterExpr::DIV:
  {
    return this->div(this->duplicate(e->lhs()), e->literal());
  }
  }

  // I think this is unreachable.
  return nullptr;
}

template<typename Number_T>
IterExprWireListBuilder<Number_T>::IterExprWireListBuilder(
    TreePools<Number_T>* p)
  : pools(p), list(p->iterExprWireLists.allocate()) { }

template<typename Number_T>
void IterExprWireListBuilder<Number_T>::single(
    wtk::irregular::TextIterExpr* expr)
{
  this->list->types.emplace_back(wtk::IterExprWireList::SINGLE);
  this->list->elements.emplace_back();
  this->list->elements.back().single = expr;
}

template<typename Number_T>
void IterExprWireListBuilder<Number_T>::range(
    wtk::irregular::TextIterExpr* first, wtk::irregular::TextIterExpr* last)
{
  wtk::irregular::TextIterExprWireRange* range =
    this->pools->iterExprWireRanges.allocate();
  range->first_ = first;
  range->last_ = last;

  this->list->types.emplace_back(wtk::IterExprWireList::RANGE);
  this->list->elements.emplace_back();
  this->list->elements.back().range = range;
}

template<typename Number_T>
void IterExprWireListBuilder<Number_T>::duplicate(
    wtk::IterExprWireList* list, IterExprBuilder<Number_T>* builder)
{
  for(size_t i = 0; i < list->size(); i++)
  {
    switch(list->type(i))
    {
    case wtk::IterExprWireList::SINGLE:
    {
      this->single(builder->duplicate(list->single(i)));
      break;
    }
    case wtk::IterExprWireList::RANGE:
    {
      this->range(builder->duplicate(list->range(i)->first()),
          builder->duplicate(list->range(i)->last()));
      break;
    }
    }
  }
}

template<typename Number_T>
TreeBuilder<Number_T>::TreeBuilder(TreePools<Number_T>* p)
  : pools(p), node(p->directiveLists.allocate()) { }

template<typename Number_T>
void TreeBuilder<Number_T>::binaryGate(wtk::BinaryGate* gate)
{
  this->binaryGate(gate->outputWire(),
      gate->calculation(), gate->leftWire(), gate->rightWire());
}

template<typename Number_T>
void TreeBuilder<Number_T>::binaryGate(wtk::index_t out,
    wtk::BinaryGate::Calculation calc, wtk::index_t left, wtk::index_t right)
{
  wtk::irregular::TextBinaryGate* gate = this->pools->binaryGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = calc;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::addGate(
    wtk::index_t out, wtk::index_t left, wtk::index_t right)
{
  wtk::irregular::TextBinaryGate* gate = this->pools->binaryGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryGate::ADD;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::mulGate(
    wtk::index_t out, wtk::index_t left, wtk::index_t right)
{
  wtk::irregular::TextBinaryGate* gate = this->pools->binaryGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryGate::MUL;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::andGate(
    wtk::index_t out, wtk::index_t left, wtk::index_t right)
{
  wtk::irregular::TextBinaryGate* gate = this->pools->binaryGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryGate::AND;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::xorGate(
    wtk::index_t out, wtk::index_t left, wtk::index_t right)
{
  wtk::irregular::TextBinaryGate* gate = this->pools->binaryGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryGate::XOR;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::BINARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::binaryConstGate(
    wtk::BinaryConstGate<Number_T>* gate)
{
  this->binaryConstGate(gate->outputWire(), gate->calculation(),
      gate->leftWire(), gate->rightValue());
}

template<typename Number_T>
void TreeBuilder<Number_T>::binaryConstGate(wtk::index_t out,
    typename wtk::BinaryConstGate<Number_T>::Calculation calc,
    wtk::index_t left, Number_T right)
{
  wtk::irregular::TextBinaryConstGate<Number_T>* gate =
    this->pools->binaryConstGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = calc;

  this->node->types.emplace_back(
      wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryConstGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::addcGate(
    wtk::index_t out, wtk::index_t left, Number_T right)
{
  wtk::irregular::TextBinaryConstGate<Number_T>* gate =
    this->pools->binaryConstGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryConstGate<Number_T>::ADDC;

  this->node->types.emplace_back(
      wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryConstGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::mulcGate(
    wtk::index_t out, wtk::index_t left, Number_T right)
{
  wtk::irregular::TextBinaryConstGate<Number_T>* gate =
    this->pools->binaryConstGates.allocate();
  gate->out = out;
  gate->left = left;
  gate->right = right;
  gate->calc = wtk::BinaryConstGate<Number_T>::MULC;

  this->node->types.emplace_back(
      wtk::DirectiveList<Number_T>::BINARY_CONST_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().binaryConstGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::unaryGate(wtk::UnaryGate* gate)
{
  this->unaryGate(gate->outputWire(), gate->calculation(), gate->inputWire());
}

template<typename Number_T>
void TreeBuilder<Number_T>::unaryGate(wtk::index_t out,
    wtk::UnaryGate::Calculation calc, wtk::index_t in)
{
  wtk::irregular::TextUnaryGate* gate = this->pools->unaryGates.allocate();
  gate->out = out;
  gate->in = in;
  gate->calc = calc;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().unaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::notGate(wtk::index_t out, wtk::index_t in)
{
  wtk::irregular::TextUnaryGate* gate = this->pools->unaryGates.allocate();
  gate->out = out;
  gate->in = in;
  gate->calc = wtk::UnaryGate::NOT;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().unaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::copy(wtk::index_t out, wtk::index_t in)
{
  wtk::irregular::TextUnaryGate* gate = this->pools->unaryGates.allocate();
  gate->out = out;
  gate->in = in;
  gate->calc = wtk::UnaryGate::COPY;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::UNARY_GATE);
  this->node->elements.emplace_back();
  this->node->elements.back().unaryGate = gate;
}

template<typename Number_T>
void TreeBuilder<Number_T>::assign(wtk::Assign<Number_T>* assign)
{
  this->assign(assign->outputWire(), assign->constValue());
}

template<typename Number_T>
void TreeBuilder<Number_T>::assign(wtk::index_t out, Number_T val)
{
  wtk::irregular::TextAssign<Number_T>* assign =
    this->pools->assigns.allocate();
  assign->out = out;
  assign->value = val;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::ASSIGN);
  this->node->elements.emplace_back();
  this->node->elements.back().assign = assign;
}

template<typename Number_T>
void TreeBuilder<Number_T>::input(wtk::Input* input)
{
  this->input(input->outputWire(), input->stream());
}

template<typename Number_T>
void TreeBuilder<Number_T>::input(wtk::index_t out, wtk::Input::Stream stream)
{
  wtk::irregular::TextInput* input = this->pools->inputs.allocate();
  input->out = out;
  input->stream_ = stream;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::INPUT);
  this->node->elements.emplace_back();
  this->node->elements.back().input = input;
}

template<typename Number_T>
void TreeBuilder<Number_T>::instance(wtk::index_t out)
{
  this->input(out, wtk::Input::INSTANCE);
}

template<typename Number_T>
void TreeBuilder<Number_T>::shortWitness(wtk::index_t out)
{
  this->input(out, wtk::Input::SHORT_WITNESS);
}

template<typename Number_T>
void TreeBuilder<Number_T>::deleteSingle(wtk::Terminal* term)
{
  this->deleteSingle(term->wire());
}

template<typename Number_T>
void TreeBuilder<Number_T>::deleteSingle(wtk::index_t wire)
{
  wtk::irregular::TextTerminal* term = this->pools->terminals.allocate();
  term->wire_ = wire;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::DELETE_SINGLE);
  this->node->elements.emplace_back();
  this->node->elements.back().deleteSingle = term;
}

template<typename Number_T>
void TreeBuilder<Number_T>::deleteRange(wtk::WireRange* range)
{
  this->deleteRange(range->first(), range->last());
}

template<typename Number_T>
void TreeBuilder<Number_T>::deleteRange(wtk::index_t first, wtk::index_t last)
{
  wtk::irregular::TextWireRange* range = this->pools->wireRanges.allocate();
  range->firstIdx = first;
  range->lastIdx = last;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::DELETE_RANGE);
  this->node->elements.emplace_back();
  this->node->elements.back().deleteRange = range;
}

template<typename Number_T>
void TreeBuilder<Number_T>::assertZero(wtk::Terminal* term)
{
  this->assertZero(term->wire());
}

template<typename Number_T>
void TreeBuilder<Number_T>::assertZero(wtk::index_t wire)
{
  wtk::irregular::TextTerminal* term = this->pools->terminals.allocate();
  term->wire_ = wire;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::ASSERT_ZERO);
  this->node->elements.emplace_back();
  this->node->elements.back().assertZero = term;
}

template<typename Number_T>
void TreeBuilder<Number_T>::functionInvoke(
    WireListBuilder<Number_T>* outputs,
    char const* name,
    WireListBuilder<Number_T>* inputs)
{
  wtk::irregular::TextFunctionInvoke* invoke =
    this->pools->functionInvokes.allocate();
  invoke->outList = outputs->list;
  invoke->inList = inputs->list;
  invoke->name_ = this->pools->strdup(name);

  this->node->types.emplace_back(
      wtk::DirectiveList<Number_T>::FUNCTION_INVOKE);
  this->node->elements.emplace_back();
  this->node->elements.back().functionInvoke = invoke;
}

template<typename Number_T>
void TreeBuilder<Number_T>::anonFunction(
    WireListBuilder<Number_T>* outputs,
    WireListBuilder<Number_T>* inputs,
    wtk::index_t instance_count,
    wtk::index_t witness_count,
    TreeBuilder<Number_T>* body)
{
  wtk::irregular::TextAnonFunction<Number_T>* anon =
    this->pools->anonFunctions.allocate();
  anon->outList = outputs->list;
  anon->inList = inputs->list;
  anon->insCount = instance_count;
  anon->witCount = witness_count;
  anon->directives = body->node;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::ANON_FUNCTION);
  this->node->elements.emplace_back();
  this->node->elements.back().anonFunction = anon;
}

template<typename Number_T>
void TreeBuilder<Number_T>::forLoopInvoke(
    WireListBuilder<Number_T>* outputs,
    char const* iter_name,
    wtk::index_t first,
    wtk::index_t last,
    IterExprWireListBuilder<Number_T>* iter_outputs,
    char const* func_name,
    IterExprWireListBuilder<Number_T>* iter_inputs)
{
  wtk::irregular::TextForLoop<Number_T>* loop =
    this->pools->forLoops.allocate();
  wtk::irregular::TextIterExprFunctionInvoke* invoke =
    this->pools->iterExprFunctionInvokes.allocate();

  loop->outList = outputs->list;
  loop->first_ = first;
  loop->last_ = last;
  loop->iterator = this->pools->findRegisteredIter(iter_name);
  loop->bodyType_ = wtk::ForLoop<Number_T>::INVOKE;
  loop->invoke = invoke;

  invoke->outList = iter_outputs->list;
  invoke->name_ = this->pools->strdup(func_name);
  invoke->inList = iter_inputs->list;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
  this->node->elements.emplace_back();
  this->node->elements.back().forLoop = loop;
}

template<typename Number_T>
void TreeBuilder<Number_T>::forLoopAnonymous(
    WireListBuilder<Number_T>* outputs,
    char const* iter_name,
    wtk::index_t first,
    wtk::index_t last,
    IterExprWireListBuilder<Number_T>* iter_outputs,
    IterExprWireListBuilder<Number_T>* iter_inputs,
    wtk::index_t num_instance,
    wtk::index_t num_witness,
    TreeBuilder<Number_T>* body)
{
  wtk::irregular::TextForLoop<Number_T>* loop =
    this->pools->forLoops.allocate();
  wtk::irregular::TextIterExprAnonFunction<Number_T>* anon =
    this->pools->iterExprAnonFunctions.allocate();

  loop->outList = outputs->list;
  loop->first_ = first;
  loop->last_ = last;
  loop->iterator = this->pools->findRegisteredIter(iter_name);
  loop->bodyType_ = wtk::ForLoop<Number_T>::ANONYMOUS;
  loop->anonymous = anon;

  anon->outList = iter_outputs->list;
  anon->inList = iter_inputs->list;
  anon->insCount = num_instance;
  anon->witCount = num_witness;
  anon->directives = body->node;

  this->node->types.emplace_back(wtk::DirectiveList<Number_T>::FOR_LOOP);
  this->node->elements.emplace_back();
  this->node->elements.back().forLoop = loop;
}

} } // namespace wtk::converters
