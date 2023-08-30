/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace irregular {

// "@plugin" was read start at space lparen...
// Trailing whitespace after the semicolon is not read.
template<typename Number_T>
bool parsePluginBinding(AutomataCtx* const ctx,
    wtk::circuit::PluginBinding<Number_T>* const binding)
{
  if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(
     ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!identifier(ctx, &binding->name)))
      || ULK(!whitespace(ctx))) || ULK(!commaOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!identifier(ctx, &binding->operation)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  RangedListE list_e = rangedListE(ctx);
  bool private_counts = false;
  bool public_counts = false;
  while(list_e != RangedListE::rparen && !private_counts && !public_counts)
  {
    if(ULK(list_e == RangedListE::invalid)) { return false; }
    else if(list_e == RangedListE::list)
    {
      if(!whitespace(ctx)) { return false; }

      Number_T num = 0;
      std::string idnt;
      switch(rangedListI(ctx, &num, &idnt))
      {
      case RangedListI::invalid: { return false; }
      case RangedListI::private_count:
      {
        private_counts = true;
        break;
      }
      case RangedListI::public_count:
      {
        public_counts = true;
        break;
      }
      case RangedListI::number:
      {
        binding->parameters.emplace_back(std::move(num));

        if(ULK(!whitespace(ctx))) { return false; }
        list_e = rangedListE(ctx);

        break;
      }
      case RangedListI::identifier:
      {
        binding->parameters.emplace_back(std::move(idnt));

        if(ULK(!whitespace(ctx))) { return false; }
        list_e = rangedListE(ctx);

        break;
      }
      }
    }
  }

  if(private_counts)
  {
    type_idx type = 0;
    size_t count = 0;

    if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(
       ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!number(ctx, &type)))
        || ULK(!whitespace(ctx))) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!number(ctx, &count)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    if((size_t) type >= binding->privateInputCount.size())
    {
      binding->privateInputCount.resize((size_t) type + 1);
    }
    binding->privateInputCount[(size_t) type] = count;

    RangedListE list_e = rangedListE(ctx);
    while(list_e == RangedListE::list && !public_counts)
    {
      if(ULK(!whitespace(ctx))) { return false; }

      switch(rangedListJ(ctx, &type))
      {
      case RangedListJ::invalid: { return false; }
      case RangedListJ::public_count:
      {
        private_counts = true;
        break;
      }
      case RangedListJ::type:
      {
        if(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
            || ULK(!whitespace(ctx))) || ULK(!number(ctx, &count)))
            || ULK(!whitespace(ctx))))
        {
          return false;
        }

        if((size_t) type >= binding->privateInputCount.size())
        {
          binding->privateInputCount.resize((size_t) type + 1);
        }
        binding->privateInputCount[(size_t) type] = count;

        list_e = rangedListE(ctx);

        break;
      }
      }
    }
  }

  if(public_counts)
  {
    type_idx type = 0;
    size_t count = 0;

    if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(
       ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!number(ctx, &type)))
        || ULK(!whitespace(ctx))) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!number(ctx, &count)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    if((size_t) type >= binding->publicInputCount.size())
    {
      binding->publicInputCount.resize((size_t) type + 1);
    }
    binding->publicInputCount[(size_t) type] = count;

    RangedListE list_e = rangedListE(ctx);
    while(list_e != RangedListE::rparen)
    {
      if(ULK(list_e == RangedListE::invalid)) { return false; }

      if(ULK(ULK(ULK(ULK(ULK(
         ULK(ULK(!whitespace(ctx)) || ULK(!number(ctx, &type)))
          || ULK(!whitespace(ctx))) || ULK(!colonOp(ctx)))
          || ULK(!whitespace(ctx))) || ULK(!number(ctx, &count)))
          || ULK(!whitespace(ctx))))
      {
        return false;
      }

      if((size_t) type >= binding->publicInputCount.size())
      {
        binding->publicInputCount.resize((size_t) type + 1);
      }
      binding->publicInputCount[(size_t) type] = count;

      list_e = rangedListE(ctx);
    }
  }

  if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx)))) { return false; }

  return true;
}

