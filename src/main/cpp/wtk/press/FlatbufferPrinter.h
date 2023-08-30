/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_PRESS_PRINTER_H_
#define WTK_FLATBUFFER_PRESS_PRINTER_H_

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include <string>
#include <vector>

#include <wtk/indexes.h>

#include <wtk/Parser.h>
#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/utils/ParserOrganizer.h>

#include <wtk/press/Printer.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>

#define LOG_IDENTIFIER "wtk::press"
#include <stealth_logging.h>

namespace wtk {
namespace press {

using namespace wtk_gen_flatbuffer;

// Made up numbers that control when to write the current flatbuffer and
// make another one (due to 2gb size limit).
constexpr size_t SIMPLE_REWRITE_THRESHOLD =
  FLATBUFFERS_MAX_BUFFER_SIZE - (1 << 16);
constexpr size_t FUNCTION_REWRITE_THRESHOLD = FLATBUFFERS_MAX_BUFFER_SIZE / 2;

template<typename Number_T>
class FlatbufferPrinter : public Printer<Number_T>
{
  size_t major;
  size_t minor;
  size_t patch;
  std::string extra;

  wtk::ResourceType resource;

  FILE* file = nullptr;

  flatbuffers::FlatBufferBuilder builder;

  bool writeBuffer()
  {
    size_t n_written = 0;
    do
    {
      n_written = fwrite(this->builder.GetBufferPointer() + n_written,
          sizeof(uint8_t), this->builder.GetSize(), this->file);
    } while(!ferror(this->file) && n_written < this->builder.GetSize());

    if(ferror(this->file))
    {
      log_perror();
      log_error("could not write to flatbuffer");
      return false;
    }

    return true;
  }

  bool writeCircuit()
  {
    std::string versionStr = wtk::utils::stringify_version(
        this->major, this->minor, this->patch, this->extra.c_str());

    this->builder.FinishSizePrefixed(
        CreateRoot(this->builder, Message_Relation,
          CreateRelation(this->builder,
            this->builder.CreateString(versionStr),
            this->builder.CreateVector(this->plugins),
            this->builder.CreateVector(this->types),
            this->builder.CreateVectorOfStructs(this->conversions),
            this->builder.CreateVector(this->directives)).Union()),
        RootIdentifier());

    bool ret = this->writeBuffer();
    this->builder.Reset();
    this->plugins.clear();
    this->types.clear();
    this->conversions.clear();
    this->directives.clear();
    return ret;
  }

  bool writePublicStream()
  {
    std::string versionStr = wtk::utils::stringify_version(
        this->major, this->minor, this->patch, this->extra.c_str());

    this->builder.FinishSizePrefixed(
        CreateRoot(this->builder, Message_PublicInputs,
          CreatePublicInputs(this->builder,
            this->builder.CreateString(versionStr),
            (this->types.size() == 0)
              ? flatbuffers::Offset<Type>(0)
              : this->types[0],
            this->builder.CreateVector(this->streamVals)).Union()),
        RootIdentifier());

    bool ret = this->writeBuffer();
    this->builder.Reset();
    this->types.clear();
    this->streamVals.clear();
    return ret;
  }

  bool writePrivateStream()
  {
    std::string versionStr = wtk::utils::stringify_version(
        this->major, this->minor, this->patch, this->extra.c_str());

    this->builder.FinishSizePrefixed(
        CreateRoot(this->builder, Message_PrivateInputs,
          CreatePrivateInputs(this->builder,
            this->builder.CreateString(versionStr),
            (this->types.size() == 0)
              ? flatbuffers::Offset<Type>(0)
              : this->types[0],
            this->builder.CreateVector(this->streamVals)).Union()),
        RootIdentifier());

    bool ret = this->writeBuffer();
    this->builder.Reset();
    this->types.clear();
    this->streamVals.clear();
    return ret;
  }

  flatbuffers::Offset<flatbuffers::Vector<uint8_t>> flattenNumber(Number_T n)
  {
    std::vector<uint8_t> buffer;

    do
    {
      buffer.push_back((uint8_t) wtk::utils::cast_size(n & 0xff));
      n = n >> 8;
    } while(n != 0);

    return this->builder.CreateVector(buffer);
  }

public:

  bool open(FILE* f)
  {
    this->file = f;
    return true;
  }

  bool printHeader(
      size_t const major, size_t const minor, size_t const patch,
      char const* const extra,
      wtk::ResourceType const resource) override
  {
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->extra = extra;

    this->resource = resource;

    return true;
  }

private:

