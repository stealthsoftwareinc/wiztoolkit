/**
 * Copyright (C) 2023, Stealth Software Technologies, Inc.
 */

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>

#include<sst/catalog/bignum.hpp>

#include <wtk/Parser.h>
#include <wtk/circuit/Parser.h>
#include <wtk/irregular/Parser.h>
#include <wtk/flatbuffer/Parser.h>

#include <wtk/versions.h>
#include <wtk/utils/ParserOrganizer.h>

#include <wtk/press/Printer.h>
#include <wtk/press/TextPrinter.h>
#include <wtk/press/FlatbufferPrinter.h>
#include <wtk/press/NothingPrinter.h>

#define LOG_IDENTIFIER "wtk-press"
#include <stealth_logging.h>

FILE* in_file = stdin;
char const* in_fname = "stdin";
FILE* out_file = stdout;
char const* out_fname = "stdout";

bool in_is_flatbuffer = false;
bool out_is_flatbuffer = true;
bool out_is_nothing = false;

bool help_flag = false;
bool version_flag = false;

bool parse_args(int argc, char const* argv[])
{
  if(argc < 2)
  {
    log_error("Direction required");
    return false;
  }

  int state = 0;
  for(size_t i = 1; i < (size_t) argc; i++)
  {
    if(0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i]))
    {
      help_flag = true;
    }
    else if(0 == strcmp("-v", argv[i]) || 0 == strcmp("--version", argv[i]))
    {
      version_flag = true;
    }
    else
    {
      switch(state)
      {
      case 0:
      {
        // read the direction
        if(0 == strcmp("t2f", argv[i]))
        {
          in_is_flatbuffer = false;
          out_is_flatbuffer = true;
          out_is_nothing = false;
        }
        else if(0 == strcmp("f2t", argv[i]))
        {
          in_is_flatbuffer = true;
          out_is_flatbuffer = false;
          out_is_nothing = false;
        }
        else if(0 == strcmp("t2t", argv[i]))
        {
          in_is_flatbuffer = false;
          out_is_flatbuffer = false;
          out_is_nothing = false;
        }
        else if(0 == strcmp("f2f", argv[i]))
        {
          in_is_flatbuffer = true;
          out_is_flatbuffer = true;
          out_is_nothing = false;
        }
        else if(0 == strcmp("t2n", argv[i]))
        {
          in_is_flatbuffer = false;
          out_is_flatbuffer = false;
          out_is_nothing = true;
        }
        else if(0 == strcmp("f2n", argv[i]))
        {
          in_is_flatbuffer = true;
          out_is_flatbuffer = false;
          out_is_nothing = true;
        }
        else
        {
          log_error("unrecognized direction: %s", argv[i]);
          return false;
        }

        break;
      }
      case 1:
      {
        // read the input file name
        in_fname = argv[i];
        in_file = nullptr;
        in_file = fopen(in_fname, in_is_flatbuffer ? "rb" : "r");
        if(in_file == nullptr)
        {
          log_perror();
          log_error("Could not open input file: %s", in_fname);
          return false;
        }

        break;
      }
      case 2:
      {
        // read the output file name
        if(out_is_nothing)
        {
          log_error("output file prohibited when outputing nothing");
          return false;
        }

        out_fname = argv[i];
        out_file = nullptr;
        out_file = fopen(out_fname, out_is_flatbuffer ? "wb" : "w");
        if(out_file == nullptr)
        {
          log_perror();
          log_error("Could not open output file: %s", out_fname);
          return false;
        }
        break;
      }
      default:
      {
        log_error("unrecognized argument %s", argv[i]);
        return false;
      }
      }

      state++;
    }
  }

  return true;
}

void print_version()
{
  printf("WizToolKit wtk-press\n");
  printf("PRESS: Printout and Representation Exchange Software Suite\n\n");

  printf("WizToolKit Version: %s\n",
      wtk::utils::stringify_version(wtk::WTK_VERSION_MAJOR,
        wtk::WTK_VERSION_MINOR, wtk::WTK_VERSION_PATCH,
        wtk::WTK_VERSION_EXTRA).c_str());
  printf("SIEVE IR Version:   %s\n",
      wtk::utils::stringify_version(wtk::IR_VERSION_MAJOR,
        wtk::IR_VERSION_MINOR, wtk::IR_VERSION_PATCH,
        wtk::IR_VERSION_EXTRA).c_str());

  printf("\nCopyright (C) 2023 Stealth Software Technologies, Inc.\n");
}

void print_help()
{
  printf("\nwtk-press is a tool for converting between the IR's formats. "
      "It can convert from text to flatbuffer or from flatbuffer to text. "
      "It also has a few testing and benchmarking modes.\n\n");

  printf("USAGE:\n");
  printf("  wtk-press [options] <direction> <input-file?> <output-file?>\n\n");

  printf("OPTIONS:\n");
  printf("  <direction>:\n");
  printf("    t2f     text to flatbuffer\n");
  printf("    f2t     flatbuffer to text\n");
  printf("    t2t     text to text\n");
  printf("    f2f     flatbuffer to flatbuffer\n");
  printf("    t2n     text to nothing\n");
  printf("    f2n     flatbuffer to nothing\n");
  printf("  <input-file?>\n");
  printf("            The input file name (optional, defaults to stdin)\n");
  printf("  <output-file?>\n");
  printf("            The output file name (optional, defaults to stdout)\n");
  printf("  --help\n");
  printf("  -h        print out help instructions.\n");
  printf("  --version\n");
  printf("  -v        print out version information.\n");
}