// The '@call' was just read and space lparen is to follow.
// Trailing whitespace is not consumed.
template<typename Number_T>
bool parseCallInputs(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler,
    wtk::circuit::FunctionCall* const call)
{
  call->lineNum = ctx->lineNum;

  if(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!identifier(ctx, &call->name)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  RangedListE list_e = rangedListE(ctx);
  if(ULK(list_e == RangedListE::invalid))
  {
    return false;
  }

  RangedListC list_c = RangedListC::invalid;

  while(list_e != RangedListE::rparen && list_c != RangedListC::rparen)
  {
    wire_idx in_first = 0;

    if(ULK(ULK(ULK(!whitespace(ctx))
        || ULK(!index(ctx, &in_first))) || ULK(!whitespace(ctx))))
    {
      return false;
    }

    wire_idx in_last = in_first;

    list_c = rangedListC(ctx);
    if(ULK(list_c == RangedListC::invalid))
    {
      return false;
    }
    else if(list_c == RangedListC::range)
    {
      if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &in_last)))
          || ULK(!whitespace(ctx))))
      {
        return false;
      }

      list_e = rangedListE(ctx);
      if(ULK(list_e == RangedListE::invalid))
      {
        return false;
      }
    }

    call->inputs.emplace_back(in_first, in_last);
  }

  if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  bool ret = handler->invoke(call);
  call->outputs.clear();
  call->inputs.clear();
  return ret;
}

