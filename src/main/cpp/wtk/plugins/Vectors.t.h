/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
bool PairwiseOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 1)
  {
    log_error("In %s plugin function %s, expected exactly 1 output range",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() != 2)
  {
    log_error("In %s plugin function %s, expected exactly 2 input ranges",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  type_idx const sig_type = signature->outputs[0].type;
  size_t range_size = signature->outputs[0].length;

  if(signature->inputs[0].type != sig_type
      || signature->inputs[0].length != range_size)
  {
    log_error("In %s plugin function %s, first input (%zu:%zu) does not match"
        " output (%zu:%zu)", binding->name.c_str(), signature->name.c_str(),
        (size_t) signature->inputs[0].type, signature->inputs[0].length,
        (size_t) sig_type, range_size);
    return false;
  }

  if(signature->inputs[1].type != sig_type
      || signature->inputs[1].length != range_size)
  {
    log_error("In %s plugin function %s, second input (%zu:%zu) does not match"
        " output (%zu:%zu)", binding->name.c_str(), signature->name.c_str(),
        (size_t) signature->inputs[0].type, signature->inputs[0].length,
        (size_t) sig_type, range_size);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void PairwiseOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 1 && inputs.size() == 2);
  log_assert(outputs[0].type == this->type && inputs[0].type == this->type
      && inputs[1].type == this->type);
  log_assert(outputs[0].size == inputs[0].size
      && outputs[0].size == inputs[1].size);

  return this->evaluatePairwise(outputs[0].size,
      static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[1])->get());
}

template<typename Number_T, typename Wire_T>
bool UniFoldOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 1)
  {
    log_error("In %s plugin function %s, expected exactly 1 output range",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() != 1)
  {
    log_error("In %s plugin function %s, expected exactly 1 input range",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  if(signature->inputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[0].type);
    return false;
  }

  if(signature->outputs[0].length != 1)
  {
    log_error("in %s plugin function %s, output length must be 1",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs[0].length <= 1)
  {
    log_error("in %s plugin function %s, input length must be greater than 1",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void UniFoldOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 1 && inputs.size() == 1);
  log_assert(outputs[0].type == this->type && inputs[0].type == this->type);
  log_assert(outputs[0].size == 1 && inputs[0].size > 1);

  return this->evaluateUniFold(inputs[0].size,
      static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get());
}

template<typename Number_T, typename Wire_T>
bool BiFoldOperation<Number_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if(signature->outputs.size() != 1)
  {
    log_error("In %s plugin function %s, expected exactly 1 output range",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs.size() != 2)
  {
    log_error("In %s plugin function %s, expected exactly 2 input ranges",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->outputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching output type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->outputs[0].type);
    return false;
  }

  if(signature->inputs[0].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[0].type);
    return false;
  }

  if(signature->inputs[1].type != this->type)
  {
    log_error("in %s plugin function %s, non-matching input type %d",
        binding->name.c_str(), signature->name.c_str(),
        (int) signature->inputs[1].type);
    return false;
  }

  if(signature->outputs[0].length != 1)
  {
    log_error("in %s plugin function %s, output length must be 1",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs[0].length <= 1)
  {
    log_error("in %s plugin function %s, input length must be greater than 1",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  if(signature->inputs[0].length != signature->inputs[1].length)
  {
    log_error(
        "in %s plugin function %s, mismatching input sizes (%zu and %zu)",
        binding->name.c_str(), signature->name.c_str(),
        signature->inputs[0].length, signature->inputs[1].length);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("In %s plugin function %s, not expecting any plugin parameters",
        binding->name.c_str(), signature->name.c_str());
    return false;
  }

  return true;
}

template<typename Number_T, typename Wire_T>
void BiFoldOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  log_assert(outputs.size() == 1 && inputs.size() == 2);
  log_assert(outputs[0].type == this->type && inputs[0].type == this->type
      && inputs[1].type == this->type);
  log_assert(outputs[0].size == 1 && inputs[0].size > 1
      && inputs[0].size == inputs[1].size);

  return this->evaluateBiFold(inputs[0].size,
      static_cast<WiresRef<Wire_T>*>(&outputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[0])->get(),
      static_cast<WiresRef<Wire_T>*>(&inputs[1])->get());
}

template<typename Number_T, typename Wire_T>
bool VectorPlugin<Number_T, Wire_T>::buildBackend(wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend,
    wtk::utils::CharMap<std::unique_ptr<
      SimpleOperation<Number_T, Wire_T>>>* const operations)
{
  // Create an addition operation
  {
    auto add_op = this->buildAdd(type, backend);
    if(add_op == nullptr) { return false; }
    operations->emplace("add", add_op);
  }

  // Create an multiplication operation
  {
    auto mul_op = this->buildMul(type, backend);
    if(mul_op == nullptr) { return false; }
    operations->emplace("mul", mul_op);
  }

  // Create a sum operation
  {
    auto sum_op = this->buildSum(type, backend);
    if(sum_op == nullptr) { return false; }
    operations->emplace("sum", sum_op);
  }

  // Create a product operation
  {
    auto product_op = this->buildProduct(type, backend);
    if(product_op == nullptr) { return false; }
    operations->emplace("product", product_op);
  }

  // Create a dot product operation
  {
    auto dot_product_op = this->buildDotProduct(type, backend);
    if(dot_product_op == nullptr) { return false; }
    operations->emplace("dotproduct", dot_product_op);
  }

  return true;
}

// ==== Fallback Operations and Plugin ====

template<typename Number_T, typename Wire_T>
void FallbackAddOperation<Number_T, Wire_T>::evaluatePairwise(
    size_t const size, Wire_T* const outs,
    Wire_T const* const lefts, Wire_T const* const rights)
{
  for(size_t i = 0; i < size; i++)
  {
    this->backend->addGate(outs + i, lefts + i, rights + i);
  }
}

template<typename Number_T, typename Wire_T>
void FallbackMulOperation<Number_T, Wire_T>::evaluatePairwise(
    size_t const size, Wire_T* const outs,
    Wire_T const* const lefts, Wire_T const* const rights)
{
  for(size_t i = 0; i < size; i++)
  {
    this->backend->mulGate(outs + i, lefts + i, rights + i);
  }
}

template<typename Number_T, typename Wire_T>
void FallbackSumOperation<Number_T, Wire_T>::evaluateUniFold(
    size_t const size, Wire_T* const out, Wire_T const* const ins)
{
  if(size == 2)
  {
    this->backend->addGate(out, ins, ins + 1);
  }
  else
  {
    Wire_T tmps[2] { };
    size_t place = 0;

    this->backend->addGate(&tmps[place], ins, ins + 1);

    for(size_t i = 2; i < size - 1; i++)
    {
      size_t alt_place = (place + 1) & 0x01;
      this->backend->addGate(&tmps[alt_place], &tmps[place], ins + i);
      place = alt_place;
    }

    this->backend->addGate(out, &tmps[place], ins + size - 1);
  }
}

template<typename Number_T, typename Wire_T>
void FallbackProductOperation<Number_T, Wire_T>::evaluateUniFold(
    size_t const size, Wire_T* const out, Wire_T const* const ins)
{
  if(size == 2)
  {
    this->backend->mulGate(out, ins, ins + 1);
  }
  else
  {
    Wire_T tmps[2] { };
    size_t place = 0;

    this->backend->mulGate(&tmps[place], ins, ins + 1);

    for(size_t i = 2; i < size - 1; i++)
    {
      size_t alt_place = (place + 1) & 0x01;
      this->backend->mulGate(&tmps[alt_place], &tmps[place], ins + i);
      place = alt_place;
    }

    this->backend->mulGate(out, &tmps[place], ins + size - 1);
  }
}

template<typename Number_T, typename Wire_T>
void FallbackDotProductOperation<Number_T, Wire_T>::evaluateBiFold(
    size_t const size, Wire_T* const out,
    Wire_T const* const lefts, Wire_T const* const rights)
{

  if(size == 2)
  {
    Wire_T muls[2] = { };
    this->backend->mulGate(&muls[0], lefts, rights);
    this->backend->mulGate(&muls[1], lefts + 1, rights + 1);
    this->backend->addGate(out, &muls[0], &muls[1]);
  }
  else
  {
    Wire_T muls[2] = { };
    Wire_T tmps[2] = { };
    size_t place = 0;

    this->backend->mulGate(&muls[0], lefts, rights);
    this->backend->mulGate(&muls[1], lefts + 1, rights + 1);
    this->backend->addGate(&tmps[place], &muls[0], &muls[1]);

    for(size_t i = 2; i < size - 1; i++)
    {
      size_t alt_place = (place + 1) & 0x01;
      this->backend->mulGate(&muls[0], lefts + i, rights + i);
      this->backend->addGate(&tmps[alt_place], &tmps[place], &muls[0]);
      place = alt_place;
    }

    this->backend->mulGate(&muls[0], lefts + size - 1, rights + size - 1);
    this->backend->addGate(out, &tmps[place], &muls[0]);
  }
}

template<typename Number_T, typename Wire_T>
PairwiseOperation<Number_T, Wire_T>*
FallbackVectorPlugin<Number_T, Wire_T>::buildAdd(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  return new FallbackAddOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
PairwiseOperation<Number_T, Wire_T>*
FallbackVectorPlugin<Number_T, Wire_T>::buildMul(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  return new FallbackMulOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
UniFoldOperation<Number_T, Wire_T>*
FallbackVectorPlugin<Number_T, Wire_T>::buildSum(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  return new FallbackSumOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
UniFoldOperation<Number_T, Wire_T>*
FallbackVectorPlugin<Number_T, Wire_T>::buildProduct(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  return new FallbackProductOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
BiFoldOperation<Number_T, Wire_T>*
FallbackVectorPlugin<Number_T, Wire_T>::buildDotProduct(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Wire_T>* const backend)
{
  return new FallbackDotProductOperation<Number_T, Wire_T>(type, backend);
}

} } // namespace wtk::plugins