  std::vector<flatbuffers::Offset<flatbuffers::String>> plugins;
  std::vector<flatbuffers::Offset<Type>> types;
  std::vector<Conversion> conversions;

public:

  bool printPluginDecl(char const* const plugin_name) override
  {
    this->plugins.emplace_back(
        this->builder.CreateString(plugin_name));
    return true;
  }

  bool printFieldType(Number_T const prime) override
  {
    this->types.push_back(CreateType(this->builder, TypeU_Field,
          CreateField(this->builder,
            CreateValue(this->builder, this->flattenNumber(prime))).Union()));
    return true;
  }

  bool printRingType(size_t const bit_width) override
  {
    this->types.push_back(CreateType(this->builder, TypeU_Ring,
          CreateRing(this->builder, bit_width).Union()));
    return true;
  }

  bool printPluginType(
      wtk::circuit::PluginBinding<Number_T> const* const plugin) override
  {
    std::vector<flatbuffers::Offset<flatbuffers::String>> params;
    for(size_t i = 0; i < plugin->parameters.size(); i++)
    {
      switch(plugin->parameters[i].form)
      {
      case wtk::circuit::PluginBinding<Number_T>::Parameter::numeric:
      {
        params.push_back(this->builder.CreateString(
              wtk::utils::short_str(plugin->parameters[i].number)));
        break;
      }
      case wtk::circuit::PluginBinding<Number_T>::Parameter::textual:
      {
        params.push_back(this->builder.CreateString(
              plugin->parameters[i].text));
        break;
      }
      }
    }

    this->types.push_back(CreateType(this->builder, TypeU_PluginType,
          CreatePluginType(this->builder,
            this->builder.CreateString(plugin->name),
            this->builder.CreateString(plugin->operation),
            this->builder.CreateVector(params)).Union()));
    return true;
  }

  bool printConversionSpec(
      wtk::circuit::ConversionSpec const* const conversion) override
  {
    this->conversions.emplace_back(
        Count(conversion->outType, conversion->outLength),
        Count(conversion->inType, conversion->inLength));
    return true;
  }

  bool printBeginKw() override { return true; }

  bool printEndKw() override
  {
    switch(this->resource)
    {
    case wtk::ResourceType::circuit:
    {
      return this->writeCircuit();
    }
    case wtk::ResourceType::public_in:
    {
      return this->writePublicStream();
    }
    case wtk::ResourceType::private_in:
    {
      return this->writePrivateStream();
    }
    default:
    {
      log_error("Resource type is not supported");
      return false;
    }
    }
  }

private:

  std::vector<flatbuffers::Offset<Value>> streamVals;

public:

  bool printStreamValue(Number_T const& val) override
  {
    if(this->resource != wtk::ResourceType::public_in
        && this->resource != wtk::ResourceType::private_in)
    {
      log_error("Resource is not a stream");
      return false;
    }

    if(this->builder.GetSize() + this->streamVals.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->resource == wtk::ResourceType::public_in
          && !this->writePublicStream())
      {
        return false;
      }
      else if(this->resource == wtk::ResourceType::private_in
          && !this->writePrivateStream())
      {
        return false;
      }
    }

    this->streamVals.push_back(
        CreateValue(this->builder, this->flattenNumber(val)));
    return true;
  }

private:

