#include <cstddef>
#include <cstdio>
#include <vector>

// Parsers
#include <wtk/Parser.h>                // top level API
#include <wtk/circuit/Parser.h>        // circuit IR API
#include <wtk/circuit/Data.h>          // structs used by the circuit parser API
#include <wtk/irregular/Parser.h>      // text parser implementation

#include <wtk/utils/ParserOrganizer.h> // "owner"/"organizer" for parsers

// Backend API
#include <wtk/TypeBackend.h>           // per-field/type backend callback API
#include <wtk/Converter.h>             // field switching API

// NAILS Interpreter
#include <wtk/nails/Interpreter.h>     // main actor of the NAILS API
#include <wtk/nails/Handler.h>         // bridge between NAILS and the Parser
#include <wtk/nails/Functions.h>       // helpers for NAILS functions

// Plugin API
#include <wtk/plugins/Plugin.h>        // The general Plugin API
#include <wtk/plugins/Multiplexer.h>   // Specific to each feature...
#include <wtk/plugins/ArithmeticRAM.h>
#include <wtk/plugins/Vectors.h>       // WizToolKit bonus plugin
#include <wtk/nails/IterPlugin.h>

// An unbounded (or big enough not to overflow) numeric type.
#include <gmpxx.h>
using Number = mpz_class;
// using Number = uint64_t; 

#include <wtk/utils/NumUtils.gmp.h> // GMP specific hacks

// Struct containing the data carried between gates ("wires")
struct MyWire { };

// Backend implementation of callbacks
struct MyBackend : wtk::TypeBackend<Number, MyWire>
{
  bool assertFailure = false;

  // TypeSpec is a wrapper for the IR's types (field/ring/...)
  MyBackend(wtk::circuit::TypeSpec<Number> const* t)
    : wtk::TypeBackend<Number, MyWire>(t) { }

  // $0 <- <0>;
  void assign(MyWire* output, Number&& input_value) override
  {
    (void) output;
    (void) input_value;
  }

  // $1 <- $0;
  void copy(MyWire* output, MyWire const* input_wire) override
  {
    (void) output;
    (void) input_wire;
  }