// "<-" was just read, start at space @gatename
// does not read trailing whitespace after the ;
template<typename Number_T>
bool parseStandardGates(AutomataCtx* const ctx,
    wtk::circuit::Handler<Number_T>* const handler, wire_idx const out,
    wtk::circuit::FunctionCall* const call)
{
  if(ULK(!whitespace(ctx))) { return false; }

  wire_idx copy_wire = 0;
  type_idx copy_type = 0;
  StandardGateOps const gate_op = standardGateOps(ctx, &copy_wire, &copy_type);

  switch(gate_op)
  {
  case StandardGateOps::invalid:
  {
    return false;
  }
  case StandardGateOps::add:  /* fallthrough */
  case StandardGateOps::mul:  /* fallthrough */
  case StandardGateOps::addc: /* fallthrough */
  case StandardGateOps::mulc:
  {
    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))))
    {
      return false;
    }

    type_idx type = 0;
    wire_idx left = 0;

    TypeOrWire type_or_wire = typeOrWire(ctx, &type, &left);
    if(ULK(type_or_wire == TypeOrWire::invalid))
    {
      return false;
    }
    else if(type_or_wire == TypeOrWire::type)
    {
      if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
          || ULK(!whitespace(ctx))) || ULK(!index(ctx, &left))))
      {
        return false;
      }
    }

    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!commaOp(ctx)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    switch(gate_op)
    {
    case StandardGateOps::add: /* fallthrough */
    case StandardGateOps::mul: /* fallthrough */
    {
      wire_idx right = 0;
      if(ULK(ULK(ULK(ULK(ULK(ULK(!index(ctx, &right)))
          || ULK(!whitespace(ctx))) || ULK(!rparenOp(ctx)))
          || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
      {
        return false;
      }

      bool okay = false;
      handler->lineNum = ctx->lineNum;
      switch(gate_op)
      {
      case StandardGateOps::add:
      {
        okay = handler->addGate(out, left, right, type);
        break;
      }
      case StandardGateOps::mul:
      {
        okay = handler->mulGate(out, left, right, type);
        break;
      }
      default: { /* unreachable */ }
      }

      return okay;
    }
    case StandardGateOps::addc: /* fallthrough */
    case StandardGateOps::mulc:
    {
      Number_T val = 0;
      if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(
          ULK(!lchevronOp(ctx)) || ULK(!whitespace(ctx)))
          || ULK(!number(ctx, &val))) || ULK(!whitespace(ctx)))
          || ULK(!rchevronOp(ctx))) || ULK(!whitespace(ctx)))
          || ULK(!rparenOp(ctx))) || ULK(!whitespace(ctx)))
          || ULK(!semiColonOp(ctx))))
      {
        return false;
      }

      bool okay = false;
      handler->lineNum = ctx->lineNum;
      switch(gate_op)
      {
      case StandardGateOps::addc:
      {
        okay = handler->addcGate(out, left, std::move(val), type);
        break;
      }
      case StandardGateOps::mulc:
      {
        okay = handler->mulcGate(out, left, std::move(val), type);
        break;
      }
      default: { /* unreachable */ }
      }

      return okay;
    }
    default: { /* unreachable */ break; }
    }

    break;
  }
  case StandardGateOps::public_:  /* fallthrough */
  case StandardGateOps::private_:
  {
    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    type_idx type = 0;
    TypeOrRparen type_or_rparen = typeOrRparen(ctx, &type);
    if(ULK(type_or_rparen == TypeOrRparen::invalid))
    {
      return false;
    }
    else if(type_or_rparen == TypeOrRparen::type)
    {
      if(ULK(ULK(!whitespace(ctx))) || ULK(!rparenOp(ctx)))
      {
        return false;
      }
    }

    if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx))))
    {
      return false;
    }

    handler->lineNum = ctx->lineNum;
    bool okay = false;
    switch(gate_op)
    {
    case StandardGateOps::public_:
    {
      okay = handler->publicIn(out, type);
      break;
    }
    case StandardGateOps::private_:
    {
      okay = handler->privateIn(out, type);
      break;
    }
    default: { /* unreachable */ }
    }

    return okay;
  }
  case StandardGateOps::call:
  {
    call->outputs.emplace_back(out, out);
    return parseCallInputs(ctx, handler, call);
  }
  case StandardGateOps::copy_wire:
  {
    if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx)))) { return false; }

    handler->lineNum = ctx->lineNum;
    return handler->copy(out, copy_wire, 0);
  }
  case StandardGateOps::copy_type:
  {
    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    wire_idx left = 0;
    CopyOp op = copyOp(ctx, &left);

    switch(op)
    {
    case CopyOp::invalid:
    {
      return false;
    }
    case CopyOp::copy:
    {
      if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx))))
      {
        return false;
      }

      handler->lineNum = ctx->lineNum;
      return handler->copy(out, left, copy_type);
    }
    case CopyOp::assign:
    {
      Number_T val = 0;
      if(ULK(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!number(ctx, &val)))
          || ULK(!whitespace(ctx))) || ULK(!rchevronOp(ctx)))
          || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
      {
        return false;
      }

      handler->lineNum = ctx->lineNum;
      return handler->assign(out, std::move(val), copy_type);
    }
    }

    break;
  }
  case StandardGateOps::lchevron:
  {
    Number_T val = 0;
    if(ULK(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!number(ctx, &val)))
        || ULK(!whitespace(ctx))) || ULK(!rchevronOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
    {
      return false;
    }

    handler->lineNum = ctx->lineNum;
    return handler->assign(out, std::move(val), 0);
  }
  }

  return false; // unreachable
}

