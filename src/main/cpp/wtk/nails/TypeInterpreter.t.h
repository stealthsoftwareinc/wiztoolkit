/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T>
TypeInterpreter<Number_T>::TypeInterpreter(char const* const fn)
  : fileName(fn) { }

template<typename Number_T, typename Wire_T>
LeadTypeInterpreter<Number_T, Wire_T>::LeadTypeInterpreter(
    char const* const fn, TypeBackend<Number_T, Wire_T>* const tb,
    InputStream<Number_T>* const ins, InputStream<Number_T>* const wit)
  : TypeInterpreter<Number_T>(fn), backend(tb), maxVal(tb->type->maxValue()),
    publicInStream(ins), privateInStream(wit), stack(1) { }

template<typename Number_T, typename Wire_T>
Scope<Wire_T>* LeadTypeInterpreter<Number_T, Wire_T>::top()
{
  return &this->stack.back();
}

template<typename Number_T, typename Wire_T>
void LeadTypeInterpreter<Number_T, Wire_T>::push()
{
  this->stack.emplace_back();
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::mapOutput(
    wire_idx first, wire_idx last)
{
  log_assert(this->stack.size() > 1);

  ScopeError err = ScopeError::success;
  Wire_T* wires = this->stack[this->stack.size() - 2].findOutputs(
      first, last, &err);
  if(wires == nullptr)
  {
    log_error("%s:%zu: Output mapping error: $%" PRIu64 " ... $%" PRIu64 " %s",
        this->fileName, this->lineNum, first, last, scopeErrorString(err));
    return false;
  }
  else
  {
    this->stack.back().mapOutputs(1 + last - first, wires);
    return true;
  }
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::checkOutput(
    wire_idx first, wire_idx last, wire_idx* place)
{
  bool ret = true;
  if(this->top()->active.hasAll(*place, *place + last - first))
  {
    this->stack[this->stack.size() - 2].assigned.insert(first, last);
    this->stack[this->stack.size() - 2].active.insert(first, last);
  }
  else
  {
    log_error("%s:%zu: Failed to assign range $%" PRIu64 " ... $%" PRIu64,
        this->fileName, this->lineNum, first, last);

    this->top()->active.forRange(*place, *place + last - first,
        [this, first, place](wire_idx const f, wire_idx const l)
        {
          wire_idx af = first + *place - f;
          wire_idx al = first + *place - l;
          this->stack[this->stack.size() - 2].assigned.insert(af, al);
          this->stack[this->stack.size() - 2].active.insert(af, al);
        });
    ret = false;
  }

  *place = *place + 1 + last - first;
  return ret;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::mapInput(
    wire_idx first, wire_idx last)
{
  log_assert(this->stack.size() > 1);

  ScopeError err = ScopeError::success;
  Wire_T* wires = this->stack[this->stack.size() - 2].findInputs(
      first, last, &err);
  if(wires == nullptr)
  {
    log_error("%s:%zu: Input mapping error: $%" PRIu64 " ... $%" PRIu64 " %s",
        this->fileName, this->lineNum, first, last, scopeErrorString(err));
    return false;
  }
  else
  {
    this->stack.back().mapInputs(1 + last - first, wires);
    return true;
  }
}

template<typename Number_T, typename Wire_T>
void LeadTypeInterpreter<Number_T, Wire_T>::pop()
{
  log_assert(this->stack.size() > 1);

  this->stack.pop_back();
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::addGate(wire_idx const out,
    wire_idx const left, wire_idx const right)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(UNLIKELY(out == left) || UNLIKELY(out == right)))
  {
    log_error("%s:%zu: Cannot retrieve and assign wire $%" PRIu64
        " in the same directive", this->fileName, this->lineNum, out);
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  Wire_T const* const right_wire = scope->retrieve(right, &err);
  if(UNLIKELY(right_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, right, scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->addGate(out_wire, left_wire, right_wire);
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::mulGate(wire_idx const out,
    wire_idx const left, wire_idx const right)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(UNLIKELY(out == left) || UNLIKELY(out == right)))
  {
    log_error("%s:%zu: Cannot retrieve and assign wire $%" PRIu64
        " in the same directive", this->fileName, this->lineNum, out);
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  Wire_T const* const right_wire = scope->retrieve(right, &err);
  if(UNLIKELY(right_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, right, scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->mulGate(out_wire, left_wire, right_wire);
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::addcGate(wire_idx const out,
    wire_idx const left, Number_T&& right)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(out == left))
  {
    log_error("%s:%zu: Cannot retrieve and assign wire $%" PRIu64
        " in the same directive", this->fileName, this->lineNum, out);
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  if(UNLIKELY(this->maxVal <= right))
  {
    log_error("%s:%zu: (constant value %s) invalid field element.",
        this->fileName, this->lineNum, wtk::utils::dec(right).c_str());
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->addcGate(out_wire, left_wire, std::move(right));
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::mulcGate(wire_idx const out,
    wire_idx const left, Number_T&& right)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(out == left))
  {
    log_error("%s:%zu: Cannot retrieve and assign wire $%" PRIu64
        " in the same directive", this->fileName, this->lineNum, out);
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  if(UNLIKELY(this->maxVal <= right))
  {
    log_error("%s:%zu: (constant value %s) invalid field element.",
        this->fileName, this->lineNum, wtk::utils::dec(right).c_str());
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->mulcGate(out_wire, left_wire, std::move(right));
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::copy(
    wire_idx const out, wire_idx const left)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(out == left))
  {
    log_error("%s:%zu: Cannot retrieve and assign wire $%" PRIu64
        " in the same directive", this->fileName, this->lineNum, out);
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->copy(out_wire, left_wire);
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::copyMulti(
    wtk::circuit::CopyMulti const* const copy)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  size_t in_total = 0;
  for(size_t i = 0; i < copy->inputs.size(); i++)
  {
    if(UNLIKELY(copy->inputs[i].first > copy->inputs[i].last))
    {
      log_error("%s:%zu: Input range is invalid $%" PRIu64 " ... $%" PRIu64 ".",
          this->fileName, this->lineNum, copy->inputs[i].first,
          copy->inputs[i].last);
      return false;
    }

    in_total += 1 + copy->inputs[i].last - copy->inputs[i].first;
  }

  if(UNLIKELY(copy->outputs.first > copy->outputs.last))
  {
    log_error("%s:%zu: Output range is invalid $%" PRIu64 " ... $%" PRIu64 ".",
        this->fileName, this->lineNum, copy->outputs.first, copy->outputs.last);
    return false;
  }

  if(UNLIKELY(in_total != 1 + copy->outputs.last - copy->outputs.first))
  {
    log_error("%s:%zu: Input and output totals do not match.", this->fileName,
        this->lineNum);
    return false;
  }

  Wire_T* outs = scope->findOutputs(
      copy->outputs.first, copy->outputs.last, &err);
  if(UNLIKELY(outs == nullptr))
  {
    log_error("%s:%zu: Copy output error: $%" PRIu64 " ... $%" PRIu64 " %s",
        this->fileName, this->lineNum, copy->outputs.first, copy->outputs.last,
        scopeErrorString(err));
    return false;
  }

  wire_idx out_place = 0;
  this->backend->lineNum = this->lineNum;

  for(size_t i = 0; i < copy->inputs.size(); i++)
  {
    Wire_T const* ins = scope->findInputs(
        copy->inputs[i].first, copy->inputs[i].last, &err);
    if(UNLIKELY(ins == nullptr))
    {
      log_error("%s:%zu: Copy input error: $%" PRIu64 " ... $%" PRIu64 " %s",
          this->fileName, this->lineNum, copy->inputs[i].first,
          copy->inputs[i].last, scopeErrorString(err));
      return false;
    }

    wire_idx in_count = 1 + copy->inputs[i].last - copy->inputs[i].first;
    for(wire_idx i = 0; i < in_count; i++)
    {
      new(outs + out_place+ i) Wire_T();
      this->backend->copy(outs + out_place + i, ins + i);
    }

    scope->assigned.insert(copy->outputs.first + out_place,
        copy->outputs.first + out_place + in_count - 1);
    scope->active.insert(copy->outputs.first + out_place,
        copy->outputs.first + out_place + in_count - 1);
    out_place += in_count;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::assign(
    wire_idx const out, Number_T&& right)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(UNLIKELY(this->maxVal <= right))
  {
    log_error("%s:%zu: (constant value %s) invalid field element.",
        this->fileName, this->lineNum, wtk::utils::dec(right).c_str());
    return false;
  }

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->assign(out_wire, std::move(right));
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::assertZero(
    wire_idx const left)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  Wire_T const* const left_wire = scope->retrieve(left, &err);
  if(UNLIKELY(left_wire == nullptr))
  {
    log_error("%s:%zu: (input wire %" PRIu64 ") %s",
        this->fileName, this->lineNum, left, scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  this->backend->assertZero(left_wire);
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::publicIn(
    wire_idx const out)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire $%" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Number_T val = 0;
  if(this->publicInStream != nullptr)
  {
    switch(this->publicInStream->next(&val))
    {
    case wtk::StreamStatus::success:
    {
      if(UNLIKELY(this->maxVal <= val))
      {
        log_error("%s:%zu: invalid field element (stream line %zu, value %s)",
            this->fileName, this->lineNum, this->publicInStream->lineNum(),
            wtk::utils::dec(val).c_str());
        return false;
      }
      break;
    }
    case wtk::StreamStatus::end:
    {
      log_error("%s:%zu: Public input stream has reached end (stream line %zu)",
          this->fileName, this->lineNum, this->publicInStream->lineNum());
      return false;
    }
    case wtk::StreamStatus::error:
    {
      return false;
    }
    }
  }

  this->backend->lineNum = this->lineNum;
  this->backend->publicIn(out_wire, std::move(val));
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::publicInMulti(
    wtk::circuit::Range const* const outs)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(outs->first > outs->last)
  {
    log_error("%s:%zu: Invalid output range: $%" PRIu64 " ... $%" PRIu64 ".",
        this->fileName, this->lineNum, outs->first, outs->last);
    return false;
  }

  Wire_T* const out_wires = scope->findOutputs(outs->first, outs->last, &err);
  if(UNLIKELY(out_wires == nullptr))
  {
    log_error("%s:%zu: (output wires $%" PRIu64 " ... $%" PRIu64 ") %s",
        this->fileName, this->lineNum, outs->first, outs->last,
        scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  bool success = true;
  wire_idx i;
  for(i = 0; i < 1 + outs->last - outs->first; i++)
  {
    Number_T val = 0;
    if(this->publicInStream != nullptr)
    {
      switch(this->publicInStream->next(&val))
      {
      case wtk::StreamStatus::success:
      {
        if(UNLIKELY(this->maxVal <= val))
        {
          log_error(
              "%s:%zu: invalid field element (stream line %zu, value %s)",
              this->fileName, this->lineNum, this->publicInStream->lineNum(),
              wtk::utils::dec(val).c_str());
          success = false;
        }
        break;
      }
      case wtk::StreamStatus::end:
      {
        log_error(
            "%s:%zu: Public input stream has reached end (stream line %zu)",
            this->fileName, this->lineNum, this->publicInStream->lineNum());
        success = false;
        break;
      }
      case wtk::StreamStatus::error:
      {
        success = false;
        break;
      }
      }
    }

    if(success) { this->backend->publicIn(out_wires + i, std::move(val)); }
  }

  scope->assigned.insert(outs->first, outs->first + i - 1);
  scope->active.insert(outs->first, outs->first + i - 1);

  return success;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::privateIn(
    wire_idx const out)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  Wire_T* const out_wire = scope->assign(out, &err);
  if(UNLIKELY(out_wire == nullptr))
  {
    log_error("%s:%zu: (output wire $%" PRIu64 ") %s",
        this->fileName, this->lineNum, out, scopeErrorString(err));
    return false;
  }

  Number_T val = 0;
  if(this->privateInStream != nullptr)
  {
    switch(this->privateInStream->next(&val))
    {
    case wtk::StreamStatus::success:
    {
      if(UNLIKELY(this->maxVal <= val))
      {
        log_error("%s:%zu: invalid field element (stream line %zu, value %s)",
            this->fileName, this->lineNum, this->privateInStream->lineNum(),
            wtk::utils::dec(val).c_str());
        return false;
      }
      break;
    }
    case wtk::StreamStatus::end:
    {
      log_error("%s:%zu: Private input stream has reached end (stream line %zu)",
          this->fileName, this->lineNum, this->privateInStream->lineNum());
      return false;
    }
    case wtk::StreamStatus::error:
    {
      return false;
    }
    }
  }

  this->backend->lineNum = this->lineNum;
  this->backend->privateIn(out_wire, std::move(val));
  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::privateInMulti(
    wtk::circuit::Range const* const outs)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  if(outs->first > outs->last)
  {
    log_error("%s:%zu: Invalid output range: $%" PRIu64 " ... $%" PRIu64 ".",
        this->fileName, this->lineNum, outs->first, outs->last);
    return false;
  }

  Wire_T* const out_wires = scope->findOutputs(outs->first, outs->last, &err);
  if(UNLIKELY(out_wires == nullptr))
  {
    log_error("%s:%zu: (output wires $%" PRIu64 " ... $%" PRIu64 ") %s",
        this->fileName, this->lineNum, outs->first, outs->last,
        scopeErrorString(err));
    return false;
  }

  this->backend->lineNum = this->lineNum;
  bool success = true;
  wire_idx i;
  for(i = 0; i < 1 + outs->last - outs->first; i++)
  {
    Number_T val = 0;
    if(this->privateInStream != nullptr)
    {
      switch(this->privateInStream->next(&val))
      {
      case wtk::StreamStatus::success:
      {
        if(UNLIKELY(this->maxVal <= val))
        {
          log_error(
              "%s:%zu: invalid field element (stream line %zu, value %s)",
              this->fileName, this->lineNum, this->privateInStream->lineNum(),
              wtk::utils::dec(val).c_str());
          success = false;
        }
        break;
      }
      case wtk::StreamStatus::end:
      {
        log_error(
            "%s:%zu: Private input stream has reached end (stream line %zu)",
            this->fileName, this->lineNum, this->privateInStream->lineNum());
        success = false;
        break;
      }
      case wtk::StreamStatus::error:
      {
        success = false;
        break;
      }
      }
    }

    if(success)
    {
      new(out_wires + i) Wire_T();
      this->backend->privateIn(out_wires + i, std::move(val));
    }
    else { break; }
  }

  scope->assigned.insert(outs->first, outs->first + i - 1);
  scope->active.insert(outs->first, outs->first + i - 1);

  return success;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::newRange(
    wire_idx const first, wire_idx const last)
{
  Scope<Wire_T>* const scope = this->top();
  ScopeError err;

  (void) scope->newRange(first, last, &err);
  if(UNLIKELY(err != ScopeError::success))
  {
    log_error("%s:%zu: (new range %" PRIu64 " ... %" PRIu64 ") %s",
        this->fileName, this->lineNum, first, last, scopeErrorString(err));
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::deleteRange(
    wire_idx const first, wire_idx const last)
{
  Scope<Wire_T>* const scope = this->top();

  ScopeError err = scope->deleteRange(first, last);
  if(UNLIKELY(err != ScopeError::success))
  {
    log_error("%s:%zu: (delete range %" PRIu64 " ... %" PRIu64 ") %s",
        this->fileName, this->lineNum, first, last, scopeErrorString(err));
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
bool LeadTypeInterpreter<Number_T, Wire_T>::checkNumber(Number_T const& value)
{
  return this->maxVal > value;
}

template<typename Number_T, typename Wire_T>
wtk::plugins::WiresRefEraser
LeadTypeInterpreter<Number_T, Wire_T>::pluginOutput(
    type_idx const type, wire_idx const first, wire_idx const last)
{
  ScopeError err = ScopeError::success;
  Wire_T* wires = this->top()->findOutputs(first, last, &err);
  log_assert(wires != nullptr);
  bool success = this->top()->assigned.insert(first, last);
  success = this->top()->active.insert(first, last) && success;
  log_assert(success);

  size_t const size = (size_t) last - first + 1;

  for(size_t i = 0; i < size; i++)
  {
    new(wires + i) Wire_T();
  }

  return wtk::plugins::WiresRef<Wire_T>(size, wires, type);
}

template<typename Number_T, typename Wire_T>
wtk::plugins::WiresRefEraser
LeadTypeInterpreter<Number_T, Wire_T>::pluginInput(
    type_idx const type, wire_idx const first, wire_idx const last)
{
  ScopeError err = ScopeError::success;
  Wire_T* wires = this->top()->findInputs(first, last, &err);
  log_assert(wires != nullptr);
  return wtk::plugins::WiresRef<Wire_T>(
      (size_t) last - first + 1, wires, type);
}

template<typename Number_T, typename Wire_T>
void LeadTypeInterpreter<Number_T, Wire_T>::iterPluginHack(
    wire_idx const first, wire_idx const last)
{
  ScopeError err;
  Wire_T* wires = this->top()->findInputs(first, last, &err);
  (void) err;
  log_assert(wires != nullptr);
  for(size_t i = 0; i < last - first + 1; i++)
  {
    (wires + i)->~Wire_T();
  }

  this->top()->assigned.remove(first, last);
  this->top()->active.remove(first, last);
}

template<typename Number_T, typename Wire_T>
Number_T LeadTypeInterpreter<Number_T, Wire_T>::getMaxValForIterPlugin()
{
  return this->maxVal;
}

} } // namespace wtk::nails
