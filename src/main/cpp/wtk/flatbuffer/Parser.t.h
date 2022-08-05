/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

#define NONULL(ptr) do { if(UNLIKELY((ptr) == nullptr)) \
  { \
    log_error("null is illegal"); \
    return false; \
  } } while(false)

template<typename Number_T>
bool InstanceInputStream<Number_T>::load(std::vector<Root const*>& roots)
{
  for(size_t i = 0; i < roots.size(); i++)
  {
    NONULL(roots[i]);
    if(roots[i]->message_type() != Message_Instance)
    {
      log_error("not an instance");
      return false;
    }

    NONULL(roots[i]->message_as_Instance());
    NONULL(roots[i]->message_as_Instance()->common_inputs());
    this->buffers.push_back(roots[i]->message_as_Instance());
  }

  return true;
}

template<typename Number_T>
StreamStatus InstanceInputStream<Number_T>::next(Number_T* num)
{
  if(this->instance_idx == this->buffers[this->buffer_idx]
      ->common_inputs()->size())
  {
    if(this->buffer_idx == this->buffers.size() - 1)
    {
      return StreamStatus::end;
    }
    else
    {
      this->instance_idx = 0;
      this->buffer_idx++;
    }
  }

  if(this->buffers[this->buffer_idx]->common_inputs()
      ->Get(this->instance_idx) == nullptr
      || this->buffers[this->buffer_idx]->common_inputs()
      ->Get(this->instance_idx)->value() == nullptr)
  {
    log_error("null is illegal");
    return StreamStatus::error;
  }

  if(!checkFlatNumber(this->buffers[this->buffer_idx]->common_inputs()
        ->Get(this->instance_idx)->value()))
  {
    return StreamStatus::error;
  }

  *num = flatToNumber<Number_T>(
      this->buffers[this->buffer_idx]->common_inputs()
      ->Get(this->instance_idx)->value());

  this->instance_idx++;
  return StreamStatus::success;
}

template<typename Number_T>
bool WitnessInputStream<Number_T>::load(std::vector<Root const*>& roots)
{
  for(size_t i = 0; i < roots.size(); i++)
  {
    NONULL(roots[i]);
    if(roots[i]->message_type() != Message_Witness)
    {
      log_error("not a witness");
      return false;
    }

    NONULL(roots[i]->message_as_Witness());
    NONULL(roots[i]->message_as_Witness()->short_witness());
    this->buffers.push_back(roots[i]->message_as_Witness());
  }

  return true;
}

template<typename Number_T>
StreamStatus WitnessInputStream<Number_T>::next(Number_T* num)
{
  if(this->witness_idx == this->buffers[this->buffer_idx]
      ->short_witness()->size())
  {
    if(this->buffer_idx == this->buffers.size() - 1)
    {
      return StreamStatus::end;
    }
    else
    {
      this->witness_idx = 0;
      this->buffer_idx++;
    }
  }

  if(this->buffers[this->buffer_idx]->short_witness()
      ->Get(this->witness_idx) == nullptr
      || this->buffers[this->buffer_idx]->short_witness()
      ->Get(this->witness_idx)->value() == nullptr)
  {
    log_error("null is illegal");
    return StreamStatus::error;
  }

  if(!checkFlatNumber(this->buffers[this->buffer_idx]->short_witness()
        ->Get(this->witness_idx)->value()))
  {
    return StreamStatus::error;
  }

  *num = flatToNumber<Number_T>(
      this->buffers[this->buffer_idx]->short_witness()
      ->Get(this->witness_idx)->value());

  this->witness_idx++;
  return StreamStatus::success;
}

template<typename Number_T>
ArithmeticParser<Number_T>::ArithmeticParser(
    std::vector<Root const*>& rs, GateSet* gs, FeatureToggles* ft)
  : roots(rs), gateSet(gs), toggles(ft) { }

