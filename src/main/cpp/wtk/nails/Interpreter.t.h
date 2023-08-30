/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

#ifdef WTK_NAILS_ENABLE_TRACES

template<typename Number_T>
void Interpreter<Number_T>::enableTrace()
{
  this->trace = true;
}

template<typename Number_T>
void Interpreter<Number_T>::enableTraceDetail()
{
  this->traceDetail = true;
}
#endif//WTK_NAILS_ENABLE_TRACES

template<typename Number_T>
template<typename Wire_T>
void Interpreter<Number_T>::addType(
    wtk::TypeBackend<Number_T, Wire_T>* const tb,
    wtk::InputStream<Number_T>* const public_in,
    wtk::InputStream<Number_T>* const private_in)
{
  this->interpreters.emplace_back(new LeadTypeInterpreter<Number_T, Wire_T>(
        this->fileName, tb, public_in, private_in));
}

template<typename Number_T>
template<typename Out_T, typename In_T>
bool Interpreter<Number_T>::addConversion(
    wtk::circuit::ConversionSpec const* const spec,
    wtk::Converter<Out_T, In_T>* conv)
{
  if(this->converters.find(*spec) != this->converters.end())
  {
    log_error("Duplicate conversion: @convert(@out: %u:%zu, @in: %u:%zu)",
        (unsigned int) spec->outType, spec->outLength,
        (unsigned int) spec->inType, spec->inLength);
    return false;
  }
  else
  {
    log_assert(spec->outLength == conv->outLength);
    log_assert(spec->inLength == conv->inLength);

    this->converters[*spec] = std::unique_ptr<Converter<Number_T>>(
      new LeadConverter<Number_T, Out_T, In_T>(this->fileName, conv));
    return true;
  }
}

template<typename Number_T>
bool Interpreter<Number_T>::addGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @add(%" PRIu8 ": $%" PRIu64 ", $%"
        PRIu64 ");", this->fileName, this->lineNum, this->indent.get(),
        out, type, left, right);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->addGate(out, left, right);
}

template<typename Number_T>
bool Interpreter<Number_T>::mulGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @mul(%" PRIu8 ": $%" PRIu64 ", $%"
        PRIu64 ");", this->fileName, this->lineNum, this->indent.get(),
        out, type, left, right);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->mulGate(out, left, right);
}

template<typename Number_T>
bool Interpreter<Number_T>::addcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @addc(%" PRIu8 ": $%" PRIu64
        ", < %s >);", this->fileName, this->lineNum, this->indent.get(),
        out, type, left, wtk::utils::dec(right).c_str());
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->addcGate(
      out, left, std::move(right));
}

template<typename Number_T>
bool Interpreter<Number_T>::mulcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @mulc(%" PRIu8 ": $%" PRIu64
        ", < %s >);", this->fileName, this->lineNum, this->indent.get(),
        out, type, left, wtk::utils::dec(right).c_str());
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->mulcGate(
      out, left, std::move(right));
}

template<typename Number_T>
bool Interpreter<Number_T>::copy(
    wire_idx const out, wire_idx const left, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- %" PRIu8 ": $%" PRIu64 ";",
        this->fileName, this->lineNum, this->indent.get(), out, type, left);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->copy(out, left);
}

template<typename Number_T>
bool Interpreter<Number_T>::copyMulti(
    wtk::circuit::CopyMulti const* const multi)
{
  log_assert(multi->type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    std::string s = "$";
    s += wtk::utils::dec(multi->outputs.first);

    if(multi->outputs.first != multi->outputs.last)
    {
      s += " ... $" + wtk::utils::dec(multi->outputs.last);
    }

    s += " <- " + wtk::utils::dec(multi->type) + ": ";

    std::string comma = "";
    for(size_t i = 0; i < multi->inputs.size(); i++)
    {
      s += comma + "$" + wtk::utils::dec(multi->inputs[i].first);
      comma = ", ";
      if(multi->inputs[i].first != multi->inputs[i].last)
      {
        s += " ... $" + wtk::utils::dec(multi->inputs[i].last);
      }
    }

    log_info("    %s:%zu: %s%s;",
        this->fileName, this->lineNum, this->indent.get(), s.c_str());
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) multi->type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) multi->type]->copyMulti(multi);
}

template<typename Number_T>
bool Interpreter<Number_T>::assign(
    wire_idx const out, Number_T&& left, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- %" PRIu8 ": < %s >);",
        this->fileName, this->lineNum, this->indent.get(), out, type,
        wtk::utils::dec(left).c_str());
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->assign(out, std::move(left));
}

template<typename Number_T>
bool Interpreter<Number_T>::assertZero(
    wire_idx const left, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s@assert_zero(%" PRIu8 ": $%" PRIu64 ");",
        this->fileName, this->lineNum, this->indent.get(), type, left);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->assertZero(left);
}

template<typename Number_T>
bool Interpreter<Number_T>::publicIn(wire_idx const out, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @public(%" PRIu8 ");",
        this->fileName, this->lineNum, this->indent.get(), out, type);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->publicIn(out);
}

