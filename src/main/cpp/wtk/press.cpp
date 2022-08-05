/**
 * Copyright (C) 2020-2021 Stealth Software Technologies, Inc.
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>

#include <memory>
#include <utility>

#include <sst/catalog/bignum.hpp>

#include <wtk/printers/BooleanTextStreamPrinter.h>
#include <wtk/printers/ArithmeticTextStreamPrinter.h>
#include <wtk/printers/printTextParameters.h>
#include <wtk/printers/TextTreePrinter.h>
#include <wtk/printers/printTextInputStream.h>
#include <wtk/converters/Multiplex.h>

#include <wtk/irregular/Parser.h>

#if ENABLE_FLATBUFFER
#include <wtk/flatbuffer/Parser.h>
#include <wtk/flatbuffer/ArithmeticFlatBufferStreamPrinter.h>
#include <wtk/flatbuffer/BooleanFlatBufferStreamPrinter.h>
#include <wtk/flatbuffer/FlatBufferTreePrinter.h>
#include <wtk/flatbuffer/FlatBufferInputStreamPrinter.h>
#endif

#if ENABLE_ANTLR
#include <wtk/antlr/Parser.h>
#endif

#include <wtk/utils/FileNameUtils.h>

#define LOG_IDENTIFIER "wtk-press"
#include <stealth_logging.h>

void print_version()
{
  printf("Wiztoolkit wtk-press\n");
  printf("PRESS: Printing and Representation Exchange Software Suite\n");
  printf("IR Version " IR_MAJOR_STR "." IR_MINOR_STR "." IR_PATCH_STR "\n");
  printf("\nCopyright (C) 2020-2021  Stealth Software Technologies, Inc.\n");
  printf("FOR SIEVE PROGRAM USE ONLY!\n");
#if !ENABLE_FLATBUFFER
  printf("Note: compiled without FlatBuffer.\n");
#endif
}

void print_help()
{
  print_version();
  printf("\nwtk-press is a tool for exchanging various IR Formats. Specifically, it converts\n");
  printf("from text to binary/FlatBuffer (t2b) and binary/FlatBufer to text (b2t). It also\n");
  printf("can convert the IR @switch feature to multiplexer using just @function and @for.\n");
  printf("For testing and debugging the parsers, it also has text to text and binary to\n");
  printf(" binary modes.");
  printf("\nUsage:\n");
  printf("  wtk-press <mode> <input> [ <output> ]\n");
  printf("  wtk-press [-h | --help | -v | --version ]\n");
  printf("\nOptions:\n");
  printf("  <mode>       Controls the conversion that wtk-press performs.\n");
#if ENABLE_FLATBUFFER
  printf("    b2b          binary to binary\n");
  printf("    b2mux        Binary to binary with @switch->multiplex conversion.\n");
  printf("    b2t          binary to text\n");
  printf("    t2b          Text to binary\n");
#else
  printf("    b2b          binary to binary (FlatBuffer disabled)\n");
  printf("    b2mux        Binary to binary with @switch->multiplex conversion (disabled).\n");
  printf("    b2t          binary to text (FlatBuffer disabled)\n");
  printf("    t2b          Text to binary (FlatBuffer disabled)\n");
#endif
  printf("    t2mux        Text to text with @switch->multiplex conversion.\n");
  printf("    t2t          Text to text\n");
  printf("    t2n          Text to nothing (parser benchmark)\n");
#if ENABLE_FLATBUFFER
  printf("    b2n          FlatBuffer to nothing (parser benchmark)\n");
#else
  printf("    b2n          FlatBuffer to nothing (parser benchmark, disabled)\n");
#endif
#if ENABLE_ANTLR
  printf("    a2n          ANTLR to nothing (parser benchmark)\n");
#else
  printf("    a2n          ANTLR to nothing (parser benchmark, disabled)\n");
#endif
  printf("  <input>      Input file-name\n");
  printf("  <output>     Output file-name (optional, defaults to stdout)\n");
  printf("  -h --help    Print help output.\n");
  printf("  -v --version Print version information.\n");
}

char const* input_filename = nullptr;
FILE* output_file = stdout;

enum class PressMode
{
  b2b,
  b2mux,
  b2t,
  t2b,
  t2t,
  t2mux,
  t2n,
  b2n,
  a2n
};

PressMode press_mode;

bool parse_args(int argc, char const* argv[])
{
  if(argc == 2)
  {
    std::string opt(argv[1]);

    if(opt == "-h" || opt == "--help")
    {
      print_help();
      exit(0);
    }
    if(opt == "-v" || opt == "--version")
    {
      print_version();
      exit(0);
    }
  }

  if(argc != 3 && argc != 4)
  {
    printf("Unrecognized argument command format\n");
    return false;
  }

  std::string mode_str(argv[1]);
  if(mode_str == "b2b")      { press_mode = PressMode::b2b; }
  else if(mode_str == "b2mux") { press_mode = PressMode::b2mux; }
  else if(mode_str == "b2t") { press_mode = PressMode::b2t; }
  else if(mode_str == "t2b") { press_mode = PressMode::t2b; }
  else if(mode_str == "t2mux") { press_mode = PressMode::t2mux; }
  else if(mode_str == "t2t") { press_mode = PressMode::t2t; }
  else if(mode_str == "t2n") { press_mode = PressMode::t2n; }
  else if(mode_str == "b2n") { press_mode = PressMode::b2n; }
  else if(mode_str == "a2n") { press_mode = PressMode::a2n; }
  else
  {
    log_error("unrecognized mode");
    return false;
  }

  input_filename = argv[2];

  if(argc == 4)
  {
    if(press_mode == PressMode::b2b || press_mode == PressMode::t2b)
    {
      output_file = fopen(argv[3], "wb");
    }
    else
    {
      output_file = fopen(argv[3], "w");
    }
    if(output_file == nullptr)
    {
      log_perror();
      log_error("Error opening output file \"%s\"", argv[3]);
      exit(1);
    }
  }

  return true;
}

int b2b()
{
#if ENABLE_FLATBUFFER
  std::string in_file_str(input_filename);
  wtk::flatbuffer::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::flatbuffer::ArithmeticFlatBufferStreamPrinter<sst::bignum> prt(
            output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(p.characteristic, p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::flatbuffer::BooleanFlatBufferStreamPrinter prt(output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(uint8_t(p.characteristic), p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<sst::bignum> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(p.characteristic, p.degree);
          prt.setGateSet(&p.gateSet);
          prt.setFeatureToggles(&p.featureToggles);
          prt.printTree(tree);
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<uint8_t> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(uint8_t(p.characteristic), p.degree);
          prt.setGateSet(&p.gateSet);
          prt.setFeatureToggles(&p.featureToggles);
          prt.printTree(tree);
          return 0;
        }
      }

      log_error("flatbuffer wip");
      return 1;
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    if(p.characteristic != 2)
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<sst::bignum> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(p.characteristic, p.degree);
      return (prt.printInstance(p.arithmetic()->instance())) ? 0 : 1;
    }
    else
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<uint8_t> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(uint8_t(p.characteristic), p.degree);
      return (prt.printInstance(p.boolean()->instance())) ? 0 : 1;
    }
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    if(p.characteristic != 2)
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<sst::bignum> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(p.characteristic, p.degree);
      return (prt.printShortWitness(p.arithmetic()->shortWitness())) ? 0 : 1;
    }
    else
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<uint8_t> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(uint8_t(p.characteristic), p.degree);
      return (prt.printShortWitness(p.boolean()->shortWitness())) ? 0 : 1;
    }
  }
  else
  {
    return 1;
  }
  log_error("binary to binary: implementation in progress");
  return 1;
#else
  log_error("binary to binary: FlatBuffer disabled");
  return 1;
#endif
}

int b2mux()
{
#if ENABLE_FLATBUFFER
  std::string in_file_str(input_filename);
  wtk::flatbuffer::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      // No @switch in a simple relation, so just pipe it back out.
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::flatbuffer::ArithmeticFlatBufferStreamPrinter<sst::bignum> prt(
            output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(p.characteristic, p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::flatbuffer::BooleanFlatBufferStreamPrinter prt(output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(uint8_t(p.characteristic), p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<sst::bignum> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(p.characteristic, p.degree);
          prt.setGateSet(&p.gateSet);
          if(p.featureToggles.switchCaseToggle)
          {
            wtk::converters::Multiplex<sst::bignum> mux(
                p.characteristic, false);
            wtk::IRTree<sst::bignum>* new_tree = mux.transform(tree);
            if(new_tree == nullptr) { return 1; }

            wtk::FeatureToggles new_features = p.featureToggles;
            new_features.switchCaseToggle = false;
            new_features.forLoopToggle = true;
            new_features.functionToggle = true;

            prt.setFeatureToggles(&new_features);
            prt.printTree(new_tree);
          }
          else
          {
            // no @switches
            prt.setFeatureToggles(&p.featureToggles);
            prt.printTree(tree);
          }
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<uint8_t> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(uint8_t(p.characteristic), p.degree);
          prt.setGateSet(&p.gateSet);
          if(p.featureToggles.switchCaseToggle)
          {
            wtk::converters::Multiplex<uint8_t> mux(
                uint8_t(p.characteristic), true);
            wtk::IRTree<uint8_t>* new_tree = mux.transform(tree);
            if(new_tree == nullptr) { return 1; }

            wtk::FeatureToggles new_features = p.featureToggles;
            new_features.switchCaseToggle = false;
            new_features.forLoopToggle = true;
            new_features.functionToggle = true;

            prt.setFeatureToggles(&new_features);
            prt.printTree(new_tree);
          }
          else
          {
            // no @switch
            prt.setFeatureToggles(&p.featureToggles);
            prt.printTree(tree);
          }
          return 0;
        }
      }
    }
  }
  log_error("cannot convert b2mux for non-relation resource");
  return 1;
#else
  log_error("binary to binary: FlatBuffer disabled");
  return 1;
#endif
}

int b2t()
{
#if ENABLE_FLATBUFFER
  std::string in_file_str(input_filename);
  wtk::flatbuffer::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);
    wtk::printers::printTextGateSetParser(output_file, &p);
    wtk::printers::printTextFeatureTogglesParser(output_file, &p);

    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::printers::ArithmeticTextStreamPrinter<sst::bignum> prt(output_file);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::printers::BooleanTextStreamPrinter prt(output_file);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::printers::TextTreePrinter<sst::bignum> prt(output_file);
          prt.printTree(tree);
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::printers::TextTreePrinter<uint8_t> prt(output_file);
          prt.printTree(tree);
          return 0;
        }
      }
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);

    if(p.characteristic != 2)
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.arithmetic()->instance())) ? 0 : 1;
    }
    else
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.boolean()->instance())) ? 0 : 1;
    }
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);

    if(p.characteristic != 2)
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.arithmetic()->shortWitness())) ? 0 : 1;
    }
    else
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.boolean()->shortWitness())) ? 0 : 1;
    }
  }
  else
  {
    return 1;
  }
#else
  log_error("binary to text: FlatBuffer disabled");
  return 1;
#endif
}

int t2b()
{
#if ENABLE_FLATBUFFER
  std::string in_file_str(input_filename);
  wtk::irregular::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::flatbuffer::ArithmeticFlatBufferStreamPrinter<sst::bignum> prt(
            output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(p.characteristic, p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::flatbuffer::BooleanFlatBufferStreamPrinter prt(output_file);
        prt.setVersion(p.version.major, p.version.minor, p.version.patch);
        prt.setField(uint8_t(p.characteristic), p.degree);
        prt.setGateSet(&p.gateSet);
        prt.setFeatureToggles(&p.featureToggles);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<sst::bignum> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(p.characteristic, p.degree);
          prt.setGateSet(&p.gateSet);
          prt.setFeatureToggles(&p.featureToggles);
          prt.printTree(tree);
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::flatbuffer::FlatBufferTreePrinter<uint8_t> prt(output_file);
          prt.setVersion(p.version.major, p.version.minor, p.version.patch);
          prt.setField(uint8_t(p.characteristic), p.degree);
          prt.setGateSet(&p.gateSet);
          prt.setFeatureToggles(&p.featureToggles);
          prt.printTree(tree);
          return 0;
        }
      }

      log_error("flatbuffer wip");
      return 1;
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    if(p.characteristic != 2)
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<sst::bignum> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(p.characteristic, p.degree);
      return (prt.printInstance(p.arithmetic()->instance())) ? 0 : 1;
    }
    else
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<uint8_t> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(uint8_t(p.characteristic), p.degree);
      return (prt.printInstance(p.boolean()->instance())) ? 0 : 1;
    }
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    if(p.characteristic != 2)
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<sst::bignum> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(p.characteristic, p.degree);
      return (prt.printShortWitness(p.arithmetic()->shortWitness())) ? 0 : 1;
    }
    else
    {
      wtk::flatbuffer::FlatBufferInputStreamPrinter<uint8_t> prt(
          output_file);
      prt.setVersion(p.version.major, p.version.minor, p.version.patch);
      prt.setField(uint8_t(p.characteristic), p.degree);
      return (prt.printShortWitness(p.boolean()->shortWitness())) ? 0 : 1;
    }
  }
  else
  {
    return 1;
  }
#else
  log_error("text to binary: FlatBuffer disabled");
  return 1;
#endif
}

int t2mux()
{
  std::string in_file_str(input_filename);
  wtk::irregular::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);
    wtk::printers::printTextGateSetParser(output_file, &p);

    if(p.featureToggles.simple())
    {
      wtk::printers::printTextFeatureTogglesParser(output_file, &p);
      // No switches in a simple relation
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::printers::ArithmeticTextStreamPrinter<sst::bignum> prt(output_file);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::printers::BooleanTextStreamPrinter prt(output_file);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          if(p.featureToggles.switchCaseToggle)
          {
            wtk::FeatureToggles new_features = p.featureToggles;
            new_features.switchCaseToggle = false;
            new_features.forLoopToggle = true;
            new_features.functionToggle = true;

            wtk::printers::printTextFeatureToggles(output_file, new_features);

            wtk::converters::Multiplex<sst::bignum> mux(
                p.characteristic, false);
            wtk::IRTree<sst::bignum>* new_tree = mux.transform(tree);
            if(new_tree == nullptr) { return 1; }
            wtk::printers::TextTreePrinter<sst::bignum> prt(output_file);
            prt.printTree(new_tree);
          }
          else
          {
            wtk::printers::printTextFeatureTogglesParser(output_file, &p);
            wtk::printers::TextTreePrinter<sst::bignum> prt(output_file);
            prt.printTree(tree);
          }
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          if(p.featureToggles.switchCaseToggle)
          {
            wtk::FeatureToggles new_features = p.featureToggles;
            new_features.switchCaseToggle = false;
            new_features.forLoopToggle = true;
            new_features.functionToggle = true;

            wtk::printers::printTextFeatureToggles(output_file, new_features);

            wtk::converters::Multiplex<uint8_t> mux(
                uint8_t(p.characteristic), true);
            wtk::IRTree<uint8_t>* new_tree = mux.transform(tree);
            if(new_tree == nullptr) { return 1; }
            wtk::printers::TextTreePrinter<uint8_t> prt(output_file);
            prt.printTree(new_tree);
          }
          else
          {
            wtk::printers::printTextFeatureTogglesParser(output_file, &p);
            wtk::printers::TextTreePrinter<uint8_t> prt(output_file);
            prt.printTree(tree);
          }
          return 0;
        }
      }
    }
  }

  log_error("Cannot convert t2mux for non-relation resource");
  return 1;
}

int t2t()
{
  std::string in_file_str(input_filename);
  wtk::irregular::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);
    wtk::printers::printTextGateSetParser(output_file, &p);
    wtk::printers::printTextFeatureTogglesParser(output_file, &p);

    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::printers::ArithmeticTextStreamPrinter<sst::bignum> prt(output_file);
        return (p.arithmetic()->parseStream(&prt)) ? 0 : 1;
      }
      else
      {
        wtk::printers::BooleanTextStreamPrinter prt(output_file);
        return (p.boolean()->parseStream(&prt)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::printers::TextTreePrinter<sst::bignum> prt(output_file);
          prt.printTree(tree);
          return 0;
        }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else
        {
          wtk::printers::TextTreePrinter<uint8_t> prt(output_file);
          prt.printTree(tree);
          return 0;
        }
      }
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);

    if(p.characteristic != 2)
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.arithmetic()->instance())) ? 0 : 1;
    }
    else
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.boolean()->instance())) ? 0 : 1;
    }
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    wtk::printers::printTextHeaderParser(output_file, &p);
    wtk::printers::printTextResourceParser(output_file, &p);

    if(p.characteristic != 2)
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.arithmetic()->shortWitness())) ? 0 : 1;
    }
    else
    {
      return (wtk::printers::printTextInputStream(
            output_file, p.boolean()->shortWitness())) ? 0 : 1;
    }
  }
  else
  {
    return 1;
  }
}

int t2n()
{
  std::string in_file_str(input_filename);
  wtk::irregular::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::ArithmeticStreamHandler<sst::bignum> handler;
        return (p.arithmetic()->parseStream(&handler)) ? 0 : 1;
      }
      else
      {
        wtk::BooleanStreamHandler handler;
        return (p.boolean()->parseStream(&handler)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    log_warn("instance not supported by t2n");
    return 1;
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    log_warn("short witness not supported by t2n");
    return 1;
  }
  else
  {
    return 1;
  }
}

int a2n()
{
#if ENABLE_ANTLR
  std::string in_file_str( input_filename);
  wtk::antlr::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::ArithmeticStreamHandler<sst::bignum> handler;
        return (p.arithmetic()->parseStream(&handler)) ? 0 : 1;
      }
      else
      {
        wtk::BooleanStreamHandler handler;
        return (p.boolean()->parseStream(&handler)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    log_warn("instance not supported by a2n");
    return 1;
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    log_warn("short witness not supported by a2n");
    return 1;
  }
  else
  {
    return 1;
  }
#else
  log_warn("ANTLR was not built");
  return 1;
#endif
}

int b2n()
{
#if ENABLE_FLATBUFFER
  std::string in_file_str(input_filename);
  wtk::flatbuffer::Parser<sst::bignum> p(in_file_str);

  if(p.parseHdrResParams() && p.resource == wtk::Resource::relation)
  {
    if(p.featureToggles.simple())
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::ArithmeticStreamHandler<sst::bignum> handler;
        return (p.arithmetic()->parseStream(&handler)) ? 0 : 1;
      }
      else
      {
        wtk::BooleanStreamHandler handler;
        return (p.boolean()->parseStream(&handler)) ? 0 : 1;
      }
    }
    else
    {
      if(p.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::IRTree<sst::bignum>* tree = p.arithmetic()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }
      else
      {
        wtk::IRTree<uint8_t>* tree = p.boolean()->parseTree();
        if(tree == nullptr) { return 1; }
        else { return 0; }
      }

      log_error("flatbuffer wip");
      return 1;
    }
  }
  else if(p.resource == wtk::Resource::instance)
  {
    log_warn("instance not supported by b2n");
    return 1;
  }
  else if(p.resource == wtk::Resource::shortWitness)
  {
    log_warn("short witness not supported by b2n");
    return 1;
  }
  else
  {
    return 1;
  }
  log_error("binary to binary: implementation in progress");
  return 1;
#else
  log_error("binary to binary: FlatBuffer disabled");
  return 1;
#endif
}

int main(int argc, char const* argv[])
{
  if(!parse_args(argc, argv))
  {
    print_help();
    exit(1);
  }

  switch(press_mode)
  {
  case PressMode::b2b:
  {
    return b2b();
  }
  case PressMode::b2mux:
  {
    return b2mux();
  }
  case PressMode::b2t:
  {
    return b2t();
  }
  case PressMode::t2b:
  {
    return t2b();
  }
  case PressMode::t2mux:
  {
    return t2mux();
  }
  case PressMode::t2t:
  {
    return t2t();
  }
  case PressMode::t2n:
  {
    return t2n();
  }
  case PressMode::b2n:
  {
    return b2n();
  }
  case PressMode::a2n:
  {
    return a2n();
  }
  }

  log_error("unreachable case");
  return 1;
}
