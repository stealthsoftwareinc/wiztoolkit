/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRESS_TEXT_PRINTER_H_
#define WTK_PRESS_TEXT_PRINTER_H_

#include <cstddef>
#include <cstdint>
#include <cinttypes>

#include <wtk/indexes.h>

#include <wtk/Parser.h>
#include <wtk/circuit/Handler.h>
#include <wtk/circuit/Data.h>

#include <wtk/press/Printer.h>
#include <wtk/utils/NumUtils.h>

#define LOG_IDENTIFIER "wtk::press"
#include <stealth_logging.h>

namespace wtk {
namespace press {

#define PRINT_HELPER(fmt, ...) do { \
  if(0 > fprintf(this->file, fmt "%s", __VA_ARGS__)) \
  { \
    log_perror(); \
    return false; \
  } } while(false)

#define PRINT(...) PRINT_HELPER(__VA_ARGS__, "")
#define PRINTLN(...) PRINT_HELPER(__VA_ARGS__, "\n")

template<typename Number_T>
class TextPrinter : public wtk::press::Printer<Number_T>
{
  FILE* file;

public:
  bool open(FILE* f)
  {
    file = f;
    return true;
  }

  bool printHeader(
      size_t const major, size_t const minor, size_t const patch,
      char const* const extra,
      wtk::ResourceType const resource) override
  {
    if(extra == nullptr || extra[0] == '\0')
    {
      PRINTLN("version %zu.%zu.%zu;", major, minor, patch);
    }
    else
    {
      PRINTLN("version %zu.%zu.%zu-%s;", major, minor, patch, extra);
    }

    switch(resource)
    {
      case wtk::ResourceType::circuit:
      {
        PRINTLN("circuit;");
        break;
      }
      case wtk::ResourceType::public_in:
      {
        PRINTLN("public_input;");
        break;
      }
      case wtk::ResourceType::private_in:
      {
        PRINTLN("private_input;");
        break;
      }
      case wtk::ResourceType::translation:
      {
        PRINTLN("translation;");
        break;
      }
      case wtk::ResourceType::configuration:
      {
        PRINTLN("configuration;");
        break;
      }
    }

    return true;
  }

  bool printPluginDecl(char const* const plugin_name) override
  {
    PRINTLN("@plugin %s;", plugin_name);
    return true;
  }

  bool printFieldType(Number_T const prime) override
  {
    PRINTLN("@type field %s;", wtk::utils::short_str(prime).c_str());
    return true;
  }

  bool printRingType(size_t const bit_width) override
  {
    PRINTLN("@type ring %s;", wtk::utils::short_str(bit_width).c_str());
    return true;
  }

  bool printPluginBinding(
      wtk::circuit::PluginBinding<Number_T> const* const plugin)
  {
    PRINT("@plugin(%s,%s", plugin->name.c_str(), plugin->operation.c_str());

    for(size_t i = 0; i < plugin->parameters.size(); i++)
    {
      switch(plugin->parameters[i].form)
      {
      case wtk::circuit::PluginBinding<Number_T>::Parameter::numeric:
      {
        PRINT(",%s",
            wtk::utils::short_str(plugin->parameters[i].number).c_str());
        break;
      }
      case wtk::circuit::PluginBinding<Number_T>::Parameter::textual:
      {
        PRINT(",%s", plugin->parameters[i].text.c_str());
        break;
      }
      }
    }

    bool has_privates = false;
    for(size_t i = 0; i < plugin->privateInputCount.size(); i++)
    {
      if(plugin->privateInputCount[i] != 0) { has_privates = true; }
    }

    if(has_privates)
    {
      PRINT(",@private:");
      char const* comma = "";
      for(size_t i = 0; i < plugin->privateInputCount.size(); i++)
      {
        if(plugin->privateInputCount[i] != 0)
        {
          PRINT("%s%zu:%zu", comma, i, plugin->privateInputCount[i]);
          comma = ",";
        }
      }
    }

    bool has_publics = false;
    for(size_t i = 0; i < plugin->publicInputCount.size(); i++)
    {
      if(plugin->publicInputCount[i] != 0) { has_publics = true; }
    }

    if(has_publics)
    {
      PRINT(",@public:");
      char const* comma = "";
      for(size_t i = 0; i < plugin->publicInputCount.size(); i++)
      {
        if(plugin->publicInputCount[i] != 0)
        {
          PRINT("%s%zu:%zu", comma, i, plugin->publicInputCount[i]);
          comma = ",";
        }
      }
    }

    PRINTLN(");");
    return true;
  }

  bool printPluginType(
      wtk::circuit::PluginBinding<Number_T> const* const plugin) override
  {
    PRINT("@type ");
    return this->printPluginBinding(plugin);
  }

  bool printConversionSpec(
      wtk::circuit::ConversionSpec const* const conversion) override
  {
    PRINTLN("@convert(@out: %zu:%zu, @in: %zu:%zu);",
        (size_t) conversion->outType, conversion->outLength,
        (size_t) conversion->inType, conversion->inLength);
    return true;
  }

  bool printBeginKw() override
  {
    PRINTLN("@begin");
    return true;
  }

