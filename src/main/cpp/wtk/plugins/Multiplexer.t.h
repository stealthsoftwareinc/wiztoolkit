/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
bool MuxOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  wtk::type_idx type = this->type;

  if(binding->parameters.size() != 0)
  {
    log_error(
        "In Multiplexer plugin function \"%s\", expected 0 plugin parameters",
        signature->name.c_str());
    return false;
  }

  if(signature->outputs.size() == 0)
  {
    log_error("In Multiplexer plugin function \"%s\", expected non-zero "
        "number of outputs", signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() - 1 < signature->outputs.size() * 2
      || (signature->inputs.size() - 1) % signature->outputs.size() != 0)
  {
    log_error("In Multiplexer plugin function \"%s\", number of inputs (%zu) "
        "must be one more than a multiple of the number of outputs (%zu)",
        signature->name.c_str(), signature->inputs.size(),
        signature->outputs.size());
    return false;
  }

  for(size_t i = 0; i < signature->outputs.size(); i++)
  {
    if(signature->outputs[i].type != type)
    {
      log_error("In Multiplexer plugin function \"%s\", output %zu type (%d) "
          "must be %d", signature->name.c_str(), i,
          (int) signature->outputs[i].type, (int) type);
      return false;
    }
  }

  if(signature->inputs[0].type != type)
  {
    log_error("In Multiplexer plugin function \"%s\", selector input must "
        "have type %d", signature->name.c_str(), (int) type);
    return false;
  }

  if(signature->inputs[0].length != 1
      && !this->backend->type->isBooleanField())
  {
    log_error("In Multiplexer plugin function \"%s\", selector input must "
        "have length 1", signature->name.c_str());
    return false;
  }

  size_t out_place = 0;
  for(size_t i = 1; i < signature->inputs.size(); i++)
  {
    if(signature->inputs[i].type != type)
    {
      log_error("In Multiplexer plugin function \"%s\", input %zu must "
          "have type %d", signature->name.c_str(), i, (int) type);
      return false;
    }

    if(signature->inputs[i].length != signature->outputs[out_place].length)
    {
      log_error("In Multiplexer plugin function \"%s\", input %zu must "
          " have length %zu corresponding to output %zu",
          signature->name.c_str(), i, signature->outputs[out_place].length,
          out_place);
      return false;
    }

    out_place = (out_place + 1) % signature->outputs.size();
  }

  Number_T n_cases = Number_T(signature->inputs.size() - 1);
  Number_T max_val = this->backend->type->maxValue();
  if(this->backend->type->isBooleanField())
  {
    Number_T n_sels = Number_T(1) << signature->inputs[0].length;
    if(n_cases > n_sels)
    {
      log_error("In Multiplexer plugin function \"%s\", the number of cases"
          "(%zu) must not exceed the bit-width 2**%zu",
          signature->name.c_str(), signature->inputs.size() - 1,
          signature->inputs[0].length);
      return false;
    }
  }
  else if(n_cases > max_val)
  {
    log_error("In Multiplexer plugin function \"%s\", the number of cases"
        "(%zu) must not exceed the prime (%s)",
        signature->name.c_str(), signature->inputs.size() - 1,
        wtk::utils::dec(max_val).c_str());
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void MuxOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  std::vector<size_t> sizes;
  std::vector<Wire_T*> mux_outs;
  Wire_T const* selector = nullptr;
  std::vector<std::vector<Wire_T const*>> mux_ins;

  log_assert(outputs.size() != 0);
  log_assert(inputs.size() > outputs.size());
  {bool f = (inputs.size() - 1) % outputs.size() == 0;log_assert(f);(void) f;}

  sizes.reserve(outputs.size());
  mux_outs.reserve(outputs.size());
  mux_ins.reserve((inputs.size() - 1) / outputs.size());

  for(size_t i = 0; i < outputs.size(); i++)
  {
    log_assert(outputs[i].size != 0);
    log_assert(outputs[i].type == this->type);

    sizes.push_back(outputs[i].size);
    mux_outs.push_back(static_cast<WiresRef<Wire_T>*>(&outputs[i])->get());
  }

  for(size_t i = 0; i < inputs.size() - 1 / outputs.size(); i++)
  {
    mux_ins.emplace_back();
    mux_ins.back().reserve(outputs.size());
  }

  log_assert(this->backend->type->isBooleanField() || inputs[0].size == 1);
  log_assert(inputs[0].type == this->type);

  selector = static_cast<WiresRef<Wire_T>*>(&inputs[0])->get();

  size_t out_place = 0;
  size_t in_group = 0;
  for(size_t i = 1; i < inputs.size(); i++)
  {
    log_assert(inputs[i].type == outputs[out_place].type
        && inputs[i].size == outputs[out_place].size);

    mux_ins[in_group].push_back(
        static_cast<WiresRef<Wire_T>*>(&inputs[i])->get());

    out_place = (out_place + 1) % outputs.size();
    in_group = (out_place == 0) ? in_group + 1 : in_group;
  }

  this->evaluateMux(sizes, mux_outs, selector, mux_ins, inputs[0].size);
}

template<typename Number_T, typename Wire_T>
bool MultiplexerPlugin<Number_T, Wire_T>::buildBackend(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    wtk::utils::CharMap<std::unique_ptr<
      SimpleOperation<Number_T, Wire_T>>>* const operations)
{
  {
    SimpleOperation<Number_T, Wire_T>* const strict =
      this->buildStrictMultiplexer(type, backend);

    if(strict == nullptr) { return false; }

    operations->emplace("strict", strict);
  }

  {
    SimpleOperation<Number_T, Wire_T>* const permissive =
      this->buildPermissiveMultiplexer(type, backend);

    if(permissive == nullptr) { return false; }

    operations->emplace("permissive", permissive);
  }

  return true;
}

template<typename Number_T, typename Wire_T>
SimpleOperation<Number_T, Wire_T>*
FallbackMultiplexerPlugin<Number_T, Wire_T>::buildStrictMultiplexer(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "Multiplexer plugin.");
    return nullptr;
  }

  if(backend->type->isBooleanField()) {
    return new StrictTreedBooleanMuxOperation<Number_T, Wire_T>(type, backend);
  }
  else {
    return new StrictFLTMuxOperation<Number_T, Wire_T>(type, backend);  
  }
  
}

