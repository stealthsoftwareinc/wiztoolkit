/**
 * Copyright (C) 2021, Stealth Software Technologies, Inc.
 */

#include <string>
#include <chrono>

#include <wtk/Parser.h>
#include <wtk/IRTree.h>
#include <wtk/IRParameters.h>
#include <wtk/irregular/Parser.h>
#include <wtk/firealarm/TreeAlarm.h>

#include <wtk/utils/IRTreeUtils.h>

#include <wtk/bolt/Builder.h>
#include <wtk/bolt/Evaluator.h>
#include <wtk/bolt/NoZKBackend.h>

#include <wtk/bolt/PLASMASnooze.h>
#include <wtk/bolt/ArithmeticPLASMASnoozeHandler.h>
#include <wtk/bolt/BooleanPLASMASnoozeHandler.h>

#define LOG_IDENTIFIER "wtk-bolt"
#include <stealth_logging.h>

#define NOW std::chrono::high_resolution_clock::now()

#define ELAPSED_MS(start, stop) static_cast<unsigned long>( \
    std::chrono::duration_cast<std::chrono::milliseconds>( \
      (stop) - (start)).count())

int main(int argc, char const* argv[])
{
  if(argc != 5)
  {
    log_error("Usage: wtk-bolt [bolt|plasmasnooze|stream] ${relation} ${instance} ${witness}");
  }

  int ret = 0;

  std::string mode(argv[1]);
  if(mode != "bolt" && mode != "plasmasnooze" && mode != "stream")
  {
    log_error("Available modes: \"bolt\", \"plasmasnooze\", or \"stream\"");
    return 1;
  }
  std::string rel_str(argv[2]);
  std::string ins_str(argv[3]);
  std::string wit_str(argv[4]);

  auto parse_start = NOW;
  wtk::irregular::Parser<uint64_t> rel(rel_str);

  if(rel.parseHdrResParams() && rel.resource == wtk::Resource::relation)
  {
    if(mode == "stream")
    {
      if(!rel.featureToggles.simple())
      {
        log_error("Cannot stream parse a non-simple relation.");
        return 1;
      }

      if(rel.gateSet.gateSet == wtk::GateSet::arithmetic)
      {
        wtk::irregular::Parser<uint64_t> ins(ins_str);
        wtk::irregular::Parser<uint64_t> wit(wit_str);
        if(!ins.parseHdrResParams() || ins.resource != wtk::Resource::instance
            || !wit.parseHdrResParams()
            || wit.resource != wtk::Resource::shortWitness)
        {
          return 1;
        }
        wtk::InputStream<uint64_t>* ins_stream = ins.arithmetic()->instance();
        wtk::InputStream<uint64_t>* wit_stream =
          wit.arithmetic()->shortWitness();

        wtk::bolt::NoZKBackend<unsigned __int128, uint64_t> backend(
            rel.characteristic, &rel.gateSet);
        wtk::bolt::ArithmeticPLASMASnoozeHandler<unsigned __int128, uint64_t>
          handler(&backend, ins_stream, wit_stream);

        int ret = 0;
        wtk::bolt::PLASMASnoozeStatus status = handler.check();

        if(!rel.arithmetic()->parseStream(&handler))
        {
          ret = 1;
        }
        else if(status == wtk::bolt::PLASMASnoozeStatus::bad_relation)
        {
          log_error("stream fail: relation is poorly formed.");
          ret = 1;
        }
        else if(status == wtk::bolt::PLASMASnoozeStatus::bad_stream)
        {
          log_error("stream fail: instance or witness is poorly formed.");
          ret = 1;
        }
        else if(!backend.check())
        {
          log_error("backend fail: proof is invalid");
          ret = 1;
        }

        backend.finish();

        auto parse_end = NOW;
        log_info("stream time: %lums", ELAPSED_MS(parse_start, parse_end));

        return ret;
      }
      else
      {
        wtk::irregular::Parser<uint64_t> ins(ins_str);
        wtk::irregular::Parser<uint64_t> wit(wit_str);
        if(!ins.parseHdrResParams() || ins.resource != wtk::Resource::instance
            || !wit.parseHdrResParams()
            || wit.resource != wtk::Resource::shortWitness)
        {
          return 1;
        }
        wtk::InputStream<uint8_t>* ins_stream = ins.boolean()->instance();
        wtk::InputStream<uint8_t>* wit_stream = wit.boolean()->shortWitness();

        wtk::bolt::NoZKBackend<uint8_t, uint8_t> backend(
            (uint8_t) rel.characteristic, &rel.gateSet);
        wtk::bolt::BooleanPLASMASnoozeHandler<uint8_t> handler(
            &backend, ins_stream, wit_stream);

        int ret = 0;
        wtk::bolt::PLASMASnoozeStatus status = handler.check();

        if(!rel.boolean()->parseStream(&handler))
        {
          ret = 1;
        }
        else if(status == wtk::bolt::PLASMASnoozeStatus::bad_relation)
        {
          log_error("stream fail: relation is poorly formed.");
          ret = 1;
        }
        else if(status == wtk::bolt::PLASMASnoozeStatus::bad_stream)
        {
          log_error("stream fail: instance or witness is poorly formed.");
          ret = 1;
        }
        else if(!backend.check())
        {
          log_error("backend fail: proof is invalid");
          ret = 1;
        }

        backend.finish();

        auto parse_end = NOW;
        log_info("stream time: %lums", ELAPSED_MS(parse_start, parse_end));

        return ret;
      }
    }
    else
    {
      wtk::IRTree<uint64_t>* rel_tree = rel.arithmetic()->parseTree();

      auto parse_end = NOW;
      log_info("parse time: %lums", ELAPSED_MS(parse_start, parse_end));

      if(rel_tree != nullptr)
      {
        // BOLT
        if(mode == "bolt")
        {
          wtk::irregular::Parser<uint64_t> ins(ins_str);
          wtk::irregular::Parser<uint64_t> wit(wit_str);
          if(!ins.parseHdrResParams() || ins.resource != wtk::Resource::instance
              || !wit.parseHdrResParams()
              || wit.resource != wtk::Resource::shortWitness)
          {
            return 1;
          }
          wtk::InputStream<uint64_t>* ins_stream = ins.arithmetic()->instance();
          wtk::InputStream<uint64_t>* wit_stream =
            wit.arithmetic()->shortWitness();

          wtk::bolt::Builder<unsigned __int128, uint64_t> tb(rel.characteristic);

          auto bolt_build_start = NOW;
          wtk::bolt::Bolt<unsigned __int128, uint64_t>* bolt = tb.build(rel_tree);
          auto bolt_build_end = NOW;
          log_info("bolt build time: %lums",
              ELAPSED_MS(bolt_build_start, bolt_build_end));

          if(bolt != nullptr)
          {
            log_info("bolt build success, relation is well formed");
            auto bolt_eval_start = NOW;
            wtk::bolt::NoZKBackend<unsigned __int128, uint64_t> backend(
                rel.characteristic, &rel.gateSet);
            wtk::bolt::Evaluator<unsigned __int128, uint64_t> eval(&backend);
            if(!eval.evaluate(bolt, ins_stream, wit_stream))
            {
              log_error("bolt eval fail, instance or witness is poorly formed");
              ret = 1;
            }
            else if(!backend.check())
            {
              log_error("backend fail: proof is invalid");
              ret = 1;
            }

            backend.finish();

            auto bolt_eval_end = NOW;
            log_info("bolt eval time: %lums",
                ELAPSED_MS(bolt_eval_start, bolt_eval_end));
          }
          else
          {
            log_error("bolt build fail: relation is poorly-formed");
            ret = 1;
          }
        }

        // PLASMASnooze
        if(mode == "plasmasnooze")
        {
          wtk::irregular::Parser<uint64_t> ins(ins_str);
          wtk::irregular::Parser<uint64_t> wit(wit_str);
          if(!ins.parseHdrResParams() || ins.resource != wtk::Resource::instance
              || !wit.parseHdrResParams()
              || wit.resource != wtk::Resource::shortWitness)
          {
            return 1;
          }
          wtk::InputStream<uint64_t>* ins_stream = ins.arithmetic()->instance();
          wtk::InputStream<uint64_t>* wit_stream =
            wit.arithmetic()->shortWitness();

          wtk::bolt::NoZKBackend<unsigned __int128, uint64_t> backend(
              rel.characteristic, &rel.gateSet);
          wtk::bolt::PLASMASnooze<unsigned __int128, uint64_t> snooze(&backend);

          auto plasmasnooze_start = NOW;
          wtk::bolt::PLASMASnoozeStatus status =
            snooze.evaluate(rel_tree, ins_stream, wit_stream);

          if(status == wtk::bolt::PLASMASnoozeStatus::bad_relation)
          {
            log_error("plasmasnooze fail: poorly formed relation");
            ret = 1;
          }
          else if(status == wtk::bolt::PLASMASnoozeStatus::bad_stream)
          {
            log_error("plasmasnooze fail: poorly formed instance or witness");
            ret = 1;
          }
          else if(!backend.check())
          {
            log_error("backend fail: proof is invalid");
            ret = 1;
          }

          backend.finish();

          auto plasmasnooze_end = NOW;
          log_info("plasmasnooze time: %lums",
              ELAPSED_MS(plasmasnooze_start, plasmasnooze_end));
        }
      }
    }
  }

  return ret;
}