  bool printEndKw() override
  {
    PRINTLN("@end");
    return true;
  }

  bool printStreamValue(Number_T const& val) override
  {
    PRINTLN("<%s>;", wtk::utils::short_str(val).c_str());
    return true;
  }

  bool addGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@add($%s,$%s);",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }
    else
    {
      PRINTLN("$%s<-@add(%" PRIu8 ":$%s,$%s);",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }

    return true;
  }

  bool mulGate(wire_idx const out,
      wire_idx const left, wire_idx const right, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@mul($%s,$%s);",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }
    else
    {
      PRINTLN("$%s<-@mul(%" PRIu8 ":$%s,$%s);",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }

    return true;
  }

  bool addcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@addc($%s,<%s>);",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }
    else
    {
      PRINTLN("$%s<-@addc(%" PRIu8 ":$%s,<%s>);",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }

    return true;
  }

  bool mulcGate(wire_idx const out,
      wire_idx const left, Number_T&& right, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@mulc($%s,<%s>);",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }
    else
    {
      PRINTLN("$%s<-@mulc(%" PRIu8 ":$%s,<%s>);",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str(),
          wtk::utils::short_str(right).c_str());
    }

    return true;
  }

  bool copy(
      wire_idx const out, wire_idx const left, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-$%s;",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str());
    }
    else
    {
      PRINTLN("$%s<-%" PRIu8 ":$%s;",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str());
    }

    return true;
  }

  bool copyMulti(wtk::circuit::CopyMulti* multi) override
  {
    if(multi->outputs.first == multi->outputs.last)
    {
      PRINT("$%s<-", wtk::utils::short_str(multi->outputs.first).c_str());
    }
    else
    {
      PRINT("$%s...$%s<-",
          wtk::utils::short_str(multi->outputs.first).c_str(),
          wtk::utils::short_str(multi->outputs.last).c_str());
    }

    size_t constexpr TYPE_SPACE_SIZE = 5; // max 3 digits, colon, nullterm
    char type_space[TYPE_SPACE_SIZE];
    char const* comma;
    if(multi->type == 0) { comma = ""; }
    else
    {
      snprintf(type_space, TYPE_SPACE_SIZE, "%" PRIu8 ":", multi->type);
      comma = type_space;
    }

    for(size_t i = 0; i < multi->inputs.size(); i++)
    {
      if(multi->inputs[i].first == multi->inputs[i].last)
      {
        PRINT("%s$%s", comma,
            wtk::utils::short_str(multi->inputs[i].first).c_str());
      }
      else
      {
        PRINT("%s$%s...$%s", comma,
            wtk::utils::short_str(multi->inputs[i].first).c_str(),
            wtk::utils::short_str(multi->inputs[i].last).c_str());
      }

      comma = ",";
    }

    PRINT(";\n");

    return true;
  }

  bool assign(
      wire_idx const out, Number_T&& left, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-<%s>;",
          wtk::utils::short_str(out).c_str(),
          wtk::utils::short_str(left).c_str());
    }
    else
    {
      PRINTLN("$%s<-%" PRIu8 ":<%s>;",
          wtk::utils::short_str(out).c_str(), type,
          wtk::utils::short_str(left).c_str());
    }

