/**
 * Copyright (C) 2020-2021 Stealth Software Technologies, Inc.
 */

#include <cstdio>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>

#if ENABLE_ANTLR
#include <antlr4-runtime.h>
#endif

#include <openssl/bn.h>
#include <sst/catalog/bignum.hpp>

// #include <wtk/BooleanIR0Handler.h>
// #include <wtk/ArithmeticIR0Handler.h>
#include <wtk/IRParameters.h>
#include <wtk/IRTree.h>
#include <wtk/Parser.h>
#include <wtk/Version.h>
#include <wtk/utils/FileNameUtils.h>
#include <wtk/utils/NumUtils.h>

// #include <wtk/irregular/Parser.h>

#if ENABLE_ANTLR
#include <wtk/antlr/Parser.h>
#endif

#if ENABLE_FLATBUFFER
#include <wtk/flatbuffer/Parser.h>
#endif

#include <wtk/irregular/Parser.h>

// #include <wtk/firealarm/ArithmeticIR0Alarm.h>
// #include <wtk/firealarm/BooleanIR0Alarm.h>
#include <wtk/firealarm/TreeAlarm.h>
#include <wtk/firealarm/TraceTreeAlarm.h>

#include <stealth_logging.h>

bool bignum_is_prime(sst::bignum candidate)
{
#if OPENSSL_VERSION_MAJOR == 3
   return 1 == BN_check_prime(candidate.peek(),sst::bignum::ctx(), nullptr);
#else
  return 1 == BN_is_prime_ex(
      candidate.peek(), BN_prime_checks, sst::bignum::ctx(), nullptr);
#endif
}

void print_version()
{
  printf("Wiztoolkit wtk-firealarm\n");
  printf("FIREALARM: Friendly IR Evaluator And Logic Assertion and Rejection Machine\n");
  printf("IR Version " IR_MAJOR_STR "." IR_MINOR_STR "." IR_PATCH_STR "\n");
  printf("\nCopyright (C) 2020-2021 Stealth Software Technologies, Inc.\n");
  printf("FOR SIEVE PROGRAM USE ONLY!\n");
#if !ENABLE_ANTLR
  printf("Note: compiled without ANTLR.\n");
#endif
#if !ENABLE_FLATBUFFER
  printf("Note: compiled without FlatBuffer.\n");
#endif
}

void print_help()
{
  print_version();
  printf("\nwtk-firealarm is a tool for checking the validity of SIEVE IR resources.\n");
  printf("Given either a single relation, it can check resource validity, or \n");
  printf("given all three resources (relation, instance, witness) together, it checks\n");
  printf("evaluation validity (evaluate the circuit).\n");
  printf("\nwtk-firealarm");
  printf("\nImplementation wise, it supports three parsers. The off-the-shelf ANTLR\n");
  printf("parser gives better line-number information and can recover after a parser\n");
  printf("error, but it uses significantly more time and memory. IRRegular, is faster\n");
  printf("and less memory-intensive, but lacks line-numbering and error recovery.\n");
  printf("Lastly is the FlatBuffer parser, for binary encoding. NOTE: that the IR\n");
  printf("Specification calls for file-suffix \".sieve\" when using the binary format,\n");
  printf("but firealarm continues to use the \".rel\", \".ins\", and \".wit\" suffixes.\n");
  printf("\nUsage:\n");
  printf("  Resource:   wtk-firealarm [ options ] [ relation.rel | instance.ins | witness.wit ]\n");
  printf("  Evaluation: wtk-firealarm [ options ] relation.rel instance.ins witness.wit\n");
  printf("\nOptions:\n");
#if ENABLE_ANTLR
  printf("  -i         Use the IRRegular parser (default is ANTLR).\n");
#else
  printf("  -i         Flag is ignored and IRRegular is always chosen. (ANTLR was not built).\n");
#endif
#if ENABLE_FLATBUFFER
  printf("  -f         Use the FlatBuffer parser (default is ANTLR).\n");
#else
  printf("  -f         Flag is ignored and error occurs (FlatBuffer was not built).");
#endif
  printf("  -t         Produce a trace of function-boundaries, inputs and assertions.\n");
  printf("  -T         Produce a detailed trace with definitions of each wire.\n");
  printf("  -h --help  Print this help text.\n");
  printf("  -v\n");
  printf("  --version  Print the version and copyright information.\n");
}

