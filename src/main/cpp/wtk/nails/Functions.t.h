/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T>
Gate<Number_T>::Gate(Operation const op,
    wire_idx const o, wire_idx const l, wire_idx const r,
    type_idx const t, size_t const ln)
  : operation(op), normal(o, l, r, t), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(Operation const op,
    wire_idx const o, wire_idx const l, Number_T&& r,
    type_idx const t, size_t const ln, bool)
  : operation(op), constant(o, l, std::move(r), t), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(
    wire_idx const fo, wire_idx const lo, type_idx const ot,
    wire_idx const fi, wire_idx const li, type_idx const it, bool const m,
    size_t const ln)
  : operation(convert_), convert(fo, lo, ot, fi, li, it, m), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(Operation const op,
    wire_idx const f, wire_idx const l, type_idx const t, size_t const ln)
  : operation(op), memory(f, l, t), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(wtk::circuit::FunctionCall&& c)
  : operation(call_), call(c), lineNum(c.lineNum) { }

template<typename Number_T>
Gate<Number_T>::Gate(wtk::circuit::CopyMulti&& m, size_t const ln)
  : operation(copyMulti), multiCopy(std::move(m)), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(Operation const op, wtk::circuit::Range&& r,
    type_idx const t, size_t const ln)
  : operation(op), multiInput(std::move(r), t), lineNum(ln) { }

template<typename Number_T>
Gate<Number_T>::Gate(Gate&& move)
{
  this->operation = move.operation;
  switch(move.operation)
  {
  case uninitialized:
  {
    break;
  }
  case add:        /* fallthrough */
  case mul:        /* fallthrough */
  case copy:       /* fallthrough */
  case assertZero: /* fallthrough */
  case publicIn:   /* fallthrough */
  case privateIn:
  {
    new(&this->normal) NormalGate(std::move(move.normal));
    break;
  }
  case addc: /* fallthrough */
  case mulc: /* fallthrough */
  case assign:
  {
    new(&this->constant) ConstantGate(std::move(move.constant));
    break;
  }
  case convert_:
  {
    new(&this->convert) ConvertGate(std::move(move.convert));
    break;
  }
  case newRange: /* fallthrough */
  case deleteRange:
  {
    new(&this->memory) MemoryGate(std::move(move.memory));
    break;
  }
  case call_:
  {
    new(&this->call) wtk::circuit::FunctionCall(std::move(move.call));
    break;
  }
  case copyMulti:
  {
    new(&this->multiCopy) wtk::circuit::CopyMulti(std::move(move.multiCopy));
    break;
  }
  case publicInMulti: /* fallthrough */
  case privateInMulti:
  {
    new(&this->multiInput) MultiInput(std::move(move.multiInput));
    break;
  }
  }

  this->lineNum = move.lineNum;
  move.lineNum = 0;

  move.operation = uninitialized;
}

template<typename Number_T>
Gate<Number_T>::~Gate()
{
  switch(this->operation)
  {
  case uninitialized:
  {
    break;
  }
  case add:        /* fallthrough */
  case mul:        /* fallthrough */
  case copy:       /* fallthrough */
  case assertZero: /* fallthrough */
  case publicIn:   /* fallthrough */
  case privateIn:
  {
    this->normal.~NormalGate();
    break;
  }
  case addc: /* fallthrough */
  case mulc: /* fallthrough */
  case assign:
  {
    this->constant.~ConstantGate();
    break;
  }
  case convert_:
  {
    this->convert.~ConvertGate();
    break;
  }
  case newRange: /* fallthrough */
  case deleteRange:
  {
    this->memory.~MemoryGate();
    break;
  }
  case call_:
  {
    this->call.~FunctionCall();
    break;
  }
  case copyMulti:
  {
    this->multiCopy.~CopyMulti();
    break;
  }
  case publicInMulti: /* fallthrough */
  case privateInMulti:
  {
    this->multiInput.~MultiInput();
    break;
  }
  }
}