    return true;
  }

  virtual bool assertZero(wire_idx const left, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("@assert_zero($%s);",
          wtk::utils::short_str(left).c_str());
    }
    else
    {
      PRINTLN("@assert_zero(%" PRIu8 ":$%s);", type,
          wtk::utils::short_str(left).c_str());
    }

    return true;
  }

  bool publicIn(wire_idx const out, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@public();", wtk::utils::short_str(out).c_str());
    }
    else
    {
      PRINTLN("$%s<-@public(%" PRIu8 ");",
          wtk::utils::short_str(out).c_str(), type);
    }

    return true;
  }

  bool publicInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    if(type == 0)
    {
      if(out->first == out->last)
      {
        PRINTLN("$%s<-@public();", wtk::utils::short_str(out->first).c_str());
      }
      else
      {
        PRINTLN("$%s...$%s<-@public();",
            wtk::utils::short_str(out->first).c_str(),
            wtk::utils::short_str(out->last).c_str());
      }
    }
    else
    {
      if(out->first == out->last)
      {
        PRINTLN("$%s<-@public(%" PRIu8 ");",
            wtk::utils::short_str(out->first).c_str(), type);
      }
      else
      {
        PRINTLN("$%s...$%s<-@public(%" PRIu8 ");",
            wtk::utils::short_str(out->first).c_str(),
            wtk::utils::short_str(out->last).c_str(), type);
      }
    }

    return true;
  }

  bool privateIn(wire_idx const out, type_idx const type) override
  {
    if(type == 0)
    {
      PRINTLN("$%s<-@private();", wtk::utils::short_str(out).c_str());
    }
    else
    {
      PRINTLN("$%s<-@private(%" PRIu8 ");",
          wtk::utils::short_str(out).c_str(), type);
    }

    return true;
  }

  bool privateInMulti(wtk::circuit::Range* out, type_idx const type) override
  {
    if(type == 0)
    {
      if(out->first == out->last)
      {
        PRINTLN("$%s<-@private();", wtk::utils::short_str(out->first).c_str());
      }
      else
      {
        PRINTLN("$%s...$%s<-@private();",
            wtk::utils::short_str(out->first).c_str(),
            wtk::utils::short_str(out->last).c_str());
      }
    }
    else
    {
      if(out->first == out->last)
      {
        PRINTLN("$%s<-@private(%" PRIu8 ");",
            wtk::utils::short_str(out->first).c_str(), type);
      }
      else
      {
        PRINTLN("$%s...$%s<-@private(%" PRIu8 ");",
            wtk::utils::short_str(out->first).c_str(),
            wtk::utils::short_str(out->last).c_str(), type);
      }
    }

    return true;
  }

  bool convert(
      wire_idx const first_out, wire_idx const last_out,
      type_idx const out_type,
      wire_idx const first_in, wire_idx const last_in,
      type_idx const in_type, bool modulus) override
  {
    if(first_out == last_out)
    {
      PRINT("%" PRIu8 ":$%s", out_type,
          wtk::utils::short_str(first_out).c_str());
    }
    else
    {
      PRINT("%" PRIu8 ":$%s...$%s", out_type,
          wtk::utils::short_str(first_out).c_str(),
          wtk::utils::short_str(last_out).c_str());
    }

    PRINT("<-@convert(%" PRIu8 ":", in_type);

    if(first_in == last_in)
    {
      PRINT("$%s", wtk::utils::short_str(first_in).c_str());
    }
    else
    {
      PRINT("$%s...$%s", wtk::utils::short_str(first_in).c_str(),
          wtk::utils::short_str(last_in).c_str());
    }

    if(modulus)
    {
      PRINTLN(",@modulus);");
    }
    else
    {
      PRINTLN(");");
    }

    return true;
  }

  bool newRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    if(type == 0) { PRINT("@new("); }
    else          { PRINT("@new(%" PRIu8 ":", type); }

    PRINTLN("$%s...$%s);", wtk::utils::short_str(first).c_str(),
        wtk::utils::short_str(last).c_str());

    return true;
  }

  bool deleteRange(
      wire_idx const first, wire_idx const last, type_idx const type) override
  {
    if(type == 0) { PRINT("@delete("); }
    else          { PRINT("@delete(%" PRIu8 ":", type); }

    PRINTLN("$%s...$%s);", wtk::utils::short_str(first).c_str(),
        wtk::utils::short_str(last).c_str());

    return true;
  }

  bool startFunction(wtk::circuit::FunctionSignature&& signature) override
  {
    PRINT("@function(%s", signature.name.c_str());

    if(signature.outputs.size() != 0)
    {
      PRINT(",@out:");
      char const* comma = "";
      for(size_t i = 0; i < signature.outputs.size(); i++)
      {
        PRINT("%s%" PRIu8 ":%s", comma, signature.outputs[i].type,
            wtk::utils::short_str(signature.outputs[i].length).c_str());
        comma = ",";
      }
    }

    if(signature.inputs.size() != 0)
    {
      PRINT(",@in:");
      char const* comma = "";
      for(size_t i = 0; i < signature.inputs.size(); i++)
      {
        PRINT("%s%" PRIu8 ":%s", comma, signature.inputs[i].type,
            wtk::utils::short_str(signature.inputs[i].length).c_str());
        comma = ",";
      }
    }

    PRINT(")\n");

    return true;
  }

  bool regularFunction() override { return true; }

  bool endFunction() override { PRINTLN("@end"); return true; }

  bool pluginFunction(wtk::circuit::PluginBinding<Number_T>&& binding) override
  {
    return this->printPluginBinding(&binding);
  }

  bool invoke(wtk::circuit::FunctionCall* const call) override
  {
    if(call->outputs.size() != 0)
    {
      char const* comma = "";
      for(size_t i = 0; i < call->outputs.size(); i++)
      {
        if(call->outputs[i].first == call->outputs[i].last)
        {
          PRINT("%s$%s", comma,
              wtk::utils::short_str(call->outputs[i].first).c_str());
        }
        else
        {
          PRINT("%s$%s...$%s", comma,
              wtk::utils::short_str(call->outputs[i].first).c_str(),
              wtk::utils::short_str(call->outputs[i].last).c_str());
        }
        comma = ",";
      }

      PRINT("<-");
    }

    PRINT("@call(%s", call->name.c_str());

    if(call->inputs.size() != 0)
    {
      for(size_t i = 0; i < call->inputs.size(); i++)
      {
        if(call->inputs[i].first == call->inputs[i].last)
        {
          PRINT(",$%s", wtk::utils::short_str(call->inputs[i].first).c_str());
        }
        else
        {
          PRINT(",$%s...$%s",
              wtk::utils::short_str(call->inputs[i].first).c_str(),
              wtk::utils::short_str(call->inputs[i].last).c_str());
        }
      }
    }

    PRINTLN(");");
    return true;
  }
};

#undef PRINT_HELPER
#undef PRINT
#undef PRINTLN

} } // namespace wtk::press

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_PRESS_TEXT_PRINTER_H_
