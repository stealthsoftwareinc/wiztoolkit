/**
 * Copyright (C) 2022 Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#define WTK_NAILS_ENABLE_TRACES

#include <wtk/versions.h>

#include <openssl/bn.h>
#include <sst/catalog/bignum.hpp>

#include <wtk/Parser.h>
#include <wtk/circuit/Parser.h>
#include <wtk/circuit/Data.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/utils/ParserOrganizer.h>
#include <wtk/utils/Pool.h>

#include <wtk/irregular/Parser.h>
#include <wtk/flatbuffer/Parser.h>
#include <wtk/firealarm/FieldBackend.h>
#include <wtk/firealarm/Converter.h>
#include <wtk/firealarm/Wire.h>
#include <wtk/firealarm/Counters.h>
#include <wtk/firealarm/RAM.h>

#include <wtk/nails/Handler.h>
#include <wtk/nails/Interpreter.h>
#include <wtk/nails/IterPlugin.h>

#include <wtk/plugins/Plugin.h>
#include <wtk/plugins/Vectors.h>
#include <wtk/plugins/Multiplexer.h>
#include <wtk/plugins/ExtendedArithmetic.h>

#define LOG_IDENTIFIER "wtk-firealarm"
#include <stealth_logging.h>

void print_version()
{
  printf("WizToolKit wtk-firealarm\n");
  printf("FIREALARM: Friendly IR Evaluator And Logic Assertion and Rejection "
      "Machine\n\n");

  printf("WizToolKit Version: %s\n",
      wtk::utils::stringify_version(wtk::WTK_VERSION_MAJOR,
        wtk::WTK_VERSION_MINOR, wtk::WTK_VERSION_PATCH,
        wtk::WTK_VERSION_EXTRA).c_str());
  printf("SIEVE IR Version:   %s\n",
      wtk::utils::stringify_version(wtk::IR_VERSION_MAJOR,
        wtk::IR_VERSION_MINOR, wtk::IR_VERSION_PATCH,
        wtk::IR_VERSION_EXTRA).c_str());

  printf("\nCopyright (C) 2022 Stealth Software Technologies, Inc.\n");
}

void print_help()
{
  print_version();
  printf("\nwtk-firealarm is a tool for checking the validity of SIEVE IR "
      "resources. Given\neither a single relation, it can check resource "
      "validity, or given all three\nresources (relation, instance, witness) "
      "together, it checks evaluation validity\n(evaluate the circuit).\n\n");

  printf("USAGE:\n");
  printf("  wtk-firealarm [options] [resources]...\n\n");

  printf("OPTIONS:\n");
  printf("  -f        Use the flatbuffer binary parser.");
  printf("  -t        Produce a short trace which lists function boundaries, "
      "function\n            inputs/outputs, and assertions.\n");
  printf("  -T        Produce a detailed trace (short trace and the extended "
      "witness).\n");
  printf("  -d        Include details when reporting on gate counts.\n");
  printf("  --fallback-ram\n"
         "            Use the fallback RAM plugin (default: firealarm RAM)\n");
  printf("  --help\n");
  printf("  -h        Print this help text.\n");
  printf("  --version\n");
  printf("  -v        Print the version information.\n\n");

  printf("RESOURCES:\n");
  printf("One resource may be provided at a time in order to check for "
      "resource\nvalidity. Or multiple may be checked together for "
      "evaluation validity.\n\n");
  printf("  circuit: A Circuit-IR relation, exclusive with translation\n");
  printf("  translation: A Translation-IR relation, exclusive with circuit "
      "(unimplemented)\n");
  printf("  public_input: A public input stream, must be paired with a "
      "private input\n      stream of the same field, and the field must "
      "match one from the relation.\n");
  printf("  private_input: A private input stream, must be paired with a "
      "public input \n      stream of the same field, and the field must match "
      "one from the relation.\n\n");

  printf("WizToolKit is no longer sensitive resource file suffixes.\n");
}

// ==== Command line arguments ====

// flags to enable additional reporting modes
bool short_trace_flag = false;
bool detail_trace_flag = false;
bool detail_counts = false;

// list of resources to open
std::vector<char const*> resource_names;

// version/help flags
bool print_version_flag = false;
bool print_help_flag = false;

// flag to use fallback RAM instead of firealarm RAM
bool fallback_ram_flag = false;

// flag to use the flatbuffer parser instead of irregular
bool flatbuffer_flag = false;

// Function to read the arguments
void read_arguments(int argc, char const* argv[])
{
  for(size_t i = 1; i < (size_t) argc; i++)
  {
    if(0 == strcmp(argv[i], "-t"))
    {
      short_trace_flag = true;
    }
    else if(0 == strcmp(argv[i], "-T"))
    {
      short_trace_flag = true;
      detail_trace_flag = true;
    }
    else if(0 == strcmp(argv[i], "-d"))
    {
      detail_counts = true;
    }
    else if(0 == strcmp(argv[i], "-v") || 0 == strcmp(argv[i], "--version"))
    {
      print_version_flag = true;
    }
    else if(0 == strcmp(argv[i], "-h") || 0 == strcmp(argv[i], "--help"))
    {
      print_help_flag = true;
    }
    else if(0 == strcmp(argv[i], "--fallback-ram"))
    {
      fallback_ram_flag = true;
    }
    else if(0 == strcmp(argv[i], "-f"))
    {
      flatbuffer_flag = true;
    }
    else
    {
      resource_names.emplace_back(argv[i]);
    }
  }
}

// ==== Memory management for the highly flexible FIREALARM setup ====

// (most ZK backends probably don't need as much flexibility and could
// have simpler setup)

// Enumeration of bit-width/precision for Wire_T arguments.
enum class Precision
{
  unlimited,
  uint64,
  uint8,
  ram_unlimited,
  ram_uint64,
  ram_uint8,
  bool_ram,
  fallback_ram_unlimited,
  fallback_ram_uint64,
  fallback_ram_uint8,
  fallback_bool_ram_uint8
};

// Memory manager for TypeBackends within FIREALARM
struct TypeManager
{
  bool suppressAsserts;

  TypeManager(bool sa) : suppressAsserts(sa) { }

  bool trace = false;
  wtk::utils::Indent const* indent = nullptr;

  void enableTrace(wtk::utils::Indent const* const idt)
  {
    this->trace = true;
    this->indent = idt;
  }

  // pool allocation
  wtk::utils::Pool<wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>, 1> unlimited;
  wtk::utils::Pool<wtk::firealarm::FieldBackend<sst::bignum, uint64_t>, 1> uint64;
  wtk::utils::Pool<wtk::firealarm::FieldBackend<sst::bignum, uint8_t>, 1> uint8;
  wtk::utils::Pool<wtk::firealarm::RAMBackend<sst::bignum, sst::bignum>, 1> ramUnlimited;
  wtk::utils::Pool<wtk::firealarm::RAMBackend<sst::bignum, uint64_t>, 1> ramUint64;
  wtk::utils::Pool<wtk::firealarm::RAMBackend<sst::bignum, uint8_t>, 1> ramUint8;
  wtk::utils::Pool<wtk::firealarm::BoolRAMBackend<sst::bignum, uint8_t>, 1> boolRam;

  // pool allocation for fallback RAM (option)
  wtk::utils::Pool<wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<sst::bignum>>, 1> fallbackRamUnlimited;
  wtk::utils::Pool<wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint64_t>>, 1> fallbackRamUint64;
  wtk::utils::Pool<wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint8_t>>, 1> fallbackRamUint8;
  wtk::utils::Pool<wtk::plugins::FallbackBoolRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint8_t>>, 1> fallbackBoolRamUint8;

  // reference to a backend. precision is indicated, but type is erased
  struct TypeRef
  {
    Precision const precision;

    wtk::TypeBackendEraser<sst::bignum>* const erasedType;

    TypeRef(Precision const pr, wtk::TypeBackendEraser<sst::bignum>* const et)
      : precision(pr), erasedType(et) { }
  };

  // List of all backends, indexed in order of declaration
  std::vector<TypeRef> typeRefs;

  // make a backend with unlimited precision wire
  wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>* makeUnlimited(
      char const* const f_name,
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      wtk::firealarm::TypeCounter* const ctr)
  {
    wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>* ret =
      this->unlimited.allocate(1, f_name, type, ctr, this->suppressAsserts);
    this->typeRefs.emplace_back(Precision::unlimited, ret);

    if(this->trace)
    {
      ret->enableTrace(this->indent);
    }

    return ret;
  }

  // make a backend with 64-bit wire
  wtk::firealarm::FieldBackend<sst::bignum, uint64_t>* makeUint64(
      char const* const f_name,
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      wtk::firealarm::TypeCounter* const ctr)
  {
    wtk::firealarm::FieldBackend<sst::bignum, uint64_t>* ret =
      this->uint64.allocate(1, f_name, type, ctr, this->suppressAsserts);
    this->typeRefs.emplace_back(Precision::uint64, ret);

    if(this->trace)
    {
      ret->enableTrace(this->indent);
    }

    return ret;
  }

  // make a backend with 8-bit wire
  wtk::firealarm::FieldBackend<sst::bignum, uint8_t>* makeUint8(
      char const* const f_name,
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      wtk::firealarm::TypeCounter* const ctr)
  {
    wtk::firealarm::FieldBackend<sst::bignum, uint8_t>* ret =
      this->uint8.allocate(1, f_name, type, ctr, this->suppressAsserts);
    this->typeRefs.emplace_back(Precision::uint8, ret);

    if(this->trace)
    {
      ret->enableTrace(this->indent);
    }

    return ret;
  }

  // make a RAM backend with unlimited precision wire
  wtk::firealarm::RAMBackend<sst::bignum, sst::bignum>* makeRAMUnlimited(
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      char const* const f_name, wtk::type_idx const idx_type,
      wtk::firealarm::TypeCounter* const counter)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::unlimited);

    wtk::firealarm::RAMBackend<sst::bignum, sst::bignum>* ret =
      this->ramUnlimited.allocate(1, f_name, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>*>(
            this->typeRefs[(size_t) idx_type].erasedType), counter, type);
    this->typeRefs.emplace_back(Precision::ram_unlimited, ret);
    return ret;
  }

  // make a RAM backend with 64-bit wire
  wtk::firealarm::RAMBackend<sst::bignum, uint64_t>* makeRAMUint64(
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      char const* const f_name, wtk::type_idx const idx_type,
      wtk::firealarm::TypeCounter* const counter)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint64);

    wtk::firealarm::RAMBackend<sst::bignum, uint64_t>* ret =
      this->ramUint64.allocate(1, f_name, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint64_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType), counter, type);
    this->typeRefs.emplace_back(Precision::ram_uint64, ret);
    return ret;
  }

  // make a RAM backend with 8-bit wire
  wtk::firealarm::RAMBackend<sst::bignum, uint8_t>* makeRAMUint8(
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      char const* const f_name, wtk::type_idx const idx_type,
      wtk::firealarm::TypeCounter* const counter)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint8);

    wtk::firealarm::RAMBackend<sst::bignum, uint8_t>* ret =
      this->ramUint8.allocate(1, f_name, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint8_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType), counter, type);
    this->typeRefs.emplace_back(Precision::ram_uint8, ret);
    return ret;
  }

  // make a Bool RAM backend with 8-bit wire
  wtk::firealarm::BoolRAMBackend<sst::bignum, uint8_t>* makeBoolRAM(
      wtk::circuit::TypeSpec<sst::bignum> const* const type,
      char const* const f_name, wtk::type_idx const idx_type,
      wtk::wire_idx const idx_bits, wtk::wire_idx const elt_bits,
      wtk::firealarm::TypeCounter* const counter)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint8);

    wtk::firealarm::BoolRAMBackend<sst::bignum, uint8_t>* ret =
      this->boolRam.allocate(1, f_name, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint8_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType),
          idx_bits, elt_bits, counter, type);
    this->typeRefs.emplace_back(Precision::bool_ram, ret);
    return ret;
  }

  // make a Fallback RAM backend with unlimited precision wire
  wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<sst::bignum>>*
    makeFallbackRAMUnlimited(
        wtk::circuit::TypeSpec<sst::bignum> const* const type,
        wtk::type_idx const idx_type)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::unlimited);

    wtk::plugins::FallbackRAMBackend<
      sst::bignum, wtk::firealarm::Wire<sst::bignum>>* ret =
      this->fallbackRamUnlimited.allocate(1, type, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>*>(
            this->typeRefs[(size_t) idx_type].erasedType));
    this->typeRefs.emplace_back(Precision::fallback_ram_unlimited, ret);
    return ret;
  }

  // make a Fallback RAM backend with uint64 precision wire
  wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint64_t>>*
    makeFallbackRAMUint64(
        wtk::circuit::TypeSpec<sst::bignum> const* const type,
        wtk::type_idx const idx_type)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint64);

    wtk::plugins::FallbackRAMBackend<
      sst::bignum, wtk::firealarm::Wire<uint64_t>>* ret =
      this->fallbackRamUint64.allocate(1, type, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint64_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType));
    this->typeRefs.emplace_back(Precision::fallback_ram_uint64, ret);
    return ret;
  }

  // make a Fallback RAM backend with uint8 precision wire
  wtk::plugins::FallbackRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint8_t>>*
    makeFallbackRAMUint8(
        wtk::circuit::TypeSpec<sst::bignum> const* const type,
        wtk::type_idx const idx_type)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint8);

    wtk::plugins::FallbackRAMBackend<
      sst::bignum, wtk::firealarm::Wire<uint8_t>>* ret =
      this->fallbackRamUint8.allocate(1, type, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint8_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType));
    this->typeRefs.emplace_back(Precision::fallback_ram_uint8, ret);
    return ret;
  }

  // make a Fallback Bool RAM backend with uint8 precision wire
  wtk::plugins::FallbackBoolRAMBackend<
    sst::bignum, wtk::firealarm::Wire<uint8_t>>*
    makeFallbackBoolRAM(
        wtk::circuit::TypeSpec<sst::bignum> const* const type,
        wtk::type_idx const idx_type, wtk::wire_idx const idx_bits,
        wtk::wire_idx const elt_bits)
  {
    log_assert(idx_type < this->typeRefs.size()
        && this->typeRefs[(size_t) idx_type].precision == Precision::uint8);

    wtk::plugins::FallbackBoolRAMBackend<
      sst::bignum, wtk::firealarm::Wire<uint8_t>>* ret =
      this->fallbackBoolRamUint8.allocate(1, type, idx_type,
          static_cast<wtk::firealarm::FieldBackend<sst::bignum, uint8_t>*>(
            this->typeRefs[(size_t) idx_type].erasedType), idx_bits, elt_bits);
    this->typeRefs.emplace_back(Precision::fallback_bool_ram_uint8, ret);
    return ret;
  }

  /* ==== pool allocators for converters ==== */

  // out type: unlimited, in type: *
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, sst::bignum, sst::bignum>, 1> convertUnlimitedUnlimited;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, sst::bignum, uint64_t>, 1> convertUnlimitedUint64;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, sst::bignum, uint8_t>, 1> convertUnlimitedUint8;

  // out type: uint64, in type: *
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint64_t, sst::bignum>, 1> convertUint64Unlimited;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint64_t, uint64_t>, 1> convertUint64Uint64;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint64_t, uint8_t>, 1> convertUint64Uint8;

  // out type: uint8, in type: *
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint8_t, sst::bignum>, 1> convertUint8Unlimited;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint8_t, uint64_t>, 1> convertUint8Uint64;
  wtk::utils::Pool<wtk::firealarm::Converter<
    sst::bignum, uint8_t, uint8_t>, 1> convertUint8Uint8;
};

