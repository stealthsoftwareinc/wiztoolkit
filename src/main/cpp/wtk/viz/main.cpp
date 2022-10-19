/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>

#include <memory>
#include <utility>

#include <sst/catalog/bignum.hpp>

#include <wtk/Version.h>
#include <wtk/IRParameters.h>

#include <wtk/irregular/Parser.h>

#if ENABLE_FLATBUFFER == 1
#include <wtk/flatbuffer/Parser.h>
#endif

#include <wtk/viz/TreeVisualizer.h>

#define LOG_IDENTIFER "wtk-viz"
#include <stealth_logging.h>

void print_version()
{
  printf("Wiztoolkit wtk-viz\n");
  printf("viz: IR Circuit Visualizer (name subject to change)\n");
  printf("IR Version " IR_MAJOR_STR "." IR_MINOR_STR "." IR_PATCH_STR "\n");
  printf("\nCopyright (C) 2021  Stealth Software Technologies, Inc.\n");
  printf("FOR SIEVE PROGRAM USE ONLY!\n");
#if !ENABLE_FLATBUFFER
  printf("Note: compiled without FlatBuffer.\n");
#endif
}

void print_help()
{
  print_version();
  printf("\nwtk-viz is a tool for visualizing IR relations by converting them to Graphviz dot.\n");
  printf("This allows it to be converted to a picture of the circuit, using the following\n");
  printf("example command.\n");
  printf("\n  dot -Tpng <input>.dot <output>.png\n");
  printf("\nwtk-viz supports visualization of all IR features (simple, functions, for loops,\n");
  printf("and switch statements), but requires the relation has resource validity (check using\n");
  printf("wtk-firealarm).\n");
  printf("\nUsage:\n");
  printf("  wtk-viz [ options ] <input>.rel [ <output>.dot ]\n");
  printf("  wtk-viz [-h | --help | -v | --version ]\n");
  printf("\nOptions:\n");
  printf("  <input>.rel     Input relation file\n");
  printf("  <output>.dot    Output dot file (optional, defaults to stdout)\n");
  printf("  --fg <color>    Foreground color (optional).\n");
  printf("  --bg <color>    Background color (optional).\n");
  printf("  -f              Parse using FlatBuffer (defaults to IRRegular)\n");
  printf("  -h --help       Print help output.\n");
  printf("  -v --version    Print version information.\n");
}

char const* input_filename = nullptr;
FILE* output_file = stdout;
bool use_flatbuffer = false;
char const* foreground = "";
char const* background = "";

bool parse_args(int argc, char const* argv[])
{
  std::string outfile;
  bool has_outfile = false;
  bool has_infile = false;

  for(size_t i = 1; i < (size_t) argc; i++)
  {
    std::string opt(argv[i]);

    if(opt == "-h" || opt == "--help")
    {
      print_help();
      exit(0);
    }
    else if(opt == "-v" || opt == "--version")
    {
      print_version();
      exit(0);
    }
    else if(opt == "--fg" && i != (size_t) argc - 1)
    {
      foreground = argv[i + 1];
      i++;
    }
    else if(opt == "--bg" && i != (size_t) argc - 1)
    {
      background = argv[i + 1];
      i++;
    }
    else if(opt == "-f" && i == (size_t) argc - 1)
    {
      printf("expected input file\n");
      return false;
    }
    else if(opt == "-f")
    {
      use_flatbuffer = true;
    }
    else if(has_infile)
    {
      has_outfile = true;
      outfile = opt;
    }
    else
    {
      has_infile = true;
      input_filename = argv[i];
    }
  }


  if(has_outfile)
  {
    output_file = fopen(outfile.c_str(), "w");
    if(output_file == nullptr)
    {
      log_perror();
      log_error("Error opening output file \"%s\"", outfile.c_str());
      exit(1);
    }
  }

  return true;
}

template<typename Parser_T, typename Number_T>
int visualize(Parser_T* p)
{
  if(p->resource != wtk::Resource::relation)
  {
    log_error("expected a relation.");
    return 1;
  }

  if(p->gateSet.gateSet == wtk::GateSet::arithmetic)
  {
    wtk::IRTree<Number_T>* tree = p->arithmetic()->parseTree();
    if(tree != nullptr)
    {
      wtk::viz::TreeVisualizer<Number_T> viz(
          output_file, foreground, background);
      viz.printTree(tree);
    }
    else
    {
      return 1;
    }
  }
  else
  {
    wtk::IRTree<uint8_t>* tree = p->boolean()->parseTree();
    if(tree != nullptr)
    {
      wtk::viz::TreeVisualizer<uint8_t> viz(
          output_file, foreground, background);
      viz.printTree(tree);
    }
    else
    {
      return 1;
    }
  }

  return 0;
}

int main(int argc, char const* argv[])
{
  if(!parse_args(argc, argv))
  {
    print_help();
    exit(1);
  }

  int ret = 1;
  std::string in_file_str(input_filename);

#if ENABLE_FLATBUFFER
  if(use_flatbuffer)
  {
    wtk::flatbuffer::Parser<sst::bignum> p(in_file_str);
    if(p.parseHdrResParams())
    {
      ret = visualize<wtk::flatbuffer::Parser<sst::bignum>, sst::bignum>(&p);
    }
  }
  else
#else
  if(use_flatbuffer)
  {
    log_error("FlatBuffer was not built.");
  }
  else
#endif
  {
    wtk::irregular::Parser<sst::bignum> p(in_file_str);
    if(p.parseHdrResParams())
    {
      ret = visualize<wtk::irregular::Parser<sst::bignum>, sst::bignum>(&p);
    }
  }

  return ret;
}