template<typename Number_T>
bool ArithmeticParser<Number_T>::parseStream(
    ArithmeticStreamHandler<Number_T>* handler)
{
  if(!this->toggles->simple())
  {
    log_error("streaming API requires simple features");
    return false;
  }

  if(this->gateSet->gateSet != GateSet::arithmetic)
  {
    log_error("must be arithmetic gateset");
    return false;
  }

  NONULL(this->roots[0]);
  if(this->roots[0]->message_type() != Message_Relation)
  {
    log_error("Not a relation");
    return false;
  }

  for(size_t i = 0; i < this->roots.size(); i++)
  {
    NONULL(this->roots[i]->message_as_Relation());
    Relation const* relation = this->roots[i]->message_as_Relation();

    NONULL(relation->functions());
    if(relation->functions()->size() != 0)
    {
      log_error("functions forbidden");
      return false;
    }

    NONULL(relation->directives());
    for(size_t j = 0; j < relation->directives()->size(); j++)
    {
      NONULL(relation->directives()->Get(j));
      Directive const* directive = relation->directives()->Get(j);

      NONULL(directive->directive());
      switch(directive->directive_type())
      {
      case DirectiveSet_GateConstant:
      {
        GateConstant const* gate = directive->directive_as_GateConstant();
        NONULL(gate->output());
        index_t out = gate->output()->id();
        NONULL(gate->constant());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        Number_T val = flatToNumber<Number_T>(gate->constant());

        handler->handleAssign(out, val);
        break;
      }
      case DirectiveSet_GateAssertZero:
      {
        GateAssertZero const* gate = directive->directive_as_GateAssertZero();
        NONULL(gate->input());
        handler->handleAssertZero(gate->input()->id());
        break;
      }
      case DirectiveSet_GateCopy:
      {
        GateCopy const* gate = directive->directive_as_GateCopy();
        NONULL(gate->output());
        NONULL(gate->input());
        handler->handleCopy(gate->output()->id(), gate->input()->id());
        break;
      }
      case DirectiveSet_GateAdd:
      {
        if(!this->gateSet->enableAdd)
        {
          log_error("add prohibited");
          return false;
        }

        GateAdd const* gate = directive->directive_as_GateAdd();
        NONULL(gate->output());
        NONULL(gate->left());
        NONULL(gate->right());

        handler->handleAdd(
            gate->output()->id(), gate->left()->id(), gate->right()->id());
        break;
      }
      case DirectiveSet_GateMul:
      {
        if(!this->gateSet->enableMul)
        {
          log_error("mul prohibited");
          return false;
        }

        GateMul const* gate = directive->directive_as_GateMul();
        NONULL(gate->output());
        NONULL(gate->left());
        NONULL(gate->right());

        handler->handleMul(
            gate->output()->id(), gate->left()->id(), gate->right()->id());
        break;
      }
      case DirectiveSet_GateAddConstant:
      {
        if(!this->gateSet->enableAddC)
        {
          log_error("addc prohibited");
          return false;
        }

        GateAddConstant const* gate =
          directive->directive_as_GateAddConstant();
        NONULL(gate->output());
        NONULL(gate->input());
        NONULL(gate->constant());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        Number_T constant = flatToNumber<Number_T>(gate->constant());

        handler->handleAddC(
            gate->output()->id(), gate->input()->id(), constant);
        break;
      }
      case DirectiveSet_GateMulConstant:
      {
        if(!this->gateSet->enableMulC)
        {
          log_error("mulc prohibited");
          return false;
        }

        GateMulConstant const* gate =
          directive->directive_as_GateMulConstant();
        NONULL(gate->output());
        NONULL(gate->input());
        NONULL(gate->constant());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        Number_T constant = flatToNumber<Number_T>(gate->constant());

        handler->handleMulC(
            gate->output()->id(), gate->input()->id(), constant);
        break;
      }
      case DirectiveSet_GateInstance:
      {
        GateInstance const* gate = directive->directive_as_GateInstance();
        NONULL(gate->output());
        handler->handleInstance(gate->output()->id());
        break;
      }
      case DirectiveSet_GateWitness:
      {
        GateWitness const* gate = directive->directive_as_GateWitness();
        NONULL(gate->output());
        handler->handleShortWitness(gate->output()->id());
        break;
      }
      case DirectiveSet_GateFree:
      {
        GateFree const* gate = directive->directive_as_GateFree();
        NONULL(gate->first());
        if(gate->last() == nullptr)
        {
          handler->handleDeleteSingle(gate->first()->id());
        }
        else
        {
          handler->handleDeleteRange(gate->first()->id(), gate->last()->id());
        }
        break;
      }
      default:
      {
        log_error("gate prohibited by features or gateset");
        return false;
      }
      }
    }
  }

  handler->handleEnd();
  return true;
}

