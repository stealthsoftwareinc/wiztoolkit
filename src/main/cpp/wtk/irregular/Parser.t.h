/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include<wtk/irregular/checkMacros.t.h>

namespace wtk {
namespace irregular {

template<typename Number_T>
wtk::StreamStatus TextInputStream<Number_T>::next(
    Number_T* num)
{
  bool check = whitespace(&this->ctx);
  LiteralOrEnd litOrEnd = literalOrEnd(&this->ctx);


  if(UNLIKELY(!check || litOrEnd == LiteralOrEnd::invalid))
  {
    return wtk::StreamStatus::error;
  }
  else if(LIKELY(litOrEnd == LiteralOrEnd::literal))
  {
    check = whitespace(&this->ctx);
    this->ctx.updateMark();

    NumericBase base = numericLiteral(&this->ctx);
    if(UNLIKELY(!to_uint(base, *num, &this->ctx)))
    {
      return wtk::StreamStatus::error;
    }

    check = whitespace(&this->ctx);
    check = fieldLiteralEnd(&this->ctx) & check;
    check = whitespace(&this->ctx) & check;
    check = semiColon(&this->ctx) & check;

    if(UNLIKELY(!check))
    {
      return wtk::StreamStatus::error;
    }
    else
    {
      return wtk::StreamStatus::success;
    }
  }
  else // litOrEnd == LiteralOrEnd::end
  {
    return wtk::StreamStatus::end;
  }
}

template<typename Number_T>
bool ArithmeticParser<Number_T>::parseStream(
    wtk::ArithmeticStreamHandler<Number_T>* handler)
{
  if(!this->featureToggles->simple())
  {
    log_error("Streaming API does not support non-simple features");
    return false;
  }

  if(this->gateSet->gateSet != wtk::GateSet::arithmetic)
  {
    log_error("Arithmetic API does not support a non-arithmetic gate set");
    return false;
  }

  BEGIN
    WSPACE_AROUND(beginKw)
  END;

  bool cont = true;
  while(cont)
  {
    StreamingDirectiveTypes directiveTypes = streamingDirective(&this->ctx);

    BEGIN
      CHECK_EXPR(directiveTypes != StreamingDirectiveTypes::invalid)
    END;

    switch(directiveTypes)
    {
    case StreamingDirectiveTypes::gate:
    {
      // Output Wire
      wtk::index_t outWire;
      NUMBER(outWire);

      BEGIN
        WSPACE_AROUND(leftArrow)
      END;

      ArithStreamingDirectives arithDirectives =
        arithStreamingDirective(&this->ctx);

      switch(arithDirectives)
      {
      case ArithStreamingDirectives::add:  // fallthrough
      case ArithStreamingDirectives::addc: // fallthrough
      case ArithStreamingDirectives::mul:  // fallthrough
      case ArithStreamingDirectives::mulc:
      {
        BEGIN
          WSPACE_AROUND(leftParen)
          CHECK(wireNumberBegin)
        END;

        wtk::index_t leftWire;
        NUMBER(leftWire);

        BEGIN WSPACE_AROUND(comma) END_SEQ;

        if(arithDirectives == ArithStreamingDirectives::add
            || arithDirectives == ArithStreamingDirectives::mul)
        {
          BEGIN CHECK(wireNumberBegin) END;

          wtk::index_t rightWire;
          NUMBER(rightWire);

          BEGIN
            WSPACE
            CHECK(rightParen)
          END_SEQ;

          if(arithDirectives == ArithStreamingDirectives::add)
          {
            if(!this->gateSet->enableAdd)
            {
              log_error("Gate '@add' is forbidden by gate set");
              return false;
            }
            handler->handleAdd(outWire, leftWire, rightWire);
          }
          else
          {
            if(!this->gateSet->enableMul)
            {
              log_error("Gate '@mul' is forbidden by gate set");
              return false;
            }
            handler->handleMul(outWire, leftWire, rightWire);
          }
        }
        else
        {
          BEGIN WSPACE_AROUND(fieldLiteralBegin) END;

          NUMBER(this->constValue);

          BEGIN
            WSPACE_AROUND(fieldLiteralEnd)
            CHECK(rightParen)
          END_SEQ;

          if(arithDirectives == ArithStreamingDirectives::addc)
          {
            if(!this->gateSet->enableAddC)
            {
              log_error("Gate '@addc' is forbidden by gate set");
              return false;
            }
            handler->handleAddC(outWire, leftWire, this->constValue);
          }
          else
          {
            if(!this->gateSet->enableMulC)
            {
              log_error("Gate '@mulc' is forbidden by gate set");
              return false;
            }
            handler->handleMulC(outWire, leftWire, this->constValue);
          }
        }

        break;
      }
      case ArithStreamingDirectives::witness:
      {
        handler->handleShortWitness(outWire);
        break;
      }
      case ArithStreamingDirectives::instance:
      {
        handler->handleInstance(outWire);
        break;
      }
      case ArithStreamingDirectives::assign:
      {
        BEGIN WSPACE END;

        NUMBER(this->constValue);

        BEGIN
          WSPACE
          CHECK(fieldLiteralEnd)
        END_SEQ;

        handler->handleAssign(outWire, this->constValue);

        break;
      }
      case ArithStreamingDirectives::copy:
      {
        wtk::index_t inWire;
        NUMBER(inWire);

        handler->handleCopy(outWire, inWire);

        break;
      }
      case ArithStreamingDirectives::invalid:
        return false;
      }

      BEGIN
        WSPACE_AROUND(semiColon)
      END;
      break;
    }
    case StreamingDirectiveTypes::assert_zero:
    {
      BEGIN
        WSPACE_AROUND(leftParen)
        CHECK(wireNumberBegin)
      END;

      // Assert Input Wire
      wtk::index_t leftWire;
      NUMBER(leftWire);

      BEGIN
        WSPACE_AROUND(rightParen)
        WSPACE_AFTER(semiColon)
      END;

      handler->handleAssertZero(leftWire);

      break;
    }
    case StreamingDirectiveTypes::delete_:
    {
      BEGIN
        WSPACE_AROUND(leftParen)
        CHECK(wireNumberBegin)
      END;

      wtk::index_t first;
      NUMBER(first);

      BEGIN WSPACE END_SEQ;

      CommaOrRightParen corp = commaOrRightParen(&this->ctx);

      switch(corp)
      {
      case CommaOrRightParen::comma:
      {
        BEGIN
          WSPACE
          CHECK(wireNumberBegin)
          END;

        wtk::index_t last;
        NUMBER(last);

        BEGIN
          WSPACE_AROUND(rightParen)
          WSPACE_AFTER(semiColon)
        END;

        handler->handleDeleteRange(first, last);
        break;
      }
      case CommaOrRightParen::rightParen:
      {
        BEGIN
          WSPACE
          WSPACE_AFTER(semiColon)
        END;

        handler->handleDeleteSingle(first);
        break;
      }
      case CommaOrRightParen::invalid:
      {
        return false;
      }
      }

      break;
    }
    case StreamingDirectiveTypes::end:
    {
      cont = false;
      break;
    }
    case StreamingDirectiveTypes::invalid:
      return false;
    }
  }

  BEGIN WSPACE END;

  handler->handleEnd();
  return true;
}

template<typename Number_T>
TextIRTree<Number_T>*
ArithmeticParser<Number_T>::parseTree()
{
  this->treeParser = std::unique_ptr<TextTreeParser<Number_T>>(
      new TextTreeParser<Number_T>(
        this->ctx, this->gateSet, this->featureToggles));

  return this->treeParser->parseTree();
}

template<typename Number_T>
TextInputStream<Number_T>* ArithmeticParser<Number_T>::instance()
{
  BEGIN WSPACE_AROUND(beginKw) END_NULL;

  this->inputStream = std::unique_ptr<TextInputStream<Number_T>>(
      new TextInputStream<Number_T>(this->ctx));

  return this->inputStream.get();
}

template<typename Number_T>
TextInputStream<Number_T>* ArithmeticParser<Number_T>::shortWitness()
{
  BEGIN WSPACE_AROUND(beginKw) END_NULL;

  this->inputStream = std::unique_ptr<TextInputStream<Number_T>>(
      new TextInputStream<Number_T>(this->ctx));

  return this->inputStream.get();
}

bool BooleanParser::parseStream(wtk::BooleanStreamHandler* handler)
{
  if(!this->featureToggles->simple())
  {
    log_error("Streaming API does not support non-simple features");
    return false;
  }

  if(this->gateSet->gateSet != wtk::GateSet::boolean)
  {
    log_error("Boolean API does not support a non-boolean gate set");
    return false;
  }

  BEGIN
    WSPACE_AROUND(beginKw)
  END;

  bool cont = true;
  while(cont)
  {
    StreamingDirectiveTypes directiveTypes = streamingDirective(&this->ctx);

    this->ctx.updateMark();

    switch(directiveTypes)
    {
    case StreamingDirectiveTypes::gate:
    {
      // Output Wire
      wtk::index_t outWire;
      NUMBER(outWire);

      BEGIN
        WSPACE_AROUND(leftArrow)
      END;

      BoolStreamingDirectives boolDirectives =
        boolStreamingDirective(&this->ctx);

      switch(boolDirectives)
      {
      case BoolStreamingDirectives::xor_:  // fallthrough
      case BoolStreamingDirectives::and_:  // fallthrough
      case BoolStreamingDirectives::not_:
      {
        BEGIN
          WSPACE_AROUND(leftParen)
          CHECK(wireNumberBegin)
        END;

        wtk::index_t leftWire;
        NUMBER(leftWire);

        if(boolDirectives != BoolStreamingDirectives::not_)
        {
          BEGIN
            WSPACE_AROUND(comma)
            CHECK(wireNumberBegin)
          END;

          wtk::index_t rightWire;
          NUMBER(rightWire);

          BEGIN
            WSPACE
            CHECK(rightParen)
          END_SEQ;

          if(boolDirectives == BoolStreamingDirectives::xor_)
          {
            if(!this->gateSet->enableXor)
            {
              log_error("Gate '@xor' is forbidden by gate set");
              return false;
            }
            handler->handleXor(outWire, leftWire, rightWire);
          }
          else
          {
            if(!this->gateSet->enableAnd)
            {
              log_error("Gate '@and' is forbidden by gate set");
              return false;
            }
            handler->handleAnd(outWire, leftWire, rightWire);
          }
        }
        else
        {
          BEGIN
            WSPACE
            CHECK(rightParen)
          END_SEQ;

          if(!this->gateSet->enableNot)
          {
            log_error("Gate '@not' is forbidden by gate set");
            return false;
          }
          handler->handleNot(outWire, leftWire);
        }

        break;
      }
      case BoolStreamingDirectives::witness:
      {
        handler->handleShortWitness(outWire);
        break;
      }
      case BoolStreamingDirectives::instance:
      {
        handler->handleInstance(outWire);
        break;
      }
      case BoolStreamingDirectives::assign:
      {
        BEGIN WSPACE END;

        uint8_t boolConst;
        NUMBER(boolConst);

        BEGIN
          WSPACE
          CHECK(fieldLiteralEnd)
        END_SEQ;

        handler->handleAssign(outWire, boolConst);

        break;
      }
      case BoolStreamingDirectives::copy:
      {
        wtk::index_t inWire;
        NUMBER(inWire);

        handler->handleCopy(outWire, inWire);

        break;
      }
      case BoolStreamingDirectives::invalid:
        return false;
      }

      BEGIN
        WSPACE_AROUND(semiColon)
      END;
      break;
    }
    case StreamingDirectiveTypes::assert_zero:
    {
      BEGIN
        WSPACE_AROUND(leftParen)
        CHECK(wireNumberBegin)
      END;

      // Assert Input Wire
      wtk::index_t leftWire;
      NUMBER(leftWire);

      BEGIN
        WSPACE_AROUND(rightParen)
        WSPACE_AFTER(semiColon)
      END;

      handler->handleAssertZero(leftWire);

      break;
    }
    case StreamingDirectiveTypes::delete_:
    {
      BEGIN
        WSPACE_AROUND(leftParen)
        CHECK(wireNumberBegin)
      END;

      wtk::index_t first;
      NUMBER(first);

      BEGIN WSPACE END_SEQ;

      CommaOrRightParen corp = commaOrRightParen(&this->ctx);

      switch(corp)
      {
      case CommaOrRightParen::comma:
      {
        BEGIN
          WSPACE
          CHECK(wireNumberBegin)
          END;

        wtk::index_t last;
        NUMBER(last);

        BEGIN
          WSPACE_AROUND(rightParen)
          WSPACE_AFTER(semiColon)
        END;

        handler->handleDeleteRange(first, last);
        break;
      }
      case CommaOrRightParen::rightParen:
      {
        BEGIN
          WSPACE
          WSPACE_AFTER(semiColon)
        END;

        handler->handleDeleteSingle(first);
        break;
      }
      case CommaOrRightParen::invalid:
      {
        return false;
      }
      }

      break;
    }
    case StreamingDirectiveTypes::end:
    {
      cont = false;
      break;
    }
    case StreamingDirectiveTypes::invalid:
      return false;
    }
  };

  BEGIN WSPACE END;

  handler->handleEnd();
  return true;
}

TextIRTree<uint8_t>* BooleanParser::parseTree()
{
  this->treeParser = std::unique_ptr<TextTreeParser<uint8_t>>(
      new TextTreeParser<uint8_t>(
        this->ctx, this->gateSet, this->featureToggles));

  return this->treeParser->parseTree();
}

TextInputStream<uint8_t>* BooleanParser::instance()
{
  BEGIN WSPACE_AROUND(beginKw) END_NULL;

  this->inputStream = std::unique_ptr<TextInputStream<uint8_t>>(
      new TextInputStream<uint8_t>(this->ctx));

  return this->inputStream.get();
}

TextInputStream<uint8_t>* BooleanParser::shortWitness()
{
  BEGIN WSPACE_AROUND(beginKw) END_NULL;

  this->inputStream = std::unique_ptr<TextInputStream<uint8_t>>(
      new TextInputStream<uint8_t>(this->ctx));

  return this->inputStream.get();
}

template<typename Number_T>
bool Parser<Number_T>::parseHeader()
{
  BEGIN
    WSPACE_AROUND(versionKw)
  END;

  // Major version number
  BEGIN
    CHECK(decLiteral)
    CHECK_EXPR(to_uint(NumericBase::dec, this->version.major, &this->ctx))
  END;

  BEGIN
    WSPACE_AROUND(dot)
  END;

  // Minor version number
  BEGIN
    CHECK(decLiteral)
    CHECK_EXPR(to_uint(NumericBase::dec, this->version.minor, &this->ctx))
  END;

  BEGIN
    WSPACE_AROUND(dot)
  END;

  // Patch version number
  BEGIN
    CHECK(decLiteral)
    CHECK_EXPR(to_uint(NumericBase::dec, this->version.patch, &this->ctx))
  END;

  BEGIN
    WSPACE_AROUND(semiColon)
    WSPACE_AFTER(fieldKw)
    WSPACE_AFTER(characteristicKw)
  END;

  // Field Characteristic
  NUMBER(this->characteristic);

  BEGIN
    WSPACE_AROUND(degreeKw)
  END;

  // Field Degree
  NUMBER(this->degree);

  BEGIN
    WSPACE
    CHECK(semiColon)
  END;

  return true;
}

template<typename Number_T>
bool Parser<Number_T>::parseResource()
{
  BEGIN WSPACE END_SEQ;
  this->resource = resourceName(&this->ctx);

  BEGIN CHECK_EXPR(this->resource != Resource::invalid) END;

  return true;
}

template<typename Number_T>
bool Parser<Number_T>::parseParameters()
{
  BEGIN
    WSPACE_AROUND(gate_setKw)
    WSPACE_AFTER(colon)
  END_SEQ;

  GateSetFirst first = gate_setFirst(&this->ctx);

  switch(first)
  {
  case GateSetFirst::arithmetic:
  {
    this->gateSet.gateSet = GateSet::arithmetic;
    this->gateSet.enableAdd = true;
    this->gateSet.enableAddC = true;
    this->gateSet.enableMul = true;
    this->gateSet.enableMulC = true;
    BEGIN WSPACE_AROUND(semiColon) END_SEQ;
    break;
  }
  case GateSetFirst::boolean:
  {
    this->gateSet.gateSet = GateSet::boolean;
    BEGIN WSPACE_AROUND(semiColon) END_SEQ;
    break;
  }
  case GateSetFirst::add:  /* fallthrough */
  case GateSetFirst::addc: /* fallthrough */
  case GateSetFirst::mul:  /* fallthrough */
  case GateSetFirst::mulc: /* fallthrough */
  {
    this->gateSet.gateSet = GateSet::arithmetic;
    this->gateSet.enableAdd = false;
    this->gateSet.enableAddC = false;
    this->gateSet.enableMul = false;
    this->gateSet.enableMulC = false;

    BEGIN WSPACE END_SEQ;
    GateSetArith arith = static_cast<GateSetArith>(first);
    bool cont = false;

    do
    {
      switch(arith)
      {
      case GateSetArith::add:  { this->gateSet.enableAdd = true; break; }
      case GateSetArith::addc: { this->gateSet.enableAddC = true; break; }
      case GateSetArith::mul:  { this->gateSet.enableMul = true; break; }
      case GateSetArith::mulc: { this->gateSet.enableMulC = true; break; }
      default: { return false; }
      }

      CommaOrSemiColon csc = commaOrSemiColon(&this->ctx);
      switch(csc)
      {
      case CommaOrSemiColon::comma:
      {
        BEGIN WSPACE END;
        arith = gate_setArith(&this->ctx);
        cont = true;
        break;
      }
      case CommaOrSemiColon::semiColon:
      {
        cont = false;
        break;
      }
      default: { return false; }
      }
    } while(cont);
    break;
  }
  case GateSetFirst::and_: /* fallthrough */
  case GateSetFirst::xor_: /* fallthrough */
  case GateSetFirst::not_: /* fallthrough */
  {
    this->gateSet.gateSet = GateSet::arithmetic;
    this->gateSet.enableAnd = false;
    this->gateSet.enableXor = false;
    this->gateSet.enableNot = false;

    BEGIN WSPACE END_SEQ;
    GateSetBool bool_ = static_cast<GateSetBool>(first);
    bool cont = false;

    do
    {
      switch(bool_)
      {
      case GateSetBool::and_: { this->gateSet.enableAnd = true; break; }
      case GateSetBool::xor_: { this->gateSet.enableXor = true; break; }
      case GateSetBool::not_: { this->gateSet.enableNot = true; break; }
      default: { return false; }
      }

      CommaOrSemiColon csc = commaOrSemiColon(&this->ctx);
      switch(csc)
      {
      case CommaOrSemiColon::comma:
      {
        BEGIN WSPACE END;
        bool_ = gate_setBool(&this->ctx);
        cont = true;
        break;
      }
      case CommaOrSemiColon::semiColon:
      {
        cont = false;
        break;
      }
      default: { return false; }
      }
    } while(cont);
    break;
  }
  default: { return false; }
  }

  BEGIN
    WSPACE_AROUND(featuresKw)
    WSPACE_AFTER(colon)
  END_SEQ;

  FeatureFirst feature_first = featureFirst(&this->ctx);

  this->featureToggles.functionToggle = false;
  this->featureToggles.forLoopToggle = false;
  this->featureToggles.switchCaseToggle = false;

  if(feature_first == FeatureFirst::invalid)
  {
    return false;
  }
  else if(feature_first == FeatureFirst::simple)
  {
    BEGIN WSPACE_AROUND(semiColon) END_SEQ;
  }
  else
  {
    BEGIN WSPACE END_SEQ;
    FeatureRest feature_rest = static_cast<FeatureRest>(feature_first);
    bool cont = false;

    do
    {
      switch(feature_rest)
      {
      case FeatureRest::function:
      {
        this->featureToggles.functionToggle = true;
        break;
      }
      case FeatureRest::for_loop:
      {
        this->featureToggles.forLoopToggle = true;
        break;
      }
      case FeatureRest::switch_case:
      {
        this->featureToggles.switchCaseToggle = true;
        break;
      }
      default: { return false; }
      }

      CommaOrSemiColon csc = commaOrSemiColon(&this->ctx);
      switch(csc)
      {
      case CommaOrSemiColon::comma:
      {
        BEGIN WSPACE END;
        feature_rest = featureRest(&this->ctx);
        cont = true;
        break;
      }
      case CommaOrSemiColon::semiColon:
      {
        cont = false;
        break;
      }
      default: { return false; }
      }
    } while(cont);
  }

  return true;
}

template<typename Number_T>
ArithmeticParser<Number_T>* Parser<Number_T>::arithmetic()
{
  this->arithmeticNumParser = std::unique_ptr<ArithmeticParser<Number_T>>(
      new ArithmeticParser<Number_T>(
        this->ctx, &this->gateSet, &this->featureToggles));
  return this->arithmeticNumParser.get();
}

template<typename Number_T>
ArithmeticParser<uint64_t>* Parser<Number_T>::arithmetic64()
{
  this->arithmetic64Parser = std::unique_ptr<ArithmeticParser<uint64_t>>(
      new ArithmeticParser<uint64_t>(
        this->ctx, &this->gateSet, &this->featureToggles));
  return this->arithmetic64Parser.get();
}

template<typename Number_T>
ArithmeticParser<uint32_t>* Parser<Number_T>::arithmetic32()
{
  this->arithmetic32Parser = std::unique_ptr<ArithmeticParser<uint32_t>>(
      new ArithmeticParser<uint32_t>(
        this->ctx, &this->gateSet, &this->featureToggles));
  return this->arithmetic32Parser.get();
}

template<typename Number_T>
BooleanParser* Parser<Number_T>::boolean()
{
  this->booleanParser = std::unique_ptr<BooleanParser>(
      new BooleanParser(this->ctx, &this->gateSet, &this->featureToggles));
  return this->booleanParser.get();
}

/*

template<typename Number_T>
bool TextParser<Number_T>::parseBristol(AutomataCtx* ctx)
{
  this->boolHandler.handleVersion(0, 1, 0);
  this->boolHandler.handleField(2, 1);
  this->boolHandler.handleResource(Resource::relation);

  // num gates, ignored
  BEGIN
    WSPACE
    CHECK_EXPR(NumericBase::invalid != numericLiteral(ctx))
    WSPACE
  END;

  // num wires
  NUMBER(this->numWires_);

  BEGIN
    WSPACE
  END;

  // number of input values, these are grouped into blocks
  // All input wires are treated as witness wires.
  size_t n_ins;
  NUMBER(n_ins);
  BEGIN
    WSPACE
  END;

  this->numShortWitness = 0;
  for(size_t i = 0; i < n_ins; i++)
  {
    size_t in_len;
    NUMBER(in_len);
    
    this->numShortWitness += in_len;

    BEGIN
      WSPACE
    END;
  }

  // number of output values, these are also grouped into blocks
  // abusing the instance here to indicate the number of output wires.
  NUMBER(n_ins);

  BEGIN
    WSPACE
  END;

  this->numInstance = 0;
  for(size_t i = 0; i < n_ins; i++)
  {
    size_t out_len;
    NUMBER(out_len);

    this->numInstance += out_len;

    BEGIN
      WSPACE
    END;
  }

  this->boolHandler.handleNumWires(this->numWires_);
  this->boolHandler.handleNumInstance(0);
  this->boolHandler.handleNumShortWitness(this->numShortWitness);

  BEGIN
    WSPACE
  END;

  uint8_t in_wires;

  while(ctx->buffer[ctx->place] != '\0')
  {
    // gate input wires (always 1 or 2)
    NUMBER(in_wires);
    BEGIN
      WSPACE
    END;

    // gate output wires (always 1)
    numericLiteral(ctx);
    BEGIN
      WSPACE
    END;

    size_t leftWire, rightWire, outWire;
    if(in_wires == 2)
    {
      NUMBER(leftWire);
      BEGIN
        WSPACE
      END;
      NUMBER(rightWire);
      BEGIN
        WSPACE
      END;
      NUMBER(outWire);
      BEGIN
        WSPACE
      END;
    }
    else // in_wires == 1
    {
      NUMBER(leftWire);
      BEGIN
        WSPACE
      END;
      NUMBER(outWire);
      BEGIN
        WSPACE
      END;
      rightWire = 0;
    }

    BristolGates gate = bristolGate(ctx);
    BEGIN
      CHECK_EXPR(gate != BristolGates::invalid)
    END_SEQ;

    switch(gate)
    {
    case BristolGates::xor_:
      this->boolHandler.handleXor(
          outWire, leftWire, rightWire);
      break;
    case BristolGates::and_:
      this->boolHandler.handleAnd(
          outWire, leftWire, rightWire);
      break;
    case BristolGates::inv:
      this->boolHandler.handleNot(
          outWire, leftWire);
      break;
    case BristolGates::eq:
      this->boolHandler.handleAssign(
          outWire, (uint8_t) leftWire);
      break;
    case BristolGates::eqw:
      this->boolHandler.handleCopy(
          outWire, leftWire);
      break;
    default:
      return false;
    }

    BEGIN
      WSPACE
    END;
  }

  for(size_t i = 0; i < this->numInstance; i++)
  {
    this->boolHandler.handleAssertZero(
        this->numWires_ - this->numInstance + i);
  }

  this->boolHandler.handleEnd();
  return true;
}
*/

} } // namespace wtk::irregular

#define UNINCLUDE_CHECK_MACROS
#include<wtk/irregular/checkMacros.t.h>