  std::vector<flatbuffers::Offset<Directive>> directives;

public:

  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateAdd,
            CreateGateAdd(
              this->builder, type, out, left, right).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateAdd,
              CreateGateAdd(
                this->builder, type, out, left, right).Union()).Union()));
    }

    return true;
  }

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateMul,
            CreateGateMul(
              this->builder, type, out, left, right).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateMul,
              CreateGateMul(
                this->builder, type, out, left, right).Union()).Union()));
    }

    return true;
  }

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateAddConstant,
            CreateGateAddConstant(this->builder, type, out, left,
              this->flattenNumber(right)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateAddConstant,
              CreateGateAddConstant(this->builder, type, out, left,
              this->flattenNumber(right)).Union()).Union()));
    }

    return true;
  }

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateMulConstant,
            CreateGateMulConstant(this->builder, type, out, left,
              this->flattenNumber(right)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateMulConstant,
              CreateGateMulConstant(this->builder, type, out, left,
              this->flattenNumber(right)).Union()).Union()));
    }

    return true;
  }

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    std::vector<WireRange> ins;
    ins.emplace_back(left, left);
    WireRange out_range(out, out);

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateCopy,
            CreateGateCopy(this->builder, type, &out_range,
              this->builder.CreateVectorOfStructs(ins)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateCopy,
              CreateGateCopy(this->builder, type, &out_range,
                this->builder.CreateVectorOfStructs(ins)).Union()).Union()));
    }

    return true;
  }

  bool copyMulti(wtk::circuit::CopyMulti* multi) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    std::vector<WireRange> ins;
    ins.reserve(multi->inputs.size());
    WireRange out_range(multi->outputs.first, multi->outputs.last);
    for(size_t i = 0; i < multi->inputs.size(); i++)
    {
      ins.emplace_back(multi->inputs[i].first, multi->inputs[i].last);
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateCopy,
            CreateGateCopy(this->builder, multi->type, &out_range,
              this->builder.CreateVectorOfStructs(ins)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateCopy,
              CreateGateCopy(this->builder, multi->type, &out_range,
                this->builder.CreateVectorOfStructs(ins)).Union()).Union()));
    }

    return true;
  }

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateConstant,
            CreateGateConstant(this->builder, type, out,
              this->flattenNumber(left)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateConstant,
              CreateGateConstant(this->builder, type, out,
              this->flattenNumber(left)).Union()).Union()));
    }

    return true;
  }

  bool assertZero(wire_idx const left, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateAssertZero,
            CreateGateAssertZero(this->builder, type, left).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateAssertZero,
              CreateGateAssertZero(this->builder, type, left).Union()).Union()));
    }

    return true;
  }

  bool publicIn(wire_idx const out, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    WireRange out_range(out, out);
    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GatePublic,
            CreateGatePublic(this->builder, type, &out_range).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GatePublic,
              CreateGatePublic(this->builder, type, &out_range).Union())
            .Union()));
    }

    return true;
  }

  bool publicInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    WireRange out_range(out->first, out->last);
    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GatePublic,
            CreateGatePublic(this->builder, type, &out_range).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GatePublic,
              CreateGatePublic(this->builder, type, &out_range).Union())
            .Union()));
    }

    return true;
  }

  bool privateIn(wire_idx const out, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    WireRange out_range(out, out);
    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GatePrivate,
            CreateGatePrivate(this->builder, type, &out_range).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GatePrivate,
              CreateGatePrivate(this->builder, type, &out_range).Union())
            .Union()));
    }

    return true;
  }

  bool privateInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    WireRange out_range(out->first, out->last);
    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GatePrivate,
            CreateGatePrivate(this->builder, type, &out_range).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GatePrivate,
              CreateGatePrivate(this->builder, type, &out_range).Union())
            .Union()));
    }

    return true;
  }

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateConvert,
            CreateGateConvert(this->builder,
              out_type, first_out, last_out,
              in_type, first_in, last_in).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateConvert,
              CreateGateConvert(this->builder,
                out_type, first_out, last_out,
                in_type, first_in, last_in, modulus).Union()).Union()));
    }

    return true;
  }

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateNew,
            CreateGateNew(this->builder, type, first, last).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateNew,
              CreateGateNew(
                this->builder, type, first, last).Union()).Union()));
    }

    return true;
  }

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
            CreateGate(this->builder, GateSet_GateDelete,
              CreateGateDelete(this->builder, type, first, last).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateDelete,
              CreateGateDelete(
                this->builder, type, first, last).Union()).Union()));
    }

    return true;
  }

private:

  bool inFunction = false;
  std::string functionName;
  std::vector<Count> functionOutputs;
  std::vector<Count> functionInputs;
  std::vector<flatbuffers::Offset<Gate>> functionBody;