template<typename Number_T>
FBIRTree<Number_T>* ArithmeticParser<Number_T>::parseTree()
{
  this->treeParser = std::unique_ptr<TreeParser<Number_T>>(
      new TreeParser<Number_T>(this->gateSet, this->toggles));

  return this->treeParser->parseIRTree(this->roots);
}

template<typename Number_T>
InstanceInputStream<Number_T>* ArithmeticParser<Number_T>::instance()
{
  this->instanceStream = std::unique_ptr<InstanceInputStream<Number_T>>(
      new InstanceInputStream<Number_T>());

  if(this->instanceStream->load(this->roots))
  {
    return this->instanceStream.get();
  }
  else
  {
    return nullptr;
  }
}

template<typename Number_T>
WitnessInputStream<Number_T>* ArithmeticParser<Number_T>::shortWitness()
{
  this->witnessStream = std::unique_ptr<WitnessInputStream<Number_T>>(
      new WitnessInputStream<Number_T>());

  if(this->witnessStream->load(this->roots))
  {
    return this->witnessStream.get();
  }
  else
  {
    return nullptr;
  }
}

BooleanParser::BooleanParser(
    std::vector<Root const*>& rs, GateSet* gs, FeatureToggles* ft)
  : roots(rs), gateSet(gs), toggles(ft) { }

bool BooleanParser::parseStream(BooleanStreamHandler* handler)
{
  if(!this->toggles->simple())
  {
    log_error("streaming API requires simple features");
    return false;
  }

    if(this->gateSet->gateSet != GateSet::boolean)
    {
      log_error("Expected boolean gateset");
      return false;
    }


  NONULL(this->roots[0]);
  if(this->roots[0]->message_type() != Message_Relation)
  {
    log_error("Not a relation");
    return false;
  }

  for(size_t i = 0; i < this->roots.size(); i++)
  {
    NONULL(this->roots[i]->message_as_Relation());
    Relation const* relation = this->roots[i]->message_as_Relation();

    NONULL(relation->functions());
    if(relation->functions()->size() != 0)
    {
      log_error("functions forbidden");
      return false;
    }

    NONULL(relation->directives());
    for(size_t j = 0; j < relation->directives()->size(); j++)
    {
      NONULL(relation->directives()->Get(j));
      Directive const* directive = relation->directives()->Get(j);

      NONULL(directive->directive());
      switch(directive->directive_type())
      {
      case DirectiveSet_GateConstant:
      {
        GateConstant const* gate = directive->directive_as_GateConstant();
        NONULL(gate->output());
        index_t out = gate->output()->id();
        NONULL(gate->constant());
        if(!checkFlatNumber(gate->constant()))
        {
          return false;
        }

        uint8_t val = flatToNumber<uint8_t>(gate->constant());

        handler->handleAssign(out, val);
        break;
      }
      case DirectiveSet_GateAssertZero:
      {
        GateAssertZero const* gate = directive->directive_as_GateAssertZero();
        NONULL(gate->input());
        handler->handleAssertZero(gate->input()->id());
        break;
      }
      case DirectiveSet_GateCopy:
      {
        GateCopy const* gate = directive->directive_as_GateCopy();
        NONULL(gate->output());
        NONULL(gate->input());
        handler->handleCopy(gate->output()->id(), gate->input()->id());
        break;
      }
      case DirectiveSet_GateAnd:
      {
        if(!this->gateSet->enableAnd)
        {
          log_error("and prohibited");
          return false;
        }

        GateAnd const* gate = directive->directive_as_GateAnd();
        NONULL(gate->output());
        NONULL(gate->left());
        NONULL(gate->right());

        handler->handleAnd(
            gate->output()->id(), gate->left()->id(), gate->right()->id());
        break;
      }
      case DirectiveSet_GateXor:
      {
        if(!this->gateSet->enableXor)
        {
          log_error("xor prohibited");
          return false;
        }

        GateXor const* gate = directive->directive_as_GateXor();
        NONULL(gate->output());
        NONULL(gate->left());
        NONULL(gate->right());

        handler->handleXor(
            gate->output()->id(), gate->left()->id(), gate->right()->id());
        break;
      }
      case DirectiveSet_GateNot:
      {
        if(!this->gateSet->enableNot)
        {
          log_error("not prohibited");
          return false;
        }

        GateNot const* gate = directive->directive_as_GateNot();
        NONULL(gate->output());
        NONULL(gate->input());

        handler->handleNot(gate->output()->id(), gate->input()->id());
        break;
      }
      case DirectiveSet_GateInstance:
      {
        GateInstance const* gate = directive->directive_as_GateInstance();
        NONULL(gate->output());
        handler->handleInstance(gate->output()->id());
        break;
      }
      case DirectiveSet_GateWitness:
      {
        GateWitness const* gate = directive->directive_as_GateWitness();
        NONULL(gate->output());
        handler->handleShortWitness(gate->output()->id());
        break;
      }
      case DirectiveSet_GateFree:
      {
        GateFree const* gate = directive->directive_as_GateFree();
        NONULL(gate->first());
        if(gate->last() == nullptr)
        {
          handler->handleDeleteSingle(gate->first()->id());
        }
        else
        {
          handler->handleDeleteRange(gate->first()->id(), gate->last()->id());
        }
        break;
      }
      default:
      {
        log_error("gate prohibited by features or gateset");
        return false;
      }
      }
    }
  }

  handler->handleEnd();
  return true;
}

