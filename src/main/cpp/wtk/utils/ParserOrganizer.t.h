/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

std::string stringify_version(
    size_t const major, size_t const minor, size_t const patch,
    char const* const extra)
{
  if(extra == nullptr || 0 == strcmp(extra, ""))
  {
    return dec(major) + "." + dec(minor) + "." + dec(patch);
  }
  else
  {
    return dec(major) + "." + dec(minor) + "." + dec(patch) + "-" + extra;
  }
}

template<typename Parser_T, typename Number_T>
bool ParserOrganizer<Parser_T, Number_T>::open(char const* const fileName)
{
  this->parsers.emplace_back();
  this->fileNames.emplace_back(fileName);
  this->parserUsed.push_back(false);
  return this->parsers.back().open(fileName)
    && this->parsers.back().parseHeader();
}

template<typename Parser_T, typename Number_T>
Setting ParserOrganizer<Parser_T, Number_T>::organize()
{
  Setting ret = Setting::failure;
  for(size_t i = 0; i < this->parsers.size(); i++)
  {
    wtk::Parser<Number_T>* p = &this->parsers[i];
    if(p->version.major != wtk::IR_VERSION_MAJOR
        || p->version.minor != wtk::IR_VERSION_MINOR
        || p->version.patch != wtk::IR_VERSION_PATCH
        || ((p->version.extra.size() != 0 && wtk::IR_VERSION_EXTRA == nullptr)
          || (wtk::IR_VERSION_EXTRA != nullptr
            && 0 != strcmp(p->version.extra.c_str(), wtk::IR_VERSION_EXTRA))))
    {
      log_warn("%s: Version %s does not match expected version (%s)",
          this->fileNames[i], stringify_version(p->version.major,
            p->version.minor, p->version.patch,
            p->version.extra.c_str()).c_str(), stringify_version(
            wtk::IR_VERSION_MAJOR, wtk::IR_VERSION_MINOR,
            wtk::IR_VERSION_PATCH, wtk::IR_VERSION_EXTRA).c_str());
    }
  }

  for(size_t i = 0; i < this->parsers.size(); i++)
  {
    wtk::Parser<Number_T>* p = &this->parsers[i];
    if(p->type == wtk::ResourceType::circuit)
    {
      if(this->circuitParser != nullptr)
      {
        log_error("Multiple circuits: %s and %s",
            this->circuitName, this->fileNames[i]);
        return Setting::failure;
      }
      else
      {
        this->circuitParser = p;
        this->circuitBodyParser = p->circuit();
        this->circuitName = this->fileNames[i];

        if(this->circuitBodyParser == nullptr)
        {
          log_error("Error initializing parser for %s", this->circuitName);
          return Setting::failure;
        }

        log_assert(!this->parserUsed[i]);
        this->parserUsed[i] = true;
      }
    }
    else if(p->type == wtk::ResourceType::translation)
    {
      log_error("Translation-IR not yet supported");
      return Setting::failure;
    }
    else if(p->type == wtk::ResourceType::configuration)
    {
      log_error("CCC not yet supported");
      return Setting::failure;
    }
  }

  if(this->circuitParser == nullptr)
  {
    log_error("No relation found");
    return Setting::failure;
  }
  else
  {
    if(!this->circuitBodyParser->parseCircuitHeader())
    {
      return Setting::failure;
    }

    this->circuitStreams.resize(this->circuitBodyParser->types.size());

    for(size_t i = 0; i < this->parsers.size(); i++)
    {
      wtk::Parser<Number_T>* p = &this->parsers[i];

      if(p->type == wtk::ResourceType::public_in)
      {
        wtk::InputStream<Number_T>* s = p->publicIn();
        if(s == nullptr)
        {
          log_error("error initializing public input stream %s",
              this->fileNames[i]);
          return Setting::failure;
        }

        if(!s->parseStreamHeader())
        {
          return Setting::failure;
        }

        for(size_t j = 0; j < this->circuitBodyParser->types.size(); j++)
        {
          wtk::circuit::TypeSpec<Number_T>* t = 
            &this->circuitBodyParser->types[j];

          if(*t == *s->type)
          {
            if(this->circuitStreams[j].publicParser != nullptr)
            {
              log_error(
                  "duplicate public input stream for type %s: %s and %s",
                  type_str(s->type.get()).c_str(),
                  this->circuitStreams[j].publicName, this->fileNames[i]);
              return Setting::failure;
            }
            else
            {
              this->circuitStreams[j].publicParser = p;
              this->circuitStreams[j].publicStream = s;
              this->circuitStreams[j].publicName = this->fileNames[i];

              log_assert(!this->parserUsed[i]);
              this->parserUsed[i] = true;
            }
          }
        }
      }
      else if(p->type == wtk::ResourceType::private_in)
      {
        wtk::InputStream<Number_T>* s = p->privateIn();
        if(s == nullptr)
        {
          log_error("error initializing private input stream %s",
              this->fileNames[i]);
          return Setting::failure;
        }

        if(!s->parseStreamHeader())
        {
          return Setting::failure;
        }

        for(size_t j = 0; j < this->circuitBodyParser->types.size(); j++)
        {
          wtk::circuit::TypeSpec<Number_T>* t = 
            &this->circuitBodyParser->types[j];

          if(*t == *s->type)
          {
            if(this->circuitStreams[j].privateParser != nullptr)
            {
              log_error(
                  "duplicate private input stream for type %s: %s and %s",
                  type_str(s->type.get()).c_str(),
                  this->circuitStreams[j].privateName, this->fileNames[i]);
              return Setting::failure;
            }
            else
            {
              this->circuitStreams[j].privateParser = p;
              this->circuitStreams[j].privateStream = s;
              this->circuitStreams[j].privateName = this->fileNames[i];

              log_assert(!this->parserUsed[i]);
              this->parserUsed[i] = true;
            }
          }
        }
      }
    }

    size_t i = 0;
    ret = Setting::preprocess;
    for(; i < this->circuitStreams.size(); i++)
    {
      if(this->circuitStreams[i].publicParser != nullptr
          && this->circuitStreams[i].privateParser == nullptr)
      {
        ret = Setting::verifier;
        break;
      }
      else if(this->circuitStreams[i].publicParser != nullptr
          && this->circuitStreams[i].privateParser != nullptr)
      {
        ret = Setting::prover;
        break;
      }
      else if(this->circuitStreams[i].publicParser == nullptr
          && this->circuitStreams[i].privateParser != nullptr)
      {
        log_error("Cannot recognize setting with relation and public input "
            "stream but without private input stream for type %zu", i);
        return Setting::failure;
      }
      else if(this->circuitBodyParser->types[i].variety
          == wtk::circuit::TypeSpec<Number_T>::field)
      {
        break;
      }
    }

    for(; i < this->circuitStreams.size(); i++)
    {
      if(this->circuitStreams[i].publicParser == nullptr
          && this->circuitStreams[i].privateParser != nullptr)
      {
        log_error("Cannot recognize setting with relation and public input "
            "stream but without private input stream for type %zu", i);
        return Setting::failure;
      }

      if(this->circuitBodyParser->types[i].variety
          != wtk::circuit::TypeSpec<Number_T>::plugin
          && (ret == Setting::verifier || ret == Setting::prover)
          && this->circuitStreams[i].publicParser == nullptr)
      {
        log_error("Missing public input stream for type %zu (type %s)",
            i, type_str(&this->circuitBodyParser->types[i]).c_str());
        return Setting::failure;
      }

      if(this->circuitBodyParser->types[i].variety
          != wtk::circuit::TypeSpec<Number_T>::plugin
          && ret == Setting::prover
          && this->circuitStreams[i].privateParser == nullptr)
      {
        log_error("Missing private input stream for type %zu (type %s)",
            i, type_str(&this->circuitBodyParser->types[i]).c_str());
        return Setting::failure;
      }
    }
  }

  for(size_t i = 0; i < this->parserUsed.size(); i++)
  {
    if(!this->parserUsed[i])
    {
      log_error("unused resource: %s", this->fileNames[i]);
      return Setting::failure;
    }
  }

  return ret;
}

template<typename Number_T>
std::string type_str(wtk::circuit::TypeSpec<Number_T> const* const type)
{
  std::string ret = "";

  switch(type->variety)
  {
  case wtk::circuit::TypeSpec<Number_T>::field:
  {
    ret = std::string("field ") + wtk::utils::dec(type->prime);
    break;
  }
  case wtk::circuit::TypeSpec<Number_T>::ring:
  {
    ret = std::string("ring ") + wtk::utils::dec(type->bitWidth);
    break;
  }
  case wtk::circuit::TypeSpec<Number_T>::plugin:
  {
    ret = std::string("@plugin(") + type->binding.name + ", "
      + type->binding.operation + ")";
    break;
  }
  }

  return ret;
}

} } // namespace wtk::utils