static bool use_irregular = false;
static bool use_flatbuffer = false;

static bool trace = false;
static bool detail_trace = false;

char const* relation_str = nullptr;
char const* instance_str = nullptr;
char const* witness_str = nullptr;

bool check_evaluation = false;

void parse_args(int const argc, char const* const argv[])
{
  bool parser_chosen = false;
  bool has_relation = false;
  bool has_instance = false;
  bool has_witness = false;
  for(size_t i = 1; i < (size_t) argc; i++)
  {
    std::string arg(argv[i]);

    if("-i" == arg)
    {
      if(parser_chosen)
      {
        log_error("cannot use multiple parser flags");
        exit(1);
      }
      use_irregular = true;
      parser_chosen = true;
    }
    else if("-f" == arg)
    {
      if(parser_chosen)
      {
        log_error("cannot use multiple parser flags");
        exit(1);
      }
#if ENABLE_FLATBUFFER
      use_flatbuffer = true;
      parser_chosen = true;
#else
      log_error("FlatBuffer was not built");
      exit(1);
#endif
    }
    else if("-t" == arg)
    {
      trace = true;
    }
    else if("-T" == arg)
    {
      detail_trace = true;
    }
    else if("--help" == arg || "-h" == arg)
    {
      print_help();
      exit(0);
    }
    else if("--version" == arg || "-v" == arg)
    {
      print_version();
      exit(0);
    }
    else if(wtk::utils::isRelation(arg))
    {
      if(has_relation)
      {
        log_error("cannot have multiple relation files.");
        exit(1);
      }
      relation_str = argv[i];
      has_relation = true;
    }
    else if(wtk::utils::isInstance(arg))
    {
      if(has_instance)
      {
        log_error("cannot have multiple instance files.");
        exit(1);
      }
      instance_str = argv[i];
      has_instance = true;
    }
    else if(wtk::utils::isWitness(arg))
    {
      if(has_witness)
      {
        log_error("cannot have multiple witness files.");
        exit(1);
      }
      witness_str = argv[i];
      has_witness = true;
    }
    else
    {
      log_error("Unrecognized argument: \'%s\'\n\n", arg.c_str());
      print_help();
      exit(1);
    }
  }

  size_t num_res = 0;
  if(has_relation) { num_res++; }
  if(has_instance) { num_res++; }
  if(has_witness) { num_res++; }

  if(num_res == 1)      { check_evaluation = false; }
  else if(num_res == 3) { check_evaluation = true; }
  else
  {
    log_error("Unrecognized IR configuration, must have either a single "
        "resource or a triple of relation, instance, witness");
    exit(1);
  }
}

template<typename Number_T, typename Parser_T>
int checkEvaluation(Parser_T* rel_parser, Parser_T* ins_parser,
    Parser_T* wit_parser, Number_T characteristic, wtk::GateSet* gateset,
    wtk::FeatureToggles* toggles)
{
  wtk::IRTree<Number_T>* relation = rel_parser->parseTree();
  if(relation == nullptr) { return 1; }

  wtk::InputStream<Number_T>* instance = ins_parser->instance();
  if(instance == nullptr) { return 1; }

  wtk::InputStream<Number_T>* short_witness = wit_parser->shortWitness();
  if(short_witness == nullptr) { return 1; }

  if(trace || detail_trace)
  {
    wtk::firealarm::TraceTreeAlarm<Number_T> alarm(
        characteristic, gateset, toggles, relation_str, detail_trace);

    int ret = 1;
    if(alarm.checkTree(relation, instance, short_witness))
    {
      log_info("Evaluation successful (non-ZK).\n");
      ret = 0;
    }
    else { log_error("Evaluation failed (non-ZK).\n"); }

    alarm.logCounts(true);
    return ret;
  }
  else
  {
    wtk::firealarm::TreeAlarm<Number_T> alarm(
        characteristic, gateset, toggles, relation_str);

    int ret = 1;
    if(alarm.checkTree(relation, instance, short_witness))
    {
      log_info("Evaluation successful (non-ZK).\n");
      ret = 0;
    }
    else { log_error("Evaluation failed (non-ZK).\n"); }

    alarm.logCounts(true);
    return ret;
  }
}

