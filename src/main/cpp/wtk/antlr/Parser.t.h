/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace antlr {

using namespace wtk_gen_antlr;

template<typename Number_T>
bool ArithmeticParser<Number_T>::parseStream(
    ArithmeticStreamHandler<Number_T>* handler)
{
  if(this->gateSet->gateSet !=  GateSet::arithmetic)
  {
    log_error("%s: Gate set must be arithmetic when using streaming API.",
        this->fname);
    return false;
  }

  if(!this->featureToggles->simple())
  {
    log_error("%s: Feature toggles must be 'simple' when using streaming API.",
        this->fname);
    return false;
  }

  SIEVEIRParser::RelationBodyContext* rel = this->parser.relationBody();
  if(rel != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    if(rel->functionDeclare().size() != 0)
    {
      log_error(
          "%s:%zu: Function gates are forbidden when using streaming API\n",
          this->fname, rel->functionDeclare()[0]->getStart()->getLine());
      return false;
    }

    for(SIEVEIRParser::DirectiveContext* directive
        : rel->directiveList()->directive())
    {
      if(directive->binaryGate() != nullptr)
      {
        SIEVEIRParser::BinaryGateContext* gate = directive->binaryGate();

        wtk::index_t out;
        wtk::index_t left;
        wtk::index_t right;

        wire_to_uint(gate->WIRE_NUM(0)->getText(), out);
        wire_to_uint(gate->WIRE_NUM(1)->getText(), left);
        wire_to_uint(gate->WIRE_NUM(2)->getText(), right);

        if(!checkBinaryGate(gate->binaryGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.\n",
              this->fname, gate->binaryGateType()->getStart()->getLine(),
              gate->binaryGateType()->getText().c_str());
          return false;
        }

        handler->setLineNum(gate->getStart()->getLine());

        if(gate->binaryGateType()->ADD() != nullptr)
        {
          handler->handleAdd(out, left, right);
        }
        else
        {
          handler->handleMul(out, left, right);
        }
      }
      else if(directive->binaryConstGate() != nullptr)
      {
        SIEVEIRParser::BinaryConstGateContext* gate =
          directive->binaryConstGate();

        wtk::index_t out;
        wtk::index_t left;
        Number_T right;

        wire_to_uint(gate->WIRE_NUM(0)->getText(), out);
        wire_to_uint(gate->WIRE_NUM(1)->getText(), left);
        num_to_uint(gate->fieldLiteral()->NUMERIC_LITERAL()->getText(), right);

        if(!checkBinaryConstGate(gate->binaryConstGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.\n",
              this->fname, gate->binaryConstGateType()->getStart()->getLine(),
              gate->binaryConstGateType()->getText().c_str());
          return false;
        }

        handler->setLineNum(gate->getStart()->getLine());

        if(gate->binaryConstGateType()->ADDC() != nullptr)
        {
          handler->handleAddC(out, left, right);
        }
        else
        {
          handler->handleMulC(out, left, right);
        }
      }
      else if(directive->unaryGate() != nullptr)
      {
        log_error("%s:%zu: Gate '%s' is forbidden by gate set'\n",
            this->fname, directive->unaryGate()->getStart()->getLine(),
            directive->unaryGate()->unaryGateType()->getText().c_str());
        return false;
      }
      else if(directive->input() != nullptr)
      {
        SIEVEIRParser::InputContext* input = directive->input();

        wtk::index_t out;

        wire_to_uint(input->WIRE_NUM()->getText(), out);

        handler->setLineNum(input->getStart()->getLine());

        if(input->INSTANCE() != nullptr)
        {
          handler->handleInstance(out);
        }
        else
        {
          handler->handleShortWitness(out);
        }
      }
      else if(directive->copy() != nullptr)
      {
        SIEVEIRParser::CopyContext* copy = directive->copy();

        wtk::index_t out;
        wtk::index_t left;

        wire_to_uint(copy->WIRE_NUM(0)->getText(), out);
        wire_to_uint(copy->WIRE_NUM(1)->getText(), left);

        handler->setLineNum(copy->getStart()->getLine());
        handler->handleCopy(out, left);
      }
      else if(directive->assign() != nullptr)
      {
        SIEVEIRParser::AssignContext* assign = directive->assign();

        wtk::index_t out;
        Number_T left;

        wire_to_uint(assign->WIRE_NUM()->getText(), out);
        num_to_uint(
            assign->fieldLiteral()->NUMERIC_LITERAL()->getText(), left);

        handler->setLineNum(assign->getStart()->getLine());
        handler->handleAssign(out, left);
      }
      else if(directive->assertZero() != nullptr)
      {
        SIEVEIRParser::AssertZeroContext* assertZero = directive->assertZero();

        wtk::index_t left;

        wire_to_uint(assertZero->WIRE_NUM()->getText(), left);

        handler->setLineNum(assertZero->getStart()->getLine());
        handler->handleAssertZero(left);
      }
      else if(directive->deleteSingle() != nullptr)
      {
        SIEVEIRParser::DeleteSingleContext* del = directive->deleteSingle();

        wtk::index_t wire;

        wire_to_uint(del->WIRE_NUM()->getText(), wire);

        handler->setLineNum(del->getStart()->getLine());
        handler->handleDeleteSingle(wire);
      }
      else if(directive->deleteRange() != nullptr)
      {
        SIEVEIRParser::DeleteRangeContext* del = directive->deleteRange();

        wtk::index_t first;
        wtk::index_t last;

        wire_to_uint(del->WIRE_NUM(0)->getText(), first);
        wire_to_uint(del->WIRE_NUM(1)->getText(), last);

        handler->setLineNum(del->getStart()->getLine());
        handler->handleDeleteRange(first, last);
      }
      else if(directive->functionInvoke() != nullptr)
      {
        log_error("%s:%zu: Function invocation forbidden by feature toggles.",
            this->fname, directive->functionInvoke()->getStart()->getLine());
        return false;
      }
      else if(directive->anonFunction() != nullptr)
      {
        log_error("%s:%zu: Anonymous function forbidden by feature toggles.",
            this->fname, directive->anonFunction()->getStart()->getLine());
        return false;
      }
      else if(directive->forLoop() != nullptr)
      {
        log_error("%s:%zu: For loops forbidden by feature toggles.",
            this->fname, directive->forLoop()->getStart()->getLine());
        return false;
      }
      else if(directive->switchStatement() != nullptr)
      {
        log_error("%s:%zu: Switch statements forbidden by feature toggles.",
            this->fname, directive->switchStatement()->getStart()->getLine());
        return false;
      }
    }

    handler->handleEnd();
    return true;
  }

  return false;
}

template<typename Number_T>
AntlrIRTree<Number_T>* ArithmeticParser<Number_T>::parseTree()
{
  SIEVEIRParser::RelationBodyContext* rel = this->parser.relationBody();
  if(rel != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->treeParser = std::unique_ptr<TreeParser<Number_T>>(
        new TreeParser<Number_T>(
          this->gateSet, this->featureToggles, this->fname));
    return this->treeParser->parseTree(rel);
  }
  else
  {
    return nullptr;
  }
}

template<typename Number_T>
void parseInputStream(
    SIEVEIRParser::InputStreamContext* in,
    wtk::QueueInputStream<Number_T>* stream)
{
  for(SIEVEIRParser::FieldLiteralContext* num : in->fieldLiteral())
  {
    Number_T n(0);
    num_to_uint(num->NUMERIC_LITERAL()->getText(), n);
    stream->insert(n, num->getStart()->getLine());
  }
}

template<typename Number_T>
wtk::QueueInputStream<Number_T>* ArithmeticParser<Number_T>::instance()
{
  SIEVEIRParser::InputStreamContext* ctx = this->parser.inputStream();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<Number_T>>(
          new wtk::QueueInputStream<Number_T>());
    parseInputStream(ctx, this->inputStream.get());
  }
  else
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<Number_T>>(
        new wtk::QueueInputStream<Number_T>(true));
  }

  return this->inputStream.get();
}

