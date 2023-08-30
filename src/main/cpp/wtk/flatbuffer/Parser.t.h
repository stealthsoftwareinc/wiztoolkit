/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

using namespace irregular;

template<typename Number_T>
bool Parser<Number_T>::open(char const* const fname)
{
  this->ctx.fileName = fname;
  if(this->fileDescriptor != -1)
  {
    log_error("cannot open multiple flatbuffers");
    return false;
  }

  this->fileDescriptor = ::open(fname, O_RDONLY);
  if(this->fileDescriptor == -1)
  {
    log_perror();
    log_error("Could not open flatbuffer %s", fname);
    return false;
  }

  return this->openHelper();
}

template<typename Number_T>
bool Parser<Number_T>::open(FILE* file, char const* const fname)
{
  this->ctx.fileName = fname;
  this->file = file;
  if(this->fileDescriptor != -1)
  {
    log_error("cannot open multiple flatbuffers");
    return false;
  }

  this->fileDescriptor = fileno(file);
  if(this->fileDescriptor == -1)
  {
    log_perror();
    log_error("flatbuffer %s is not open", fname);
    return false;
  }

  return this->openHelper();
}

template<typename Number_T>
bool Parser<Number_T>::openHelper()
{
  off_t len = lseek(this->fileDescriptor, 0, SEEK_END);
  if(len == -1)
  {
    log_perror();
    log_error("Could not open flatbuffer %s", this->ctx.fileName);
    return false;
  }
  else
  {
    this->fileSize = (size_t) len;
  }

  this->buffer = (uint8_t*) mmap(nullptr, this->fileSize, PROT_READ,
      MAP_SHARED, this->fileDescriptor, 0);
  if(this->buffer == MAP_FAILED)
  {
    log_perror();
    log_error("Could not open flatbuffer %s", this->ctx.fileName);
    return false;
  }

  // technically this is the shortest flatbuffer, 4-byte 'siev' constant,
  // followed by size = 0...
  if(this->fileSize < 8)
  {
    log_error("flatbuffer file \"%s\" is not large enough to be valid",
        this->ctx.fileName);
    return false;
  }

  size_t place = 0;
  size_t segment = 0;
  while(place + 4 < this->fileSize)
  {
    uint32_t new_size = flatbuffers::GetPrefixedSize(this->buffer + place);

    if(new_size + place + 4 > this->fileSize)
    {
      log_error("flatbuffer file \"%s\" has invalid size constant",
          this->ctx.fileName);
      return false;
    }

    flatbuffers::Verifier fb_verifier(&this->buffer[place],
        new_size + sizeof(new_size), FLATBUFFERS_MAX_BUFFER_SIZE,
        FLATBUFFERS_MAX_BUFFER_SIZE);
    if(!fb_verifier.VerifySizePrefixedBuffer<Root>(nullptr))
    {
      log_error(
          "flatbuffer file %s has invalid internal structure in segment %zu",
          this->ctx.fileName, segment);
      return false;
    }

    Root const* new_root = flatbuffers::GetSizePrefixedRoot<Root>(
        (void*) (this->buffer + place));
    if(new_root == nullptr)
    {
      log_error("flatbuffer error \"%s\"", this->ctx.fileName);
      return false;
    }

    this->ctx.sizes.push_back(new_size);
    this->ctx.roots.push_back(new_root);
    place = place + sizeof(new_size) + new_size;
    segment++;
  }

  return true;
};

template<typename Number_T>
Parser<Number_T>::Parser(Parser&& move)
  : wtk::Parser<Number_T>(std::move(move)),
    file(move.file), fileDescriptor(move.fileDescriptor),
    fileSize(move.fileSize), buffer(move.buffer), ctx(std::move(move.ctx)),
    translationParser(std::move(move.translationParser)),
    circuitParser(std::move(move.circuitParser)),
    publicInputStream(std::move(move.publicInputStream)),
    privateInputStream(std::move(move.privateInputStream)),
    configurationParser(std::move(move.configurationParser))
{
  move.file = nullptr;
  move.fileDescriptor = -1;
  move.fileSize = 0;
  move.buffer = nullptr;
}

template<typename Number_T>
Parser<Number_T>::~Parser()
{
  munmap(this->buffer, this->fileSize);
  if(this->file != nullptr)
  {
    fclose(this->file);
  }
  else
  {
    close(this->fileDescriptor);
  }
}

#define NONULL(ptr, ret) do { \
  if(UNLIKELY(ptr == nullptr)) { \
    log_error("flatbuffer encountered a null pointer"); \
    return ret; \
  } \
} while(0)

