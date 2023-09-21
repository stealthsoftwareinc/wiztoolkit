#include <cstddef>
#include <cstdio>
#include <vector>
#include <cstdint>
#include <memory>

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
// We'll use this struct for arithmetic
struct MyWire { };
// And for boolean, we'll bit-bang everything into a uint8
using BoolWire = uint8_t;

// This struct implements the TypeBackend callbacks for arithmetic types.
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

// This struct implements the TypeBackend callbacks for the Boolean type.
struct BoolBackend : wtk::TypeBackend<Number, BoolWire>
{
  bool assertFailure = false;

  // We should be safe to default construct this and assume prime==2
  BoolBackend(wtk::circuit::TypeSpec<Number> const* t)
    : wtk::TypeBackend<Number, BoolWire>(t) { }

  // $0 <- <0>;
  void assign(BoolWire* output, Number&& input_value) override
  {
    (void) output;
    (void) input_value;
  }

  // $1 <- $0;
  void copy(BoolWire* output, BoolWire const* input_wire) override
  {
    (void) output;
    (void) input_wire;
  }

  // $2 <- @add($0, $1);
  void addGate(BoolWire* output,
      BoolWire const* left_input, BoolWire const* right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $3 <- @add($1, $2);
  void mulGate(BoolWire* output,
      BoolWire const* left_input, BoolWire const* right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $4 <- @addc($3, <1>);
  void addcGate(BoolWire* output,
      BoolWire const* left_input, Number&& right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // $4 <- @addc($3, <2>);
  void mulcGate(BoolWire* output,
      BoolWire const* left_input, Number&& right_input) override
  {
    (void) output;
    (void) left_input;
    (void) right_input;
  }

  // @assert_zero($4);
  // Failures may occur, but should get cached until the end when "check()"
  // is called
  void assertZero(BoolWire const* input_wire) override
  {
    (void) input_wire;
    // check that the input wire == 0
    this->assertFailure = false || this->assertFailure;
  }

  // $5 <- @public_in();
  void publicIn(BoolWire* output, Number&& input_value) override
  {
    (void) output;
    (void) input_value;
  }

  // $5 <- @private_in();
  void privateIn(BoolWire* output, Number&& input_value) override
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
// Due to the <MyWire, MyWire> template fill, these converters goes from one
// arithmetic type to another arithmetic type
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

// Subclass the Converter type to implement conversion.
// Due to the <MyWire, BoolWire> template fill, these converters goes from the
// boolean type to an arithmetic type
struct ConverterFromBool
  : public wtk::Converter</*out*/ MyWire, /*in*/ BoolWire>
{
  // The output prime is probably relevant, but it should be safe to assume
  // the input prime is 2
  Number outPrime;

  // The parent constructor requires the length of output and input which
  // this Converter will handle.
  ConverterFromBool(size_t out_len, Number op, size_t in_len)
    : wtk::Converter<MyWire, BoolWire>(out_len, in_len), outPrime(op) { }

  void convert(MyWire* const out_wires,
      BoolWire const* const in_wires, bool modulus) override
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

// Subclass the Converter type to implement conversion.
// Due to the <BoolWire, MyWire> template fill, these converters goes from an
// arithmetic type to a boolean type
struct ConverterToBool : public wtk::Converter</*out*/ BoolWire, /*in*/ MyWire>
{
  // The input prime is probably relevant, but it should be safe to assume
  // the output prime is 2
  Number inPrime;

  // The parent constructor requires the length of output and input which
  // this Converter will handle.
  ConverterToBool(size_t out_len, size_t in_len, Number ip)
    : wtk::Converter<BoolWire, MyWire>(out_len, in_len), inPrime(ip) { }

  void convert(BoolWire* const out_wires,
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

  // Check for plugins, because they are not yet supported.
  if(organizer.circuitBodyParser->plugins.size() != 0)
  {
    printf("plugins are not yet supported\n");
    return 1;
  }

  // Create storage for backends, one backend for each type/field
  std::vector<MyBackend> backends;
  std::vector<std::unique_ptr<BoolBackend>> bool_backends;
  // Reserve is necessary with non-pointer storage, otherwise additional
  // fields (triggering vector growth) can invalidate prior pointers.
  backends.reserve(organizer.circuitBodyParser->types.size());

  // If it is necessary to access all backends from a single vector, then their
  // type erased parent class may be used.
  std::vector<wtk::TypeBackendEraser<Number>*> all_backends;

  // Create each type/field in the same order as required by the relation
  for(size_t i = 0; i < organizer.circuitBodyParser->types.size(); i++)
  {
    wtk::circuit::TypeSpec<Number>* const type =
      &organizer.circuitBodyParser->types[i];

    // Check that its a field not a plugin type.
    if(type->variety != wtk::circuit::TypeSpec<Number>::field)
    {
      printf("Type %zu is not a prime field (not yet supported)\n", i);
      return 1;
    }
    if(type->prime == 2)
    {
      // construct a boolean backend with this prime
      bool_backends.emplace_back(new BoolBackend(type));
      // Add the backend and its streams to the interpreter
      interpreter.addType(bool_backends.back().get(),
          organizer.circuitStreams[i].publicStream,
          organizer.circuitStreams[i].privateStream);

      // Add the backend to the all_backends tracker
      all_backends.push_back(bool_backends.back().get());
    }
    else
    {
      // construct another backend with this prime
      backends.emplace_back(type);
      // add the backend and its streams to the interpreter
      interpreter.addType(&backends.back(),
          organizer.circuitStreams[i].publicStream,
          organizer.circuitStreams[i].privateStream);

      // Add the backend to the all_backends tracker
      all_backends.push_back(&backends.back());
    }
  }

  // Create a storage vector for converters, one for each ConversionSpec
  std::vector<MyConverter> converters;
  std::vector<ConverterToBool> to_bool_converters;
  std::vector<ConverterFromBool> from_bool_converters;
  // Reserve necessary space, otherwise vector growth will invalidate pointers
  converters.reserve(organizer.circuitBodyParser->conversions.size());
  to_bool_converters.reserve(organizer.circuitBodyParser->conversions.size());
  from_bool_converters.reserve(organizer.circuitBodyParser->conversions.size());

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

    if(out_type->prime == 2)
    {
      // construct a "to bool" converter
      to_bool_converters.emplace_back(
          spec->outLength, spec->inLength, in_type->prime);
      // add the converter to the inpterpreter
      interpreter.addConversion(spec, &to_bool_converters.back());
    }
    else if(in_type->prime == 2)
    {
      // construct a "from bool" converter
      from_bool_converters.emplace_back(
          spec->outLength, out_type->prime, spec->inLength);
      // add the converter to the inpterpreter
      interpreter.addConversion(spec, &from_bool_converters.back());
    }
    else
    {
      // call the constructor for our converter
      converters.emplace_back(
          spec->outLength, out_type->prime, spec->inLength, in_type->prime);
      // add the converter to the interpreter
      interpreter.addConversion(spec, &converters.back());
    }
  }

  // Create the nails handler (accepts/directs gates from the parser)
  wtk::nails::GatesFunctionFactory<Number> func_factory;        // boilerplate
  wtk::plugins::PluginsManager<Number, MyWire, BoolWire> plugins_manager; // boilerplate
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

  // Check that all the to-bool conversions succeeded
  for(size_t i = 0; i < to_bool_converters.size(); i++)
  {
    if(!to_bool_converters[i].check())
    {
      printf("failure during conversion\n");
      ret = 1;
    }
  }

  // Check that all the from-bool conversions succeeded
  for(size_t i = 0; i < from_bool_converters.size(); i++)
  {
    if(!from_bool_converters[i].check())
    {
      printf("failure during conversion\n");
      ret = 1;
    }
  }

  return ret;
}
