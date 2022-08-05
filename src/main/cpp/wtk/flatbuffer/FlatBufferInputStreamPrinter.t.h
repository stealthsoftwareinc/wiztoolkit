/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
FlatBufferInputStreamPrinter<Number_T>::FlatBufferInputStreamPrinter(
    FILE* ofile) : FlatBufferPrinter<Number_T>(ofile) { }

template<typename Number_T>
bool FlatBufferInputStreamPrinter<Number_T>::printInstance(
    wtk::InputStream<Number_T>* stream)
{
  this->isWitness = false;
  return this->printStream(stream);
}

template<typename Number_T>
bool FlatBufferInputStreamPrinter<Number_T>::printShortWitness(
    wtk::InputStream<Number_T>* stream)
{
  this->isWitness = true;
  return this->printStream(stream);
}

template<typename Number_T>
bool FlatBufferInputStreamPrinter<Number_T>::printStream(
    wtk::InputStream<Number_T>* stream)
{
  if(stream == nullptr) { return false; }

  Number_T num(0);
  wtk::StreamStatus status = stream->next(&num);
  while(status == wtk::StreamStatus::success)
  {
    this->values.emplace_back(CreateValue(this->builder,
          flattenNumber(&this->builder, num)));
    this->checkFlush();
    status = stream->next(&num);
  }

  this->flush();
  return status == wtk::StreamStatus::end;
}

template<typename Number_T>
void FlatBufferInputStreamPrinter<Number_T>::checkFlush()
{
  if(this->builder.GetSize() + this->values.size() * sizeof(uint32_t)
      > UINT32_MAX - 256)
  {
    this->flush();
  }
}

template<typename Number_T>
void FlatBufferInputStreamPrinter<Number_T>::flush()
{
  if(this->isWitness)
  {
    this->writeWitness(&this->values);
  }
  else
  {
    this->writeInstance(&this->values);
  }
}

} } // namespace wtk::flatbuffer
