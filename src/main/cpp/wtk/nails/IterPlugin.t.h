/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T, typename Wire_T>
bool IterPlugin<Number_T, Wire_T>::addBackend(wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  (void) type;
  (void) backend;
  return true;
}

template<typename Number_T, typename Wire_T>
wtk::plugins::Operation<Number_T>* IterPlugin<Number_T, Wire_T>::create(
    type_idx const type,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) type;

  return this->operation->typeCheck(signature, binding)
    ? this->operation
    : nullptr;
}

template<typename Number_T>
template<typename Wire_T>
std::unique_ptr<wtk::plugins::Plugin<Number_T, Wire_T>>
MapOperation<Number_T>::makePlugin()
{
  return std::unique_ptr<wtk::plugins::Plugin<Number_T, Wire_T>>(
      new IterPlugin<Number_T, Wire_T>(this));
}

template<typename Number_T>
void MapOperation<Number_T>::evaluate(
    std::vector<wtk::plugins::WiresRefEraser>& outputs,
    std::vector<wtk::plugins::WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{

  (void) outputs;
  (void) inputs;

  bool enumerated = false;
  if(binding->operation == "map") { enumerated = false; }
  else if(binding->operation == "map_enumerated") { enumerated = true; }

  log_assert(binding->parameters.size() == 3);
  log_assert(binding->parameters[0].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual);
  log_assert(binding->parameters[1].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::numeric);
  log_assert(binding->parameters[2].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::numeric);

  auto finder =
    this->interpreter->functions.find(binding->parameters[0].text.c_str());

  log_assert(finder != this->interpreter->functions.end());

  Function<Number_T>* const func = finder->second;
  wtk::circuit::FunctionSignature const* const func_sig = &func->signature;
  size_t const env_count =
    wtk::utils::cast_size(binding->parameters[1].number);
  wire_idx const iter_count =
    wtk::utils::cast_wire(binding->parameters[2].number);

  log_assert(signature->outputs.size() == func_sig->outputs.size());
  log_assert(signature->inputs.size() + (enumerated ? 1 : 0)
      == func_sig->inputs.size());

  // Hack to unassign the outputs, because the plugin-interface assumes
  // all will be assigned
  std::vector<wtk::wire_idx> places(this->interpreter->interpreters.size(), 0);
  for(size_t i = 0; i < signature->outputs.size(); i++)
  {
    size_t const type = (size_t) signature->outputs[i].type;
    wtk::wire_idx const first = places[type];
    wtk::wire_idx const last = places[type] + signature->outputs[i].length - 1;

    this->interpreter->interpreters[type]->iterPluginHack(first, last);

    places[type] = last + 1;
  }

  wtk::wire_idx enum_place = UINT64_MAX;
  wtk::type_idx const enum_type =
    (enumerated) ? func_sig->inputs[env_count].type : 0;
  wtk::wire_idx const enum_length =
    (enumerated) ? func_sig->inputs[env_count].length : 0;
  Number_T enum_max = (enumerated)
    ? this->interpreter->interpreters[
        (size_t) enum_type]->getMaxValForIterPlugin()
    : 0;

  // loop for iterations
  for(wtk::wire_idx j = 0; j < iter_count; j++)
  {
#ifdef WTK_NAILS_ENABLE_TRACES
    if(this->interpreter->trace)
    {
      log_info("       %s:%zu: %siteration %" PRIu64 "",
          this->interpreter->fileName, this->interpreter->lineNum,
          this->interpreter->indent.get(), j);
      log_info("       %s:%zu: %s@call(%s)", this->interpreter->fileName,
          this->interpreter->lineNum, this->interpreter->indent.get(),
          func_sig->name.c_str());
      this->interpreter->indent.inc();
    }
#endif//WTK_NAILS_ENABLE_TRACES
    log_assert(signature->inputs.size() + (enumerated ? 1 : 0)
        == func_sig->inputs.size());

    // Check if work needs to be done in enumerating the loop index
    wtk::wire_idx enum_first = 0;
    wtk::wire_idx enum_last = 0;
    if(enumerated)
    {
      enum_last = enum_place;
      enum_first = enum_place + 1 - enum_length;
      enum_place = enum_place - enum_length;

      if(enum_length == 1)
      {
        Number_T j_num = j;
        this->interpreter->assign(
            enum_first, j_num % enum_max, enum_type);
      }
      else
      {
        this->interpreter->newRange(enum_first, enum_last, enum_type);
        wtk::wire_idx alt_j = j;
        for(wtk::wire_idx i = enum_last; i >= enum_first; i--)
        {
          this->interpreter->assign(i, Number_T(alt_j & 1), enum_type);
          alt_j = alt_j >> 1;
        }
      }
    }

    // Zero out the places vector and push new scopes to each type's stacks
    for(size_t i = 0; i < this->interpreter->interpreters.size(); i++)
    {
      places[i] = 0;
      this->interpreter->interpreters[i]->push();
    }

#ifdef WTK_NAILS_ENABLE_TRACES
    std::vector<wtk::wire_idx> inside_places(
        this->interpreter->interpreters.size(), 0);
#endif//WTK_NAILS_ENABLE_TRACES

    // Remap outputs
    for(size_t i = 0; i < signature->outputs.size(); i++)
    {
      log_assert(signature->outputs[i].type == func_sig->outputs[i].type);
      log_assert(signature->outputs[i].length
          == iter_count * func_sig->outputs[i].length);

      size_t const type = (size_t) signature->outputs[i].type;
      wtk::wire_idx const func_first =
        places[type] + j * func_sig->outputs[i].length;
      wtk::wire_idx const func_last =
        places[type] + (j + 1) * func_sig->outputs[i].length - 1;

      this->interpreter->interpreters[type]->mapOutput(func_first, func_last);

#ifdef WTK_NAILS_ENABLE_TRACES
      if(this->interpreter->traceDetail)
      {
        wtk::wire_idx f = inside_places[(size_t) func_sig->outputs[i].type];
        wtk::wire_idx l = f + func_sig->outputs[i].length - 1;
        log_info("       %s:%zu: %sremapped output: %" PRIu8 ": $%" PRIu64
            " ... $%" PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "",
            this->interpreter->fileName, this->interpreter->lineNum,
            this->interpreter->indent.get(), func_sig->outputs[i].type,
            func_first, func_last, f, l);
        inside_places[(size_t) func_sig->outputs[i].type] = l + 1;
      }
#endif//WTK_NAILS_ENABLE_TRACES

      places[type] += signature->outputs[i].length;
    }

    // Remap inputs
    size_t plugin_place = 0;
    size_t iter_place = 0;
    for(size_t i = 0; i < env_count; i++)
    {
      log_assert(signature->inputs[plugin_place].type
          == func_sig->inputs[iter_place].type);
      log_assert(signature->inputs[plugin_place].length
          == func_sig->inputs[iter_place].length);

      size_t const type = (size_t) signature->inputs[plugin_place].type;
      wtk::wire_idx const func_first = places[type];
      wtk::wire_idx const func_last =
        func_first + func_sig->inputs[iter_place].length - 1;

      this->interpreter->interpreters[type]->mapInput(func_first, func_last);

#ifdef WTK_NAILS_ENABLE_TRACES
      if(this->interpreter->traceDetail)
      {
        wtk::wire_idx f =
          inside_places[(size_t) func_sig->inputs[iter_place].type];
        wtk::wire_idx l = f + func_sig->inputs[iter_place].length - 1;
        log_info("       %s:%zu: %sremapped input: %" PRIu8 ": $%" PRIu64
            " ... $%" PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "",
            this->interpreter->fileName, this->interpreter->lineNum,
            this->interpreter->indent.get(), func_sig->inputs[iter_place].type,
            func_first, func_last, f, l);
        inside_places[(size_t) func_sig->inputs[iter_place].type] = l + 1;
      }
#endif//WTK_NAILS_ENABLE_TRACES

      places[type] += signature->inputs[iter_place].length;
      plugin_place++;
      iter_place++;
    }

    if(enumerated)
    {
      this->interpreter->interpreters[enum_type]->mapInput(
          enum_first, enum_last);

#ifdef WTK_NAILS_ENABLE_TRACES
      if(this->interpreter->traceDetail)
      {
        wtk::wire_idx f =
          inside_places[(size_t) func_sig->inputs[iter_place].type];
        wtk::wire_idx l = f + func_sig->inputs[iter_place].length - 1;
        log_info("       %s:%zu: %sremapped input: %" PRIu8 ": $%" PRIu64
            " ... $%" PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "",
            this->interpreter->fileName, this->interpreter->lineNum,
            this->interpreter->indent.get(), func_sig->inputs[iter_place].type,
            enum_first, enum_last, f, l);
        inside_places[(size_t) func_sig->inputs[iter_place].type] = l + 1;
      }
#endif//WTK_NAILS_ENABLE_TRACES

      iter_place++;
    }

    for(size_t i = iter_place; i < func_sig->inputs.size(); i++)
    {
      log_assert(signature->inputs[plugin_place].type
          == func_sig->inputs[iter_place].type);
      log_assert(signature->inputs[plugin_place].length
          == func_sig->inputs[iter_place].length * iter_count);

      size_t const type = (size_t) signature->inputs[plugin_place].type;
      wtk::wire_idx const func_first =
        places[type] + j * func_sig->inputs[plugin_place].length;
      wtk::wire_idx const func_last =
        places[type] + (j + 1) * func_sig->inputs[plugin_place].length - 1;

      this->interpreter->interpreters[type]->mapInput(func_first, func_last);

#ifdef WTK_NAILS_ENABLE_TRACES
      if(this->interpreter->traceDetail)
      {
        wtk::wire_idx f =
          inside_places[(size_t) func_sig->inputs[iter_place].type];
        wtk::wire_idx l = f + func_sig->inputs[iter_place].length - 1;
        log_info("       %s:%zu: %sremapped input: %" PRIu8 ": $%" PRIu64
            " ... $%" PRIu64 " -> $%" PRIu64 " ... $%" PRIu64 "",
            this->interpreter->fileName, this->interpreter->lineNum,
            this->interpreter->indent.get(), func_sig->inputs[iter_place].type,
            func_first, func_last, f, l);
        inside_places[(size_t) func_sig->inputs[iter_place].type] = l + 1;
      }
#endif//WTK_NAILS_ENABLE_TRACES

      places[type] += signature->inputs[iter_place].length;
      plugin_place++;
      iter_place++;
    }

    // invoke the function
    func->evaluate(this->interpreter);

    // check the outputs
    for(size_t i = 0; i < places.size(); i++) { places[i] = 0; }
    std::vector<wtk::wire_idx> more_places(places.size(), 0);

    for(size_t i = 0; i < func_sig->outputs.size(); i++)
    {
      size_t const type = (size_t) func_sig->outputs[i].type;
      wtk::wire_idx const func_first =
        places[type] + j * func_sig->outputs[i].length;
      wtk::wire_idx const func_last =
        places[type] + (j + 1) * func_sig->outputs[i].length - 1;

      this->interpreter->interpreters[type]->checkOutput(
          func_first, func_last, &more_places[type]);

      places[i] += signature->outputs[i].length;
    }

    for(size_t i = 0; i < this->interpreter->interpreters.size(); i++)
    {
      this->interpreter->interpreters[i]->pop();
    }

    if(enumerated)
    {
      this->interpreter->deleteRange(enum_first, enum_last, enum_type);
    }

#ifdef WTK_NAILS_ENABLE_TRACES
    if(this->interpreter->trace)
    {
      this->interpreter->indent.dec();
      log_info("       %s:%zu: %s@end (%s)", this->interpreter->fileName,
          this->interpreter->lineNum, this->interpreter->indent.get(),
          func_sig->name.c_str());
    }
#endif//WTK_NAILS_ENABLE_TRACES
  }
}

template<typename Number_T>
bool MapOperation<Number_T>::typeCheck(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  bool enumerated;
  if(binding->operation == "map") { enumerated = false; }
  else if(binding->operation == "map_enumerated") { enumerated = true; }
  else
  {
    log_error(
        "Unrecognized iter operation \"%s\"", binding->operation.c_str());
    return false;
  }

  if(binding->parameters.size() != 3)
  {
    log_error("map operations require three plugin parameters");
    return false;
  }

  if(binding->parameters[0].form
      != wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error("map operation's function must be a name");
    return false;
  }

  auto finder =
    this->interpreter->functions.find(binding->parameters[0].text.c_str());
  if(finder == this->interpreter->functions.end())
  {
    log_error("No such function \"%s\"", binding->parameters[0].text.c_str());
    return false;
  }

  wtk::circuit::FunctionSignature const* const func_sig =
    &finder->second->signature;

  if(binding->parameters[1].form
      != wtk::circuit::PluginBinding<Number_T>::Parameter::numeric)
  {
    log_error("map_operation's environment count must be a number");
  }

  size_t const env_count =
    wtk::utils::cast_size(binding->parameters[1].number);

  if(binding->parameters[2].form
      != wtk::circuit::PluginBinding<Number_T>::Parameter::numeric)
  {
    log_error("map_operation's iteration count must be a number");
    return false;
  }

  wire_idx const iter_count =
    wtk::utils::cast_wire(binding->parameters[2].number);

  if(signature->outputs.size() != func_sig->outputs.size())
  {
    log_error("In iteration plugin-function \"%s\", map function \"%s\" must "
        "have the same number of outputs",
        signature->name.c_str(), func_sig->name.c_str());
    return false;
  }

  for(size_t i = 0; i < signature->outputs.size(); i++)
  {
    if(signature->outputs[i].type != func_sig->outputs[i].type)
    {
      log_error("In iteration plugin-function \"%s\", output %zu types do "
          "not match (%d and %d)", signature->name.c_str(), i,
          (int) signature->outputs[i].type, (int) func_sig->outputs[i].type);
      return false;
    }

    if(signature->outputs[i].length
        != iter_count * func_sig->outputs[i].length)
    {
      log_error("In iteration plugin-function \"%s\", output %zu lengths do "
          "not match with %" PRIu64 " iterations (%zu and %zu)",
          signature->name.c_str(), i, iter_count, signature->outputs[i].length,
          func_sig->outputs[i].length);
      return false;
    }
  }

  if(signature->inputs.size() + (enumerated ? 1 : 0)
      != func_sig->inputs.size())
  {
    log_error("In iteration plugin-function \"%s\", map function \"%s\" must "
        "have the same number of inputs",
        signature->name.c_str(), func_sig->name.c_str());
    return false;
  }

  if(env_count > signature->inputs.size())
  {
    log_error("In iteration plugin-function \"%s\", the environment count "
        "outnumbers the available inputs", signature->name.c_str());
    return false;
  }

  size_t plugin_place = 0;
  size_t iter_place = 0;
  for(size_t i = 0; i < env_count; i++)
  {
    if(signature->inputs[plugin_place].type
        != func_sig->inputs[iter_place].type)
    {
      log_error("In iteration plugin-function \"%s\", input %zu types do "
          "not match (%d and %d)", signature->name.c_str(), i,
          (int) signature->inputs[plugin_place].type,
          (int) func_sig->inputs[iter_place].type);
      return false;
    }

    if(signature->inputs[plugin_place].length
        != func_sig->inputs[iter_place].length)
    {
      log_error("In iteration plugin-function \"%s\", environment input "
          "lengths %zu do not match (%zu and %zu)", signature->name.c_str(), i,
          signature->inputs[plugin_place].length,
          func_sig->inputs[iter_place].length);
      return false;
    }

    plugin_place++;
    iter_place++;
  }

  if(enumerated) { iter_place++; }

  for(size_t i = iter_place; i < func_sig->inputs.size(); i++)
  {
    if(signature->inputs[plugin_place].type
        != func_sig->inputs[iter_place].type)
    {
      log_error("In iteration plugin-function \"%s\", input %zu types do "
          "not match (%d and %d)", signature->name.c_str(), i,
          (int) signature->inputs[plugin_place].type,
          (int) func_sig->inputs[iter_place].type);
      return false;
    }

    if(signature->inputs[plugin_place].length
        != iter_count * func_sig->inputs[iter_place].length)
    {
      log_error("In iteration plugin-function \"%s\", input %zu lengths do "
          "not match with %" PRIu64 " iterations (%zu and %zu)",
          signature->name.c_str(), i, iter_count,
          signature->inputs[plugin_place].length,
          func_sig->inputs[iter_place].length);
      return false;
    }

    plugin_place++;
    iter_place++;
  }

  return true;
}

} } // namespace wtk::nails
