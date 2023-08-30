/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Wire_T>
void SimpleOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) binding;
  this->backend->lineNum = signature->lineNum;
  return this->evaluate(outputs, inputs);
}

template<typename Number_T, typename Wire_T>
void SimpleOperation<Number_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs)
{
  (void) outputs;
  (void) inputs;
}

template<typename Number_T, typename Wire_T>
bool SimplePlugin<Number_T, Wire_T>::addBackend(
    type_idx const type, TypeBackend<Number_T, Wire_T>* const backend)
{
  if((size_t) type >= this->reverseTypes.size())
  {
    this->reverseTypes.resize((size_t) type + 1, SIZE_MAX);
  }

  this->reverseTypes[(size_t) type] = this->types.size();
  this->types.emplace_back();
  return this->buildBackend(type, backend, &this->types.back());
}

template<typename Number_T, typename Wire_T>
Operation<Number_T>* SimplePlugin<Number_T, Wire_T>::create(
    type_idx const type,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  if((size_t) type >= this->reverseTypes.size()
      || this->reverseTypes[(size_t) type] == SIZE_MAX)
  {
    log_error("%s plugin could not locate type %zu",
        binding->name.c_str(), (size_t) type);
    return nullptr;
  }

  wtk::utils::CharMap<std::unique_ptr<
    SimpleOperation<Number_T, Wire_T>>>* const operations =
      &this->types[this->reverseTypes[(size_t) type]];
  auto finder = operations->find(binding->operation.c_str());
  if(finder == operations->end())
  {
    log_error("%s plugin has no operation %s",
        binding->name.c_str(), binding->operation.c_str());
    return nullptr;
  }

  if(finder->second->checkSignature(signature, binding))
  {
    return finder->second.get();
  }
  else { return nullptr; }
}

} } // namespace wtk::plugins