template<typename Number_T, typename Wire_T>
SimpleOperation<Number_T, Wire_T>*
FallbackMultiplexerPlugin<Number_T, Wire_T>::buildPermissiveMultiplexer(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  if(backend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "Multiplexer plugin.");
    return nullptr;
  }

  if(backend->type->isBooleanField()) {
    return
      new PermissiveTreedBooleanMuxOperation<Number_T, Wire_T>(type, backend);
  }
  else {
    return new PermissiveFLTMuxOperation<Number_T, Wire_T>(type, backend);
  }
}

template<typename Number_T, typename Wire_T>
void exponentiate(wtk::TypeBackend<Number_T, Wire_T>* const backend,
    Wire_T* out, Wire_T const* const base, Number_T exp)
{
  Wire_T tmp_base[2] = { };
  Wire_T const* base_in_ptr = base;
  size_t base_place = 0;
  Wire_T* base_out_ptr = &tmp_base[base_place];

  Wire_T tmp_aux[2] = { };
  backend->assign(&tmp_aux[0], Number_T(1));
  size_t aux_place = 1;

  while(exp > 1)
  {
    if(exp % 2 == 0)
    {
      backend->mulGate(base_out_ptr, base_in_ptr, base_in_ptr);

      base_in_ptr = base_out_ptr;
      base_place = (base_place + 1) & 0x01;
      base_out_ptr = &tmp_base[base_place];

      exp = exp / 2;
    }
    else
    {
      size_t const aux_inc = (aux_place + 1) & 0x01;
      backend->mulGate(&tmp_aux[aux_place], base_in_ptr, &tmp_aux[aux_inc]);
      backend->mulGate(base_out_ptr, base_in_ptr, base_in_ptr);

      aux_place = aux_inc;
      base_in_ptr = base_out_ptr;
      base_place = (base_place + 1) & 0x01;
      base_out_ptr = &tmp_base[base_place];

      exp = (exp - 1) / 2;
    }
  }

  backend->mulGate(out, base_in_ptr, &tmp_aux[(aux_place + 1) & 0x01]);
}