FBIRTree<uint8_t>* BooleanParser::parseTree()
{
  this->treeParser = std::unique_ptr<TreeParser<uint8_t>>(
      new TreeParser<uint8_t>(this->gateSet, this->toggles));

  return this->treeParser->parseIRTree(this->roots);
}

InstanceInputStream<uint8_t>* BooleanParser::instance()
{
  this->instanceStream = std::unique_ptr<InstanceInputStream<uint8_t>>(
      new InstanceInputStream<uint8_t>());

  if(this->instanceStream->load(this->roots))
  {
    return this->instanceStream.get();
  }
  else
  {
    return nullptr;
  }
}

WitnessInputStream<uint8_t>* BooleanParser::shortWitness()
{
  this->witnessStream = std::unique_ptr<WitnessInputStream<uint8_t>>(
      new WitnessInputStream<uint8_t>());

  if(this->witnessStream->load(this->roots))
  {
    return this->witnessStream.get();
  }
  else
  {
    return nullptr;
  }
}

#undef NONULL
#define NONULL(ptr) do { if(UNLIKELY((ptr) == nullptr)) \
  { \
    log_error("null is illegal"); \
    this->failure = true; \
    return false; \
  } } while(false)

template<typename Number_T>
Parser<Number_T>::Parser(std::string& f_name)
 : fileName(f_name)
{
  this->file = fopen(this->fileName.c_str(), "rb");
  if(nullptr == this->file)
  {
    log_error("failed to open file \"%s\"", this->fileName.c_str());
    log_perror();
    this->failure = true;
    return;
  }

  if(0 != fseeko(this->file, 0, SEEK_END))
  {
    log_error("failed to read file size \"%s\"", this->fileName.c_str());
    log_perror();
    this->failure = true;
    return;
  }

  off_t off_len = ftello(this->file);
  if(off_len < 0)
  {
    log_error("failed to read file size \"%s\"", this->fileName.c_str());
    log_perror();
    this->failure = true;
    return;
  }

  if(0 != fseeko(this->file, 0, SEEK_SET))
  {
    log_error("failed to reset file \"%s\"", this->fileName.c_str());
    log_perror();
    this->failure = true;
    return;
  }


  this->bufferSize = (size_t) off_len;

  this->buffer = (uint8_t*) malloc(this->bufferSize * sizeof(uint8_t));
  if(this->buffer == nullptr)
  {
    log_error("failed to allocate buffer for \"%s\"", this->fileName.c_str());
    log_perror();
    this->failure = true;
  }


  size_t read_total = 0;
  do
  {
    size_t n_read = fread(this->buffer + read_total, sizeof(uint8_t),
        this->bufferSize - read_total, this->file);
    if(ferror(this->file))
    {
      log_error("failed to read file \"%s\"", this->fileName.c_str());
      log_perror();
      this->failure = true;
      return;
    }
    else
    {
      read_total += n_read;
    }
  } while(read_total < this->bufferSize);


  // technically this is the shortest flatbuffer, 4-byte 'siev' constant,
  // followed by size = 0...
  if(this->bufferSize < 8)
  {
    log_error("file \"%s\" is not large enough to be valid",
        this->fileName.c_str());
    this->failure = true;
    return;
  }

  size_t place = 0;
  size_t segment = 0;
  while(place + 4 < this->bufferSize)
  {
    uint32_t new_size = this->buffer[place + 0]
      + (this->buffer[place + 1] << 8)
      + (this->buffer[place + 2] << 16)
      + (this->buffer[place + 3] << 24);
    if(new_size + place + 4 > this->bufferSize)
    {
      log_error("file \"%s\" has invalid size constant",
          this->fileName.c_str());
      this->failure = true;
      return;
    }

    flatbuffers::Verifier fb_verifier(&this->buffer[place + 4], new_size,
        INT32_MAX, INT32_MAX);
    if(!VerifyRootBuffer(fb_verifier))
    {
      log_error(
          "flatbuffer internal structure verification failed (segment %zu)",
          segment);
      this->failure = true;
      return;
    }

    Root const* new_root =
      flatbuffers::GetRoot<Root>((void*) (this->buffer + place + 4));
    if(new_root == nullptr)
    {
      log_error("flatbuffer error \"%s\"", this->fileName.c_str());
      this->failure = true;
      return;
    }

    this->sizes.push_back(new_size);
    this->roots.push_back((Root*) new_root);
    place = place + 4 + new_size;
    segment++;
  }
}

