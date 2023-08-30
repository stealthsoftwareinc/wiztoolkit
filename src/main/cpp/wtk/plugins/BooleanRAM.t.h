/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Buffer_T, typename Wire_T>
void BoolRAMInitOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;

  log_assert(outputs.size() == 1 && outputs[0].type == this->type
      && outputs[0].size == 1);
  log_assert(inputs.size() == 1
      && inputs[0].size == this->ramBackend()->elementBits
      && inputs[0].type == this->ramBackend()->wireType);

  Buffer_T* const buffer =
    static_cast<WiresRef<Buffer_T>*>(&outputs[0])->get();
  Wire_T const* const fill =
    static_cast<WiresRef<Wire_T>*>(&inputs[0])->get();
  wire_idx const size = wtk::utils::cast_wire(binding->parameters[0].number);

  return this->init(size, buffer, fill);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool BoolRAMInitOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 1 || signature->outputs[0].type != this->type
      || signature->outputs[0].length != 1)
  {
    log_error(
        "Boolean RAM init operation expects one output of RAM Buffer type");
    return false;
  }

  if(signature->inputs.size() != 1
      || signature->inputs[0].type != ram->wireType
      || signature->inputs[0].length != ram->elementBits)
  {
    log_error("Boolean RAM init operation expects one input of element type "
        "and width %" PRIu64 "", ram->elementBits);
    return false;
  }

  if(binding->parameters.size() != 1 || binding->parameters[0].form
      != wtk::circuit::PluginBinding<Number_T>::Parameter::numeric)
  {
    log_error(
        "Boolean RAM init operation expects one numeric plugin parameter.");
    return false;
  }

  Number_T max_val = Number_T(1) << ram->indexBits;
  if(binding->parameters[0].number > max_val)
  {
    log_error("Boolean RAM buffer cannot exceed maximum size %s "
        "(%" PRIu64 " bits)",
        wtk::utils::dec(max_val).c_str(), ram->indexBits);
    return false;
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
void BoolRAMReadOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;
  (void) binding;

  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  log_assert(outputs.size() == 1 && outputs[0].type == ram->wireType
      && outputs[0].size == ram->elementBits); (void) ram;
  log_assert(inputs.size() == 2 && inputs[0].type == this->type
      && inputs[0].size == 1 && inputs[1].type == ram->wireType
      && inputs[1].size == ram->indexBits);

  Buffer_T* const buffer = static_cast<WiresRef<Buffer_T>*>(&inputs[0])->get();
  Wire_T const* const index = static_cast<WiresRef<Wire_T>*>(&inputs[1])->get();
  Wire_T* const out = static_cast<WiresRef<Wire_T>*>(&outputs[0])->get();

  return this->read(out, buffer, index);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool BoolRAMReadOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 1
      || signature->outputs[0].type != ram->wireType
      || signature->outputs[0].length != ram->elementBits)
  {
    log_error("Boolean RAM read operation expects one output of element type"
        " and width (%" PRIu64 ")", ram->elementBits);
    return false;
  }

  if(signature->inputs.size() != 2 || signature->inputs[0].type != this->type
      || signature->inputs[0].length != 1)
  {
    log_error(
        "Boolean RAM read operation expects one input of RAM Buffer type");
    return false;
  }

  if(signature->inputs[1].type != ram->wireType
      || signature->inputs[1].length != ram->indexBits)
  {
    log_error("Boolean RAM read operation expects one input of index type"
        " and width (%" PRIu64 ")", ram->indexBits);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("Boolean RAM read operation expects no plugin parameter.");
    return false;
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
void BoolRAMWriteOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;
  (void) binding;

  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  log_assert(outputs.size() == 0); (void) outputs;
  log_assert(inputs.size() == 3 && inputs[0].type == this->type
      && inputs[0].size == 1 && inputs[1].type == ram->wireType
      && inputs[1].size == ram->indexBits && inputs[2].type == ram->wireType
      && inputs[2].size == ram->elementBits); (void) ram;

  Buffer_T* const buffer = static_cast<WiresRef<Buffer_T>*>(&inputs[0])->get();
  Wire_T const* const idx = static_cast<WiresRef<Wire_T>*>(&inputs[1])->get();
  Wire_T const* const in = static_cast<WiresRef<Wire_T>*>(&inputs[2])->get();

  return this->write(buffer, idx, in);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool BoolRAMWriteOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 0)
  {
    log_error("Boolean RAM write operation expects no outputs");
    return false;
  }

  if(signature->inputs.size() != 3 || signature->inputs[0].type != this->type
      || signature->inputs[0].length != 1)
  {
    log_error(
        "Boolean RAM write operation expects one input of RAM Buffer type");
    return false;
  }

  if(signature->inputs[1].type != ram->wireType
      || signature->inputs[1].length != ram->indexBits)
  {
    log_error("Boolean RAM write operation expects one input of index type"
        " and width (%" PRIu64 ")", ram->indexBits);
    return false;
  }

  if(signature->inputs[2].type != ram->wireType
      || signature->inputs[2].length != ram->elementBits)
  {
    log_error("Boolean RAM write operation expects one input of element type"
        " and width (%" PRIu64 ")", ram->elementBits);
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("Boolean RAM write operation expects no plugin parameter.");
    return false;
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool BoolRAMPlugin<Number_T, Buffer_T, Wire_T>::buildBackend(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Buffer_T>* const backend,
    wtk::utils::CharMap<std::unique_ptr<
      SimpleOperation<Number_T, Buffer_T>>>* const operations)
{
  BoolRAMBackend<Number_T, Buffer_T, Wire_T>* const ram_backend =
    static_cast<BoolRAMBackend<Number_T, Buffer_T, Wire_T>*>(backend);

  auto init = this->buildInitOperation(type, ram_backend);
  if(init == nullptr) { return false; }
  operations->emplace("init", init);

  auto read = this->buildReadOperation(type, ram_backend);
  if(read == nullptr) { return false; }
  operations->emplace("read", read);

  auto write = this->buildWriteOperation(type, ram_backend);
  if(write == nullptr) { return false; }
  operations->emplace("write", write);

  return true;
}

template<typename Number_T>
bool checkBoolRAMType(wtk::circuit::TypeSpec<Number_T> const* const type,
    wtk::type_idx* const idx_type, wtk::wire_idx* const idx_bits,
    wtk::wire_idx* const elt_bits, wtk::wire_idx* const num_allocs,
    wtk::wire_idx* const total_allocs, wtk::wire_idx* const max_alloc)
{
  if(type->variety != wtk::circuit::TypeSpec<Number_T>::plugin)
  {
    log_error(
        "Cannot initialize Boolean RAM Type from non-plugin type declaration");
    return false;
  }

  if(type->binding.name != "ram_bool_v0")
  {
    log_error("Cannot initialize Boolean RAM type from \"%s\" plugin binding",
        type->binding.name.c_str());
    return false;
  }

  if(type->binding.operation != "ram")
  {
    log_error("Boolean RAM plugin cannot recognize type \"%s\"",
        type->binding.name.c_str());
    return false;
  }

  if(type->binding.parameters.size() != 6)
  {
    log_error("Boolean RAM plugin type expects 6 arguments");
    return false;
  }

  if(type->binding.parameters[0].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error("RAM plugin parameter 0 (index type) should be numeric");
    return false;
  }
  else if(idx_type != nullptr)
  {
    *idx_type = wtk::utils::cast_type(type->binding.parameters[0].number);
  }

  if(type->binding.parameters[1].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error("RAM plugin parameter 1 (index bit-width) should be numeric");
    return false;
  }
  else if(idx_bits != nullptr)
  {
    *idx_bits = wtk::utils::cast_type(type->binding.parameters[1].number);
  }

  if(type->binding.parameters[2].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error("RAM plugin parameter 2 (element bit-width) should be numeric");
    return false;
  }
  else if(elt_bits != nullptr)
  {
    *elt_bits = wtk::utils::cast_type(type->binding.parameters[2].number);
  }

  if(type->binding.parameters[3].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error(
        "RAM plugin parameter 3 (number of allocations) should be numeric");
    return false;
  }
  else if(num_allocs != nullptr)
  {
    *num_allocs = wtk::utils::cast_wire(type->binding.parameters[3].number);
  }

  if(type->binding.parameters[4].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error(
        "RAM plugin parameter 4 (total allocated space) should be numeric");
    return false;
  }
  else if(total_allocs != nullptr)
  {
    *total_allocs = wtk::utils::cast_wire(type->binding.parameters[4].number);
  }

  if(type->binding.parameters[5].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error(
        "RAM plugin parameter 5 (maximum allocated space) should be numeric");
    return false;
  }
  else if(max_alloc != nullptr)
  {
    *max_alloc = wtk::utils::cast_wire(type->binding.parameters[5].number);
  }

  return true;
}


template<typename Wire_T>
FallbackBoolRAMBuffer<Wire_T>::~FallbackBoolRAMBuffer()
{
  
  for(size_t i = 0; i < this->raw_size; i++)
  {
    (this->raw_buffer + i)->~Wire_T();
  }

  free(this->raw_buffer);
}

template<typename Number_T, typename Wire_T>
bool FallbackBoolRAMBackend<Number_T, Wire_T>::check()
{
  return true; // errors are reported through this->wireBackend->assertZero()
}

template<typename Number_T, typename Wire_T>
void FallbackBoolRAMInitOperation<Number_T, Wire_T>::init(wtk::wire_idx const size,
    FallbackBoolRAMBuffer<Wire_T>* const buffer, Wire_T const* const fill)
{
  buffer->size = (size_t) size;
  buffer->raw_size = (size_t) size * this->ramBackend()->elementBits;
  buffer->raw_buffer = (Wire_T*) malloc(sizeof(Wire_T) * buffer->size * this->ramBackend()->elementBits);

  buffer->buffer = std::vector<std::vector<Wire_T const*>>(size, std::vector<Wire_T const*>(1));

  for(size_t i = 0; i < buffer->size; i++)
  {
    for(size_t j = 0; j < this->ramBackend()->elementBits; j++)
    {
      new(buffer->raw_buffer + i * this->ramBackend()->elementBits + j) Wire_T();
      this->ramBackend()->wireBackend->copy(buffer->raw_buffer + i * this->ramBackend()->elementBits + j, fill + j);
    }
    buffer->buffer[i][0] = &buffer->raw_buffer[i * this->ramBackend()->elementBits];
  }
}

template<typename Number_T, typename Wire_T>
void FallbackBoolRAMReadOperation<Number_T, Wire_T>::read(Wire_T* const out,
    FallbackBoolRAMBuffer<Wire_T>* const buffer, Wire_T const* const idx)
{

  std::vector<size_t> sizes(1);
  sizes[0] = this->ramBackend()->elementBits;
  std::vector<Wire_T*> outputs(1);
  outputs[0] = out;

  (&BoolMuxOperation)->evaluateMux(sizes,
      outputs,
      idx,
      buffer->buffer,
      this->ramBackend()->indexBits);
} 

// Equality check of a known and unknown wire range
// Eq is 0 if inequal and 1 if equal
template<typename Number_T, typename Wire_T>
void bool_equality(wtk::TypeBackend<Number_T, Wire_T>* const backend,
    Wire_T* const eq, Wire_T const* const in, size_t const c, size_t num_bits)
{

  Wire_T temp;
  Wire_T temp2;

  Number_T c_copy = c;

  backend->assign(eq, Number_T(1));

  for(size_t i = 0; i < num_bits; i++) {
    size_t c_bit_plus_one = wtk::utils::cast_size((c_copy+1) % 2);
    backend->addcGate(&temp, in + num_bits - i - 1, c_bit_plus_one);
    c_copy /= 2;
    backend->copy(&temp2, eq);
    backend->mulGate(eq, &temp2, &temp);
  }
}

template<typename Number_T, typename Wire_T>
void FallbackBoolRAMWriteOperation<Number_T, Wire_T>::write(
    FallbackBoolRAMBuffer<Wire_T>* const buffer,
    Wire_T const* const idx, Wire_T const* const in)
{
  wtk::TypeBackend<Number_T, Wire_T>* const backend =
    this->ramBackend()->wireBackend;

  Wire_T eq;
  Wire_T tmp;
  Wire_T tmp2;

  Wire_T sum_eq;
  backend->assign(&sum_eq, Number_T(1));

  for(size_t i = 0; i < buffer->size; i++)
  {
    bool_equality(backend, &eq, idx, i, this->ramBackend()->indexBits);
    backend->addGate(&tmp, &sum_eq, &eq);
    backend->copy(&sum_eq, &tmp);
    for(size_t j = 0; j < this->ramBackend()->elementBits; j++)
    {
      backend->addGate(&tmp, buffer->buffer[i][0] + j, in + j);
      backend->mulGate(&tmp2, &eq, &tmp);
      backend->copy(&tmp, buffer->buffer[i][0] + j);
      backend->addGate(buffer->raw_buffer + i * this->ramBackend()->elementBits + j, &tmp, &tmp2);
    }
  }
  backend->assertZero(&sum_eq);
}

template<typename Number_T, typename Wire_T>
BoolRAMInitOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>*
FallbackBoolRAMPlugin<Number_T, Wire_T>::buildInitOperation(
    wtk::type_idx const type, BoolRAMBackend<
      Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
{
  return new FallbackBoolRAMInitOperation<Number_T, Wire_T>(type, be);
}

template<typename Number_T, typename Wire_T>
BoolRAMReadOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>*
FallbackBoolRAMPlugin<Number_T, Wire_T>::buildReadOperation(
    wtk::type_idx const type, BoolRAMBackend<
      Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
{
  return new FallbackBoolRAMReadOperation<Number_T, Wire_T>(type, be);
}

template<typename Number_T, typename Wire_T>
BoolRAMWriteOperation<Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>*
FallbackBoolRAMPlugin<Number_T, Wire_T>::buildWriteOperation(
    wtk::type_idx const type, BoolRAMBackend<
      Number_T, FallbackBoolRAMBuffer<Wire_T>, Wire_T>* const be)
{
  return new FallbackBoolRAMWriteOperation<Number_T, Wire_T>(type, be);
}


} } // namespace wtk::plugins