// Checks if the input wire is equal to the input constant.
// Output is 0 if inequal and 1 if equal
template<typename Number_T, typename Wire_T>
void FLT_equality_const(wtk::TypeBackend<Number_T, Wire_T>* const backend,
    Wire_T* const out, Wire_T const* const in, size_t const c)
{
  Wire_T base;
  backend->addcGate(&base, in, backend->type->prime - c);
  Wire_T exp;
  exponentiate<Number_T>(backend, &exp, &base, backend->type->prime - 1);
  backend->mulcGate(&base, &exp, backend->type->prime - 1);
  backend->addcGate(out, &base, Number_T(1));
}

// Checks if the input wire is equal to the input constant.
// Eq is 0 if inequal and 1 if equal, ne is 1 if inequal, 0 if equal
template<typename Number_T, typename Wire_T>
void FLT_inequality_const(wtk::TypeBackend<Number_T, Wire_T>* const backend,
    Wire_T* const eq, Wire_T* const ne, Wire_T const* const in, size_t const c)
{
  Wire_T base;
  backend->addcGate(&base, in, backend->type->prime - c);
  exponentiate<Number_T>(backend, ne, &base, backend->type->prime - 1);
  backend->mulcGate(&base, ne, backend->type->prime - 1);
  backend->addcGate(eq, &base, Number_T(1));
}

template<typename Number_T, typename Wire_T>
void StrictFLTMuxOperation<Number_T, Wire_T>::evaluateMux(
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    Wire_T const* const selector,
    std::vector<std::vector<Wire_T const*>>& inputs,
    size_t const selector_bits)
{
  (void) selector_bits;
  wtk::TypeBackend<Number_T, Wire_T>* const backend = this->backend;
  std::vector<Wire_T> sel_bits(inputs.size());

  for(size_t i = 0; i < inputs.size(); i++)
  {
    FLT_equality_const(backend, &sel_bits[i], selector, i);
  }

  // Check that the sel_bits sum to 1
  {
    Wire_T sums[2] = { };
    backend->addcGate(&sums[0], &sel_bits[0], backend->type->prime - 1);
    size_t sum_idx = 0;

    for(size_t i = 1; i < sel_bits.size(); i++)
    {
      size_t sum_inv = (sum_idx + 1) & 1llu;
      backend->addGate(&sums[sum_inv], &sums[sum_idx], &sel_bits[i]);
      sum_idx = sum_inv;
    }

    backend->assertZero(&sums[sum_idx]);
  }

  // Multiply all the inputs by a selector bit, and add everything up.
  for(size_t i = 0; i < sizes.size(); i++)
  {
    for(size_t j = 0; j < sizes[i]; j++)
    {
      Wire_T inits[2] = { };
      backend->mulGate(&inits[0], inputs[0][i] + j, &sel_bits[0]);
      backend->mulGate(&inits[1], inputs[1][i] + j, &sel_bits[1]);

      if(inputs.size() == 2)
      {
        backend->addGate(outputs[i] + j, &inits[0], &inits[1]);
      }
      else
      {
        Wire_T tmp;
        Wire_T sums[2] = { };

        backend->addGate(&sums[0], &inits[0], &inits[1]);
        size_t sum_idx = 0;

        for(size_t k = 2; k < inputs.size() - 1; k++)
        {
          backend->mulGate(&tmp, inputs[k][i] + j, &sel_bits[k]);
          size_t sum_inv = (sum_idx + 1) & 1llu;
          backend->addGate(&sums[sum_inv], &tmp, &sums[sum_idx]);
          sum_idx = sum_inv;
        }

        backend->mulGate(&tmp,
            inputs[inputs.size() - 1][i] + j, &sel_bits[inputs.size() - 1]);
        backend->addGate(outputs[i] + j, &tmp, &sums[sum_idx]);
      }
    }
  }
}