template<typename Number_T>
Parser<Number_T>::~Parser()
{
  if(this->file != nullptr) { fclose(this->file); }
  free(this->buffer);
}

template<typename Number_T>
bool Parser<Number_T>::parseHeader()
{
  if(this->failure) { return false; }
  if(this->roots.size() == 0)
  {
    log_error("no flatbuffers present");
    this->failure = true;
    return false;
  }

  Root const* first = this->roots[0];

  NONULL(first);
  NONULL(first->message());

  Header const* first_header;
  if(first->message_type() == Message_Relation)
  {
    NONULL(first->message_as_Relation()->header());
    first_header = first->message_as_Relation()->header();
  }
  else if(first->message_type() == Message_Instance)
  {
    NONULL(first->message_as_Instance()->header());
    first_header = first->message_as_Instance()->header();
  }
  else if(first->message_type() == Message_Witness)
  {
    NONULL(first->message_as_Witness()->header());
    first_header = first->message_as_Witness()->header();
  }
  else
  {
    log_error("invalid message type");
    this->failure = true;
    return false;
  }

  NONULL(first_header->version());
  NONULL(first_header->field_characteristic());
  NONULL(first_header->field_characteristic()->value());
  if(!checkFlatNumber(first_header->field_characteristic()->value()))
  {
    this->failure = true;
    return false;
  }

  std::string version_str = first_header->version()->str();
  Number_T characteristic =
    flatToNumber<Number_T>(first_header->field_characteristic()->value());
  uint32_t degree = first_header->field_degree();

  Message msg_type = first->message_type();

  for(size_t i = 1; i < this->roots.size(); i++)
  {
    Root const* ith = this->roots[i];
    NONULL(ith);
    if(msg_type != ith->message_type())
    {
      log_error("message type mismatch");
      this->failure = true;
      return false;
    }

    NONULL(ith->message());

    Header const* ith_header;
    if(msg_type == Message_Relation)
    {
      NONULL(ith->message_as_Relation()->header());
      ith_header = ith->message_as_Relation()->header();
    }
    else if(msg_type == Message_Instance)
    {
      NONULL(ith->message_as_Instance()->header());
      ith_header = ith->message_as_Instance()->header();
    }
    else
    {
      NONULL(ith->message_as_Witness()->header());
      ith_header = ith->message_as_Witness()->header();
    }

    NONULL(ith_header->version());
    NONULL(ith_header->field_characteristic());
    NONULL(ith_header->field_characteristic()->value());
    if(!checkFlatNumber(ith_header->field_characteristic()->value()))
    {
      this->failure = true;
      return false;
    }

    std::string ith_version_str = ith_header->version()->str();
    Number_T ith_characteristic = flatToNumber<Number_T>(
        ith_header->field_characteristic()->value());
    uint32_t ith_degree = ith_header->field_degree();

    if(ith_version_str != version_str
        || ith_characteristic != characteristic
        || ith_degree != degree)
    {
      log_error("headers in root sequence don't match");
      this->failure = true;
      return false;
    }
  }

  // c_str() is guaranteed null-terminated
  if(3 != sscanf(version_str.c_str(), "%zu.%zu.%zu",
        &this->version.major, &this->version.minor, &this->version.patch))
  {
    log_error("version string invalid");
    this->failure = true;
    return false;
  }

  this->characteristic = characteristic;
  this->degree = degree;

  return true;
}

