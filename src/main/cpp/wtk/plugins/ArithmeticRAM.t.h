/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace plugins {

template<typename Number_T, typename Buffer_T, typename Wire_T>
void RAMInitOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;

  log_assert(outputs.size() == 1 && outputs[0].type == this->type
      && outputs[0].size == 1);
  log_assert(inputs.size() == 1 && inputs[0].size == 1
      && inputs[0].type == this->ramBackend()->wireType);

  Buffer_T* const buffer =
    static_cast<WiresRef<Buffer_T>*>(&outputs[0])->get();
  Wire_T const* const fill =
    static_cast<WiresRef<Wire_T>*>(&inputs[0])->get();
  wire_idx const size = wtk::utils::cast_wire(binding->parameters[0].number);

  return this->init(size, buffer, fill);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool RAMInitOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  RAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 1 || signature->outputs[0].type != this->type
      || signature->outputs[0].length != 1)
  {
    log_error("RAM init operation expects one output of RAM Buffer type");
    return false;
  }

  if(signature->inputs.size() != 1
      || signature->inputs[0].type != ram->wireType
      || signature->inputs[0].length != 1)
  {
    log_error("RAM init operation expects one input of element type");
    return false;
  }

  if(binding->parameters.size() != 1 || binding->parameters[0].form
      != wtk::circuit::PluginBinding<Number_T>::Parameter::numeric)
  {
    log_error("RAM init operation expects one numeric plugin parameter.");
    return false;
  }

  if(binding->parameters[0].number > ram->wireBackend->type->maxValue())
  {
    log_error("RAM buffer size (%s) cannot exceed maximum size (%s)",
        wtk::utils::dec(binding->parameters[0].number).c_str(),
        wtk::utils::dec(ram->wireBackend->type->maxValue()).c_str());
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
void RAMReadOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;
  (void) binding;

  RAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  log_assert(outputs.size() == 1 && outputs[0].type == ram->wireType
      && outputs[0].size == 1); (void) ram;
  log_assert(inputs.size() == 2 && inputs[0].type == this->type
      && inputs[0].size == 1 && inputs[1].type == ram->wireType
      && inputs[1].size == 1);

  Buffer_T* const buffer = static_cast<WiresRef<Buffer_T>*>(&inputs[0])->get();
  Wire_T const* const index = static_cast<WiresRef<Wire_T>*>(&inputs[1])->get();
  Wire_T* const out = static_cast<WiresRef<Wire_T>*>(&outputs[0])->get();

  return this->read(out, buffer, index);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool RAMReadOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  RAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 1
      || signature->outputs[0].type != ram->wireType
      || signature->outputs[0].length != 1)
  {
    log_error("RAM read operation expects one output of element type");
    return false;
  }

  if(signature->inputs.size() != 2 || signature->inputs[0].type != this->type
      || signature->inputs[0].length != 1)
  {
    log_error("RAM read operation expects one input of RAM Buffer type");
    return false;
  }

  if(signature->inputs[1].type != ram->wireType
      || signature->inputs[1].length != 1)
  {
    log_error("RAM read operation expects one input of index type");
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("RAM read operation expects no plugin parameter.");
    return false;
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
void RAMWriteOperation<Number_T,  Buffer_T, Wire_T>::evaluate(
    std::vector<WiresRefEraser>& outputs,
    std::vector<WiresRefEraser>& inputs,
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  (void) signature;
  (void) binding;

  log_assert(outputs.size() == 0); (void) outputs;
  log_assert(inputs.size() == 3 && inputs[0].type == this->type
      && inputs[0].size == 1 && inputs[1].type == this->ramBackend()->wireType
      && inputs[1].size == 1 && inputs[2].type == this->ramBackend()->wireType
      && inputs[2].size == 1);

  Buffer_T* const buffer = static_cast<WiresRef<Buffer_T>*>(&inputs[0])->get();
  Wire_T const* const idx = static_cast<WiresRef<Wire_T>*>(&inputs[1])->get();
  Wire_T const* const in = static_cast<WiresRef<Wire_T>*>(&inputs[2])->get();

  return this->write(buffer, idx, in);
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool RAMWriteOperation<Number_T,  Buffer_T, Wire_T>::checkSignature(
    wtk::circuit::FunctionSignature const* const signature,
    wtk::circuit::PluginBinding<Number_T> const* const binding)
{
  RAMBackend<Number_T, Buffer_T, Wire_T>* const ram = this->ramBackend();

  if(signature->outputs.size() != 0)
  {
    log_error("RAM write operation expects no outputs");
    return false;
  }

  if(signature->inputs.size() != 3 || signature->inputs[0].type != this->type
      || signature->inputs[0].length != 1)
  {
    log_error("RAM write operation expects one input of RAM Buffer type");
    return false;
  }

  if(signature->inputs[1].type != ram->wireType
      || signature->inputs[1].length != 1)
  {
    log_error("RAM write operation expects one input of index type");
    return false;
  }

  if(signature->inputs[2].type != ram->wireType
      || signature->inputs[2].length != 1)
  {
    log_error("RAM write operation expects one input of element type");
    return false;
  }

  if(binding->parameters.size() != 0)
  {
    log_error("RAM write operation expects no plugin parameter.");
    return false;
  }

  return true;
}

template<typename Number_T, typename Buffer_T, typename Wire_T>
bool RAMPlugin<Number_T, Buffer_T, Wire_T>::buildBackend(
    wtk::type_idx const type,
    wtk::TypeBackend<Number_T, Buffer_T>* const backend,
    wtk::utils::CharMap<std::unique_ptr<
      SimpleOperation<Number_T, Buffer_T>>>* const operations)
{
  RAMBackend<Number_T, Buffer_T, Wire_T>* const ram_backend =
    static_cast<RAMBackend<Number_T, Buffer_T, Wire_T>*>(backend);

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
bool checkRAMv0Type(wtk::circuit::TypeSpec<Number_T> const* const type,
    wtk::type_idx* const idx_type, wtk::wire_idx* const num_allocs,
    wtk::wire_idx* const total_allocs, wtk::wire_idx* const max_alloc)
{
  if(type->variety != wtk::circuit::TypeSpec<Number_T>::plugin)
  {
    log_error("Cannot initialize RAM Type from non-plugin type declaration");
    return false;
  }

  if(type->binding.name != "ram_arith_v0")
  {
    log_error("Cannot initialize RAM type from \"%s\" plugin binding",
        type->binding.name.c_str());
    return false;
  }

  if(type->binding.operation != "ram")
  {
    log_error("RAM plugin cannot recognize type \"%s\"",
        type->binding.name.c_str());
    return false;
  }

  if(type->binding.parameters.size() != 4)
  {
    log_error("RAM plugin type expects 4 arguments");
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
    log_error(
        "RAM plugin parameter 1 (number of allocations) should be numeric");
    return false;
  }
  else if(num_allocs != nullptr)
  {
    *num_allocs = wtk::utils::cast_wire(type->binding.parameters[1].number);
  }

  if(type->binding.parameters[2].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error(
        "RAM plugin parameter 2 (total allocated space) should be numeric");
    return false;
  }
  else if(total_allocs != nullptr)
  {
    *total_allocs = wtk::utils::cast_wire(type->binding.parameters[2].number);
  }

  if(type->binding.parameters[3].form
      == wtk::circuit::PluginBinding<Number_T>::Parameter::textual)
  {
    log_error(
        "RAM plugin parameter 3 (maximum allocated space) should be numeric");
    return false;
  }
  else if(max_alloc != nullptr)
  {
    *max_alloc = wtk::utils::cast_wire(type->binding.parameters[3].number);
  }

  return true;
}

template<typename Wire_T>
FallbackRAMBuffer<Wire_T>::~FallbackRAMBuffer()
{
  for(size_t i = 0; i < this->size; i++)
  {
    (this->buffer + i)->~Wire_T();
  }

  free(this->buffer);
}

template<typename Number_T, typename Wire_T>
bool FallbackRAMBackend<Number_T, Wire_T>::check()
{
  return true; // errors are reported through this->wireBackend->assertZero()
}

template<typename Number_T, typename Wire_T>
void FallbackRAMInitOperation<Number_T, Wire_T>::init(wtk::wire_idx const size,
    FallbackRAMBuffer<Wire_T>* const buffer, Wire_T const* const fill)
{
  buffer->size = (size_t) size;
  buffer->buffer = (Wire_T*) malloc(sizeof(Wire_T) * buffer->size);

  for(size_t i = 0; i < buffer->size; i++)
  {
    new(buffer->buffer + i) Wire_T();
    this->ramBackend()->wireBackend->copy(buffer->buffer + i, fill);
  }
}

template<typename Number_T, typename Wire_T>
void FLTRAMReadOperation<Number_T, Wire_T>::read(Wire_T* const out,
    FallbackRAMBuffer<Wire_T>* const buffer, Wire_T const* const idx)
{
  wtk::TypeBackend<Number_T, Wire_T>* const backend =
    this->ramBackend()->wireBackend;

  Wire_T eq;
  Wire_T tmp;
  Wire_T sum[2] = { };
  Wire_T eq_sum[2] = { };

  FLT_equality_const(backend, eq_sum + 1, idx, 0);
  backend->mulGate(sum + 0, eq_sum + 1, buffer->buffer + 0);
  backend->addcGate(eq_sum + 0, eq_sum + 1, backend->type->prime - 1);
  size_t place = 0;

  for(size_t i = 1; i < buffer->size - 1; i++)
  {
    size_t const alt_place = (place + 1) & 1;

    FLT_equality_const(backend, &eq, idx, i);
    backend->mulGate(&tmp, &eq, buffer->buffer + i);
    backend->addGate(sum + alt_place, sum + place, &tmp);
    backend->addGate(eq_sum + alt_place, eq_sum + place, &eq);

    place = alt_place;
  }

  size_t const alt_place = (place + 1) & 1;
  FLT_equality_const(backend, &eq, idx, buffer->size - 1);
  backend->mulGate(&tmp, &eq, buffer->buffer + buffer->size - 1);
  backend->addGate(out, sum + place, &tmp);
  backend->addGate(eq_sum + alt_place, eq_sum + place, &eq);
  backend->assertZero(eq_sum + alt_place);
}

template<typename Number_T, typename Wire_T>
void FLTRAMWriteOperation<Number_T, Wire_T>::write(
    FallbackRAMBuffer<Wire_T>* const buffer,
    Wire_T const* const idx, Wire_T const* const in)
{
  wtk::TypeBackend<Number_T, Wire_T>* const backend =
    this->ramBackend()->wireBackend;

  Wire_T eq;
  Wire_T ne;
  Wire_T tmp[2] = { };
  Wire_T eq_sum[2] = { };
  size_t place = 0;

  backend->assign(eq_sum + place, backend->type->prime - 1);

  for(size_t i = 0; i < buffer->size; i++)
  {
    FLT_inequality_const(backend, &eq, &ne, idx, i);
    backend->mulGate(tmp + 0, &eq, in);
    backend->mulGate(tmp + 1, &ne, buffer->buffer + i);
    backend->addGate(buffer->buffer + i, tmp + 0, tmp + 1);
    size_t const alt_place = (place + 1) & 1;
    backend->addGate(eq_sum + alt_place, eq_sum + place, &eq);
    place = alt_place;
  }

  backend->assertZero(eq_sum + place);
}

template<typename Number_T, typename Wire_T>
RAMInitOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>*
FallbackRAMPlugin<Number_T, Wire_T>::buildInitOperation(
    wtk::type_idx const type, RAMBackend<
      Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
{
  if(be->wireBackend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "ArithmeticRAM plugin.");
    return nullptr;
  }

  return new FallbackRAMInitOperation<Number_T, Wire_T>(type, be);
}

template<typename Number_T, typename Wire_T>
RAMReadOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>*
FallbackRAMPlugin<Number_T, Wire_T>::buildReadOperation(
    wtk::type_idx const type, RAMBackend<
      Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
{
  if(be->wireBackend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "ArithmeticRAM plugin.");
    return nullptr;
  }

  return new FLTRAMReadOperation<Number_T, Wire_T>(type, be);
}

template<typename Number_T, typename Wire_T>
RAMWriteOperation<Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>*
FallbackRAMPlugin<Number_T, Wire_T>::buildWriteOperation(
    wtk::type_idx const type, RAMBackend<
      Number_T, FallbackRAMBuffer<Wire_T>, Wire_T>* const be)
{
  if(be->wireBackend->type->variety != wtk::circuit::TypeSpec<Number_T>::field)
  {
    log_error("Only prime field types are currently compatible with the "
        "ArithmeticRAM plugin.");
    return nullptr;
  }

  return new FLTRAMWriteOperation<Number_T, Wire_T>(type, be);
}

} } // namespace wtk::plugins
