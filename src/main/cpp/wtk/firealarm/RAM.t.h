/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace firealarm {

template<typename Wire_T>
RAMBuffer<Wire_T>::~RAMBuffer()
{
  if(this->counter != nullptr)
  {
    this->counter->decrement(this->length);
  }

  if(this->buffer != nullptr)
  {
    for(size_t i = 0; i < this->length; i++)
    {
      (this->buffer + i)->~Wire_T();
    }

    free(this->buffer);
  }
}

template<typename Number_T, typename Wire_T>
bool RAMBackend<Number_T, Wire_T>::check()
{
  return !this->failure;
}

template<typename Number_T, typename Wire_T>
void RAMInitOperation<Number_T, Wire_T>::init(wire_idx const size,
    RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const fill)
{
  RAMBackend<Number_T, Wire_T>* const ram =
    static_cast<RAMBackend<Number_T, Wire_T>*>(this->backend);

  buffer->length = static_cast<size_t>(size);
  buffer->buffer = (Wire_T*) malloc(buffer->length * sizeof(Wire_T));
  buffer->counter = ram->counter;
  ram->counter->init++;
  ram->counter->increment(buffer->length);

  for(size_t i = 0; i < buffer->length; i++)
  {
    new(buffer->buffer + i) Wire_T(fill->value);
  }
}

template<typename Number_T, typename Wire_T>
void RAMReadOperation<Number_T, Wire_T>::read(Wire<Wire_T>* const out,
    RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const idx)
{
  RAMBackend<Number_T, Wire_T>* const ram =
    static_cast<RAMBackend<Number_T, Wire_T>*>(this->backend);
  FieldBackend<Number_T, Wire_T>* const backend =
    static_cast<FieldBackend<Number_T, Wire_T>*>(ram->wireBackend);

  size_t idx_sz = static_cast<size_t>(idx->value);
  if(idx_sz >= buffer->length)
  {
    log_error("Index %zu exceeds RAM buffer size %zu", idx_sz, buffer->length);
    ram->failure = true;
    out->value = 0;
  }
  else
  {
    out->value = buffer->buffer[idx_sz];
  }

  out->counter = backend->counter;
  backend->counter->increment();
  backend->counter->read++;
  ram->counter->read++;
}

template<typename Number_T, typename Wire_T>
void RAMWriteOperation<Number_T, Wire_T>::write(RAMBuffer<Wire_T>* const buffer,
    Wire<Wire_T> const* const idx, Wire<Wire_T> const* const in)
{
  RAMBackend<Number_T, Wire_T>* const ram =
    static_cast<RAMBackend<Number_T, Wire_T>*>(this->backend);

  size_t idx_sz = static_cast<size_t>(idx->value);
  if(idx_sz >= buffer->length)
  {
    log_error("Index %zu exceeds RAM buffer size %zu", idx_sz, buffer->length);
    ram->failure = true;
  }
  else
  {
    buffer->buffer[idx_sz] = in->value;
  }

  ram->counter->write++;
}

template<typename Number_T, typename Wire_T>
RAMInitOperation<Number_T, Wire_T>*
RAMPlugin<Number_T, Wire_T>::buildInitOperation(
    type_idx const type, wtk::plugins::RAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new RAMInitOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
RAMReadOperation<Number_T, Wire_T>*
RAMPlugin<Number_T, Wire_T>::buildReadOperation(
    type_idx const type, wtk::plugins::RAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new RAMReadOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
RAMWriteOperation<Number_T, Wire_T>*
RAMPlugin<Number_T, Wire_T>::buildWriteOperation(
    type_idx const type, wtk::plugins::RAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new RAMWriteOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
bool BoolRAMBackend<Number_T, Wire_T>::check()
{
  return !this->failure;
}

template<typename Number_T, typename Wire_T>
void BoolRAMInitOperation<Number_T, Wire_T>::init(wire_idx const size,
    RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const fill)
{
  BoolRAMBackend<Number_T, Wire_T>* const ram =
    static_cast<BoolRAMBackend<Number_T, Wire_T>*>(this->backend);

  buffer->length = static_cast<size_t>(size) * ram->elementBits;
  buffer->buffer = (Wire_T*) malloc(buffer->length * sizeof(Wire_T));
  buffer->counter = ram->counter;
  ram->counter->init++;
  ram->counter->increment(buffer->length);

  for(size_t i = 0; i < (size_t) size; i++)
  {
    for(size_t j = 0; j < (size_t) ram->elementBits; j++)
    {
      new(buffer->buffer + i * ram->elementBits + j) Wire_T((fill + j)->value);
    }
  }
}

template<typename Number_T, typename Wire_T>
void BoolRAMReadOperation<Number_T, Wire_T>::read(Wire<Wire_T>* const out,
    RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const idx)
{
  BoolRAMBackend<Number_T, Wire_T>* const ram =
    static_cast<BoolRAMBackend<Number_T, Wire_T>*>(this->backend);
  FieldBackend<Number_T, Wire_T>* const backend =
    static_cast<FieldBackend<Number_T, Wire_T>*>(ram->wireBackend);

  size_t idx_sz = 0;
  for(size_t i = 0; i < (size_t) ram->indexBits; i++)
  {
    idx_sz = idx_sz << 1 | (static_cast<size_t>((idx + i)->value) & 1);
  }

  if(idx_sz * ram->elementBits >= buffer->length)
  {
    log_error("Index %zu exceeds RAM buffer size %zu",
        idx_sz, size_t(buffer->length / ram->elementBits));
    ram->failure = true;
    for(size_t i = 0; i < (size_t) ram->elementBits; i++)
    {
      out[i].value = 0;
    }
  }
  else
  {
    for(size_t i = 0; i < (size_t) ram->elementBits; i++)
    {
      out[i].value = buffer->buffer[idx_sz * ram->elementBits + i];
    }
  }

  for(size_t i = 0; i < (size_t) ram->elementBits; i++)
  {
    out[i].counter = backend->counter;
    backend->counter->increment();
    backend->counter->read++;
    ram->counter->read++;
  }
}

template<typename Number_T, typename Wire_T>
void BoolRAMWriteOperation<Number_T, Wire_T>::write(
    RAMBuffer<Wire_T>* const buffer, Wire<Wire_T> const* const idx,
    Wire<Wire_T> const* const in)
{
  BoolRAMBackend<Number_T, Wire_T>* const ram =
    static_cast<BoolRAMBackend<Number_T, Wire_T>*>(this->backend);

  size_t idx_sz = 0;
  for(size_t i = 0; i < (size_t) ram->indexBits; i++)
  {
    idx_sz = idx_sz << 1 | (static_cast<size_t>((idx + i)->value) & 1);
  }

  if(idx_sz * ram->elementBits >= buffer->length)
  {
    log_error("Index %zu exceeds RAM buffer size %zu",
        idx_sz, size_t(buffer->length / ram->elementBits));
    ram->failure = true;
  }
  else
  {
    for(size_t i = 0; i < (size_t) ram->elementBits; i++)
    {
      buffer->buffer[idx_sz * ram->elementBits + i] = in[i].value;
    }
  }

  ram->counter->write++;
}

template<typename Number_T, typename Wire_T>
BoolRAMInitOperation<Number_T, Wire_T>*
BoolRAMPlugin<Number_T, Wire_T>::buildInitOperation(
    type_idx const type, wtk::plugins::BoolRAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new BoolRAMInitOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
BoolRAMReadOperation<Number_T, Wire_T>*
BoolRAMPlugin<Number_T, Wire_T>::buildReadOperation(
    type_idx const type, wtk::plugins::BoolRAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new BoolRAMReadOperation<Number_T, Wire_T>(type, backend);
}

template<typename Number_T, typename Wire_T>
BoolRAMWriteOperation<Number_T, Wire_T>*
BoolRAMPlugin<Number_T, Wire_T>::buildWriteOperation(
    type_idx const type, wtk::plugins::BoolRAMBackend<
    Number_T, RAMBuffer<Wire_T>, Wire<Wire_T>>* const backend)
{
  return new BoolRAMWriteOperation<Number_T, Wire_T>(type, backend);
}

} } // namespace wtk::firealarm