template<typename Number_T>
bool Parser<Number_T>::parseResource()
{
  if(this->failure) { return false; }

  Root const* first = this->roots[0];
  if(first->message_type() == Message_Relation)
  {
    this->resource = Resource::relation;
  }
  else if(first->message_type() == Message_Instance)
  {
    this->resource = Resource::instance;
  }
  else if(first->message_type() == Message_Witness)
  {
    this->resource = Resource::shortWitness;
  }
  else
  {
    log_error("unrecognized resource type");
    this->failure = true;
    return false;
  }

  return true;
}

inline std::vector<std::string> strSplit(
    std::string const& str, char const split)
{
  std::vector<std::string> ret;

  size_t prev = 0;
  size_t spot = str.find(split, prev);

  while(spot != std::string::npos)
  {
    ret.emplace_back(str, prev, spot - prev);
    prev = spot + 1;
    spot = str.find(split, prev);
  }

  ret.emplace_back(str, prev);

  return ret;
}

template<typename Number_T>
bool Parser<Number_T>::parseParameters()
{
  if(this->failure || this->resource != Resource::relation) { return false; }

  Relation const* first = this->roots[0]->message_as_Relation();

  NONULL(first);
  NONULL(first->gateset());
  NONULL(first->features());

  std::string gateset_str = first->gateset()->str();
  std::string features_str = first->features()->str();

  for(size_t i = 1; i < this->roots.size(); i++)
  {
    Relation const* ith = this->roots[i]->message_as_Relation();
    NONULL(ith);
    NONULL(ith->gateset());
    NONULL(ith->features());

    std::string ith_gateset_str = ith->gateset()->str();
    std::string ith_features_str = ith->features()->str();

    if(gateset_str != ith_gateset_str
        || features_str != ith_features_str)
    {
      log_error("mismatching gateset or features");
      this->failure = true;
      return false;
    }
  }

  bool success = true;
  if(gateset_str == "arithmetic")
  {
    this->gateSet.gateSet = GateSet::arithmetic;
    this->gateSet.enableAdd = true;
    this->gateSet.enableAddC = true;
    this->gateSet.enableMul = true;
    this->gateSet.enableMulC = true;
  }
  else if(gateset_str == "boolean")
  {
    this->gateSet.gateSet = GateSet::boolean;
    this->gateSet.enableAnd = true;
    this->gateSet.enableXor = true;
    this->gateSet.enableNot = true;
  }
  else
  {
    std::vector<std::string> gateset = strSplit(gateset_str, ',');
    if(gateset.size() == 0)
    {
      log_error("gateset invalid \"%s\"", gateset_str.c_str());
      success = false;
      this->failure = true;
    }
    else if(gateset[0] == "@xor"
        || gateset[0] == "@and"
        || gateset[0] == "@not")
    {
      this->gateSet.gateSet = GateSet::boolean;
      this->gateSet.enableAnd = false;
      this->gateSet.enableXor = false;
      this->gateSet.enableNot = false;

      for(size_t i = 0; i < gateset.size(); i++)
      {
        if(gateset[i] == "@xor")      { this->gateSet.enableXor = true; }
        else if(gateset[i] == "@and") { this->gateSet.enableAnd = true; }
        else if(gateset[i] == "@not") { this->gateSet.enableNot = true; }
        else if(gateset[i] == "")     { /* trailing, */ (void) gateset[i]; }
        else
        {
          log_error("invalid gateset \"%s\"", gateset[i].c_str());
          this->failure = true;
          success = false;
          break;
        }
      }
    }
    else
    {
      this->gateSet.gateSet = GateSet::arithmetic;
      this->gateSet.enableAdd = false;
      this->gateSet.enableAddC = false;
      this->gateSet.enableMul = false;
      this->gateSet.enableMulC = false;

      for(size_t i = 0; i < gateset.size(); i++)
      {
        if(gateset[i] == "@add")       { this->gateSet.enableAdd = true; }
        else if(gateset[i] == "@mul")  { this->gateSet.enableMul = true; }
        else if(gateset[i] == "@addc") { this->gateSet.enableAddC = true; }
        else if(gateset[i] == "@mulc") { this->gateSet.enableMulC = true; }
        else if(gateset[i] == "")     { /* trailing, */ (void) gateset[i]; }
        else
        {
          log_error("invalid gateset \"%s\"", gateset[i].c_str());
          this->failure = true;
          success = false;
          break;
        }
      }
    }
  }

  if(features_str == "simple")
  {
    this->featureToggles.functionToggle = false;
    this->featureToggles.forLoopToggle = false;
    this->featureToggles.switchCaseToggle = false;
  }
  else
  {
    this->featureToggles.functionToggle = false;
    this->featureToggles.forLoopToggle = false;
    this->featureToggles.switchCaseToggle = false;

    std::vector<std::string> features = strSplit(features_str, ',');
    for(size_t i = 0; i < features.size(); i++)
    {
      if(features[i] == "@function")
      {
        this->featureToggles.functionToggle = true;
      }
      else if(features[i] == "@for")
      {
        this->featureToggles.forLoopToggle = true;
      }
      else if(features[i] == "@switch")
      {
        this->featureToggles.switchCaseToggle = true;
      }
      else if(features[i] == "")
      {
        /* allow trailing commas, */
        (void) features[i];
      }
      else
      {
        log_error("unrecognized feature: %s", features[i].c_str());
        this->failure = true;
        success = false;
      }
    }
  }

  return success;
}