// An output wire was read, space then either <- or an out range list follows
// Trailing whitespace after a semicolon is not consumed
template<typename Number_T>
bool parseTopScopeItemWireIdx(AutomataCtx* const ctx,
    wtk::circuit::Handler<Number_T>* const handler, wire_idx const out_wire,
    wtk::circuit::FunctionCall* const call)
{
  if(ULK(!whitespace(ctx))) { return false; }

  wire_idx first = out_wire;
  wire_idx last = first;

  RangedListA list_a = rangedListA(ctx);
  switch(list_a)
  {
  case RangedListA::invalid:
  {
    return false;
  }
  case RangedListA::arrow:
  {
    return parseStandardGates(ctx, handler, out_wire, call);
  }
  case RangedListA::range:
  {
    if(ULK(ULK(ULK(!whitespace(ctx))|| ULK(!index(ctx, &last)))
          || ULK(!whitespace(ctx))))
    {
      return false;
    }

    RangedListB list_b = rangedListB(ctx);
    switch(list_b)
    {
    case RangedListB::invalid:
    {
      return false;
    }
    case RangedListB::arrow:
    {
      if(ULK(!whitespace(ctx))) { return false; }

      type_idx copy_type = 0;
      wire_idx copy_wire = 0;
      RangeOutDirectives range_out_directives =
        rangeOutDirectives(ctx, &copy_type, &copy_wire);
      switch(range_out_directives)
      {
      case RangeOutDirectives::invalid:
      {
        return false;
      }
      case RangeOutDirectives::call:
      {
        call->outputs.emplace_back(first, last);
        return parseCallInputs(ctx, handler, call);
      }
      case RangeOutDirectives::public_: /* fallthrough */
      case RangeOutDirectives::private_:
      {
        if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
              || ULK(!whitespace(ctx))))
        {
          return false;
        }

        type_idx input_type = 0;
        switch(typeOrRparen(ctx, &input_type))
        {
        case TypeOrRparen::invalid:
        {
          return false;
        }
        case TypeOrRparen::rparen:
        {
          break;
        }
        case TypeOrRparen::type:
        {
          if(ULK(ULK(!whitespace(ctx)) || ULK(!rparenOp(ctx))))
          {
            return false;
          }

          break;
        }
        }

        if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx))))
        {
          return false;
        }

        wtk::circuit::Range outs(first, last);
        handler->lineNum = ctx->lineNum;
        if(range_out_directives == RangeOutDirectives::public_)
        {
          return handler->publicInMulti(&outs, input_type);
        }
        else
        {
          return handler->privateInMulti(&outs, input_type);
        }
      }
      case RangeOutDirectives::type: /* fallthrough */
      case RangeOutDirectives::wire:
      {
        wtk::circuit::CopyMulti multi(first, last, copy_type);
        first = copy_wire;

        if(range_out_directives == RangeOutDirectives::type)
        {
          /* TODO */
          if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
                    || ULK(!whitespace(ctx))) || ULK(!index(ctx, &first))))
          {
            return false;
          }
        }

        bool end = false;
        do
        {
          if(ULK(!whitespace(ctx))) { return false; }

          switch(rangedListK(ctx))
          {
          case RangedListK::invalid:
          {
            return false;
          }
          case RangedListK::range:
          {
            if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &last)))
                  || ULK(!whitespace(ctx))))
            {
              return false;
            }

            multi.inputs.emplace_back(first, last);

            switch(rangedListL(ctx))
            {
            case RangedListL::invalid:
            {
              return false;
            }
            case RangedListL::list:
            {
              if(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &first))))
              {
                return false;
              }

              break;
            }
            case RangedListL::end:
            {
              end = true;
              break;
            }
            }

            break;
          }
          case RangedListK::list:
          {
            multi.inputs.emplace_back(first, first);

            if(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &first))))
            {
              return false;
            }

            break;
          }
          case RangedListK::end:
          {
            multi.inputs.emplace_back(first, first);
            end = true;
            break;
          }
          }
        } while(!end);

        handler->lineNum = ctx->lineNum;
        return handler->copyMulti(&multi);
      }
      }

      // Unreachable?
      return true;
    }
    case RangedListB::list:
    {
      break;
    }
    }
    break;
  }
  case RangedListA::list:
  {
    break;
  }
  }

  call->outputs.emplace_back(first, last);

  bool end = false;
  do
  {
    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &first)))
            || ULK(!whitespace(ctx))))
    {
      return false;
    }

    switch(rangedListA(ctx))
    {
    case RangedListA::invalid:
    {
      return false;
    }
    case RangedListA::arrow:
    {
      call->outputs.emplace_back(first, first);
      end = true;
      break;
    }
    case RangedListA::range:
    {
      if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &last)))
            || ULK(!whitespace(ctx))))
      {
        return false;
      }

      call->outputs.emplace_back(first, last);

      switch(rangedListB(ctx))
      {
      case RangedListB::invalid:
      {
        return false;
      }
      case RangedListB::arrow:
      {
        end = true;
        break;
      }
      case RangedListB::list:
      {
        break;
      }
      }

      break;
    }
    case RangedListA::list:
    {
      call->outputs.emplace_back(first, first);
      break;
    }
    }
  } while(!end);

  if(ULK(ULK(!whitespace(ctx)) || ULK(!callKw(ctx)))) { return false; }

  return parseCallInputs(ctx, handler, call);
}