template<typename Number_T>
wtk::QueueInputStream<Number_T>* ArithmeticParser<Number_T>::shortWitness()
{
  SIEVEIRParser::InputStreamContext* ctx = this->parser.inputStream();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<Number_T>>(
          new wtk::QueueInputStream<Number_T>());
    parseInputStream(ctx, this->inputStream.get());
  }
  else
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<Number_T>>(
        new wtk::QueueInputStream<Number_T>(true));
  }

  return this->inputStream.get();
}

bool BooleanParser::parseStream(BooleanStreamHandler* handler)
{
  if(this->gateSet->gateSet !=  GateSet::boolean)
  {
    log_error("%s: Gate set must be boolean when using streaming API.",
        this->fname);
    return false;
  }

  if(!this->featureToggles->simple())
  {
    log_error("%s: Feature toggles must be 'simple' when using streaming API.",
        this->fname);
    return false;
  }

  SIEVEIRParser::RelationBodyContext* rel = this->parser.relationBody();
  if(rel != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    if(rel->functionDeclare().size() != 0)
    {
      log_error(
          "%s:%zu: Function gates are forbidden when using streaming API\n",
          this->fname, rel->functionDeclare()[0]->getStart()->getLine());
      return false;
    }

    for(SIEVEIRParser::DirectiveContext* directive
        : rel->directiveList()->directive())
    {
      if(directive->binaryGate() != nullptr)
      {
        SIEVEIRParser::BinaryGateContext* gate = directive->binaryGate();

        wtk::index_t out;
        wtk::index_t left;
        wtk::index_t right;

        wire_to_uint(gate->WIRE_NUM(0)->getText(), out);
        wire_to_uint(gate->WIRE_NUM(1)->getText(), left);
        wire_to_uint(gate->WIRE_NUM(2)->getText(), right);

        if(!checkBinaryGate(gate->binaryGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.\n",
              this->fname, gate->binaryGateType()->getStart()->getLine(),
              gate->binaryGateType()->getText().c_str());
          return false;
        }

        handler->setLineNum(gate->getStart()->getLine());

        if(gate->binaryGateType()->XOR() != nullptr)
        {
          handler->handleXor(out, left, right);
        }
        else
        {
          handler->handleAnd(out, left, right);
        }
      }
      else if(directive->binaryConstGate() != nullptr)
      {
        log_error("%s:%zu: Gate '%s' is forbidden by gate set'\n",
            this->fname, directive->binaryConstGate()->getStart()->getLine(),
            directive->binaryConstGate()->binaryConstGateType()
            ->getText().c_str());
        return false;
      }
      else if(directive->unaryGate() != nullptr)
      {
        SIEVEIRParser::UnaryGateContext* gate = directive->unaryGate();

        wtk::index_t out;
        wtk::index_t left;

        wire_to_uint(gate->WIRE_NUM(0)->getText(), out);
        wire_to_uint(gate->WIRE_NUM(1)->getText(), left);

        if(!checkUnaryGate(gate->unaryGateType(), this->gateSet))
        {
          log_error("%s:%zu: Gate '%s' is forbidden by gate set.\n",
              this->fname, gate->unaryGateType()->getStart()->getLine(),
              gate->unaryGateType()->getText().c_str());
          return false;
        }

        handler->setLineNum(gate->getStart()->getLine());

        if(gate->unaryGateType()->NOT() != nullptr)
        {
          handler->handleNot(out, left);
        }
      }
      else if(directive->input() != nullptr)
      {
        SIEVEIRParser::InputContext* input = directive->input();

        wtk::index_t out;

        wire_to_uint(input->WIRE_NUM()->getText(), out);

        handler->setLineNum(input->getStart()->getLine());

        if(input->INSTANCE() != nullptr)
        {
          handler->handleInstance(out);
        }
        else
        {
          handler->handleShortWitness(out);
        }
      }
      else if(directive->copy() != nullptr)
      {
        SIEVEIRParser::CopyContext* copy = directive->copy();

        wtk::index_t out;
        wtk::index_t left;

        wire_to_uint(copy->WIRE_NUM(0)->getText(), out);
        wire_to_uint(copy->WIRE_NUM(1)->getText(), left);

        handler->setLineNum(copy->getStart()->getLine());
        handler->handleCopy(out, left);
      }
      else if(directive->assign() != nullptr)
      {
        SIEVEIRParser::AssignContext* assign = directive->assign();

        wtk::index_t out;
        uint8_t left;

        wire_to_uint(assign->WIRE_NUM()->getText(), out);
        num_to_uint(
            assign->fieldLiteral()->NUMERIC_LITERAL()->getText(), left);

        handler->setLineNum(assign->getStart()->getLine());
        handler->handleAssign(out, left);
      }
      else if(directive->assertZero() != nullptr)
      {
        SIEVEIRParser::AssertZeroContext* assertZero = directive->assertZero();

        wtk::index_t left;

        wire_to_uint(assertZero->WIRE_NUM()->getText(), left);

        handler->setLineNum(assertZero->getStart()->getLine());
        handler->handleAssertZero(left);
      }
      else if(directive->deleteSingle() != nullptr)
      {
        SIEVEIRParser::DeleteSingleContext* del = directive->deleteSingle();

        wtk::index_t wire;

        wire_to_uint(del->WIRE_NUM()->getText(), wire);

        handler->setLineNum(del->getStart()->getLine());
        handler->handleDeleteSingle(wire);
      }
      else if(directive->deleteRange() != nullptr)
      {
        SIEVEIRParser::DeleteRangeContext* del = directive->deleteRange();

        wtk::index_t first;
        wtk::index_t last;

        wire_to_uint(del->WIRE_NUM(0)->getText(), first);
        wire_to_uint(del->WIRE_NUM(1)->getText(), last);

        handler->setLineNum(del->getStart()->getLine());
        handler->handleDeleteRange(first, last);
      }
      else if(directive->functionInvoke() != nullptr)
      {
        log_error("%s:%zu: Function invocation forbidden by feature toggles.",
            this->fname, directive->functionInvoke()->getStart()->getLine());
        return false;
      }
      else if(directive->anonFunction() != nullptr)
      {
        log_error("%s:%zu: Anonymous function forbidden by feature toggles.",
            this->fname, directive->anonFunction()->getStart()->getLine());
        return false;
      }
      else if(directive->forLoop() != nullptr)
      {
        log_error("%s:%zu: For loops forbidden by feature toggles.",
            this->fname, directive->forLoop()->getStart()->getLine());
        return false;
      }
      else if(directive->switchStatement() != nullptr)
      {
        log_error("%s:%zu: Switch statements forbidden by feature toggles.",
            this->fname, directive->switchStatement()->getStart()->getLine());
        return false;
      }
    }

    handler->handleEnd();
    return true;
  }

  return false;
}