template<typename Number_T>
bool Parser<Number_T>::parseHeader()
{
  log_assert(this->ctx.roots.size() == this->ctx.sizes.size());

  if(this->ctx.roots.size() == 0)
  {
    log_error("%s: empty flatbuffer", this->ctx.fileName);
  }

  Message msg_type = Message_NONE;
  char const* version_string = nullptr;

  for(size_t i = 0; i < this->ctx.roots.size(); i++)
  {
    NONULL(this->ctx.roots[0], false);

    switch(this->ctx.roots[0]->message_type())
    {
    case Message_Relation:
    {
      Relation const* const relation =
        this->ctx.roots[0]->message_as_Relation();
      NONULL(relation, false);
      NONULL(relation->version(), false);

      if(i == 0)
      {
        msg_type = Message_Relation;
        this->type = wtk::ResourceType::circuit;
        version_string = relation->version()->c_str();

        if(version_string == nullptr)
        {
          log_error("%s: First relation segment must have a version string",
              this->ctx.fileName);
          return false;
        }

        if(relation->types()->size() == 0)
        {
          log_error("%s: First relation segment must declare some types",
              this->ctx.fileName);
          return false;
        }
      }
      else
      {
        if(msg_type != Message_Relation)
        {
          log_error("%s: Message types do not match", this->ctx.fileName);
          return false;
        }

        if(relation->version()->c_str() == nullptr)
        {
          log_warn("%s: Non-first version-string is null", this->ctx.fileName);
        }

        if(0 != strcmp(version_string, relation->version()->c_str()))
        {
          log_warn("%s: Version strings do not match", this->ctx.fileName);
        }

        NONULL(relation->plugins(), false);
        NONULL(relation->types(), false);
        NONULL(relation->conversions(), false);
        if(relation->plugins()->size() != 0
            || relation->types()->size() != 0
            || relation->conversions()->size() != 0)
        {
          log_warn("%s: Plugin, type, and conversion specifications from "
              "Non-first relation segments will be ignored.",
              this->ctx.fileName);
        }
      }

      break;
    }
    case Message_PublicInputs:
    {
      PublicInputs const* const pub_inps =
        this->ctx.roots[0]->message_as_PublicInputs();
      NONULL(pub_inps, false);
      NONULL(pub_inps->version(), false);

      if(i == 0)
      {
        msg_type = Message_PublicInputs;
        this->type = wtk::ResourceType::public_in;
        version_string = pub_inps->version()->c_str();

        if(version_string == nullptr)
        {
          log_error("%s: First segment must have a version string",
              this->ctx.fileName);
          return false;
        }

        if(pub_inps->type() == nullptr)
        {
          log_error("%s: First segment must declare a type",
              this->ctx.fileName);
          return false;
        }
      }
      else
      {
        if(msg_type != Message_PublicInputs)
        {
          log_error("%s: Message types do not match", this->ctx.fileName);
          return false;
        }

        if(pub_inps->version()->c_str() == nullptr)
        {
          log_warn("%s: Non-first version-string is null", this->ctx.fileName);
        }

        if(0 != strcmp(version_string, pub_inps->version()->c_str()))
        {
          log_warn("%s: Version strings do not match", this->ctx.fileName);
        }

        if(pub_inps->type() != nullptr)
        {
          log_warn("%s: Type specification from a non-first relation segment "
              "will be ignored.", this->ctx.fileName);
        }
      }
      break;
    }
    case Message_PrivateInputs:
    {
      PrivateInputs const* const prv_inps =
        this->ctx.roots[0]->message_as_PrivateInputs();
      NONULL(prv_inps, false);
      NONULL(prv_inps->version(), false);

      if(i == 0)
      {
        msg_type = Message_PrivateInputs;
        this->type = wtk::ResourceType::private_in;
        version_string = prv_inps->version()->c_str();

        if(version_string == nullptr)
        {
          log_error("%s: First segment must have a version string",
              this->ctx.fileName);
          return false;
        }

        if(prv_inps->type() == nullptr)
        {
          log_error("%s: First segment must declare a some type",
              this->ctx.fileName);
          return false;
        }
      }
      else
      {
        if(msg_type != Message_PrivateInputs)
        {
          log_error("%s: Message types do not match", this->ctx.fileName);
          return false;
        }

        if(prv_inps->version()->c_str() == nullptr)
        {
          log_warn("%s: Non-first version-string is null", this->ctx.fileName);
        }

        if(0 != strcmp(version_string, prv_inps->version()->c_str()))
        {
          log_warn("%s: Version strings do not match", this->ctx.fileName);
        }

        if(prv_inps->type() != nullptr)
        {
          log_warn("%s: Type specification from a non-first relation segment "
              "will be ignored.", this->ctx.fileName);
        }
      }
      break;
    }
    default:
    {
      log_error("%s: unrecognized message type", this->ctx.fileName);
      return false;
    }
    }
  }

  CharStarAutomataCtx version_ctx(version_string);

  if(!number(&version_ctx, &this->version.major)
      || !dotOp(&version_ctx)
      || !number(&version_ctx, &this->version.minor)
      || !dotOp(&version_ctx))
  {
    return false;
  }

  NumberOptNterm num_opt_nterm =
    numberOptNterm(&version_ctx, &this->version.patch);
  if(num_opt_nterm == NumberOptNterm::invalid)
  {
    return false;
  }
  else if(num_opt_nterm == NumberOptNterm::opt)
  {
    if(!dashOp(&version_ctx)
        || !identifierThenNterm(&version_ctx, &this->version.extra))
    {
      return false;
    }
  }

  return true;
}

