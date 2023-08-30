/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#define ULK(expr) UNLIKELY((expr))

#include <wtk/irregular/CircuitIR.i.h>

namespace wtk {
namespace irregular {

template<typename Number_T>
bool Parser<Number_T>::open(char const* const fname)
{
  FileAutomataCtx* f_ctx = new FileAutomataCtx();
  this->ctx = std::unique_ptr<AutomataCtx>(f_ctx);

  return f_ctx->open(fname);
}

template<typename Number_T>
bool Parser<Number_T>::open(FILE* const file, char const* const fname)
{
  FileAutomataCtx* f_ctx = new FileAutomataCtx();
  this->ctx = std::unique_ptr<AutomataCtx>(f_ctx);

  return f_ctx->open(file, fname);
}

template<typename Number_T>
bool Parser<Number_T>::parseHeader()
{
  if(ULK(ULK(!whitespace(this->ctx.get())) || ULK(!versionKw(this->ctx.get()))
      || ULK(!whitespace(this->ctx.get()))
      || ULK(!number(this->ctx.get(), &this->version.major))
      || ULK(!dotOp(this->ctx.get()))
      || ULK(!number(this->ctx.get(), &this->version.minor))
      || ULK(!dotOp(this->ctx.get()))
      || ULK(!number(this->ctx.get(), &this->version.patch))))
  {
    return false;
  }

  switch(dashOrSemiColonOps(this->ctx.get()))
  {
  case DashOrSemiColon::dash:
  {
    if(ULK(ULK(!identifier(this->ctx.get(), &this->version.extra))
      || ULK(!whitespace(this->ctx.get()))
      || ULK(!semiColonOp(this->ctx.get()))))
    {
      return false;
    }
    break;
  }
  case DashOrSemiColon::semiColon:
  {
    break;
  }
  case DashOrSemiColon::invalid:
  {
    return false;
  }
  }

  if(ULK(!whitespace(this->ctx.get())))
  {
    return false;
  }

  switch(resourceTypeKws(this->ctx.get()))
  {
  case ResourceType::invalid:
    return false;
  case ResourceType::translation:
    this->type = wtk::ResourceType::translation;
    break;
  case ResourceType::circuit:
    this->type = wtk::ResourceType::circuit;
    break;
  case ResourceType::publicIn:
    this->type = wtk::ResourceType::public_in;
    break;
  case ResourceType::privateIn:
    this->type = wtk::ResourceType::private_in;
    break;
  case ResourceType::configuration:
    this->type = wtk::ResourceType::configuration;
    break;
  }

  if(ULK(ULK(!whitespace(this->ctx.get()))
      || ULK(!semiColonOp(this->ctx.get()))))
  {
    return false;
  }

  return true;
}

template<typename Number_T>
TranslationParser<Number_T>* Parser<Number_T>::translation()
{
  if(ULK(this->type != wtk::ResourceType::translation))
  {
    log_error(
        "Cannot create a TranslationParser for non-translation resource");
    return nullptr;
  }

  if(this->translationParser == nullptr)
  {
    this->translationParser = std::unique_ptr<TranslationParser<Number_T>>(
        new TranslationParser<Number_T>(this->ctx.get()));
  }

  return this->translationParser.get();
}

template<typename Number_T>
CircuitParser<Number_T>* Parser<Number_T>::circuit()
{
  if(ULK(this->type != wtk::ResourceType::circuit))
  {
    log_error("Cannot create a CircuitParser for non-circuit resource");
    return nullptr;
  }

  if(this->circuitParser == nullptr)
  {
    this->circuitParser = std::unique_ptr<CircuitParser<Number_T>>(
        new CircuitParser<Number_T>(this->ctx.get()));
  }

  return this->circuitParser.get();
}

template<typename Number_T>
InputStream<Number_T>* Parser<Number_T>::publicIn()
{
  if(ULK(this->type != wtk::ResourceType::public_in))
  {
    log_error(
        "Cannot create a public InputStream for non-public input resource");
    return nullptr;
  }

  if(this->inputStream == nullptr)
  {
    this->inputStream = std::unique_ptr<InputStream<Number_T>>(
        new InputStream<Number_T>(this->ctx.get()));
  }

  return this->inputStream.get();
}

template<typename Number_T>
InputStream<Number_T>* Parser<Number_T>::privateIn()
{
  if(ULK(this->type != wtk::ResourceType::private_in))
  {
    log_error(
        "Cannot create a private InputStream for non-private input resource");
    return nullptr;
  }

  if(this->inputStream == nullptr)
  {
    this->inputStream = std::unique_ptr<InputStream<Number_T>>(
        new InputStream<Number_T>(this->ctx.get()));
  }

  return this->inputStream.get();
}

template<typename Number_T>
ConfigurationParser<Number_T>* Parser<Number_T>::configuration()
{
  if(ULK(this->type != wtk::ResourceType::configuration))
  {
    log_error(
        "Cannot create a ConfigurationParser for non-configuration resource");
    return nullptr;
  }

  if(this->configurationParser == nullptr)
  {
    this->configurationParser = std::unique_ptr<ConfigurationParser<Number_T>>(
        new ConfigurationParser<Number_T>(this->ctx.get()));
  }

  return this->configurationParser.get();
}

template<typename Number_T>
bool CircuitParser<Number_T>::parseCircuitHeader()
{
  if(ULK(!whitespace(ctx))) { return false; }

  PluginOrType plugin_or_type = pluginOrType(this->ctx);
  while(plugin_or_type == PluginOrType::plugin)
  {
    std::string name;
    if(ULK(ULK(ULK(!whitespace(this->ctx))
        || ULK(!identifier(this->ctx, &name)) || ULK(!whitespace(this->ctx)))
        || ULK(!semiColonOp(this->ctx))) || ULK(!whitespace(this->ctx)))
    {
      return false;
    }

    this->plugins.emplace_back(std::move(name));

    plugin_or_type = pluginOrType(this->ctx);
  }

  if(ULK(plugin_or_type == PluginOrType::invalid))
  {
    return false;
  }

  TypeOrConvertOrBegin type_or_convert_or_begin =
    TypeOrConvertOrBegin::invalid;

  do
  {
    if(ULK(!whitespace(this->ctx)))
    {
      return false;
    }

    switch(fieldOrRingOrPluginKws(this->ctx))
    {
    case FieldOrRingOrPlugin::invalid:
    {
      return false;
    }
    case FieldOrRingOrPlugin::field:
    {
      Number_T prime;
      if(ULK(ULK(ULK(ULK(!whitespace(this->ctx))
          || ULK(!number(this->ctx, &prime))) || ULK(!whitespace(this->ctx)))
          || ULK(!semiColonOp(this->ctx))))
      {
        return false;
      }
      this->types.emplace_back(std::move(prime));

      break;
    }
    case FieldOrRingOrPlugin::ring:
    {
      size_t bit_width;
      if(ULK(ULK(ULK(ULK(!whitespace(this->ctx))
          || ULK(!number(this->ctx, &bit_width)))
          || ULK(!whitespace(this->ctx))) || ULK(!semiColonOp(this->ctx))))
      {
        return false;
      }
      this->types.emplace_back(bit_width);

      break;
    }
    case FieldOrRingOrPlugin::plugin:
    {
      wtk::circuit::PluginBinding<Number_T> binding;
      if(ULK(!parsePluginBinding(this->ctx, &binding)))
      {
        return false;
      }
      this->types.emplace_back(std::move(binding));

      break;
    }
    }

    if(ULK(!whitespace(this->ctx)))
    {
      return false;
    }

    type_or_convert_or_begin = typeOrConvertOrBeginKws(this->ctx);
  } while(type_or_convert_or_begin == TypeOrConvertOrBegin::type);

  if(ULK(type_or_convert_or_begin == TypeOrConvertOrBegin::invalid))
  {
    return false;
  }
  else if(type_or_convert_or_begin == TypeOrConvertOrBegin::convert)
  {
    ConvertOrBegin convert_or_begin = ConvertOrBegin::invalid;
    do
    {
      type_idx out_type = 0;
      size_t out_len = 0;
      type_idx in_type = 0;
      size_t in_len = 0;
      if(ULK(ULK(!whitespace(this->ctx)) || ULK(!lparenOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!outKw(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!colonOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!number(this->ctx, &out_type))
          || ULK(!whitespace(this->ctx)) || ULK(!colonOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!number(this->ctx, &out_len))
          || ULK(!whitespace(this->ctx)) || ULK(!commaOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!inKw(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!colonOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!number(this->ctx, &in_type))
          || ULK(!whitespace(this->ctx)) || ULK(!colonOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!number(this->ctx, &in_len))
          || ULK(!whitespace(this->ctx)) || ULK(!rparenOp(this->ctx))
          || ULK(!whitespace(this->ctx)) || ULK(!semiColonOp(this->ctx))))
      {
        return false;
      }

      this->conversions.emplace_back(out_type, out_len, in_type, in_len);

      if(ULK(!whitespace(this->ctx)))
      {
        return false;
      }

      convert_or_begin = convertOrBeginKws(this->ctx);
    } while(convert_or_begin == ConvertOrBegin::convert);

    if(ULK(convert_or_begin == ConvertOrBegin::invalid))
    {
      return false;
    }
  }

  return true;
}

template<typename Number_T>
bool CircuitParser<Number_T>::parse(
    wtk::circuit::Handler<Number_T>* const handler)
{
  return parseTopScope(ctx, handler);
}

template<typename Number_T>
bool InputStream<Number_T>::parseStreamHeader()
{
  if(!whitespace(this->ctx) || !typeKw(this->ctx) || !whitespace(this->ctx))
  {
    return false;
  }

  FieldOrRingKws kw = fieldOrRingKws(this->ctx);

  switch(kw)
  {
  case FieldOrRingKws::invalid:
  {
    return false;
  }
  case FieldOrRingKws::field:
  {
    Number_T prime = 0;
    if(!whitespace(this->ctx) || !number(this->ctx, &prime)
        || !whitespace(this->ctx) || !semiColonOp(this->ctx)
        || !whitespace(this->ctx) || !beginKw(this->ctx)
        || !whitespace(this->ctx))
    {
      return false;
    }

    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(std::move(prime)));
    break;
  }
  case FieldOrRingKws::ring:
  {
    size_t bit_width = 0;
    if(!whitespace(this->ctx) || !number(this->ctx, &bit_width)
        || !whitespace(this->ctx) || !semiColonOp(this->ctx)
        || !whitespace(this->ctx) || !beginKw(this->ctx)
        || !whitespace(this->ctx))
    {
      return false;
    }

    this->type = std::unique_ptr<wtk::circuit::TypeSpec<Number_T>>(
        new wtk::circuit::TypeSpec<Number_T>(bit_width));
    break;
  }
  }

  return true;
}

template<typename Number_T>
wtk::StreamStatus InputStream<Number_T>::next(Number_T* num)
{
  switch(lchevronOrEnd(this->ctx))
  {
  case LchevronOrEnd::invalid: { return wtk::StreamStatus::error; }
  case LchevronOrEnd::lchevron:
  {
    this->line = this->ctx->lineNum;
    if(ULK(ULK(ULK(ULK(ULK(ULK(
        ULK(!whitespace(this->ctx)) || ULK(!number(this->ctx, num)))
        || ULK(!whitespace(this->ctx))) || ULK(!rchevronOp(this->ctx)))
        || ULK(!whitespace(this->ctx))) || ULK(!semiColonOp(this->ctx)))
        || ULK(!whitespace(this->ctx))))
    {
      return wtk::StreamStatus::error;
    }

    return wtk::StreamStatus::success;
  }
  case LchevronOrEnd::end:
  {
    this->line = this->ctx->lineNum;
    whitespace(this->ctx);
    return wtk::StreamStatus::end;
  }
  }

  // Unreachable?
  return wtk::StreamStatus::error;
}

template<typename Number_T>
size_t InputStream<Number_T>::lineNum()
{
  return this->line;
}

} } // namespace wtk::irregular

#undef ULK