AntlrIRTree<uint8_t>* BooleanParser::parseTree()
{
  SIEVEIRParser::RelationBodyContext* rel = this->parser.relationBody();
  if(rel != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->treeParser = std::unique_ptr<TreeParser<uint8_t>>(
        new TreeParser<uint8_t>(
          this->gateSet, this->featureToggles, this->fname));
    return this->treeParser->parseTree(rel);
  }
  else
  {
    return nullptr;
  }
}

wtk::QueueInputStream<uint8_t>* BooleanParser::instance()
{
  SIEVEIRParser::InputStreamContext* ctx = this->parser.inputStream();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<uint8_t>>(
          new wtk::QueueInputStream<uint8_t>());
    parseInputStream(ctx, this->inputStream.get());
  }
  else
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<uint8_t>>(
        new wtk::QueueInputStream<uint8_t>(true));
  }

  return this->inputStream.get();
}

wtk::QueueInputStream<uint8_t>* BooleanParser::shortWitness()
{
  SIEVEIRParser::InputStreamContext* ctx = this->parser.inputStream();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<uint8_t>>(
          new wtk::QueueInputStream<uint8_t>());
    parseInputStream(ctx, this->inputStream.get());
  }
  else
  {
    this->inputStream = std::unique_ptr<wtk::QueueInputStream<uint8_t>>(
        new wtk::QueueInputStream<uint8_t>(true));
  }

  return this->inputStream.get();
}