template<typename Parser_T>
int submain(wtk::utils::ParserOrganizer<Parser_T, sst::bignum>& parsers);

int main(int argc, char const* argv[])
{
  read_arguments(argc, argv);

  if(print_help_flag)
  {
    print_help();
    return 0;
  }
  else if(print_version_flag)
  {
    print_version();
    return 0;
  }
  else
  {
    // start by parsing/organizing all the resources
    if(flatbuffer_flag)
    {
      wtk::utils::ParserOrganizer<
        wtk::flatbuffer::Parser<sst::bignum>, sst::bignum> parsers;

      return submain(parsers);
    }
    else
    {
      wtk::utils::ParserOrganizer<
        wtk::irregular::Parser<sst::bignum>, sst::bignum> parsers;

      return submain(parsers);
    }
  }
}

template<typename Parser_T>
int submain(wtk::utils::ParserOrganizer<Parser_T, sst::bignum>& parsers)
{
  for(size_t i = 0; i < resource_names.size(); i++)
  {
    if(!parsers.open(resource_names[i])) { return 1; }
  }

  wtk::utils::Setting setting = parsers.organize();
  bool suppress_asserts = false;
  if(setting == wtk::utils::Setting::failure) { return 1; }
  else if(setting == wtk::utils::Setting::verifier)
  {
    log_warn("Running wtk-firealarm in the verifier setting will suppress "
        "@assert_zero gates to avoid false positives.");
    suppress_asserts = true;
  }
  else if(setting == wtk::utils::Setting::preprocess)
  {
    suppress_asserts = true;
  }

  // Counters for current/maximum active wire reporting.
  // These are pointed to/updated by all the FIREALARM backends
  size_t totalCurrentCount = 0;
  size_t totalMaximumCount = 0;

  // Allocate counters for each backend
  std::vector<wtk::firealarm::TypeCounter> counters;
  counters.reserve(parsers.circuitBodyParser->types.size());

  // NAILS Interpreter
  wtk::nails::Interpreter<sst::bignum> interpreter(parsers.circuitName);

  if(short_trace_flag) { interpreter.enableTrace(); }
  if(detail_trace_flag) { interpreter.enableTraceDetail(); }

  TypeManager manager(suppress_asserts);
  if(detail_trace_flag)
  {
    manager.enableTrace(&interpreter.indent);
  }

  // Plugins Manager
  wtk::plugins::PluginsManager<sst::bignum,
    wtk::firealarm::Wire<sst::bignum>,
    wtk::firealarm::Wire<uint64_t>,
    wtk::firealarm::Wire<uint8_t>,
    wtk::firealarm::RAMBuffer<sst::bignum>,
    wtk::firealarm::RAMBuffer<uint8_t>,
    wtk::firealarm::RAMBuffer<uint64_t>,
    wtk::plugins::FallbackRAMBuffer<wtk::firealarm::Wire<sst::bignum>>,
    wtk::plugins::FallbackRAMBuffer<wtk::firealarm::Wire<uint64_t>>,
    wtk::plugins::FallbackRAMBuffer<wtk::firealarm::Wire<uint8_t>>,
    wtk::plugins::FallbackBoolRAMBuffer<wtk::firealarm::Wire<uint8_t>>>
      plugins_manager;

  wtk::nails::MapOperation<sst::bignum> map_op(&interpreter);

  // load all the plugins indicated in the relation's header.
  for(size_t i = 0; i < parsers.circuitBodyParser->plugins.size(); i++)
  {
    if(detail_counts)
    {
      log_info("loading plugin: %s",
          parsers.circuitBodyParser->plugins[i].c_str());
    }

    if("wizkit_vectors" == parsers.circuitBodyParser->plugins[i])
    {
      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<sst::bignum>>> vector_bignum_plugin_ptr(
            new wtk::plugins::FallbackVectorPlugin<
            sst::bignum, wtk::firealarm::Wire<sst::bignum>>());
      plugins_manager.addPlugin(
          "wizkit_vectors", std::move(vector_bignum_plugin_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint64_t>>> vector_uint64_plugin_ptr(
            new wtk::plugins::FallbackVectorPlugin<
            sst::bignum, wtk::firealarm::Wire<uint64_t>>());
      plugins_manager.addPlugin(
          "wizkit_vectors", std::move(vector_uint64_plugin_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint8_t>>> vector_uint8_plugin_ptr(
            new wtk::plugins::FallbackVectorPlugin<
            sst::bignum, wtk::firealarm::Wire<uint8_t>>());
      plugins_manager.addPlugin(
          "wizkit_vectors", std::move(vector_uint8_plugin_ptr));
    }
    else if("ram_arith_v0" == parsers.circuitBodyParser->plugins[i]
        || "ram_arith_v1" == parsers.circuitBodyParser->plugins[i])
    {
      char const* const plugin_name =
        parsers.circuitBodyParser->plugins[i].c_str();
      if(fallback_ram_flag)
      {
        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<sst::bignum>>>> ram_bignum_plugin_ptr(
                new wtk::plugins::FallbackRAMPlugin<
                  sst::bignum, wtk::firealarm::Wire<sst::bignum>>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_bignum_plugin_ptr));

        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<uint64_t>>>> ram_uint64_plugin_ptr(
                new wtk::plugins::FallbackRAMPlugin<
                  sst::bignum, wtk::firealarm::Wire<uint64_t>>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_uint64_plugin_ptr));

        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<uint8_t>>>> ram_uint8_plugin_ptr(
                new wtk::plugins::FallbackRAMPlugin<
                  sst::bignum, wtk::firealarm::Wire<uint8_t>>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_uint8_plugin_ptr));
      }
      else
      {
        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::firealarm::RAMBuffer<sst::bignum>>> ram_bignum_plugin_ptr(
              new wtk::firealarm::RAMPlugin<sst::bignum, sst::bignum>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_bignum_plugin_ptr));

        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::firealarm::RAMBuffer<uint64_t>>> ram_uint64_plugin_ptr(
              new wtk::firealarm::RAMPlugin<sst::bignum, uint64_t>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_uint64_plugin_ptr));

        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::firealarm::RAMBuffer<uint8_t>>> ram_uint8_plugin_ptr(
              new wtk::firealarm::RAMPlugin<sst::bignum, uint8_t>());
        plugins_manager.addPlugin(
            plugin_name, std::move(ram_uint8_plugin_ptr));
      }
    }
    else if("ram_bool_v0" == parsers.circuitBodyParser->plugins[i])
    {
      if(fallback_ram_flag)
      {
        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::plugins::FallbackBoolRAMBuffer<
            wtk::firealarm::Wire<uint8_t>>>> bool_ram_plugin_ptr(
              new wtk::plugins::FallbackBoolRAMPlugin<
                sst::bignum, wtk::firealarm::Wire<uint8_t>>());
        plugins_manager.addPlugin(
            "ram_bool_v0", std::move(bool_ram_plugin_ptr));
      }
      else
      {
        std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
          wtk::firealarm::RAMBuffer<uint8_t>>> bool_ram_plugin_ptr(
              new wtk::firealarm::BoolRAMPlugin<sst::bignum, uint8_t>());
        plugins_manager.addPlugin(
            "ram_bool_v0", std::move(bool_ram_plugin_ptr));
      }
    }
    else if("iter_v0" == parsers.circuitBodyParser->plugins[i])
    {
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::Wire<sst::bignum>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::Wire<uint64_t>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::Wire<uint8_t>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::RAMBuffer<sst::bignum>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::RAMBuffer<uint64_t>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::firealarm::RAMBuffer<uint8_t>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::plugins::FallbackRAMBuffer<
          wtk::firealarm::Wire<sst::bignum>>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::plugins::FallbackRAMBuffer<
          wtk::firealarm::Wire<uint64_t>>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::plugins::FallbackRAMBuffer<
          wtk::firealarm::Wire<uint8_t>>>());
      plugins_manager.addPlugin("iter_v0",
          map_op.makePlugin<wtk::plugins::FallbackBoolRAMBuffer<
          wtk::firealarm::Wire<uint8_t>>>());
    }
    else if("mux_v0" == parsers.circuitBodyParser->plugins[i])
    {
      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<sst::bignum>>> multiplexer_bignum_plugin_ptr(
            new wtk::plugins::FallbackMultiplexerPlugin<
            sst::bignum, wtk::firealarm::Wire<sst::bignum>>());
      plugins_manager.addPlugin(
          "mux_v0", std::move(multiplexer_bignum_plugin_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint64_t>>> multiplexer_uint64_plugin_ptr(
            new wtk::plugins::FallbackMultiplexerPlugin<
            sst::bignum, wtk::firealarm::Wire<uint64_t>>());
      plugins_manager.addPlugin(
          "mux_v0", std::move(multiplexer_uint64_plugin_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint8_t>>> multiplexer_uint8_plugin_ptr(
            new wtk::plugins::FallbackMultiplexerPlugin<
            sst::bignum, wtk::firealarm::Wire<uint8_t>>());
      plugins_manager.addPlugin(
          "mux_v0", std::move(multiplexer_uint8_plugin_ptr));
    }
    else if("extended_arithmetic_v1" == parsers.circuitBodyParser->plugins[i])
    {
      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<sst::bignum>>> arith_plugin_bignum_ptr(
            new wtk::plugins::FallbackExtendedArithmeticPlugin<
            sst::bignum, wtk::firealarm::Wire<sst::bignum>>(setting));
      plugins_manager.addPlugin(
          "extended_arithmetic_v1", std::move(arith_plugin_bignum_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint64_t>>> arith_plugin_uint64_ptr(
            new wtk::plugins::FallbackExtendedArithmeticPlugin<
            sst::bignum, wtk::firealarm::Wire<uint64_t>>(setting));
      plugins_manager.addPlugin(
          "extended_arithmetic_v1", std::move(arith_plugin_uint64_ptr));

      std::unique_ptr<wtk::plugins::Plugin<sst::bignum,
        wtk::firealarm::Wire<uint8_t>>> arith_plugin_uint8_ptr(
            new wtk::plugins::FallbackExtendedArithmeticPlugin<
            sst::bignum, wtk::firealarm::Wire<uint8_t>>(setting));
      plugins_manager.addPlugin(
          "extended_arithmetic_v1", std::move(arith_plugin_uint8_ptr));
    }
    else
    {
      log_error("Unrecognized plugin \"%s\"",
          parsers.circuitBodyParser->plugins[i].c_str());
      return 1;
    }
  }

  // Check that the number of types doesn't exceed the 8-bit type-index limit
  if(parsers.circuitBodyParser->types.size() >= UINT8_MAX)
  {
    log_error("Amount of types exceeds type index limit: %zu",
        parsers.circuitBodyParser->types.size());
    return 1;
  }

  // Create a backend for each type indicated by the relation's header.
  for(size_t i = 0; i < parsers.circuitBodyParser->types.size(); i++)
  {
    wtk::circuit::TypeSpec<sst::bignum>* type =
      &parsers.circuitBodyParser->types[i];

    if(type->variety == wtk::circuit::TypeSpec<sst::bignum>::plugin
        && (type->binding.name == "ram_arith_v0"
          || type->binding.name == "ram_arith_v1"))
    {
      wtk::type_idx idx_type = 0;
      wtk::wire_idx num_allocs = 0;
      wtk::wire_idx total_alloc = 0;
      wtk::wire_idx max_alloc = 0;

      if(type->binding.name == "ram_arith_v0" && !wtk::plugins::checkRAMv0Type(
            type, &idx_type, &num_allocs, &total_alloc, &max_alloc))
      {
        return 1;
      }

      counters.emplace_back(nullptr, nullptr, true);
      wtk::firealarm::TypeCounter* const ctr = &counters.back();

      switch(manager.typeRefs[(size_t) idx_type].precision)
      {
      case Precision::unlimited:
      {
        if(fallback_ram_flag)
        {
          wtk::plugins::FallbackRAMBackend<
            sst::bignum, wtk::firealarm::Wire<sst::bignum>>* backend =
              manager.makeFallbackRAMUnlimited(type, idx_type);

          interpreter.addType<wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<sst::bignum>>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        else
        {
          wtk::firealarm::RAMBackend<sst::bignum, sst::bignum>* backend =
            manager.makeRAMUnlimited(type, parsers.circuitName, idx_type, ctr);

          interpreter.addType<wtk::firealarm::RAMBuffer<sst::bignum>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        break;
      }
      case Precision::uint64:
      {
        if(fallback_ram_flag)
        {
          wtk::plugins::FallbackRAMBackend<
            sst::bignum, wtk::firealarm::Wire<uint64_t>>* backend =
              manager.makeFallbackRAMUint64(type, idx_type);

          interpreter.addType<wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<uint64_t>>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        else
        {
          wtk::firealarm::RAMBackend<sst::bignum, uint64_t>* backend =
            manager.makeRAMUint64(type, parsers.circuitName, idx_type, ctr);

          interpreter.addType<wtk::firealarm::RAMBuffer<uint64_t>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        break;
      }
      case Precision::uint8:
      {
        if(fallback_ram_flag)
        {
          wtk::plugins::FallbackRAMBackend<
            sst::bignum, wtk::firealarm::Wire<uint8_t>>* backend =
              manager.makeFallbackRAMUint8(type, idx_type);

          interpreter.addType<wtk::plugins::FallbackRAMBuffer<
            wtk::firealarm::Wire<uint8_t>>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        else
        {
          wtk::firealarm::RAMBackend<sst::bignum, uint8_t>* backend =
            manager.makeRAMUint8(type, parsers.circuitName, idx_type, ctr);

          interpreter.addType<wtk::firealarm::RAMBuffer<uint8_t>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        break;
      }
      default:
      {
        log_error("Cannot use RAM with type %d", (int) idx_type);
        return 1;
      }
      }

      continue;
    }
    else if(type->variety == wtk::circuit::TypeSpec<sst::bignum>::plugin
        && type->binding.name == "ram_bool_v0")
    {
      wtk::type_idx idx_type = 0;
      wtk::wire_idx idx_bits = 0;
      wtk::wire_idx elt_bits = 0;
      wtk::wire_idx num_allocs = 0;
      wtk::wire_idx total_alloc = 0;
      wtk::wire_idx max_alloc = 0;

      if(!wtk::plugins::checkBoolRAMType(
            type, &idx_type, &idx_bits, &elt_bits,
            &num_allocs, &total_alloc, &max_alloc))
      {
        return 1;
      }

      counters.emplace_back(nullptr, nullptr, true);
      wtk::firealarm::TypeCounter* const ctr = &counters.back();

      if(manager.typeRefs[(size_t) idx_type].erasedType->type->variety
          == wtk::circuit::TypeSpec<sst::bignum>::field
          && manager.typeRefs[(size_t) idx_type].erasedType->type->prime == 2)
      {
        if(fallback_ram_flag)
        {
          wtk::plugins::FallbackBoolRAMBackend<
            sst::bignum, wtk::firealarm::Wire<uint8_t>>* backend =
              manager.makeFallbackBoolRAM(type, idx_type, idx_bits, elt_bits);

          interpreter.addType<wtk::plugins::FallbackBoolRAMBuffer<
            wtk::firealarm::Wire<uint8_t>>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        else
        {
          wtk::firealarm::BoolRAMBackend<sst::bignum, uint8_t>* backend =
            manager.makeBoolRAM(
                type, parsers.circuitName, idx_type, idx_bits, elt_bits, ctr);

          interpreter.addType<wtk::firealarm::RAMBuffer<uint8_t>>(
              backend, nullptr, nullptr);

          plugins_manager.addBackend((wtk::type_idx) i, backend);
        }
        break;
      }
      else
      {
        log_error("Cannot instantiate Boolean RAM with non-boolean type");
        return 1;
      }
    }
    else if(type->variety != wtk::circuit::TypeSpec<sst::bignum>::field)
    {
      log_error(
          "FIREALARM currently only supports prime field and RAM types");
      return 1;
    }

    counters.emplace_back(&totalCurrentCount, &totalMaximumCount);

    if(type->prime >= UINT32_MAX) // overflows uint64_t during multiply
    {
      wtk::firealarm::FieldBackend<sst::bignum, sst::bignum>* backend =
        manager.makeUnlimited(parsers.circuitName, type, &counters.back());

      interpreter.addType<wtk::firealarm::Wire<sst::bignum>>(backend,
          parsers.circuitStreams[i].publicStream,
          parsers.circuitStreams[i].privateStream);

      plugins_manager.addBackend((wtk::type_idx) i, backend);
    }
    else if(type->prime >= 16) // overflows uint8_t during multiply
    {
      wtk::firealarm::FieldBackend<sst::bignum, uint64_t>* backend =
        manager.makeUint64(parsers.circuitName, type, &counters.back());

      interpreter.addType<wtk::firealarm::Wire<uint64_t>>(backend,
          parsers.circuitStreams[i].publicStream,
          parsers.circuitStreams[i].privateStream);

      plugins_manager.addBackend((wtk::type_idx) i, backend);
    }
    else
    {
      wtk::firealarm::FieldBackend<sst::bignum, uint8_t>* backend =
        manager.makeUint8(parsers.circuitName, type, &counters.back());

      interpreter.addType<wtk::firealarm::Wire<uint8_t>>(backend,
          parsers.circuitStreams[i].publicStream,
          parsers.circuitStreams[i].privateStream);

      plugins_manager.addBackend((wtk::type_idx) i, backend);
    }
  }

  /* ==== Add all applicable converters ==== */

  std::vector<size_t> conv_counters(
      parsers.circuitBodyParser->conversions.size(), 0);

  for(size_t i = 0; i < parsers.circuitBodyParser->conversions.size(); i++)
  {
    wtk::circuit::ConversionSpec const* spec =
      &parsers.circuitBodyParser->conversions[i];

    wtk::circuit::TypeSpec<sst::bignum> const* out_type =
      &parsers.circuitBodyParser->types[(size_t) spec->outType];
    wtk::circuit::TypeSpec<sst::bignum> const* in_type =
      &parsers.circuitBodyParser->types[(size_t) spec->inType];

    // Switch on the output type's precision (Wire_T template)
    switch(manager.typeRefs[(size_t) spec->outType].precision)
    {
    case Precision::unlimited:
    {
      // Switch on the intput type's precision (Wire_T template)
      switch(manager.typeRefs[(size_t) spec->inType].precision)
      {
      case Precision::unlimited:
      {
        interpreter.addConversion(
            spec, manager.convertUnlimitedUnlimited.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint64:
      {
        interpreter.addConversion(
            spec, manager.convertUnlimitedUint64.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint8:
      {
        interpreter.addConversion(
            spec, manager.convertUnlimitedUint8.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      default:
      {
        log_error("Cannot convert from type %d to type %d",
            (int) spec->inType, (int) spec->outType);
        return 1;
      }
      }

      break;
    }
    case Precision::uint64:
    {
      // Switch on the intput type's precision (Wire_T template)
      switch(manager.typeRefs[(size_t) spec->inType].precision)
      {
      case Precision::unlimited:
      {
        interpreter.addConversion(
            spec, manager.convertUint64Unlimited.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint64:
      {
        interpreter.addConversion(
            spec, manager.convertUint64Uint64.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint8:
      {
        interpreter.addConversion(
            spec, manager.convertUint64Uint8.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      default:
      {
        log_error("Cannot convert from type %d to type %d",
            (int) spec->inType, (int) spec->outType);
        return 1;
      }
      }

      break;
    }
    case Precision::uint8:
    {
      // Switch on the intput type's precision (Wire_T template)
      switch(manager.typeRefs[(size_t) spec->inType].precision)
      {
      case Precision::unlimited:
      {
        interpreter.addConversion(
            spec, manager.convertUint8Unlimited.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint64:
      {
        interpreter.addConversion(
            spec, manager.convertUint8Uint64.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      case Precision::uint8:
      {
        interpreter.addConversion(
            spec, manager.convertUint8Uint8.allocate(1,
              out_type->prime, in_type->prime, spec->outLength,
              spec->inLength, &counters[(size_t) spec->outType],
              &conv_counters[i], parsers.circuitName));
        break;
      }
      default:
      {
        log_error("Cannot convert from type %d to type %d",
            (int) spec->inType, (int) spec->outType);
        return 1;
      }
      }

      break;
    }
    default:
    {
      log_error("Cannot convert from type %d to type %d",
          (int) spec->inType, (int) spec->outType);
      return 1;
    }
    }
  }

  // NAILS boilerplate.
  wtk::nails::GatesFunctionFactory<sst::bignum> func_fact;
  wtk::nails::Handler<sst::bignum> handler(
      &interpreter, &func_fact, &plugins_manager);

  bool win = true;

  // Parse/stream and check for success criteria
  if(!parsers.circuitBodyParser->parse(&handler))
  {
    win = false;
  }
  else
  {
    for(size_t i = 0; i < manager.typeRefs.size(); i++)
    {
      wtk::TypeBackendEraser<sst::bignum>* const backend =
        manager.typeRefs[i].erasedType;

      if(!backend->check())
      {
        switch(backend->type->variety)
        {
        case wtk::circuit::TypeSpec<sst::bignum>::field:
        {
          log_error("failure in field %zu (prime %s)",
              i, wtk::utils::dec(backend->type->prime).c_str());
          break;
        }
        case wtk::circuit::TypeSpec<sst::bignum>::ring:
        case wtk::circuit::TypeSpec<sst::bignum>::plugin:
        {
          /* TODO */
          break;
        }
        }

        win = false;
      }
    }

    if(win)
    {
      log_info("Relation evaluated successfully");
    }

    // Add up all the total gate counts
    wtk::firealarm::TypeCounter total(
        &totalCurrentCount, &totalMaximumCount);
    for(size_t i = 0; i < counters.size(); i++)
    {
      if(detail_counts)
      {
        std::string name;
        if(parsers.circuitBodyParser->types[i].variety ==
            wtk::circuit::TypeSpec<sst::bignum>::field)
        {
          name = "field ";
          name += wtk::utils::dec(i);
          name += ":";
          name += wtk::utils::dec(parsers.circuitBodyParser->types[i].prime);
        }
        else if(parsers.circuitBodyParser->types[i].binding.name
            == "ram_arith_v0")
        {
          name = "type " + wtk::utils::dec(i) + " arithmetic ram";
        }
        else if(parsers.circuitBodyParser->types[i].binding.name
            == "ram_bool_v0")
        {
          name = "type " + wtk::utils::dec(i) + " boolean ram";
        }

        counters[i].print(true, name.c_str());
      }
      total.addTotal(&counters[i]);
    }

    // print out total gate counts
    total.maximumActive = totalMaximumCount;
    total.print(detail_counts, "totals");

    // print out detailed gate counts (per field/per convert)
    if(detail_counts && conv_counters.size() > 0)
    {
      log_info("Convert Gate Counts");

      /* TODO check for success/failure */

      for(size_t i = 0;
          i < parsers.circuitBodyParser->conversions.size(); i++)
      {
        wtk::circuit::ConversionSpec const* const spec =
          &parsers.circuitBodyParser->conversions[i];

        log_info("@convert(@out: %u:%zu, @in %u:%zu):  %zu",
            (unsigned int) spec->outType, spec->outLength,
            (unsigned int) spec->inType, spec->inLength,
            conv_counters[i]);
      }
    }
  }

  return win ? 0 : 1;
}