template<typename Number_T>
TranslationParser<Number_T>* Parser<Number_T>::translation()
{
  if(this->type != wtk::ResourceType::translation)
  {
    log_error("Not a translation");
    return nullptr;
  }

  if(this->translationParser == nullptr)
  {
    this->translationParser = std::unique_ptr<TranslationParser<Number_T>>(
        new TranslationParser<Number_T>());
  }

  return this->translationParser.get();
}

template<typename Number_T>
CircuitParser<Number_T>* Parser<Number_T>::circuit()
{
  if(this->type != wtk::ResourceType::circuit)
  {
    log_error("Not a circuit");
    return nullptr;
  }

  if(this->circuitParser == nullptr)
  {
    this->circuitParser = std::unique_ptr<CircuitParser<Number_T>>(
        new CircuitParser<Number_T>(&this->ctx));
  }

  return this->circuitParser.get();
}

template<typename Number_T>
wtk::InputStream<Number_T>* Parser<Number_T>::publicIn()
{
  if(this->type != wtk::ResourceType::public_in)
  {
    log_error("Not a public input stream");
    return nullptr;
  }

  if(this->publicInputStream == nullptr)
  {
    this->publicInputStream = std::unique_ptr<PublicInputStream<Number_T>>(
        new PublicInputStream<Number_T>(&this->ctx));
  }

  return this->publicInputStream.get();
}

template<typename Number_T>
wtk::InputStream<Number_T>* Parser<Number_T>::privateIn()
{
  if(this->type != wtk::ResourceType::private_in)
  {
    log_error("Not a private input stream");
    return nullptr;
  }

  if(this->privateInputStream == nullptr)
  {
    this->privateInputStream = std::unique_ptr<PrivateInputStream<Number_T>>(
        new PrivateInputStream<Number_T>(&this->ctx));
  }

  return this->privateInputStream.get();
}

template<typename Number_T>
ConfigurationParser<Number_T>* Parser<Number_T>::configuration()
{
  if(this->type != wtk::ResourceType::configuration)
  {
    log_error("Not a circuit configuration communication");
    return nullptr;
  }

  if(this->configurationParser == nullptr)
  {
    this->configurationParser = std::unique_ptr<ConfigurationParser<Number_T>>(
        new ConfigurationParser<Number_T>());
  }

  return this->configurationParser.get();
}

template<typename Number_T>
bool base256ToNumber(
    flatbuffers::Vector<uint8_t> const* const base256, Number_T* const out)
{
  NONULL(base256, false);

  if(UNLIKELY(base256->size() == 0))
  {
    log_error("base256 number must have at least one digit");
    return false;
  }

  *out = 0;
  for(flatbuffers::uoffset_t i = base256->size(); i != 0; i--)
  {
    *out = Number_T(Number_T(*out << 8) | Number_T(base256->Get(i - 1)));
  }

  return true;
}