template<typename Number_T>
Parser<Number_T>::Parser(std::string& str)
  : instream(str),
    antlrStream(this->instream),
    lexer(&this->antlrStream),
    tknStream(&this->lexer),
    parser(&this->tknStream),
    errorListener(str)
{
  this->lexer.getErrorListenerDispatch().removeErrorListeners();
  this->lexer.getErrorListenerDispatch().addErrorListener(
      &this->errorListener);
  this->parser.getErrorListenerDispatch().removeErrorListeners();
  this->parser.getErrorListenerDispatch().addErrorListener(
      &this->errorListener);
}
 

template<typename Number_T>
bool Parser<Number_T>::parseHeader()
{
  SIEVEIRParser::HeaderContext* ctx = this->parser.header();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    num_to_uint(ctx->versionDecl()->NUMERIC_LITERAL(0)->getText(),
        this->version.major);
    num_to_uint(ctx->versionDecl()->NUMERIC_LITERAL(1)->getText(),
        this->version.minor);
    num_to_uint(ctx->versionDecl()->NUMERIC_LITERAL(2)->getText(),
        this->version.patch);

    num_to_uint(
        ctx->fieldDecl()->NUMERIC_LITERAL(0)->getText(), this->characteristic);
    num_to_uint(ctx->fieldDecl()->NUMERIC_LITERAL(1)->getText(), this->degree);
    
    return true;
  }
  return false;
}

template<typename Number_T>
bool Parser<Number_T>::parseResource()
{
  SIEVEIRParser::ResourceDeclContext* ctx = this->parser.resourceDecl();
  if(ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    if(ctx->RES_RELATION() != nullptr)
    {
      this->resource = Resource::relation;
    }
    else if(ctx->RES_INSTANCE() != nullptr)
    {
      this->resource = Resource::instance;
    }
    else
    {
      this->resource = Resource::shortWitness;
    }

    return true;
  }
  return false;
}