template<typename Number_T, typename Wire_T>
void PermissiveFLTMuxOperation<Number_T, Wire_T>::evaluateMux(
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    Wire_T const* const selector,
    std::vector<std::vector<Wire_T const*>>& inputs,
    size_t const selector_bits)
{
  (void) selector_bits;
  wtk::TypeBackend<Number_T, Wire_T>* const backend = this->backend;
  std::vector<Wire_T> sel_bits(inputs.size());

  for(size_t i = 0; i < inputs.size(); i++)
  {
    FLT_equality_const(backend, &sel_bits[i], selector, i);
  }

  // Multiply all the inputs by a selector bit, and add everything up.
  for(size_t i = 0; i < sizes.size(); i++)
  {
    for(size_t j = 0; j < sizes[i]; j++)
    {
      Wire_T inits[2] = { };
      backend->mulGate(&inits[0], inputs[0][i] + j, &sel_bits[0]);
      backend->mulGate(&inits[1], inputs[1][i] + j, &sel_bits[1]);

      if(inputs.size() == 2)
      {
        backend->addGate(outputs[i] + j, &inits[0], &inits[1]);
      }
      else
      {
        Wire_T tmp;
        Wire_T sums[2] = { };

        backend->addGate(&sums[0], &inits[0], &inits[1]);
        size_t sum_idx = 0;

        for(size_t k = 2; k < inputs.size() - 1; k++)
        {
          backend->mulGate(&tmp, inputs[k][i] + j, &sel_bits[k]);
          size_t sum_inv = (sum_idx + 1) & 1llu;
          backend->addGate(&sums[sum_inv], &tmp, &sums[sum_idx]);
          sum_idx = sum_inv;
        }

        backend->mulGate(&tmp,
            inputs[inputs.size() - 1][i] + j, &sel_bits[inputs.size() - 1]);
        backend->addGate(outputs[i] + j, &tmp, &sums[sum_idx]);
      }
    }
  }
}



template<typename Number_T, typename Wire_T>
void oneMux(
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    std::vector<Wire_T const*>* input_pointer,
    Wire_T const* const selector)
{
  Wire_T selector_plus_one;
  backend->addcGate(&selector_plus_one, selector, Number_T(1));

  for(size_t i = 0; i < sizes.size(); i++)
  {
    for(size_t j = 0; j < sizes[i]; j++)
    {
      backend->mulGate(outputs[i] + j, input_pointer[0][i] + j, &selector_plus_one);
    } 
  }
}

template<typename Number_T, typename Wire_T>
void twoMux(
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    std::vector<Wire_T const*>* input_pointer,
    Wire_T const* const selector)
{
  Wire_T selector_plus_one;
  backend->addcGate(&selector_plus_one, selector, Number_T(1));

  for(size_t i = 0; i < sizes.size(); i++)
  {
    for(size_t j = 0; j < sizes[i]; j++)
    {
      Wire_T ands[2] = { };
      backend->mulGate(&ands[0], input_pointer[0][i] + j, &selector_plus_one);
      backend->mulGate(&ands[1], input_pointer[1][i] + j, selector);
      backend->addGate(outputs[i] + j, &ands[0], &ands[1]);  
      //backend->assign(outputs[i] + j, Number_T(0));
    } 
  }
}