template<typename Number_T>
bool CircuitParser<Number_T>::parseCircuitHeader()
{
  NONULL(this->ctx->roots[0], false);
  Relation const* const header = this->ctx->roots[0]->message_as_Relation();
  NONULL(header, false);

  NONULL(header->plugins(), false);
  for(flatbuffers::uoffset_t i = 0; i < header->plugins()->size(); i++)
  {
    NONULL(header->plugins()->Get(i), false);
    NONULL(header->plugins()->Get(i)->c_str(), false);

    this->plugins.emplace_back();
    CharStarAutomataCtx plugin_ctx(header->plugins()->Get(i)->c_str());
    if(!identifierThenNterm(&plugin_ctx, &this->plugins.back()))
    {
      return false;
    }
  }

  NONULL(header->types(), false);
  for(flatbuffers::uoffset_t i = 0; i < header->types()->size(); i++)
  {
    NONULL(header->types()->Get(i), false);
    switch(header->types()->Get(i)->element_type())
    {
    case TypeU_NONE:
    {
      log_error("TypeU_NONE encountered in circuit's type list");
      return false;
    }
    case TypeU_Field:
    {
      Field const* const field = header->types()->Get(i)->element_as_Field();
      NONULL(field, false);
      NONULL(field->modulo(), false);
      NONULL(field->modulo()->value(), false);
      Number_T p = 0;
      if(!base256ToNumber(field->modulo()->value(), &p)) { return false; }
      this->types.emplace_back(std::move(p));
      break;
    }
    case TypeU_ExtField:
    {
      log_error("Extension Fields are currently unsupported");
      return false;
    }
    case TypeU_Ring:
    {
      Ring const* const ring = header->types()->Get(i)->element_as_Ring();
      NONULL(ring, false);
      this->types.emplace_back(ring->nbits());
      break;
    }
    case TypeU_PluginType:
    {
      wtk::circuit::PluginBinding<Number_T> binding;
      PluginType const* const plugin =
        header->types()->Get(i)->element_as_PluginType();
      NONULL(plugin, false);

      NONULL(plugin->name(), false);
      NONULL(plugin->name()->c_str(), false);
      CharStarAutomataCtx name_ctx(plugin->name()->c_str());
      if(!identifierThenNterm(&name_ctx, &binding.name)) { return false; }

      NONULL(plugin->operation(), false);
      NONULL(plugin->operation()->c_str(), false);
      CharStarAutomataCtx op_ctx(plugin->operation()->c_str());
      if(!identifierThenNterm(&op_ctx, &binding.operation)) { return false; }

      NONULL(plugin->params(), false);
      for(flatbuffers::uoffset_t j = 0; j < plugin->params()->size(); j++)
      {
        NONULL(plugin->params()->Get(j), false);
        NONULL(plugin->params()->Get(j)->c_str(), false);

        CharStarAutomataCtx param_ctx(plugin->params()->Get(j)->c_str());
        std::string idnt;
        Number_T num;
        switch(idOrNumThenNterm(&param_ctx, &idnt, &num))
        {
        case IdOrNumThenNterm::invalid:
        {
          return false;
        }
        case IdOrNumThenNterm::identifier:
        {
          binding.parameters.emplace_back(std::move(idnt));
          break;
        }
        case IdOrNumThenNterm::number:
        {
          binding.parameters.emplace_back(std::move(num));
          break;
        }
        }
      }

      this->types.emplace_back(std::move(binding));
      break;
    }
    }
  }

  NONULL(header->conversions(), false);
  for(flatbuffers::uoffset_t i = 0; i < header->conversions()->size(); i++)
  {
    Conversion const* const conversion = header->conversions()->Get(i);
    NONULL(conversion, false);

    this->conversions.emplace_back(
        conversion->output_count().type_id(),
        conversion->output_count().count(),
        conversion->input_count().type_id(),
        conversion->input_count().count());
  }

  return true;
}