int doCircuit(
    wtk::circuit::Parser<sst::bignum>* const parser,
    wtk::press::Printer<sst::bignum>* const printer)
{
  if(parser == nullptr) { return 1; }
  if(!parser->parseCircuitHeader()) { return 1; }

  for(size_t i = 0; i < parser->plugins.size(); i++)
  {
    if(!printer->printPluginDecl(parser->plugins[i].c_str())) { return 1; }
  }

  for(size_t i = 0; i < parser->types.size(); i++)
  {
    switch(parser->types[i].variety)
    {
    case wtk::circuit::TypeSpec<sst::bignum>::field:
    {
      if(!printer->printFieldType(parser->types[i].prime)) { return 1; }
      break;
    }
    case wtk::circuit::TypeSpec<sst::bignum>::ring:
    {
      if(!printer->printRingType(parser->types[i].bitWidth)) { return 1; }
      break;
    }
    case wtk::circuit::TypeSpec<sst::bignum>::plugin:
    {
      if(!printer->printPluginType(&parser->types[i].binding)) { return 1; }
      break;
    }
    }
  }

  for(size_t i = 0; i < parser->conversions.size(); i++)
  {
    if(!printer->printConversionSpec(&parser->conversions[i])) { return 1; }
  }

  if(!printer->printBeginKw() || !parser->parse(printer)
      || !printer->printEndKw())
  {
    return 1;
  }

  return 0;
}

int doStream(
    wtk::InputStream<sst::bignum>* const stream,
    wtk::press::Printer<sst::bignum>* const printer)
{
  if(stream == nullptr) { return 1; }
  if(!stream->parseStreamHeader()) { return 1; }

  if(stream->type->variety == wtk::circuit::TypeSpec<sst::bignum>::field)
  {
    if(!printer->printFieldType(stream->type->prime)) { return 1; }
  }
  else if(stream->type->variety == wtk::circuit::TypeSpec<sst::bignum>::ring)
  {
    if(!printer->printRingType(stream->type->bitWidth)) { return 1; }
  }

  if(!printer->printBeginKw()) { return 1; }

  wtk::StreamStatus status;
  sst::bignum num = 0;
  while(true)
  {
    status = stream->next(&num);
    if(status == wtk::StreamStatus::error) { return 1; }
    else if(status == wtk::StreamStatus::end) { break; }
    else if(!printer->printStreamValue(num)) { return 1; }
  }

  if(!printer->printEndKw()) { return 1; }

  return 0;
}

int main(int argc, char const* argv[])
{
  int ret = 0;
  if(!parse_args(argc, argv))
  {
    print_help();
    ret = 1;
  }
  else if(help_flag)
  {
    print_version();
    print_help();
    ret = 0;
  }
  else if(version_flag)
  {
    print_version();
    ret = 0;
  }
  else
  {
    std::unique_ptr<wtk::Parser<sst::bignum>> parser;
    bool parser_ok = true;
    std::unique_ptr<wtk::press::Printer<sst::bignum>> printer;
    bool printer_ok = true;

    if(in_is_flatbuffer)
    {
      wtk::flatbuffer::Parser<sst::bignum>* const fb_parser =
        new wtk::flatbuffer::Parser<sst::bignum>();
      if(!fb_parser->open(in_file, in_fname))
      {
        ret = 1;
        parser_ok = false;
      }

      parser = std::unique_ptr<wtk::Parser<sst::bignum>>(fb_parser);
    }
    else
    {
      wtk::irregular::Parser<sst::bignum>* const txt_parser =
          new wtk::irregular::Parser<sst::bignum>();
      if(!txt_parser->open(in_file, in_fname))
      {
        ret = 1;
        parser_ok = false;
      }

      parser = std::unique_ptr<wtk::Parser<sst::bignum>>(txt_parser);
    }

    if(out_is_flatbuffer)
    {
      wtk::press::FlatbufferPrinter<sst::bignum>* const fb_printer =
        new wtk::press::FlatbufferPrinter<sst::bignum>();
      if(!fb_printer->open(out_file))
      {
        ret = 1;
        printer_ok = false;
      }

      printer = std::unique_ptr<wtk::press::Printer<sst::bignum>>(fb_printer);
    }
    else if(out_is_nothing)
    {
      printer = std::unique_ptr<wtk::press::Printer<sst::bignum>>(
          new wtk::press::NothingPrinter<sst::bignum>());
    }
    else
    {
      wtk::press::TextPrinter<sst::bignum>* const txt_printer =
        new wtk::press::TextPrinter<sst::bignum>();
      if(!txt_printer->open(out_file))
      {
        ret = 1;
        printer_ok = false;
      }

      printer = std::unique_ptr<wtk::press::Printer<sst::bignum>>(txt_printer);
    }

    if(parser_ok && printer_ok)
    {
      if(!parser->parseHeader()
          || !printer->printHeader(parser->version.major,
            parser->version.minor, parser->version.patch,
            parser->version.extra.c_str(), parser->type))
      {
        ret = 1;
      }
      else
      {
        switch(parser->type)
        {
        case wtk::ResourceType::circuit:
        {
          ret = doCircuit(parser->circuit(), printer.get());
          break;
        }
        case wtk::ResourceType::public_in:
        {
          ret = doStream(parser->publicIn(), printer.get());
          break;
        }
        case wtk::ResourceType::private_in:
        {
          ret = doStream(parser->privateIn(), printer.get());
          break;
        }
        case wtk::ResourceType::translation:
        {
          log_error("TODO translation");
          ret = 1;
          break;
        }
        case wtk::ResourceType::configuration:
        {
          log_error("TODO configuration");
          ret = 1;
          break;
        }
        }
      }
    }
  }

  fflush(out_file);
  if(out_file != stdout) { fclose(out_file); }

  return ret;
}