template<typename Number_T>
BooleanParser* Parser<Number_T>::boolean()
{
  if(this->failure) { return nullptr; }

  this->boolParser = std::unique_ptr<BooleanParser>(
      new BooleanParser(this->roots, &this->gateSet, &this->featureToggles));

  return this->boolParser.get();
}

template<typename Number_T>
ArithmeticParser<Number_T>* Parser<Number_T>::arithmetic()
{
  if(this->failure) { return nullptr; }

  this->arithParser = std::unique_ptr<ArithmeticParser<Number_T>>(
      new ArithmeticParser<Number_T>(
        this->roots, &this->gateSet, &this->featureToggles));

  return this->arithParser.get();
}

template<typename Number_T>
ArithmeticParser<uint32_t>* Parser<Number_T>::arithmetic32()
{
  if(this->failure) { return nullptr; }

  this->arith32Parser = std::unique_ptr<ArithmeticParser<uint32_t>>(
      new ArithmeticParser<uint32_t>(
        this->roots, &this->gateSet, &this->featureToggles));

  return this->arith32Parser.get();
}

template<typename Number_T>
ArithmeticParser<uint64_t>* Parser<Number_T>::arithmetic64()
{
  if(this->failure) { return nullptr; }

  this->arith64Parser = std::unique_ptr<ArithmeticParser<uint64_t>>(
      new ArithmeticParser<uint64_t>(
        this->roots, &this->gateSet, &this->featureToggles));

  return this->arith64Parser.get();
}

#undef NONULL

} } // namespace wtk::flatbuffer