template<typename Number_T>
bool CircuitParser<Number_T>::parseGate(
    Gate const* const gate, wtk::circuit::Handler<Number_T>* const handler)
{
  NONULL(gate, false);

  switch(gate->gate_type())
  {
  case GateSet_NONE:
  {
    log_error("invalid gate");
    return false;
  }
  case GateSet_GateConstant:
  {
    GateConstant const* const gate_const = gate->gate_as_GateConstant();
    NONULL(gate_const, false);

    wtk::type_idx const type = gate_const->type_id();
    wtk::wire_idx const out = gate_const->out_id();
    Number_T in_val;
    if(UNLIKELY(!base256ToNumber(gate_const->constant(), &in_val)))
    {
      return false;
    }

    if(UNLIKELY(!handler->assign(out, std::move(in_val), type)))
    {
      return false;
    }

    break;
  }
  case GateSet_GateAssertZero:
  {
    GateAssertZero const* const assert_zero = gate->gate_as_GateAssertZero();
    NONULL(assert_zero, false);

    if(UNLIKELY(!handler->assertZero(
            assert_zero->in_id(), assert_zero->type_id())))
    {
      return false;
    }

    break;
  }
  case GateSet_GateCopy:
  {
    GateCopy const* const copy = gate->gate_as_GateCopy();
    NONULL(copy, false);
    NONULL(copy->out_id(), false);
    NONULL(copy->in_id(), false);
    if(copy->in_id()->size() < 1)
    {
      log_error("Flatbuffer encountered an unexpected empty list");
      return false;
    }
    NONULL(copy->in_id()->Get(0), false);

    if(copy->out_id()->first_id() == copy->out_id()->last_id()
        && copy->in_id()->size() == 1
        && copy->in_id()->Get(0)->first_id()
            == copy->in_id()->Get(0)->last_id())
    {
      if(UNLIKELY(!handler->copy(
              copy->out_id()->first_id(), copy->in_id()->Get(0)->first_id(),
              copy->type_id())))
      {
        return false;
      }
    }
    else
    {
      wtk::circuit::CopyMulti c(
          copy->out_id()->first_id(), copy->out_id()->last_id(),
          copy->type_id());

      for(flatbuffers::uoffset_t i = 0; i < copy->in_id()->size(); i++)
      {
        c.inputs.emplace_back(
            copy->in_id()->Get(i)->first_id(),
            copy->in_id()->Get(i)->last_id());
      }

      if(UNLIKELY(!handler->copyMulti(&c)))
      {
        return false;
      }
    }

    break;
  }
  case GateSet_GateAdd:
  {
    GateAdd const* const add = gate->gate_as_GateAdd();
    NONULL(add, false);

    if(UNLIKELY(!handler->addGate(
            add->out_id(), add->left_id(), add->right_id(), add->type_id())))
    {
      return false;
    }

    break;
  }
  case GateSet_GateMul:
  {
    GateMul const* const mul = gate->gate_as_GateMul();
    NONULL(mul, false);

    if(UNLIKELY(!handler->mulGate(
            mul->out_id(), mul->left_id(), mul->right_id(), mul->type_id())))
    {
      return false;
    }

    break;
  }
  case GateSet_GateAddConstant:
  {
    GateAddConstant const* const gate_add_const =
      gate->gate_as_GateAddConstant();
    NONULL(gate_add_const, false);

    wtk::type_idx const type = gate_add_const->type_id();
    wtk::wire_idx const out = gate_add_const->out_id();
    wtk::wire_idx const in = gate_add_const->in_id();
    Number_T in_val;
    if(UNLIKELY(!base256ToNumber(gate_add_const->constant(), &in_val)))
    {
      return false;
    }

    if(UNLIKELY(!handler->addcGate(out, in, std::move(in_val), type)))
    {
      return false;
    }

    break;
  }
  case GateSet_GateMulConstant:
  {
    GateMulConstant const* const gate_mul_const =
      gate->gate_as_GateMulConstant();
    NONULL(gate_mul_const, false);

    wtk::type_idx const type = gate_mul_const->type_id();
    wtk::wire_idx const out = gate_mul_const->out_id();
    wtk::wire_idx const in = gate_mul_const->in_id();
    Number_T in_val;
    if(UNLIKELY(!base256ToNumber(gate_mul_const->constant(), &in_val)))
    {
      return false;
    }

    if(UNLIKELY(!handler->mulcGate(out, in, std::move(in_val), type)))
    {
      return false;
    }

    break;
  }
  case GateSet_GatePrivate:
  {
    GatePrivate const* const gate_private = gate->gate_as_GatePrivate();
    NONULL(gate_private, false);
    NONULL(gate_private->out_id(), false);

    if(gate_private->out_id()->first_id() == gate_private->out_id()->last_id())
    {
      if(UNLIKELY(!handler->privateIn(
              gate_private->out_id()->first_id(), gate_private->type_id())))
      {
        return false;
      }
    }
    else
    {
      wtk::circuit::Range r(
          gate_private->out_id()->first_id(),
          gate_private->out_id()->last_id());

      if(UNLIKELY(!handler->privateInMulti(&r, gate_private->type_id())))
      {
        return false;
      }
    }

    break;
  }
  case GateSet_GatePublic:
  {
    GatePublic const* const gate_public = gate->gate_as_GatePublic();
    NONULL(gate_public, false);
    NONULL(gate_public->out_id(), false);

    if(gate_public->out_id()->first_id() == gate_public->out_id()->last_id())
    {
      if(UNLIKELY(!handler->publicIn(
              gate_public->out_id()->first_id(), gate_public->type_id())))
      {
        return false;
      }
    }
    else
    {
      wtk::circuit::Range r(
          gate_public->out_id()->first_id(),
          gate_public->out_id()->last_id());

      if(UNLIKELY(!handler->publicInMulti(&r, gate_public->type_id())))
      {
        return false;
      }
    }

    break;
  }
  case GateSet_GateNew:
  {
    GateNew const* const gate_new = gate->gate_as_GateNew();
    NONULL(gate_new, false);

    if(UNLIKELY(!handler->newRange(
            gate_new->first_id(), gate_new->last_id(), gate_new->type_id())))
    {
      return false;
    }
    break;
  }
  case GateSet_GateDelete:
  {
    GateDelete const* const gate_delete = gate->gate_as_GateDelete();
    NONULL(gate_delete, false);

    if(UNLIKELY(!handler->deleteRange(gate_delete->first_id(),
            gate_delete->last_id(), gate_delete->type_id())))
    {
      return false;
    }
    break;
  }
  case GateSet_GateConvert:
  {
    GateConvert const* const gate_convert = gate->gate_as_GateConvert();
    NONULL(gate_convert, false);

    if(UNLIKELY(!handler->convert(
            gate_convert->out_first_id(), gate_convert->out_last_id(),
            gate_convert->out_type_id(),
            gate_convert->in_first_id(), gate_convert->in_last_id(),
            gate_convert->in_type_id(), gate_convert->modulus())))
    {
      return false;
    }

    break;
  }
  case GateSet_GateCall:
  {
    GateCall const* const gate_call = gate->gate_as_GateCall();
    NONULL(gate_call, false);

    wtk::circuit::FunctionCall call;

    NONULL(gate_call->name(), false);
    NONULL(gate_call->name()->c_str(), false);
    CharStarAutomataCtx name_ctx(gate_call->name()->c_str());
    if(UNLIKELY(!identifierThenNterm(&name_ctx, &call.name)))
    {
      return false;
    }

    NONULL(gate_call->out_ids(), false);
    for(flatbuffers::uoffset_t i = 0; i < gate_call->out_ids()->size(); i++)
    {
      WireRange const* const out_range = gate_call->out_ids()->Get(i);
      NONULL(out_range, false);

      call.outputs.emplace_back(out_range->first_id(), out_range->last_id());
    }

    NONULL(gate_call->in_ids(), false);
    for(flatbuffers::uoffset_t i = 0; i < gate_call->in_ids()->size(); i++)
    {
      WireRange const* const in_range = gate_call->in_ids()->Get(i);
      NONULL(in_range, false);

      call.inputs.emplace_back(in_range->first_id(), in_range->last_id());
    }

    if(UNLIKELY(!handler->invoke(&call))) { return false; }

    break;
  }
  }

  return true;
}

