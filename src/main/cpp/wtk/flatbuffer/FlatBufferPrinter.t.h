/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

template<typename Number_T>
FlatBufferPrinter<Number_T>::FlatBufferPrinter(FILE* out) : outFile(out) { }

template<typename Number_T>
void FlatBufferPrinter<Number_T>::writeBuffer()
{
  size_t n_written = fwrite(this->builder.GetBufferPointer(), sizeof(uint8_t),
      this->builder.GetSize(), this->outFile);
  while(!ferror(this->outFile) && n_written < this->builder.GetSize())
  {
    n_written += fwrite(this->builder.GetBufferPointer() + n_written,
        sizeof(uint8_t), this->builder.GetSize() - n_written, this->outFile);
  }

  if(ferror(this->outFile))
  {
    log_perror();
    log_fatal("could not write flatbuffer");
  }
}

template<typename Number_T>
void FlatBufferPrinter<Number_T>::writeRelation(
    std::vector<flatbuffers::Offset<Directive>>* body,
    std::vector<flatbuffers::Offset<Function>>* functions)
{
  this->builder.FinishSizePrefixed(
      CreateRoot(this->builder, Message_Relation,
        CreateRelation(this->builder,
          CreateHeader(this->builder,
            this->builder.CreateString(this->version),
            CreateValue(this->builder,
              flattenNumber<Number_T>(&this->builder, this->characteristic)),
            this->degree),
          this->builder.CreateString(this->gateSet),
          this->builder.CreateString(this->features),
          this->builder.CreateVector(*functions),
          this->builder.CreateVector(*body)).Union()), RootIdentifier());

  this->writeBuffer();
  this->builder.Reset();
}

template<typename Number_T>
void FlatBufferPrinter<Number_T>::writeInstance(
    std::vector<flatbuffers::Offset<Value>>* values)
{
  this->builder.FinishSizePrefixed(
      CreateRoot(this->builder, Message_Instance,
        CreateInstance(this->builder,
          CreateHeader(this->builder,
            this->builder.CreateString(this->version),
            CreateValue(this->builder,
              flattenNumber<Number_T>(&this->builder, this->characteristic)),
            this->degree),
          this->builder.CreateVector(*values)).Union()), RootIdentifier());

  this->writeBuffer();
  this->builder.Reset();
}

template<typename Number_T>
void FlatBufferPrinter<Number_T>::writeWitness(
    std::vector<flatbuffers::Offset<Value>>* values)
{
  this->builder.FinishSizePrefixed(
      CreateRoot(this->builder, Message_Witness,
        CreateWitness(this->builder,
          CreateHeader(this->builder,
            this->builder.CreateString(this->version),
            CreateValue(this->builder,
              flattenNumber<Number_T>(&this->builder, this->characteristic)),
            this->degree),
          this->builder.CreateVector(*values)).Union()), RootIdentifier());

  this->writeBuffer();
  this->builder.Reset();
}

template <typename Number_T>
void FlatBufferPrinter<Number_T>::setVersion(
    size_t const major, size_t const minor, size_t const patch)
{
  this->version = wtk::utils::dec(major) + "."
    + wtk::utils::dec(minor) + "."
    + wtk::utils::dec(patch);
}

template <typename Number_T>
void FlatBufferPrinter<Number_T>::setField(
    Number_T const characteristic, size_t const degree)
{
  this->characteristic = characteristic;
  this->degree = degree;
  // size of the characteristic when serialized in base 256
  this->characteristicLen = wtk::utils::hex(characteristic).size() / 2;
}

template<typename Number_T>
void FlatBufferPrinter<Number_T>::setGateSet(
    wtk::GateSet const* const gate_set)
{
  if(gate_set->cannonical())
  {
    if(gate_set->gateSet == wtk::GateSet::arithmetic)
    {
      this->gateSet = "arithmetic";
    }
    else if(gate_set->gateSet == wtk::GateSet::boolean)
    {
      this->gateSet = "boolean";
    }
    else
    {
      this->gateSet = "invalid";
    }
  }
  else
  {
    if(gate_set->gateSet == wtk::GateSet::arithmetic)
    {
      std::string comma;
      if(gate_set->enableAdd)
      {
        this->gateSet = "@add";
        comma = ",";
      }
      if(gate_set->enableMul)
      {
        this->gateSet += comma + "@mul";
        comma = ",";
      }
      if(gate_set->enableAddC)
      {
        this->gateSet += comma + "@addc";
        comma = ",";
      }
      if(gate_set->enableMulC)
      {
        this->gateSet += comma + "@mulc";
        comma = ",";
      }
    }
    else if(gate_set->gateSet == wtk::GateSet::boolean)
    {
      std::string comma;
      if(gate_set->enableAnd)
      {
        this->gateSet = "@add";
        comma = ",";
      }
      if(gate_set->enableXor)
      {
        this->gateSet += comma + "@xor";
        comma = ",";
      }
      if(gate_set->enableNot)
      {
        this->gateSet += comma + "@not";
        comma = ",";
      }
    }
    else
    {
      this->gateSet = "invalid";
    }
  }
}

template<typename Number_T>
void FlatBufferPrinter<Number_T>::setFeatureToggles(
    wtk::FeatureToggles const* const toggles)
{
  if(toggles->simple())
  {
    this->features = "simple";
  }
  else
  {
    std::string comma;
    if(toggles->functionToggle)
    {
      this->features = "@function";
      comma = ",";
    }
    if(toggles->forLoopToggle)
    {
      this->features += comma + "@for";
      comma = ",";
    }
    if(toggles->switchCaseToggle)
    {
      this->features += comma + "@switch";
      comma = ",";
    }
  }
}

} } // wtk::flatbuffer