template<typename Number_T>
bool Interpreter<Number_T>::publicInMulti(
    wtk::circuit::Range const* const out, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    if(out->first == out->last)
    {
      log_info("    %s:%zu: %s$%" PRIu64 " <- @public(%" PRIu8 ");",
          this->fileName, this->lineNum, this->indent.get(), out->first, type);
    }
    else
    {
      log_info("    %s:%zu: %s$%" PRIu64 " ... $%" PRIu64 "<- @public(%"
          PRIu8 ");", this->fileName, this->lineNum, this->indent.get(),
          out->first, out->last, type);
    }
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->publicInMulti(out);
}

template<typename Number_T>
bool Interpreter<Number_T>::privateIn(wire_idx const out, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s$%" PRIu64 " <- @private(%" PRIu8 ");",
        this->fileName, this->lineNum, this->indent.get(), out, type);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->privateIn(out);
}

template<typename Number_T>
bool Interpreter<Number_T>::privateInMulti(
    wtk::circuit::Range const* const out, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    if(out->first == out->last)
    {
      log_info("    %s:%zu: %s$%" PRIu64 " <- @private(%" PRIu8 ");",
          this->fileName, this->lineNum, this->indent.get(), out->first, type);
    }
    else
    {
      log_info("    %s:%zu: %s$%" PRIu64 " ... $%" PRIu64 "<- @private(%"
          PRIu8 ");", this->fileName, this->lineNum, this->indent.get(),
          out->first, out->last, type);
    }
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->privateInMulti(out);
}

template<typename Number_T>
bool Interpreter<Number_T>::convert(
    wire_idx const first_out, wire_idx const last_out,
    type_idx const out_type,
    wire_idx const first_in, wire_idx const last_in,
    type_idx const in_type, bool modulus)
{
  log_assert(out_type < this->interpreters.size());
  log_assert(in_type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s%" PRIu8 ": $%" PRIu64 " ... $%" PRIu64
        " <- @convert(%" PRIu8 ": $%" PRIu64 " ... $%" PRIu64 ", %s);",
        this->fileName, this->lineNum, this->indent.get(), out_type,
        first_out, last_out, in_type, first_in, last_in,
        modulus ? "@modulus" : "@no_modulus");
  }
#endif//WTK_NAILS_ENABLE_TRACES

  wtk::circuit::ConversionSpec spec(
      out_type, 1 + last_out - first_out, in_type, 1 + last_in - first_in);

  auto finder = this->converters.find(spec);
  if(finder == this->converters.end())
  {
    log_error("%s:%zu: No such conversion: "
        "@convert(@out: %u:%zu, @in: %u:%zu)", this->fileName, this->lineNum,
        (unsigned int) spec.outType, spec.outLength,
        (unsigned int) spec.inType, spec.inLength);
  }

  finder->second->lineNum = this->lineNum;
  return finder->second->convert(
      first_out, last_out, this->interpreters[(size_t) out_type].get(),
      first_in, last_in, this->interpreters[(size_t) in_type].get(), modulus);
}

template<typename Number_T>
bool Interpreter<Number_T>::newRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s@new(%" PRIu8 ": $%" PRIu64 " ... $%" PRIu64 ");",
        this->fileName, this->lineNum, this->indent.get(), type, first, last);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->newRange(first, last);
}

template<typename Number_T>
bool Interpreter<Number_T>::deleteRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  log_assert(type < this->interpreters.size());

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    log_info("    %s:%zu: %s@delete(%" PRIu8 ": $%" PRIu64 " ... $%"
        PRIu64 ");", this->fileName, this->lineNum, this->indent.get(), type,
        first, last);
  }
#endif//WTK_NAILS_ENABLE_TRACES

  this->interpreters[(size_t) type]->lineNum = this->lineNum;
  return this->interpreters[(size_t) type]->deleteRange(first, last);
}