template<typename Number_T, typename Parser_T>
int checkRelation(Parser_T* rel_parser, Number_T characteristic,
    wtk::GateSet* gateset, wtk::FeatureToggles* toggles)
{
  wtk::IRTree<Number_T>* relation = rel_parser->parseTree();
  if(relation == nullptr) { return 1; }

  wtk::firealarm::TreeAlarm<Number_T> alarm(
      characteristic, gateset, toggles, relation_str);

  if(alarm.checkTree(relation))
  {
    log_info("Relation is valid.\n");
    alarm.logCounts();
    return 0;
  }
  else
  {
    log_error("Relation is invalid.\n");
    alarm.logCounts();
    return 1;
  }
}

template<typename Number_T>
int checkInputStream( wtk::InputStream<Number_T>* stream,
    Number_T characteristic, std::string const& filename)
{
  if(stream == nullptr) { return 1; }

  bool success = true;
  bool at_end = false;
  size_t count = 0;
  while(!at_end)
  {
    Number_T num;
    wtk::StreamStatus status = stream->next(&num);
    if(status == wtk::StreamStatus::success)
    {
      count++;
      if(characteristic <= num)
      {
        log_error("%s:%zu: Input value %s exceeds characteristic (%s)",
            filename.c_str(), stream->lineNum(), wtk::utils::dec(num).c_str(),
            wtk::utils::dec(characteristic).c_str());
        success = false;
      }
    }
    else if(status == wtk::StreamStatus::end)
    {
      at_end = true;
    }
    else
    {
      log_error("%s:%zu: Error parsing stream",
          filename.c_str(), stream->lineNum());
      at_end = true;
      success = false;
    }
  }

  int ret = 0;
  if(success)
  {
    log_info("Stream is well-formed.\n");
  }
  else
  {
    log_error("Stream is not well-formed.\n");
    ret = 1;
  }

  log_info("  stream value count: %zu", count);

  return ret;
}

template<typename Parser_T>
bool checkVersion(Parser_T* parser)
{
  if(parser->version.major != IR_MAJOR_INT
      && parser->version.minor != IR_MINOR_INT
      && parser->version.patch != IR_PATCH_INT)
  {
    log_error("IR Version not supported %zu.%zu.%zu.",
        parser->version.major, parser->version.minor,
        parser->version.patch);
    return false;
  }
  return true;
}