template<typename Number_T, typename Wire_T>
void treeMux(
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    Wire_T const* const selector,
    std::vector<Wire_T const*>* input_pointer,
    size_t const selector_bits,
    size_t const num_inputs)
{
  if(selector_bits == 1) {
    if(num_inputs == 1) {
      oneMux(backend, sizes, outputs, input_pointer, selector);
    }
    else {
      twoMux(backend, sizes, outputs, input_pointer, selector);
    }

    return;
  }

  size_t two_to_the_sel_minus_one = (size_t(1) << (selector_bits - 1)); 
  std::vector<std::vector<Wire_T *>> temp_outputs (2, std::vector<Wire_T*>(outputs.size()));

  size_t total_output_wires = 0;

  for(size_t i = 0; i < sizes.size(); i++) {
    total_output_wires += sizes[i];
  }
  total_output_wires *= 2;

  std::vector<Wire_T> tmp_out_wires(total_output_wires);

  size_t temp_outputs_place = 0;
  for(size_t i = 0; i < 2; i++) {
    for(size_t j = 0; j < sizes.size(); j++) {

      temp_outputs[i][j] = &tmp_out_wires[temp_outputs_place];      
      temp_outputs_place += sizes[j];
      for (size_t k = 0; k < sizes[j]; k ++) {
        backend->assign(temp_outputs[i][j] + k, Number_T(0));
      }
    }
  }

  if(num_inputs <= two_to_the_sel_minus_one)
  {
    treeMux(backend, sizes, temp_outputs[0], selector + 1, input_pointer, selector_bits - 1, num_inputs);
  }
  else 
  {
    treeMux(backend, sizes, temp_outputs[0], selector + 1, input_pointer, selector_bits - 1, two_to_the_sel_minus_one);
    treeMux(backend, sizes, temp_outputs[1], selector + 1, input_pointer + two_to_the_sel_minus_one, selector_bits - 1 , num_inputs - two_to_the_sel_minus_one);
  }

  std::vector<std::vector<Wire_T const*>> temp_outputs_hack (2);


  for(size_t i = 0; i < 2; i++) {
    for(size_t j = 0; j < sizes.size(); j++) {
      temp_outputs_hack[i].push_back(temp_outputs[i][j]);
    }
  }

  twoMux(
    backend,
    sizes,
    outputs,
    &temp_outputs_hack[0],
    selector);
}



template<typename Number_T, typename Wire_T>
void PermissiveTreedBooleanMuxOperation<Number_T, Wire_T>::evaluateMux(
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    Wire_T const* const selector,
    std::vector<std::vector<Wire_T const*>>& inputs,
    size_t const selector_bits)
{
  wtk::TypeBackend<Number_T, Wire_T>* const backend = this->backend;

  treeMux(
    backend,
    sizes,
    outputs,
    selector,
    &inputs[0],
    selector_bits,
    inputs.size());

}


template<typename Number_T, typename Wire_T>
void StrictTreedBooleanMuxOperation<Number_T, Wire_T>::evaluateMux(
    std::vector<size_t> const& sizes,
    std::vector<Wire_T*>& outputs,
    Wire_T const* const selector,
    std::vector<std::vector<Wire_T const*>>& inputs,
    size_t const selector_bits)
{
  wtk::TypeBackend<Number_T, Wire_T>* const backend = this->backend;

  std::vector<size_t> sizes_copy(sizes.begin(), sizes.end());
  sizes_copy.push_back(1);

  Wire_T tmp_output_check_wire;
  backend->assign(&tmp_output_check_wire, Number_T(1));
  outputs.push_back(&tmp_output_check_wire);

  Wire_T tmp_input_check_wire;
  backend->assign(&tmp_input_check_wire, Number_T(1));

  for(size_t i = 0; i < inputs.size(); i++) {
    inputs[i].push_back(&tmp_input_check_wire);    
  }

  treeMux(
      backend,
      sizes_copy,
      outputs,
      selector,
      &inputs[0],
      selector_bits,
      inputs.size());

  Wire_T check_plus_one;
  backend->addcGate(&check_plus_one, outputs[outputs.size() - 1], Number_T(1));

  backend->assertZero(&check_plus_one);

  sizes_copy.pop_back();
  outputs.pop_back();
  for(size_t i = 0; i < inputs.size(); i++) {
    inputs[i].pop_back();
  }
}

} } // namespace wtk::plugins