template<typename Number_T>
bool CircuitParser<Number_T>::parse(
    wtk::circuit::Handler<Number_T>* const handler)
{
  for(size_t i = 0; i < this->ctx->roots.size(); i++)
  {
    NONULL(this->ctx->roots[i], false);
    Relation const* const relation =
      this->ctx->roots[i]->message_as_Relation();

    NONULL(relation, false);
    NONULL(relation->directives(), false);

    for(flatbuffers::uoffset_t j = 0; j < relation->directives()->size(); j++)
    {
      Directive const* const directive = relation->directives()->Get(j);
      NONULL(directive, false);

      switch(directive->directive_type())
      {
      case DirectiveSet_Gate:
      {
        if(UNLIKELY(!this->parseGate(directive->directive_as_Gate(), handler)))
        {
          return false;
        }
        break;
      }
      case DirectiveSet_Function:
      {
        Function const* const function = directive->directive_as_Function();
        NONULL(function, false);

        wtk::circuit::FunctionSignature signature;


        NONULL(function->name(), false);
        NONULL(function->name()->c_str(), false);
        CharStarAutomataCtx name_ctx(function->name()->c_str());
        if(UNLIKELY(!identifierThenNterm(&name_ctx, &signature.name)))
        {
          return false;
        }


        NONULL(function->output_count(), false);
        for(flatbuffers::uoffset_t k = 0;
            k < function->output_count()->size(); k++)
        {
          Count const* const count = function->output_count()->Get(k);
          NONULL(count, false);

          signature.outputs.emplace_back(count->type_id(), count->count());
        }

        NONULL(function->input_count(), false);
        for(flatbuffers::uoffset_t k = 0;
            k < function->input_count()->size(); k++)
        {
          Count const* const count = function->input_count()->Get(k);
          NONULL(count, false);

          signature.inputs.emplace_back(count->type_id(), count->count());
        }

        if(!UNLIKELY(handler->startFunction(std::move(signature))))
        {
          return false;
        }

        switch(function->body_type())
        {
        case FunctionBody_NONE:
        {
          log_error("invalid function body");
          return false;
        }
        case FunctionBody_Gates:
        {
          if(UNLIKELY(!handler->regularFunction())) { return false; }

          Gates const* const gates = function->body_as_Gates();
          NONULL(gates, false);
          NONULL(gates->gates(), false);
          for(flatbuffers::uoffset_t k = 0; k < gates->gates()->size(); k++)
          {
            NONULL(gates->gates()->Get(k), false);
            if(UNLIKELY(!this->parseGate(gates->gates()->Get(k), handler)))
            {
              return false;
            }
          }

          if(UNLIKELY(!handler->endFunction())) { return false; }

          break;
        }
        case FunctionBody_PluginBody:
        {
          PluginBody const* const plugin_body = function->body_as_PluginBody();
          NONULL(plugin_body, false);

          wtk::circuit::PluginBinding<Number_T> binding;

          NONULL(plugin_body->name(), false);
          NONULL(plugin_body->name()->c_str(), false);
          CharStarAutomataCtx name_ctx(plugin_body->name()->c_str());
          if(UNLIKELY(!identifierThenNterm(&name_ctx, &binding.name)))
          {
            return false;
          }

          NONULL(plugin_body->operation(), false);
          NONULL(plugin_body->operation()->c_str(), false);
          CharStarAutomataCtx operation_ctx(plugin_body->operation()->c_str());
          if(UNLIKELY(!identifierThenNterm(
                  &operation_ctx, &binding.operation)))
          {
            return false;
          }

          NONULL(plugin_body->params(), false);
          for(flatbuffers::uoffset_t k = 0;
              k < plugin_body->params()->size(); k++)
          {
            NONULL(plugin_body->params()->Get(k), false);
            NONULL(plugin_body->params()->Get(k)->c_str(), false);
            CharStarAutomataCtx param_ctx(
                plugin_body->params()->Get(k)->c_str());
            std::string idnt;
            Number_T num;
            switch(idOrNumThenNterm(&param_ctx, &idnt, &num))
            {
            case IdOrNumThenNterm::invalid:
            {
              return false;
            }
            case IdOrNumThenNterm::identifier:
            {
              binding.parameters.emplace_back(std::move(idnt));
              break;
            }
            case IdOrNumThenNterm::number:
            {
              binding.parameters.emplace_back(std::move(num));
              break;
            }
            }
          }

          NONULL(plugin_body->public_count(), false);
          for(flatbuffers::uoffset_t k = 0;
              k < plugin_body->public_count()->size(); k++)
          {
            Count const* const count = plugin_body->public_count()->Get(k);
            NONULL(count, false);

            if(binding.publicInputCount.size() < (size_t) count->type_id())
            {
              binding.publicInputCount.resize((size_t) count->type_id() + 1);
            }

            if(UNLIKELY(
                  0 != binding.publicInputCount[(size_t) count->type_id()]))
            {
              log_error("duplicate public input count");
            }

            binding.publicInputCount[(size_t) count->type_id()] =
              (size_t) count->count();
          }

          NONULL(plugin_body->private_count(), false);
          for(flatbuffers::uoffset_t k = 0;
              k < plugin_body->private_count()->size(); k++)
          {
            Count const* const count = plugin_body->private_count()->Get(k);
            NONULL(count, false);

            if(binding.privateInputCount.size() < (size_t) count->type_id())
            {
              binding.privateInputCount.resize((size_t) count->type_id() + 1);
            }

            if(UNLIKELY(
                  0 != binding.privateInputCount[(size_t) count->type_id()]))
            {
              log_error("duplicate private input count");
            }

            binding.privateInputCount[(size_t) count->type_id()] =
              (size_t) count->count();
          }

          if(UNLIKELY(!handler->pluginFunction(std::move(binding))))
          {
            return false;
          }

          break;
        }
        }

        break;
      }
      default:
      {
        log_error("bad directive");
        return false;
      }
      }
    }
  }

  return true;
}