template<typename Parser_T>
int submain()
{
  if(check_evaluation)
  {
    std::string rel_string(relation_str);
    Parser_T rel_parser(rel_string);
    std::string ins_string(instance_str);
    Parser_T ins_parser(ins_string);
    std::string wit_string(witness_str);
    Parser_T wit_parser(wit_string);

    if(!rel_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(rel_parser.resource != wtk::Resource::relation)
    {
      log_error("file \"%s\" is not a relation.", relation_str);
      return 1;
    }

    if(!ins_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(ins_parser.resource != wtk::Resource::instance)
    {
      log_error("file \"%s\" is not a instance.", instance_str);
      return 1;
    }

    if(!wit_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(wit_parser.resource != wtk::Resource::shortWitness)
    {
      log_error("file \"%s\" is not a witness.", witness_str);
      return 1;
    }

    if(!checkVersion(&rel_parser))
    {
      return 1;
    }

    if(rel_parser.version.major != ins_parser.version.major
        || rel_parser.version.major != wit_parser.version.major
        || rel_parser.version.minor != ins_parser.version.minor
        || rel_parser.version.minor != wit_parser.version.minor
        || rel_parser.version.patch != ins_parser.version.patch
        || rel_parser.version.patch != wit_parser.version.patch)
    {
      log_error("IR Versions do not match between relation (%zu.%zu.%zu),"
          " instance (%zu.%zu.%zu), and witness (%zu.%zu.%zu)",
          rel_parser.version.major, rel_parser.version.minor,
          rel_parser.version.patch, ins_parser.version.major,
          ins_parser.version.minor, ins_parser.version.patch,
          wit_parser.version.major, wit_parser.version.minor,
          wit_parser.version.patch);
      return 1;
    }

    if(!bignum_is_prime(rel_parser.characteristic))
    {
      log_error("Characteristic is not prime (%s)",
          wtk::utils::dec(rel_parser.characteristic).c_str());
      return 1;
    }

    if(rel_parser.characteristic != ins_parser.characteristic ||
        rel_parser.characteristic != wit_parser.characteristic)
    {
      log_error("Characteristic does not match between relation (%s), "
          "instance (%s), and witness (%s)",
          wtk::utils::dec(rel_parser.characteristic).c_str(),
          wtk::utils::dec(ins_parser.characteristic).c_str(),
          wtk::utils::dec(wit_parser.characteristic).c_str());
      return 1;
    }

    if(rel_parser.degree != 1)
    {
      log_error("Degree must be exactly 1 (currently %zu)", rel_parser.degree);
      return 1;
    }

    if(rel_parser.degree != ins_parser.degree ||
        rel_parser.degree != wit_parser.degree)
    {
      log_error("Degree does not match between relation (%zu), "
          "instance (%zu), and witness (%zu)", rel_parser.degree,
          ins_parser.degree, wit_parser.degree);
      return 1;
    }

    if(rel_parser.gateSet.gateSet == wtk::GateSet::boolean)
    {
      if(rel_parser.characteristic != sst::bignum(2))
      {
        log_error("Characteristic must be 2 when gate_set is boolean.");
        return 1;
      }

      return checkEvaluation<uint8_t>(rel_parser.boolean(),
          ins_parser.boolean(), wit_parser.boolean(), 2, &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }

    if(sst::bignum(UINT16_MAX) > rel_parser.characteristic)
    {
      return checkEvaluation<uint32_t>(rel_parser.arithmetic32(),
          ins_parser.arithmetic32(), wit_parser.arithmetic32(),
          uint32_t(rel_parser.characteristic), &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
    else if(sst::bignum(UINT32_MAX) > rel_parser.characteristic)
    {
      return checkEvaluation<uint64_t>(rel_parser.arithmetic64(),
          ins_parser.arithmetic64(), wit_parser.arithmetic64(),
          uint64_t(rel_parser.characteristic), &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
    else
    {
      return checkEvaluation<sst::bignum>(rel_parser.arithmetic(),
          ins_parser.arithmetic(), wit_parser.arithmetic(),
          rel_parser.characteristic, &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
  }
  else if(relation_str != nullptr)
  {
    std::string rel_string(relation_str);
    Parser_T rel_parser(rel_string);

    if(!rel_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(rel_parser.resource != wtk::Resource::relation)
    {
      log_error("file \"%s\" is not a relation.", relation_str);
      return 1;
    }

    if(!checkVersion(&rel_parser))
    {
      return 1;
    }

    if(!bignum_is_prime(rel_parser.characteristic))
    {
      log_error("Characteristic is not prime (%s)",
          wtk::utils::dec(rel_parser.characteristic).c_str());
      return 1;
    }

    if(rel_parser.degree != 1)
    {
      log_error("Degree must be exactly 1 (currently %zu)", rel_parser.degree);
      return 1;
    }

    if(rel_parser.gateSet.gateSet == wtk::GateSet::boolean)
    {
      if(rel_parser.characteristic != sst::bignum(2))
      {
        log_error("Characteristic must be 2 when gate_set is boolean.");
        return 1;
      }

      return checkRelation<uint8_t>(rel_parser.boolean(), 2,
          &rel_parser.gateSet, &rel_parser.featureToggles);
    }

    if(sst::bignum(UINT16_MAX) > rel_parser.characteristic)
    {
      return checkRelation<uint32_t>(rel_parser.arithmetic32(),
          uint32_t(rel_parser.characteristic), &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
    else if(sst::bignum(UINT32_MAX) > rel_parser.characteristic)
    {
      return checkRelation<uint64_t>(rel_parser.arithmetic64(),
          uint64_t(rel_parser.characteristic), &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
    else
    {
      return checkRelation<sst::bignum>(rel_parser.arithmetic(),
          rel_parser.characteristic, &rel_parser.gateSet,
          &rel_parser.featureToggles);
    }
  }
  else if(instance_str != nullptr)
  {
    std::string ins_string(instance_str);
    Parser_T ins_parser(ins_string);

    if(!ins_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(ins_parser.resource != wtk::Resource::instance)
    {
      log_error("file \"%s\" is not an instance.", instance_str);
      return 1;
    }

    if(!checkVersion(&ins_parser))
    {
      return 1;
    }

    if(!bignum_is_prime(ins_parser.characteristic))
    {
      log_error("Characteristic is not prime (%s)",
          wtk::utils::dec(ins_parser.characteristic).c_str());
      return 1;
    }

    if(ins_parser.degree != 1)
    {
      log_error("Degree must be exactly 1 (currently %zu)", ins_parser.degree);
      return 1;
    }

    if(sst::bignum(2) == ins_parser.characteristic)
    {
      return checkInputStream<uint8_t>(ins_parser.boolean()->instance(),
          uint8_t(ins_parser.characteristic), instance_str);
    }
    if(sst::bignum(UINT16_MAX) > ins_parser.characteristic)
    {
      return checkInputStream<uint32_t>(ins_parser.arithmetic32()->instance(),
          uint32_t(ins_parser.characteristic), instance_str);
    }
    else if(sst::bignum(UINT32_MAX) > ins_parser.characteristic)
    {
      return checkInputStream<uint64_t>(ins_parser.arithmetic64()->instance(),
          uint64_t(ins_parser.characteristic), instance_str);
    }
    else
    {
      return checkInputStream<sst::bignum>(ins_parser.arithmetic()->instance(),
          ins_parser.characteristic, instance_str);
    }
  }
  else if(witness_str != nullptr)
  {
    std::string wit_string(witness_str);
    Parser_T wit_parser(wit_string);

    if(!wit_parser.parseHdrResParams())
    {
      return 1;
    }
    else if(wit_parser.resource != wtk::Resource::shortWitness)
    {
      log_error("file \"%s\" is not a witness.", witness_str);
      return 1;
    }

    if(!checkVersion(&wit_parser))
    {
      return 1;
    }

    if(!bignum_is_prime(wit_parser.characteristic))
    {
      log_error("Characteristic is not prime (%s)",
          wtk::utils::dec(wit_parser.characteristic).c_str());
      return 1;
    }

    if(wit_parser.degree != 1)
    {
      log_error("Degree must be exactly 1 (currently %zu)", wit_parser.degree);
      return 1;
    }

    if(sst::bignum(2) == wit_parser.characteristic)
    {
      return checkInputStream<uint8_t>(
          wit_parser.boolean()->shortWitness(),
          uint8_t(wit_parser.characteristic), witness_str);
    }
    if(sst::bignum(UINT16_MAX) > wit_parser.characteristic)
    {
      return checkInputStream<uint32_t>(
          wit_parser.arithmetic32()->shortWitness(),
          uint32_t(wit_parser.characteristic), witness_str);
    }
    else if(sst::bignum(UINT32_MAX) > wit_parser.characteristic)
    {
      return checkInputStream<uint64_t>(
          wit_parser.arithmetic64()->shortWitness(),
          uint64_t(wit_parser.characteristic), witness_str);
    }
    else
    {
      return checkInputStream<sst::bignum>(
          wit_parser.arithmetic()->shortWitness(),
          wit_parser.characteristic, witness_str);
    }
  }
  else
  {
    log_error("missing input");
    return 1;
  }
}

int main(int const argc, char const* const argv[])
{
  LOG_FILE = stdout;
  log_set_identifier("wtk-firealarm");

  parse_args(argc, argv);

  int ret = 0;
  try
  {
#if ENABLE_ANTLR
    if(!use_irregular && !use_flatbuffer)
    {
      ret = submain<wtk::antlr::Parser<sst::bignum>>();
    }
    else
#endif
    if(use_flatbuffer)
    {
#if ENABLE_FLATBUFFER
      ret = submain<wtk::flatbuffer::Parser<sst::bignum>>();
#else
      ret = 1;
      log_error("FlatBuffer disabled by build.");
#endif
    }
    else
    {
      ret = submain<wtk::irregular::Parser<sst::bignum>>();
    }
  }
  catch(...)
  {
    log_error("exiting after failure. (Did you use the correct parser?)\n");
    ret = 1;
  }

  return ret;
}
