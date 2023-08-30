/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace nails {

template<typename Number_T>
bool PluginFunction<Number_T>::evaluate(
    Interpreter<Number_T>* const interpreter)
{
  std::vector<size_t> places(interpreter->interpreters.size(), 0);

  std::vector<wtk::plugins::WiresRefEraser> outputs;
  std::vector<wtk::plugins::WiresRefEraser> inputs;

  outputs.reserve(this->signature.outputs.size());
  for(size_t i = 0; i < this->signature.outputs.size(); i++)
  {
    size_t const type = (size_t) this->signature.outputs[i].type;
    wtk::wire_idx first = places[type];
    wtk::wire_idx last = places[type] + this->signature.outputs[i].length - 1;
    places[type] += this->signature.outputs[i].length;
    outputs.push_back(
        interpreter->interpreters[type]->pluginOutput(
          (type_idx) type, first, last));
  }

  inputs.reserve(this->signature.inputs.size());
  for(size_t i = 0; i < this->signature.inputs.size(); i++)
  {
    size_t const type = (size_t) this->signature.inputs[i].type;
    wtk::wire_idx first = places[type];
    wtk::wire_idx last = places[type] + this->signature.inputs[i].length - 1;
    places[type] += this->signature.inputs[i].length;
    inputs.push_back(
        interpreter->interpreters[type]->pluginInput(
          (type_idx) type, first, last));
  }

  this->operation->evaluate(outputs, inputs, &this->signature, &this->binding);
  return true;
}

} } // namespace wtk::nails