template<typename Number_T>
bool PublicInputStream<Number_T>::parseStreamHeader()
{
  NONULL(this->ctx->roots[0], false);
  PublicInputs const* const public_inputs =
    this->ctx->roots[0]->message_as_PublicInputs();
  NONULL(public_inputs, false);

  NONULL(public_inputs->type(), false);
  switch(public_inputs->type()->element_type())
  {
  case TypeU_NONE:
  {
    log_error("invalid type");
    return false;
  }
  case TypeU_Field:
  {
    Field const* const field = public_inputs->type()->element_as_Field();
    NONULL(field, false);
    NONULL(field->modulo(), false);
    NONULL(field->modulo()->value(), false);

    Number_T prime = 0;
    if(UNLIKELY(!base256ToNumber(field->modulo()->value(), &prime)))
    {
      return false;
    }

    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(std::move(prime)));

    break;
  }
  case TypeU_ExtField:
  {
    log_error("Extension Fields Currently Unsupported");
    return false;
  }
  case TypeU_Ring:
  {
    Ring const* const ring = public_inputs->type()->element_as_Ring();
    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(ring->nbits()));
    NONULL(ring, false);
    break;
  }
  case TypeU_PluginType:
  {
    log_error("flatbuffer allowance for plugin type in stream is inconsistent"
        " with the rest of the IR");
    return false;
  }
  }

  return true;
}