public:

  bool startFunction(wtk::circuit::FunctionSignature&& signature) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    this->functionName = signature.name;

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > FUNCTION_REWRITE_THRESHOLD && !this->writeCircuit())
    {
      return false;
    }

    for(size_t i = 0; i < signature.outputs.size(); i++)
    {
      this->functionOutputs.emplace_back(
          signature.outputs[i].type, signature.outputs[i].length);
    }

    for(size_t i = 0; i < signature.inputs.size(); i++)
    {
      this->functionInputs.emplace_back(
          signature.inputs[i].type, signature.inputs[i].length);
    }

    return true;
  }

  bool regularFunction() override
  {
    this->inFunction = true;
    return true;
  }

  bool endFunction() override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    this->directives.push_back(
        CreateDirective(this->builder, DirectiveSet_Function,
          CreateFunction(this->builder,
            this->builder.CreateString(this->functionName),
            this->builder.CreateVectorOfStructs(this->functionOutputs),
            this->builder.CreateVectorOfStructs(this->functionInputs),
            FunctionBody_Gates,
            CreateGates(this->builder,
              this->builder.CreateVector(this->functionBody)
              ).Union()).Union()));

    this->inFunction = false;
    this->functionName.clear();
    this->functionOutputs.clear();
    this->functionInputs.clear();
    this->functionBody.clear();

    return true;
  }

  bool pluginFunction(wtk::circuit::PluginBinding<Number_T>&& binding) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    std::vector<flatbuffers::Offset<flatbuffers::String>> params;
    for(size_t i = 0; i < binding.parameters.size(); i++)
    {
      switch(binding.parameters[i].form)
      {
      case wtk::circuit::PluginBinding<Number_T>::Parameter::numeric:
      {
        params.push_back(this->builder.CreateString(
              wtk::utils::short_str(binding.parameters[i].number)));
        break;
      }
      case wtk::circuit::PluginBinding<Number_T>::Parameter::textual:
      {
        params.push_back(this->builder.CreateString(
              binding.parameters[i].text));
        break;
      }
      }
    }

    std::vector<Count> public_counts;
    for(size_t i = 0; i < binding.publicInputCount.size(); i++)
    {
      if(binding.publicInputCount[i] != 0)
      {
        public_counts.emplace_back(
            (wtk::type_idx) i, (wtk::wire_idx) binding.publicInputCount[i]);
      }
    }

    std::vector<Count> private_counts;
    for(size_t i = 0; i < binding.privateInputCount.size(); i++)
    {
      if(binding.privateInputCount[i] != 0)
      {
        public_counts.emplace_back(
            (wtk::type_idx) i, (wtk::wire_idx) binding.privateInputCount[i]);
      }
    }

    this->directives.push_back(
        CreateDirective(this->builder, DirectiveSet_Function,
          CreateFunction(this->builder,
            this->builder.CreateString(this->functionName),
            this->builder.CreateVectorOfStructs(this->functionOutputs),
            this->builder.CreateVectorOfStructs(this->functionInputs),
            FunctionBody_PluginBody, CreatePluginBody(this->builder,
              this->builder.CreateString(binding.name),
              this->builder.CreateString(binding.operation),
              this->builder.CreateVector(params),
              this->builder.CreateVectorOfStructs(public_counts),
              this->builder.CreateVectorOfStructs(private_counts)).Union()
            ).Union()));

    this->functionName.clear();
    this->functionOutputs.clear();
    this->functionInputs.clear();

    return true;
  }

  bool invoke(wtk::circuit::FunctionCall* const call) override
  {
    if(this->resource != wtk::ResourceType::circuit)
    {
      log_error("Resource is not a circuit");
      return false;
    }

    if(this->builder.GetSize() + this->directives.size() * sizeof(uint32_t)
        > SIMPLE_REWRITE_THRESHOLD)
    {
      if(this->inFunction)
      {
        log_error("Function is too big for flatbuffers due to 2gb limit");
        return false;
      }
      else if(!this->writeCircuit())
      {
        return false;
      }
    }

    std::vector<wtk_gen_flatbuffer::WireRange> outputs;
    std::vector<wtk_gen_flatbuffer::WireRange> inputs;

    for(size_t i = 0; i < call->outputs.size(); i++)
    {
      outputs.emplace_back(call->outputs[i].first, call->outputs[i].last);
    }

    for(size_t i = 0; i < call->inputs.size(); i++)
    {
      inputs.emplace_back(call->inputs[i].first, call->inputs[i].last);
    }

    if(this->inFunction)
    {
      this->functionBody.push_back(
          CreateGate(this->builder, GateSet_GateCall,
            CreateGateCall(this->builder,
              this->builder.CreateString(call->name),
              this->builder.CreateVectorOfStructs(outputs),
              this->builder.CreateVectorOfStructs(inputs)).Union()));
    }
    else
    {
      this->directives.push_back(
          CreateDirective(this->builder, DirectiveSet_Gate,
            CreateGate(this->builder, GateSet_GateCall,
              CreateGateCall(this->builder,
                this->builder.CreateString(call->name),
                this->builder.CreateVectorOfStructs(outputs),
                this->builder.CreateVectorOfStructs(inputs)).Union()).Union()));
    }

    return true;
  }
};

} } // namespace wtk::press

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_PRESS_PRINTER_H_