template<typename Number_T>
bool GatesFunction<Number_T>::addGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::add, out, left, right, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::mulGate(wire_idx const out,
    wire_idx const left, wire_idx const right, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::mul, out, left, right, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::addcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  this->gates.emplace_back(Gate<Number_T>::addc,
      out, left, std::move(right), type, this->lineNum, true);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::mulcGate(wire_idx const out,
    wire_idx const left, Number_T&& right, type_idx const type)
{
  this->gates.emplace_back(Gate<Number_T>::mulc,
      out, left, std::move(right), type, this->lineNum, true);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::copy(wire_idx const out,
    wire_idx const left, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::copy, out, left, 0, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::copyMulti(wtk::circuit::CopyMulti* multi)
{
  this->gates.emplace_back(std::move(*multi), this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::assign(wire_idx const out,
    Number_T&& left, type_idx const type)
{
  this->gates.emplace_back(Gate<Number_T>::assign,
      out, 0, std::move(left), type, this->lineNum, true);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::assertZero(
    wire_idx const left, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::assertZero, 0, left, 0, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::publicIn(
    wire_idx const out, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::publicIn, out, 0, 0, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::publicInMulti(
    wtk::circuit::Range* out, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::publicInMulti, std::move(*out), type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::privateIn(
    wire_idx const out, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::privateIn, out, 0, 0, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::privateInMulti(
    wtk::circuit::Range* out, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::privateInMulti, std::move(*out), type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::convert(
    wire_idx const first_out, wire_idx const last_out, type_idx const out_type,
    wire_idx const first_in, wire_idx const last_in, type_idx const in_type,
    bool const m)
{
  this->gates.emplace_back(
      first_out, last_out, out_type,
      first_in, last_in, in_type, m, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::newRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::newRange, first, last, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::deleteRange(
    wire_idx const first, wire_idx const last, type_idx const type)
{
  this->gates.emplace_back(
      Gate<Number_T>::deleteRange, first, last, type, this->lineNum);

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::invoke(wtk::circuit::FunctionCall&& call)
{
  this->gates.emplace_back(std::move(call));

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::typeCheck(
    Interpreter<Number_T> const* const interpreter)
{
  char const* const file_name = interpreter->fileName;
  size_t const num_fields = interpreter->interpreters.size();
  std::vector<wtk::utils::SkipList<wire_idx>> assigns(num_fields);
  std::vector<wtk::utils::SkipList<wire_idx>> actives(num_fields);
  std::vector<std::vector<std::pair<wire_idx, wire_idx>>>
    allocations(num_fields);
  std::vector<size_t> remap_counts(num_fields, 0);

  auto checkInputRange = [file_name, num_fields, &actives, &allocations]
    (wire_idx const first, wire_idx const last, size_t const type,
        size_t const line_num) -> bool
    {
      log_assert(type < num_fields); (void) num_fields;

      if(UNLIKELY(first > last))
      {
        log_error("%s:%zu: Invalid range ($%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, first, last);
        return false;
      }

      if(UNLIKELY(!actives[type].hasAll(first, last)))
      {
        log_error("%s:%zu: Input range is not entirely active (%zu: "
            "$%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, type, first, last);
        return false;
      }

      // early return for single wires.
      if(first == last) { return true; }

      bool found = false;
      auto iter = allocations[type].begin();
      while(iter != allocations[type].end())
      {
        found = found || (iter->first <= first && iter->second >= last);
        iter++;
      }

      if(UNLIKELY(!found))
      {
        log_error("%s:%zu: Input range is discontiguous (%zu: "
            "$%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, type, first, last);
        return false;
      }

      return true;
    };

  auto checkNewRange = [file_name, num_fields, &assigns, &allocations]
    (wire_idx const first, wire_idx const last, size_t const type,
        size_t const line_num) -> bool
    {
      log_assert(type < num_fields); (void) num_fields;

      if(UNLIKELY(first > last))
      {
        log_error("%s:%zu: Invalid range ($%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, first, last);
        return false;
      }

      if(UNLIKELY(assigns[type].has(first, last)))
      {
        log_error("%s:%zu: New range was previously assigned (%zu: "
            "$%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, type, first, last);
        return false;
      }

      if(allocations[type].size() == 0
          || allocations[type].back().second < first)
      {
        allocations[type].emplace_back(first, last);
        return true;
      }
      else
      {
        auto prev = allocations[type].begin();

        if(prev->first > last)
        {
          allocations[type].emplace(prev, first, last);
          return true;
        }

        auto iter = allocations[type].begin() + 1;

        while(iter != allocations[type].end())
        {
          if(prev->second < first && iter->first > last)
          {
            allocations[type].emplace(iter, first, last);
            return true;
          }

          prev++;
          iter++;
        }

        log_error("%s:%zu: New range conflicts with a previous allocation "
          "(%zu: $%" PRIu64 " ... $%" PRIu64 ")",
          file_name, line_num, type, first, last);
        return false;
      }
    };

  auto checkOutputRange =
    [file_name, num_fields, &assigns, &actives, &allocations]
    (wire_idx const first, wire_idx const last, size_t const type,
        size_t const line_num) -> bool
    {
      log_assert(type < num_fields); (void) num_fields;

      if(UNLIKELY(first > last))
      {
        log_error("%s:%zu: Invalid range ($%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, first, last);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(first, last)))
      {
        log_error("%s:%zu: Output range was previously assigned (%zu: "
            "$%" PRIu64 " ... $%" PRIu64 ")",
            file_name, line_num, type, first, last);
        return false;
      }

      actives[type].insert(first, last);

      if(allocations[type].size() == 0
          || allocations[type].back().second < first)
      {
        allocations[type].emplace_back(first, last);
        return true;
      }
      else
      {
        auto prev = allocations[type].begin();

        if(prev->first > last)
        {
          allocations[type].emplace(prev, first, last);
          return true;
        }
        else if(prev->first <= first && prev->second >= last)
        {
          return true;
        }

        auto iter = allocations[type].begin() + 1;

        while(iter != allocations[type].end())
        {
          if(prev->second < first && iter->first > last)
          {
            allocations[type].emplace(iter, first, last);
            return true;
          }
          else if(iter->first <= first && iter->second >= last)
          {
            return true;
          }

          prev++;
          iter++;
        }

        log_error("%s:%zu: Output range conflicts with a previous allocation "
          "(%zu: $%" PRIu64 " ... $%" PRIu64 ")",
          file_name, line_num, type, first, last);
        return false;
      }
    };

  // Map the outputs and inputs into this scope
  {
    std::vector<wire_idx> places(interpreter->interpreters.size(), 0);

    // outputs
    for(size_t i = 0; i < this->signature.outputs.size(); i++)
    {
      wtk::circuit::FunctionSignature::Parameter const* const output =
        &this->signature.outputs[i];

      size_t const type = (size_t) output->type;
      log_assert(type < num_fields);

      if(output->length == 0)
      {
        log_error("%s: %zu: Output %zu length cannot be zero",
            file_name, this->signature.lineNum, i);
        return false;
      }

      wire_idx const first = places[type];
      wire_idx const last = places[type] + output->length - 1;
      places[type] += output->length;

      allocations[type].emplace_back(first, last);
      remap_counts[type] += 1;
    }

    // inputs
    for(size_t i = 0; i < this->signature.inputs.size(); i++)
    {
      wtk::circuit::FunctionSignature::Parameter const* const input =
        &this->signature.inputs[i];

      size_t const type = (size_t) input->type;
      log_assert(type < num_fields);

      if(input->length == 0)
      {
        log_error("%s: %zu: Input %zu length cannot be zero",
            file_name, this->signature.lineNum, i);
        return false;
      }

      wire_idx const first = places[type];
      wire_idx const last = places[type] + input->length - 1;
      places[type] += input->length;

      allocations[type].emplace_back(first, last);
      remap_counts[type] += 1;
      assigns[type].insert(first, last);
      actives[type].insert(first, last);
    }
  }

  for(size_t i = 0; i < this->gates.size(); i++)
  {
    Gate<Number_T> const* const gate = &this->gates[i];

    switch(gate->operation)
    {
    case Gate<Number_T>::uninitialized:
    {
      log_assert(false, "uninitalized gate");
      break;
    }
    case Gate<Number_T>::add: /* fallthrough */
    case Gate<Number_T>::mul:
    {
      size_t const type = (size_t) gate->normal.type;
      if(UNLIKELY(!actives[type].has(gate->normal.left)))
      {
        log_error("%s:%zu: Wire not yet assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.left);
        return false;
      }

      if(UNLIKELY(!actives[type].has(gate->normal.right)))
      {
        log_error("%s:%zu: Wire not yet assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.right);
        return false;
      }

      if(UNLIKELY(!actives[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire already assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire assigned but deleted $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      break;
    }
    case Gate<Number_T>::copy:
    {
      size_t const type = (size_t) gate->normal.type;
      if(UNLIKELY(!actives[type].has(gate->normal.left)))
      {
        log_error("%s:%zu: Wire not yet assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.left);
        return false;
      }

      if(UNLIKELY(!actives[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire already assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire assigned but deleted $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      break;
    }
    case Gate<Number_T>::assertZero:
    {
      size_t const type = (size_t) gate->normal.type;
      if(UNLIKELY(!actives[type].has(gate->normal.left)))
      {
        log_error("%s:%zu: Wire not yet assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.left);
        return false;
      }
      break;
    }
    case Gate<Number_T>::publicIn: /* fallthrough */
    case Gate<Number_T>::privateIn:
    {
      size_t const type = (size_t) gate->normal.type;
      if(UNLIKELY(!actives[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire already assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(gate->normal.out)))
      {
        log_error("%s:%zu: Wire assigned but deleted $%" PRIu64 ".",
            file_name, gate->lineNum, gate->normal.out);
        return false;
      }

      break;
    }
    case Gate<Number_T>::addc: /* fallthrough */
    case Gate<Number_T>::mulc:
    {
      size_t const type = (size_t) gate->constant.type;
      if(UNLIKELY(!actives[type].has(gate->constant.left)))
      {
        log_error("%s:%zu: Wire not yet assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->constant.left);
        return false;
      }

      if(UNLIKELY(
          !interpreter->interpreters[type]->checkNumber(gate->constant.right)))
      {
        log_error("%s:%zu: (constant value %s) invalid field element.",
            file_name, gate->lineNum,
            wtk::utils::dec(gate->constant.right).c_str());
        return false;
      }

      if(UNLIKELY(!actives[type].insert(gate->constant.out)))
      {
        log_error("%s:%zu: Wire already assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->constant.out);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(gate->constant.out)))
      {
        log_error("%s:%zu: Wire assigned but deleted $%" PRIu64 ".",
            file_name, gate->lineNum, gate->constant.out);
        return false;
      }

      break;
    }
    case Gate<Number_T>::assign:
    {
      size_t const type = (size_t) gate->constant.type;
      if(UNLIKELY(
          !interpreter->interpreters[type]->checkNumber(gate->constant.right)))
      {
        log_error("%s:%zu: (constant value %s) invalid field element.",
            file_name, gate->lineNum,
            wtk::utils::dec(gate->constant.right).c_str());
        return false;
      }

      if(UNLIKELY(!actives[type].insert(gate->constant.out)))
      {
        log_error("%s:%zu: Wire already assigned $%" PRIu64 ".",
            file_name, gate->lineNum, gate->constant.out);
        return false;
      }

      if(UNLIKELY(!assigns[type].insert(gate->constant.out)))
      {
        log_error("%s:%zu: Wire assigned but deleted $%" PRIu64 ".",
            file_name, gate->lineNum, gate->constant.out);
        return false;
      }

      break;
    }
    case Gate<Number_T>::convert_:
    {
      if(UNLIKELY(!checkInputRange(gate->convert.firstIn, gate->convert.lastIn,
          (size_t) gate->convert.inType, gate->lineNum)))
      {
        return false;
      }

      if(UNLIKELY(!checkOutputRange(
          gate->convert.firstOut, gate->convert.lastOut,
          (size_t) gate->convert.outType, gate->lineNum)))
      {
        return false;
      }

      wtk::circuit::ConversionSpec spec(gate->convert.outType,
          1 + gate->convert.lastOut - gate->convert.firstOut,
          gate->convert.inType,
          1 + gate->convert.lastIn - gate->convert.firstIn);

      if(UNLIKELY(
          interpreter->converters.find(spec) == interpreter->converters.end()))
      {
        log_error("%s:%zu: No such conversion "
            "@convert(@out: %u:%zu, @in: %u:%zu)", file_name, gate->lineNum,
            (unsigned int) spec.outType, spec.outLength,
            (unsigned int) spec.inType, spec.inLength);
        return false;
      }

      break;
    }
    case Gate<Number_T>::newRange:
    {
      if(UNLIKELY(!checkNewRange(
          gate->memory.first, gate->memory.last, (size_t) gate->memory.type,
          gate->lineNum)))
      {
        return false;
      }

      break;
    }
    case Gate<Number_T>::deleteRange:
    {
      size_t const type = (size_t) gate->memory.type;
      wire_idx const first = gate->memory.first;
      wire_idx const last = gate->memory.last;

      if(UNLIKELY(first > last))
      {
        log_error("%s:%zu: Invalid range ($%" PRIu64 " ...$%" PRIu64 ")",
            file_name, gate->lineNum, first, last);
        return false;
      }

      if(UNLIKELY(!actives[type].remove(first, last)))
      {
        log_error("%s:%zu: Cannot delete non-assigned wires "
            "(%zu: $%" PRIu64 " ... $%" PRIu64 ")", file_name, gate->lineNum,
            type, first, last);
        return false;
      }

      auto iter = allocations[type].begin();
      while(iter != allocations[type].end())
      {
        if(first <= iter->first && last >= iter->second)
        {
          iter = allocations[type].erase(iter);
        }
        else if(iter->second < first)
        {
          iter++;
        }
        else if(iter->first > last)
        {
          break;
        }
        else
        {
          log_error("%s:%zu: Delete cannot divide ranges "
              "(%zu: $%" PRIu64 " ... $%" PRIu64 ")", file_name, gate->lineNum,
              type, first, last);
          return false;
        }
      }

      break;
    }
    case Gate<Number_T>::call_:
    {
      auto finder = interpreter->functions.find(gate->call.name.c_str());
      if(finder == interpreter->functions.end())
      {
        log_error("%s:%zu: Function \'%s\' is not defined", file_name,
            gate->lineNum, gate->call.name.c_str());
        return false;
      }
      wtk::circuit::FunctionSignature const* const signature =
        &finder->second->signature;

      if(UNLIKELY(signature->inputs.size() != gate->call.inputs.size()))
      {
        log_info("%s:%zu: Wrong number of inputs to function \'%s\': "
            "%zu, expected %zu",
            file_name, gate->lineNum, signature->name.c_str(),
            gate->call.inputs.size(), signature->inputs.size());
        return false;
      }

      for(size_t i = 0; i < signature->inputs.size(); i++)
      {
        size_t const type = (size_t) signature->inputs[i].type;
        wire_idx const first = gate->call.inputs[i].first;
        wire_idx const last = gate->call.inputs[i].last;
        size_t len = (size_t) (1 + last - first);

        if(UNLIKELY(len != signature->inputs[i].length))
        {
          log_error("%s:%zu: Input range (%zu: $%" PRIu64 " ... $%" PRIu64 ")"
              " has the incorrect length (expected %zu)", file_name,
              gate->lineNum, type, first, last, signature->inputs[i].length);
          return false;
        }

        if(UNLIKELY(!checkInputRange(first, last, type, gate->lineNum)))
        {
          return false;
        }
      }

      if(UNLIKELY(signature->outputs.size() != gate->call.outputs.size()))
      {
        log_info("%s:%zu: Wrong number of outputs to function \'%s\': "
            "%zu, expected %zu",
            file_name, gate->lineNum, signature->name.c_str(),
            gate->call.outputs.size(), signature->outputs.size());
        return false;
      }

      for(size_t i = 0; i < signature->outputs.size(); i++)
      {
        size_t const type = (size_t) signature->outputs[i].type;
        wire_idx const first = gate->call.outputs[i].first;
        wire_idx const last = gate->call.outputs[i].last;
        size_t len = (size_t) (1 + last - first);

        if(UNLIKELY(len != signature->outputs[i].length))
        {
          log_error("%s:%zu: Output range (%zu: $%" PRIu64 " ... $%" PRIu64 ")"
              " has the incorrect length (expected %zu)", file_name,
              gate->lineNum, type, first, last, signature->outputs[i].length);
          return false;
        }

        if(UNLIKELY(!checkOutputRange(first, last, type, gate->lineNum)))
        {
          return false;
        }
      }

      break;
    }
    case Gate<Number_T>::copyMulti:
    {
      size_t const type = (size_t) gate->multiCopy.type;
      for(size_t i = 0; i < gate->multiCopy.inputs.size(); i++)
      {
        wire_idx const first = gate->multiCopy.inputs[i].first;
        wire_idx const last = gate->multiCopy.inputs[i].last;

        if(UNLIKELY(!checkInputRange(first, last, type, gate->lineNum)))
        {
          return false;
        }
      }

      wire_idx const o_first = gate->multiCopy.outputs.first;
      wire_idx const o_last = gate->multiCopy.outputs.last;

      if(UNLIKELY(!checkOutputRange(o_first, o_last, type, gate->lineNum)))
      {
        return false;
      }

      break;
    }
    case Gate<Number_T>::publicInMulti: /* fallthrough */
    case Gate<Number_T>::privateInMulti:
    {
      size_t const type = (size_t) gate->normal.type;
      wire_idx const first = gate->multiInput.outputs.first;
      wire_idx const last = gate->multiInput.outputs.last;
      if(UNLIKELY(!checkOutputRange(first, last, type, gate->lineNum)))
      {
        return false;
      }

      break;
    }
    }
  }

  return true;
}

template<typename Number_T>
bool GatesFunction<Number_T>::evaluate(
    Interpreter<Number_T>* const interpreter)
{
  for(size_t i = 0; i < this->gates.size(); i++)
  {
    Gate<Number_T> const* const gate = &this->gates[i];
    interpreter->lineNum = gate->lineNum;

    switch(gate->operation)
    {
    case Gate<Number_T>::uninitialized:
    {
      log_assert(false, "uninitialized gate");
      break;
    }
    case Gate<Number_T>::add:
    {
      log_debug("%zu: add gate", gate->lineNum);

      if(UNLIKELY(!interpreter->addGate(gate->normal.out,
            gate->normal.left, gate->normal.right, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::mul:
    {
      log_debug("%zu: mul gate", gate->lineNum);

      if(UNLIKELY(!interpreter->mulGate(gate->normal.out,
            gate->normal.left, gate->normal.right, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::copy:
    {
      log_debug("%zu: copy gate", gate->lineNum);

      if(UNLIKELY(!interpreter->copy(
            gate->normal.out, gate->normal.left, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::assertZero:
    {
      log_debug("%zu: assert zero gate", gate->lineNum);

      if(UNLIKELY(!interpreter->assertZero(
            gate->normal.left, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::publicIn:
    {
      log_debug("%zu: public input gate", gate->lineNum);

      if(UNLIKELY(!interpreter->publicIn(
            gate->normal.out, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::privateIn:
    {
      log_debug("%zu: private input gate", gate->lineNum);

      if(UNLIKELY(!interpreter->privateIn(
            gate->normal.out, gate->normal.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::addc:
    {
      log_debug("%zu: addc gate", gate->lineNum);

      Number_T copy(gate->constant.right);
      if(UNLIKELY(!interpreter->addcGate(
            gate->constant.out, gate->constant.left, std::move(copy),
            gate->constant.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::mulc:
    {
      log_debug("%zu: mulc gate", gate->lineNum);

      Number_T copy(gate->constant.right);
      if(UNLIKELY(!interpreter->mulcGate(
            gate->constant.out, gate->constant.left, std::move(copy),
            gate->constant.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::assign:
    {
      log_debug("%zu: assign gate", gate->lineNum);

      Number_T copy(gate->constant.right);
      if(UNLIKELY(!interpreter->assign(
            gate->constant.out, std::move(copy), gate->constant.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::convert_:
    {
      log_debug("%zu: convert gate", gate->lineNum);

      if(UNLIKELY(!interpreter->convert(
            gate->convert.firstOut, gate->convert.lastOut,
            gate->convert.outType,
            gate->convert.firstIn, gate->convert.lastIn,
            gate->convert.inType, gate->convert.modulus)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::newRange:
    {
      log_debug("%zu: new range", gate->lineNum);

      if(UNLIKELY(!interpreter->newRange(
            gate->memory.first, gate->memory.last, gate->memory.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::deleteRange:
    {
      log_debug("%zu: delete range", gate->lineNum);

      if(UNLIKELY(!interpreter->deleteRange(
            gate->memory.first, gate->memory.last, gate->memory.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::call_:
    {
      log_debug("%zu: function call: %s",
          gate->lineNum, gate->call.name.c_str());

      if(UNLIKELY(!interpreter->invoke(&gate->call)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::copyMulti:
    {
      log_debug("%zu: copy multi gate", gate->lineNum);

      if(UNLIKELY(!interpreter->copyMulti(&gate->multiCopy)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::publicInMulti:
    {
      log_debug("%zu: public input multi gate", gate->lineNum);

      if(UNLIKELY(!interpreter->publicInMulti(
            &gate->multiInput.outputs, gate->multiInput.type)))
      {
        return false;
      }
      break;
    }
    case Gate<Number_T>::privateInMulti:
    {
      log_debug("%zu: private input multi gate", gate->lineNum);

      if(UNLIKELY(!interpreter->privateInMulti(
            &gate->multiInput.outputs, gate->multiInput.type)))
      {
        return false;
      }
      break;
    }
    }
  }

  return true;
}

template<typename Number_T>
RegularFunction<Number_T>* GatesFunctionFactory<Number_T>::createFunction(
    wtk::circuit::FunctionSignature&& sig)
{
  return this->gatesFunctionPool.allocate(1, std::move(sig));
}

} } // namespace wtk::nails