template<typename Number_T>
wtk::StreamStatus PublicInputStream<Number_T>::next(Number_T* num)
{
  PublicInputs const* public_inputs =
    this->ctx->roots[this->bufferIdx]->message_as_PublicInputs();
  NONULL(public_inputs->inputs(), wtk::StreamStatus::error);

  if(this->streamIdx == public_inputs->inputs()->size())
  {
    if(this->bufferIdx + 1 < this->ctx->roots.size())
    {
      public_inputs =
        this->ctx->roots[++this->bufferIdx]->message_as_PublicInputs();
      this->streamIdx = 0;

      NONULL(public_inputs->inputs(), wtk::StreamStatus::error);
      if(public_inputs->inputs()->size() == 0)
      {
        return wtk::StreamStatus::end;
      }
    }
    else
    {
      return wtk::StreamStatus::end;
    }
  }

  NONULL(
      public_inputs->inputs()->Get(this->streamIdx), wtk::StreamStatus::error);
  NONULL(public_inputs->inputs()->Get(this->streamIdx)->value(),
      wtk::StreamStatus::error);

  if(UNLIKELY(!base256ToNumber(
          public_inputs->inputs()->Get(this->streamIdx++)->value(), num)))
  {
    return wtk::StreamStatus::error;
  }
  else
  {
    return wtk::StreamStatus::success;
  }
}

template<typename Number_T>
bool PrivateInputStream<Number_T>::parseStreamHeader()
{
  NONULL(this->ctx->roots[0], false);
  PrivateInputs const* const private_inputs =
    this->ctx->roots[0]->message_as_PrivateInputs();
  NONULL(private_inputs, false);

  NONULL(private_inputs->type(), false);
  switch(private_inputs->type()->element_type())
  {
  case TypeU_NONE:
  {
    log_error("invalid type");
    return false;
  }
  case TypeU_Field:
  {
    Field const* const field = private_inputs->type()->element_as_Field();
    NONULL(field, false);
    NONULL(field->modulo(), false);
    NONULL(field->modulo()->value(), false);

    Number_T prime = 0;
    if(UNLIKELY(!base256ToNumber(field->modulo()->value(), &prime)))
    {
      return false;
    }

    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(std::move(prime)));

    break;
  }
  case TypeU_ExtField:
  {
    log_error("Extension Fields Currently Unsupported");
    return false;
  }
  case TypeU_Ring:
  {
    Ring const* const ring = private_inputs->type()->element_as_Ring();
    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(ring->nbits()));
    NONULL(ring, false);
    break;
  }
  case TypeU_PluginType:
  {
    log_error("flatbuffer allowance for plugin type in stream is inconsistent"
        " with the rest of the IR");
    return false;
  }
  }

  return true;
}

template<typename Number_T>
wtk::StreamStatus PrivateInputStream<Number_T>::next(Number_T* num)
{
  PrivateInputs const* private_inputs =
    this->ctx->roots[this->bufferIdx]->message_as_PrivateInputs();
  NONULL(private_inputs->inputs(), wtk::StreamStatus::error);

  if(this->streamIdx == private_inputs->inputs()->size())
  {
    if(this->bufferIdx + 1 < this->ctx->roots.size())
    {
      private_inputs =
        this->ctx->roots[++this->bufferIdx]->message_as_PrivateInputs();
      this->streamIdx = 0;

      NONULL(private_inputs->inputs(), wtk::StreamStatus::error);
      if(private_inputs->inputs()->size() == 0)
      {
        return wtk::StreamStatus::end;
      }
    }
    else
    {
      return wtk::StreamStatus::end;
    }
  }

  NONULL(private_inputs->inputs()->Get(this->streamIdx),
      wtk::StreamStatus::error);
  NONULL(private_inputs->inputs()->Get(this->streamIdx)->value(),
      wtk::StreamStatus::error);

  if(UNLIKELY(!base256ToNumber(
          private_inputs->inputs()->Get(this->streamIdx++)->value(), num)))
  {
    return wtk::StreamStatus::error;
  }
  else
  {
    return wtk::StreamStatus::success;
  }
}

} } // namespace wtk::flatbuffer