// The output type was just read, next up whitespace, colon, output range...
// Trailing whitespace after the semicolon is not consumed.
template<typename Number_T>
bool parseConvertGate(AutomataCtx* const ctx,
    wtk::circuit::Handler<Number_T>* const handler, type_idx const out_type)
{
  wire_idx out_first = 0;
  if(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &out_first)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  wire_idx out_last = out_first;

  RangedListH list_h = rangedListH(ctx);
  if(ULK(list_h == RangedListH::invalid)) { return false; }
  else if(list_h == RangedListH::range)
  {
    if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &out_last)))
        || ULK(!whitespace(ctx))) || ULK(!arrowOp(ctx))))
    {
      return false;
    }
  }

  type_idx in_type = 0;
  wire_idx in_first = 0;
  if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(
      ULK(!whitespace(ctx)) || ULK(!convertKw(ctx)))
      || ULK(!whitespace(ctx))) ||  ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!number(ctx, &in_type)))
      || ULK(!whitespace(ctx))) || ULK(!colonOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &in_first)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  bool modulus = false;
  wire_idx in_last = in_first;
  RangedListD list_d = rangedListD(ctx);
  if(ULK(list_d == RangedListD::invalid))
  {
    return false;
  }
  else if(list_d == RangedListD::range)
  {
    if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!index(ctx, &in_last)))
        || ULK(!whitespace(ctx))))
    {
      return false;
    }

    switch(rangedListE(ctx))
    {
    case RangedListE::invalid:
    {
      return false;
    }
    case RangedListE::list:
    {
      if(ULK(!whitespace(ctx))) { return false; }

      switch(modulusKws(ctx))
      {
      case ModulusKws::invalid:
      {
        return false;
      }
      case ModulusKws::no_modulus:
      {
        modulus = false;
        break;
      }
      case ModulusKws::modulus:
      {
        modulus = true;
        break;
      }
      }

      if(ULK(ULK(!whitespace(ctx)) || ULK(!rparenOp(ctx)))) { return false; }

      break;
    }
    case RangedListE::rparen:
    {
      break;
    }
    }
  }

  if(ULK(ULK(!whitespace(ctx)) || ULK(!semiColonOp(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  return handler->convert(
      out_first, out_last, out_type, in_first, in_last, in_type, modulus);
}

// '@assert_zero' has been read and up next is space, lparen, ...
// Trailing whitespace after the semicolon is not read.
template<typename Number_T>
bool parseAssertZero(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler)
{
  if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  type_idx type = 0;
  wire_idx wire = 0;

  switch(typeOrWire(ctx, &type, &wire))
  {
  case TypeOrWire::invalid: { return false; }
  case TypeOrWire::type:
  {
    if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
        || ULK(!whitespace(ctx))) || ULK(!index(ctx, &wire))))
    {
      return false;
    }

    break;
  }
  case TypeOrWire::wire: { break; }
  }

  if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!rparenOp(ctx)))
    || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  return handler->assertZero(wire, type);
}

// Just read '@new', up next is space, lparen ...
// trailing whitespace is not consumed.
template<typename Number_T>
bool parseNew(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler)
{
  type_idx type = 0;
  wire_idx first = 0;
  wire_idx last = 0;

  if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  switch(typeOrWire(ctx, &type, &first))
  {
  case TypeOrWire::invalid: { return false; }
  case TypeOrWire::type:
  {
    if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &first))))
    {
      return false;
    }

    break;
  }
  case TypeOrWire::wire: { break; }
  }

  if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!rangeOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &last)))
      || ULK(!whitespace(ctx))) || ULK(!rparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  return handler->newRange(first, last, type);
}