  // $2 <- @add($0, $1);
  void addGate(MyWire* output,
      MyWire const* left_input, MyWire const* right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $3 <- @add($1, $2);
  void mulGate(MyWire* output,
      MyWire const* left_input, MyWire const* right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $4 <- @addc($3, <1>);
  void addcGate(MyWire* output,
      MyWire const* left_input, Number&& right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $4 <- @addc($3, <2>);
  void mulcGate(MyWire* output,
      MyWire const* left_input, Number&& right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // @assert_zero($4);
  // Failures may occur, but should get cached until the end when "check()"
  // is called
  void assertZero(MyWire const* input_wire) override
  {
    (void) input_wire;
    // check that the input wire == 0
    this->assertFailure = false || this->assertFailure;
  }

  // $5 <- @public_in();
  void publicIn(MyWire* output, Number&& input_value) override
  {
    (void) output;
    (void) input_value;
  }

  // $5 <- @private_in();
  void privateIn(MyWire* output, Number&& input_value) override
  {
    (void) output;
    (void) input_value;
  }

  // indicates if any failures occured.
  // Normally true, false on failure.
  bool check() override
  {
    return !this->assertFailure;
  }

  void finish() override
  {
    // optional cleanup tasks.
  }
};

// Subclass the Converter type to implement conversion.
struct MyConverter : public wtk::Converter<MyWire, MyWire>
{
  // Maybe you need the output and input primes
  Number outPrime;
  Number inPrime;

  // The parent constructor requires the length of output and input which
  // this Converter will handle.
  MyConverter(size_t out_len, Number op, size_t in_len, Number ip)
    : wtk::Converter<MyWire, MyWire>(out_len, in_len),
      outPrime(op), inPrime(ip) { }

  void convert(MyWire* const out_wires,
      MyWire const* const in_wires, bool modulus) override
  {
    // Your Code Here!

    // out_wires has length this->outLength
    (void) out_wires;
    // in_wires has the length this->inLength
    (void) in_wires;


    if(modulus)
    {
      // An overflowing conversion should behave modularly
    }
    else
    {
      // An overflowing conversion should fail
      if(false/* an overflow occurred */) { this->success = false; }
    }
  }

  // The success or failure is cached until the user calls check at the end.
  bool success = false;
  bool check() override { return success; }
};

int main(int argc, char const* argv[])
{
  wtk::utils::ParserOrganizer<wtk::irregular::Parser<Number>, Number> organizer;

  // Open and organize files
  for(size_t i = 1; i < (size_t) argc; i++)
  {
    if(!organizer.open(argv[i]))
    {
      printf("failed to open %s\n", argv[i]);
      return 1;
    }
  }

  wtk::utils::Setting setting = organizer.organize();
  if(setting == wtk::utils::Setting::failure)
  {
    printf("failed to organize inputs\n");
    return 1;
  }

  // Create a nails interpreter
  wtk::nails::Interpreter<Number> interpreter(organizer.circuitName);

  // Create a plugins manager, which requires templates for all possible wires
  wtk::plugins::PluginsManager<Number,
      MyWire,                                 // most plugins
      wtk::plugins::FallbackRAMBuffer<MyWire> // RAM has special buffer wires
    > plugins_manager;

  // Hoist the iteration plugin's allocation, for memory lifetime purposes.
  wtk::nails::MapOperation<Number> map_operation(&interpreter);

  // Scan the plugins required in the circuit's header.
  for(size_t i = 0; i < organizer.circuitBodyParser->plugins.size(); i++)
  {
    // Recognize each plugin by name
    if("mux_v0" == organizer.circuitBodyParser->plugins[i])
    {
      // Allocate a multiplexer plugin, and hand it off to the plugins_manager.
      std::unique_ptr<wtk::plugins::Plugin<Number, MyWire>> mux_plugin(
          new wtk::plugins::FallbackMultiplexerPlugin<Number, MyWire>());
      plugins_manager.addPlugin("mux_v0", std::move(mux_plugin));
    }
    else if("wizkit_vectors" == organizer.circuitBodyParser->plugins[i])
    {
      // Allocate a vectors plugin, and hand it off to the plugins_manager.
      std::unique_ptr<wtk::plugins::Plugin<Number, MyWire>> vector_plugin(
          new wtk::plugins::FallbackVectorPlugin<Number, MyWire>());
      plugins_manager.addPlugin("wizkit_vectors", std::move(vector_plugin));
    }
    else if("iter_v0" == organizer.circuitBodyParser->plugins[i])
    {
      // The iter plugin is supplied by the NAILS interpreter as a single
      // instantiable operation, contrary to most other plugins.
      plugins_manager.addPlugin("iter_v0", map_operation.makePlugin<MyWire>());
    }
    else if("ram_arith_v0" == organizer.circuitBodyParser->plugins[i]
        || "ram_arith_v1" == organizer.circuitBodyParser->plugins[i])
    {
      char const* const ram_name =
        organizer.circuitBodyParser->plugins[i].c_str();
      // When allocating the RAM plugin, notice how the parent pointer's type
      // changes to have a buffer template, while the allocation still has
      // the ordinary wire type. This is due to how the RAM plugin specializes
      // its parent class.
      //
      // Note, the fallback for RAM is currently a naive linear scan.
      std::unique_ptr<wtk::plugins::Plugin<Number,
        wtk::plugins::FallbackRAMBuffer<MyWire>>> RAM_plugin(
            new wtk::plugins::FallbackRAMPlugin<Number, MyWire>());
      plugins_manager.addPlugin(ram_name, std::move(RAM_plugin));
    }
    else
    {
      printf("unrecognized plugin \"%s\"\n",
          organizer.circuitBodyParser->plugins[i].c_str());
      return 1;
    }
  }

  // Create storage for backends, one backend for each type/field
  std::vector<MyBackend> backends;
  // Reserve is necessary otherwise multiple fields (vector growths)
  // invalidate prior pointers.
  backends.reserve(organizer.circuitBodyParser->types.size());

  // Create storage for RAM backends, Note, its possible to have multiple
  // RAM backends, if multiple types and corresponding RAM types are declared.
  std::vector<wtk::plugins::FallbackRAMBackend<Number, MyWire>> ram_backends;
  ram_backends.reserve(organizer.circuitBodyParser->types.size() / 2);

  // To instantiate RAM backends, we need to know where the corresponding
  // element's backend is.
  std::vector<bool> is_ram;
  std::vector<size_t> backend_place;

  // Create each type/field in the same order as required by the relation
  // Notice that the counter, i, may be used as the type index.
  for(size_t i = 0; i < organizer.circuitBodyParser->types.size(); i++)
  {
    wtk::circuit::TypeSpec<Number>* const type =
      &organizer.circuitBodyParser->types[i];

    // Check if it's a RAM or regular field type.
    if(type->variety == wtk::circuit::TypeSpec<Number>::plugin
        && (type->binding.name == "ram_arith_v0"
          || type->binding.name == "ram_arith_v1")
        && type->binding.operation == "ram")
    {
      // The plugin must supply the following values, although not all are used
      wtk::type_idx type_index = 0;
      wtk::wire_idx num_allocs = 0;
      wtk::wire_idx total_allocs = 0;
      wtk::wire_idx max_alloc = 0;
      bool has_alloc_hints = type->binding.name == "ram_arith_v0";

      // WizToolKit helper for getting the hintsfrom the type's plugin binding,
      // But only the v0 plugin has the hints.
      if(has_alloc_hints && !wtk::plugins::checkRAMv0Type(
            type, &type_index, &num_allocs, &total_allocs, &max_alloc))
      {
        return 1;
      }

      // these ones don't get used by the fallback.
      if(has_alloc_hints)
      {
        (void) num_allocs;
        (void) total_allocs;
        (void) max_alloc;
      }

      // check that the type index refers to a field wire.
      if((size_t) type_index >= is_ram.size()
          || is_ram[(size_t) type_index])
      {
        printf("type %zu is unsuitable as a RAM element", (size_t) type_index);
        return 1;
      }

      // finally allocate and supply the backend
      ram_backends.emplace_back(
          type, type_index, &backends[backend_place[(size_t) type_index]]);
      interpreter.addType(&ram_backends.back(), nullptr, nullptr);
      plugins_manager.addBackend((wtk::type_idx) i, &ram_backends.back());

      // record the RAM backend's position
      is_ram.push_back(true);
      backend_place.push_back(ram_backends.size() - 1);
    }
    else if(type->variety != wtk::circuit::TypeSpec<Number>::field)
    {
      printf("Type %zu is a plugin (not yet supported)\n", i);
      return 1;
    }
    else
    {
      // construct another backend with this prime
      backends.emplace_back(type);
      // add the backend and its streams to the interpreter
      interpreter.addType(&backends.back(),
          organizer.circuitStreams[i].publicStream,
          organizer.circuitStreams[i].privateStream);
      plugins_manager.addBackend((wtk::type_idx) i, &backends.back());

      // record the backend's position incase it's used as a RAM element
      is_ram.push_back(false);
      backend_place.push_back(backends.size() - 1);
    }
  }

  // Create a storage vector for converters, one for each ConversionSpec
  std::vector<MyConverter> converters;
  // Reserve necessary space, otherwise vector growth will invalidate pointers
  converters.reserve(organizer.circuitBodyParser->conversions.size());

  for(size_t i = 0; i < organizer.circuitBodyParser->conversions.size(); i++)
  {
    // pointer to the conversion spec (outType, outLength, inType, inLength)
    wtk::circuit::ConversionSpec* const spec =
      &organizer.circuitBodyParser->conversions[i];

    // get the TypeSpecs corresponding to the out and input types.
    wtk::circuit::TypeSpec<Number>* const out_type =
      &organizer.circuitBodyParser->types[(size_t) spec->outType];
    wtk::circuit::TypeSpec<Number>* const in_type =
      &organizer.circuitBodyParser->types[(size_t) spec->inType];

    // call the constructor for our converter
    converters.emplace_back(
        spec->outLength, out_type->prime, spec->inLength, in_type->prime);
    // add the converter to the interpreter
    interpreter.addConversion(spec, &converters.back());
  }

  // Create the nails handler (accepts/directs gates from the parser)
  wtk::nails::GatesFunctionFactory<Number> func_factory;        // boilerplate
  wtk::nails::Handler<Number> handler(
      &interpreter, &func_factory, &plugins_manager);

  // parse the relation and pass each gate off through NAILS and into
  // the backends.
  if(!organizer.circuitBodyParser->parse(&handler))
  {
    printf("parser failure\n");
    return 1;
  }

  // Check that all the fields succeeded
  int ret = 0;
  for(size_t i = 0; i < backends.size(); i++)
  {
    if(!backends[i].check())
    {
      printf("failure in field %zu\n", i);
      ret = 1;
    }

    backends[i].finish();
  }

  // Check that all the conversions succeeded
  for(size_t i = 0; i < converters.size(); i++)
  {
    if(!converters[i].check())
    {
      printf("failure during conversion\n");
      ret = 1;
    }
  }

  return ret;
}
