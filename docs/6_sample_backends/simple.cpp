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

// Plugin API (boilerplate in this example)
#include <wtk/plugins/Plugin.h>

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

  MyBackend(Number p) : wtk::TypeBackend<Number, MyWire>(p) { }

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

  bool convert(
      MyWire* const out_wires, MyWire const* const in_wires) override
  {
    // Your Code Here!

    // out_wires has length this->outLength
    (void) out_wires;
    // in_wires has the length this->inLength
    (void) in_wires;

    return true;
  }
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

  // Check for plugins, because they are not yet supported.
  if(organizer.circuitBodyParser->plugins.size() != 0)
  {
    printf("plugins are not yet supported\n");
    return 1;
  }

  // Create storage for backends, one backend for each type/field
  std::vector<MyBackend> backends;
  // Reserve is necessary otherwise multiple fields (vector growths)
  // invalidate prior pointers.
  backends.reserve(organizer.circuitBodyParser->types.size());

  // Create each type/field in the same order as required by the relation
  for(size_t i = 0; i < organizer.circuitBodyParser->types.size(); i++)
  {
    wtk::circuit::TypeSpec<Number>* const type =
      &organizer.circuitBodyParser->types[i];

    // Check that its a field not a plugin type.
    if(type->variety != wtk::circuit::TypeSpec<Number>::field)
    {
      printf("Type %zu is a plugin (not yet supported)\n", i);
      return 1;
    }
    else
    {
      // construct another backend with this prime
      backends.emplace_back(type->prime);
      // add the backend and its streams to the interpreter
      interpreter.addType(&backends.back(),
          organizer.circuitStreams[i].publicStream,
          organizer.circuitStreams[i].privateStream);
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
  wtk::plugins::PluginsManager<Number, MyWire> plugins_manager; // boilerplate
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

  return ret;
}