// Just read '@delete', up next is space, lparen ...
// trailing whitespace is not consumed.
template<typename Number_T>
bool parseDelete(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler)
{
  type_idx type = 0;
  wire_idx first = 0;
  wire_idx last = 0;

  if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  switch(typeOrWire(ctx, &type, &first))
  {
  case TypeOrWire::invalid: { return false; }
  case TypeOrWire::type:
  {
    if(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &first))))
    {
      return false;
    }

    break;
  }
  case TypeOrWire::wire: { break; }
  }

  if(ULK(ULK(ULK(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!rangeOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!index(ctx, &last)))
      || ULK(!whitespace(ctx))) || ULK(!rparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!semiColonOp(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  return handler->deleteRange(first, last, type);
}

// The @function keyword was just consumed, up next space, (...
// Trailing whitespace after @end will not be consumed.
template<typename Number_T>
bool parseFunctionDecl(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler)
{
  wtk::circuit::FunctionSignature signature;

  if(ULK(ULK(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!lparenOp(ctx)))
      || ULK(!whitespace(ctx))) || ULK(!identifier(ctx, &signature.name)))
      || ULK(!whitespace(ctx))))
  {
    return false;
  }

  handler->lineNum = ctx->lineNum;
  signature.lineNum = ctx->lineNum;

  RangedListE list_e = rangedListE(ctx);
  if(ULK(list_e == RangedListE::invalid))
  {
    return false;
  }
  else if(list_e == RangedListE::list)
  {
    if(ULK(!whitespace(ctx)))
    {
      return false;
    }

    RangedListF list_f = rangedListF(ctx);

    RangedListG list_g = RangedListG::invalid;
    type_idx type = 0;
    size_t len = 0;

    if(ULK(list_f == RangedListF::invalid))
    {
      return false;
    }
    else if(list_f == RangedListF::out)
    {
      if(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx))))
      {
        return false;
      }

      if(ULK(ULK(!whitespace(ctx)) || ULK(!number(ctx, &type))))
      {
        return false;
      }

      do
      {
        if(ULK(ULK(ULK(
           ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
            || ULK(!whitespace(ctx))) || ULK(!number(ctx, &len)))
            || ULK(!whitespace(ctx))))
        {
          return false;
        }

        signature.outputs.emplace_back(type, len);

        list_e = rangedListE(ctx);
        if(ULK(list_e == RangedListE::invalid))
        {
          return false;
        }
        else if(list_e == RangedListE::list)
        {
          if(ULK(!whitespace(ctx)))
          {
            return false;
          }

          list_g = rangedListG(ctx, &type);
          if(ULK(list_g == RangedListG::invalid))
          {
            return false;
          }
        }

      } while(list_e != RangedListE::rparen && list_g != RangedListG::in);
    }

    if(list_f == RangedListF::in || list_g == RangedListG::in)
    {
      if(ULK(ULK(ULK(!whitespace(ctx)) || ULK(!colonOp(ctx)))
          || ULK(!whitespace(ctx))))
      {
        return false;
      }

      do
      {
        if(ULK(ULK(ULK(ULK(
           ULK(ULK(!number(ctx, &type)) || ULK(!whitespace(ctx)))
            || ULK(!colonOp(ctx))) || ULK(!whitespace(ctx)))
            || ULK(!number(ctx, &len))) || ULK(!whitespace(ctx))))
        {
          return false;
        }

        signature.inputs.emplace_back(type, len);

        list_e = rangedListE(ctx);
        if(ULK(ULK(list_e == RangedListE::invalid) || ULK(!whitespace(ctx))))
        {
          return false;
        }
      } while(list_e == RangedListE::list);
    }
  }

  if(ULK(!handler->startFunction(std::move(signature)))) { return false; }

  FuncScopeItemStart func_scope_item_start = FuncScopeItemStart::invalid;
  wire_idx out_wire = 0;
  type_idx out_type = 0;

  if(ULK(!whitespace(ctx))) { return false; }

  switch(funcScopeFirstItemStart(ctx, &out_wire, &out_type))
  {
  case FuncScopeFirstItemStart::invalid: { return false; }
  case FuncScopeFirstItemStart::plugin:
  {
    wtk::circuit::PluginBinding<Number_T> binding;
    if(ULK(!parsePluginBinding<Number_T>(ctx, &binding))) { return false; }

    handler->lineNum = ctx->lineNum;
    return handler->pluginFunction(std::move(binding));
  }
  case FuncScopeFirstItemStart::wireIdx:
  {
    func_scope_item_start = FuncScopeItemStart::wireIdx;
    break;
  }
  case FuncScopeFirstItemStart::typeIdx:
  {
    func_scope_item_start = FuncScopeItemStart::typeIdx;
    break;
  }
  case FuncScopeFirstItemStart::assertZero:
  {
    func_scope_item_start = FuncScopeItemStart::assertZero;
    break;
  }
  case FuncScopeFirstItemStart::new_:
  {
    func_scope_item_start = FuncScopeItemStart::new_;
    break;
  }
  case FuncScopeFirstItemStart::delete_:
  {
    func_scope_item_start = FuncScopeItemStart::delete_;
    break;
  }
  case FuncScopeFirstItemStart::call:
  {
    func_scope_item_start = FuncScopeItemStart::call;
    break;
  }
  case FuncScopeFirstItemStart::end:
  {
    func_scope_item_start = FuncScopeItemStart::end;
    break;
  }
  }

  handler->lineNum = ctx->lineNum;
  if(ULK(!handler->regularFunction())) { return false; }

  wtk::circuit::FunctionCall call;

  do
  {
    switch(func_scope_item_start)
    {
    case FuncScopeItemStart::invalid:
    {
      return false;
    }
    case FuncScopeItemStart::wireIdx:
    {
      if(ULK(!parseTopScopeItemWireIdx(ctx, handler, out_wire, &call)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::typeIdx:
    {
      if(ULK(!parseConvertGate(ctx, handler, out_type)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::assertZero:
    {
      if(ULK(!parseAssertZero(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::new_:
    {
      if(ULK(!parseNew(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::delete_:
    {
      if(ULK(!parseDelete(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::call:
    {
      if(ULK(!parseCallInputs(ctx, handler, &call)))
      {
        return false;
      }

      break;
    }
    case FuncScopeItemStart::end:
    {
      return LIKELY(handler->endFunction()) && whitespace(ctx);
    }
    }

    if(ULK(!whitespace(ctx))) { return false; }

    func_scope_item_start = funcScopeItemStart(ctx, &out_wire, &out_type);

  } while(true); // mid-test
}

template<typename Number_T>
bool parseTopScope(
    AutomataCtx* const ctx, wtk::circuit::Handler<Number_T>* const handler)
{
  wtk::circuit::FunctionCall call;

  do
  {
    if(ULK(!whitespace(ctx))) { return false; }

    wire_idx out_wire = 0;
    type_idx out_type = 0;
    switch(topScopeItemStart(ctx, &out_wire, &out_type))
    {
    case TopScopeItemStart::invalid:
    {
      return false;
    }
    case TopScopeItemStart::wireIdx:
    {
      if(ULK(!parseTopScopeItemWireIdx(ctx, handler, out_wire, &call)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::typeIdx:
    {
      if(ULK(!parseConvertGate(ctx, handler, out_type)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::assertZero:
    {
      if(ULK(!parseAssertZero(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::new_:
    {
      if(ULK(!parseNew(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::delete_:
    {
      if(ULK(!parseDelete(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::call:
    {
      if(ULK(!parseCallInputs(ctx, handler, &call)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::function:
    {
      if(ULK(!parseFunctionDecl(ctx, handler)))
      {
        return false;
      }

      break;
    }
    case TopScopeItemStart::end:
    {
      return whitespace(ctx);
    }
    }
  } while(true); // mid-test
}

} } // namespace wtk::irregular