template<typename Number_T>
bool Parser<Number_T>::parseParameters()
{
  SIEVEIRParser::GateSetContext* gateset_ctx = this->parser.gateSet();
  if(gateset_ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
  {
    SIEVEIRParser::FeatureTogglesContext* toggle_ctx =
      this->parser.featureToggles();
    if(toggle_ctx != nullptr && this->parser.getNumberOfSyntaxErrors() == 0)
    {
      if(gateset_ctx->BOOLEAN() != nullptr)
      {
        this->gateSet.gateSet =  GateSet::boolean;
        this->gateSet.enableXor = true;
        this->gateSet.enableAnd = true;
        this->gateSet.enableNot = true;
      }
      else if(gateset_ctx->booleanGateNames().size() != 0)
      {
        this->gateSet.gateSet = GateSet::boolean;
        this->gateSet.enableXor = false;
        this->gateSet.enableAnd = false;
        this->gateSet.enableNot = false;

        for(SIEVEIRParser::BooleanGateNamesContext* gate
            : gateset_ctx->booleanGateNames())
        {
          if(gate->XOR() != nullptr)      { this->gateSet.enableXor = true; }
          else if(gate->AND() != nullptr) { this->gateSet.enableAnd = true; }
          else if(gate->NOT() != nullptr) { this->gateSet.enableNot = true; }
        }
      }
      else if(gateset_ctx->ARITHMETIC() != nullptr)
      {
        this->gateSet.gateSet =  GateSet::arithmetic;
        this->gateSet.enableAdd = true;
        this->gateSet.enableAddC = true;
        this->gateSet.enableMul = true;
        this->gateSet.enableMulC = true;
      }
      else
      {
        this->gateSet.gateSet = GateSet::arithmetic;
        this->gateSet.enableAdd = false;
        this->gateSet.enableAddC = false;
        this->gateSet.enableMul = false;
        this->gateSet.enableMulC = false;

        for(SIEVEIRParser::ArithmeticGateNamesContext* gate
            : gateset_ctx->arithmeticGateNames())
        {
          if(gate->ADD() != nullptr)       { this->gateSet.enableAdd = true; }
          else if(gate->ADDC() != nullptr) { this->gateSet.enableAddC = true; }
          else if(gate->MUL() != nullptr)  { this->gateSet.enableMul = true; }
          else if(gate->MULC() != nullptr) { this->gateSet.enableMulC = true; }
        }
      }

      this->featureToggles.functionToggle = false;
      this->featureToggles.forLoopToggle = false;
      this->featureToggles.switchCaseToggle = false;

      for(SIEVEIRParser::FeatureNameContext* feature
          : toggle_ctx->featureName())
      {
        if(feature->FUNCTION() != nullptr)
        {
          this->featureToggles.functionToggle = true;
        }
        else if(feature->FOR() != nullptr)
        {
          this->featureToggles.forLoopToggle = true;
        }
        else if(feature->SWITCH() != nullptr)
        {
          this->featureToggles.switchCaseToggle = true;
        }
      }

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

template<typename Number_T>
ArithmeticParser<Number_T>* Parser<Number_T>::arithmetic()
{
  this->arithmeticNumParser = std::unique_ptr<ArithmeticParser<Number_T>>(
      new ArithmeticParser<Number_T>(this->parser, &this->gateSet,
        &this->featureToggles, this->errorListener.name()));

  return this->arithmeticNumParser.get();
}

template<typename Number_T>
ArithmeticParser<uint64_t>* Parser<Number_T>::arithmetic64()
{
  this->arithmetic64Parser = std::unique_ptr<ArithmeticParser<uint64_t>>(
      new ArithmeticParser<uint64_t>(this->parser, &this->gateSet,
        &this->featureToggles, this->errorListener.name()));

  return this->arithmetic64Parser.get();
}

template<typename Number_T>
ArithmeticParser<uint32_t>* Parser<Number_T>::arithmetic32()
{
  this->arithmetic32Parser = std::unique_ptr<ArithmeticParser<uint32_t>>(
      new ArithmeticParser<uint32_t>(this->parser, &this->gateSet,
        &this->featureToggles, this->errorListener.name()));

  return this->arithmetic32Parser.get();
}

template<typename Number_T>
BooleanParser* Parser<Number_T>::boolean()
{
  this->booleanParser = std::unique_ptr<BooleanParser>(
      new BooleanParser(this->parser, &this->gateSet, &this->featureToggles,
        this->errorListener.name()));

  return this->booleanParser.get();
}

} } // namespace wtk::antlr