template<typename Number_T>
bool Interpreter<Number_T>::invoke(
    wtk::circuit::FunctionCall const* const call)
{
#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->trace)
  {
    log_info("    %s:%zu: %s@call(%s)", this->fileName, this->lineNum,
        this->indent.get(), call->name.c_str());
    this->indent.inc();
  }
#endif//WTK_NAILS_ENABLE_TRACES

  auto finder = this->functions.find(call->name.c_str());
  if(UNLIKELY(finder == this->functions.end()))
  {
    log_error("%s:%zu: No such function \'%s\'.",
        this->fileName, this->lineNum, call->name.c_str());
    return false;
  }

  Function<Number_T>* function = finder->second;

  if(UNLIKELY(function->signature.outputs.size() != call->outputs.size()))
  {
    log_error(
        "%s:%zu: Call to \'%s\' has %zu output parameters (expected %zu)",
        this->fileName, this->lineNum, call->name.c_str(),
        call->outputs.size(), function->signature.outputs.size());
    return false;
  }

  if(UNLIKELY(function->signature.inputs.size() != call->inputs.size()))
  {
    log_error("%s:%zu: Call to \'%s\' has %zu input parameters (expected %zu)",
        this->fileName, this->lineNum, call->name.c_str(),
        call->inputs.size(), function->signature.outputs.size());
    return false;
  }

  for(size_t i = 0; i < this->interpreters.size(); i++)
  {
    this->interpreters[i]->lineNum = this->lineNum;
    this->interpreters[i]->push();
  }

  for(size_t i = 0; i < call->outputs.size(); i++)
  {
    log_assert(function->signature.outputs[i].type < this->interpreters.size());

    if(UNLIKELY(call->outputs[i].first > call->outputs[i].last))
    {
      log_error("%s:%zu: Invalid range $%" PRIu64 " ... $%" PRIu64 "",
          this->fileName, this->lineNum, call->outputs[i].first,
          call->outputs[i].last);
      return false;
    }

    size_t const len = 1 + call->outputs[i].last - call->outputs[i].first;

    if(UNLIKELY(len != function->signature.outputs[i].length))
    {
      log_error("%s:%zu: Range $%" PRIu64 " ... $%" PRIu64
          " does not match length %zu", this->fileName, this->lineNum,
          call->outputs[i].first, call->outputs[i].last,
          function->signature.outputs[i].length);
      return false;
    }

    if(UNLIKELY(
        !this->interpreters[(size_t) function->signature.outputs[i].type]
        ->mapOutput(call->outputs[i].first, call->outputs[i].last)))
    {
      return false;
    }
  }

  for(size_t i = 0; i < call->inputs.size(); i++)
  {
    log_assert(function->signature.inputs[i].type < this->interpreters.size());

    if(UNLIKELY(call->inputs[i].first > call->inputs[i].last))
    {
      log_error("%s:%zu: Invalid range $%" PRIu64 " ... $%" PRIu64 "",
          this->fileName, this->lineNum, call->inputs[i].first,
          call->inputs[i].last);
      return false;
    }

    size_t const len = 1 + call->inputs[i].last - call->inputs[i].first;

    if(UNLIKELY(len != function->signature.inputs[i].length))
    {
      log_error("%s:%zu: Range $%" PRIu64 " ... $%" PRIu64
          " does not match length %zu", this->fileName, this->lineNum,
          call->inputs[i].first, call->inputs[i].last,
          function->signature.inputs[i].length);
      return false;
    }

    if(UNLIKELY(
        !this->interpreters[(size_t) function->signature.inputs[i].type]
        ->mapInput(call->inputs[i].first, call->inputs[i].last)))
    {
      return false;
    }
  }

#ifdef WTK_NAILS_ENABLE_TRACES
  if(this->traceDetail)
  {
    std::vector<wtk::wire_idx> places(this->interpreters.size(), 0);
    for(size_t i = 0; i < function->signature.outputs.size(); i++)
    {
      wtk::wire_idx f = places[(size_t) function->signature.outputs[i].type];
      wtk::wire_idx l = f + function->signature.outputs[i].length - 1;
      log_info("    %s:%zu: %sremapped output: %" PRIu8 ": $%" PRIu64 " ... $%"
          PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "", this->fileName,
          this->lineNum, this->indent.get(),
          function->signature.outputs[i].type, call->outputs[i].first,
          call->outputs[i].last, f, l);
      places[(size_t) function->signature.outputs[i].type] = l + 1;
    }

    for(size_t i = 0; i < function->signature.inputs.size(); i++)
    {
      wtk::wire_idx f = places[(size_t) function->signature.inputs[i].type];
      wtk::wire_idx l = f + function->signature.inputs[i].length - 1;
      log_info("    %s:%zu: %sremapped input: %" PRIu8 ": $%" PRIu64 " ... $%"
          PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "", this->fileName,
          this->lineNum, this->indent.get(),
          function->signature.inputs[i].type, call->inputs[i].first,
          call->inputs[i].last, f, l);
      places[(size_t) function->signature.inputs[i].type] = l + 1;
    }
  }

  size_t trace_line_num = this->lineNum;
#endif//WTK_NAILS_ENABLE_TRACES

  bool ret = true;
  if(UNLIKELY(!function->evaluate(this)))
  {
    ret = false;
  }

  std::vector<wire_idx> places(this->interpreters.size(), 0);
  for(size_t i = 0; i < call->outputs.size(); i++)
  {
    size_t type = (size_t) function->signature.outputs[i].type;

    this->interpreters[type]->lineNum = this->lineNum;
    if(UNLIKELY(!this->interpreters[type]->checkOutput(
        call->outputs[i].first, call->outputs[i].last, &places[type])))
    {
      ret = false;
    }
  }

  for(size_t i = 0; i < this->interpreters.size(); i++)
  {
    this->interpreters[i]->pop();
  }

#ifdef WTK_NAILS_ENABLE_TRACES
  this->lineNum = trace_line_num;
  if(this->trace)
  {
    this->indent.dec();
    log_info("    %s:%zu: %s@end (%s)", this->fileName, this->lineNum,
        this->indent.get(), call->name.c_str());
  }
#endif//WTK_NAILS_ENABLE_TRACES

  return ret;
}

} } // namespace wtk::nails
