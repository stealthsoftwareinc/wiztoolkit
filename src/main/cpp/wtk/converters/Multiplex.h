/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_CONVERTERS_MULTIPLEX_H_
#define WTK_CONVERTERS_MULTIPLEX_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/utils/IRTreeUtils.h>

#include <wtk/converters/TreeHelper.h>

namespace wtk {
namespace converters {

// helper struct for adjusting wires.
struct WireAdjuster
{
  wtk::index_t numOutput;
  wtk::index_t numInput;
  wtk::index_t numInstance;
  wtk::index_t numWitness;

  wtk::index_t currInstance = 0;
  wtk::index_t currWitness = 0;

  wtk::index_t selector();
  wtk::index_t adjust(wtk::index_t wire);
  wtk::index_t nextInstance();
  wtk::index_t nextWitness();

  template<typename Number_T>
  void adjust(WireListBuilder<Number_T>* builder, wtk::WireList* list);
  template<typename Number_T>
  void nextInstances(WireListBuilder<Number_T>* builder, wtk::index_t num);
  template<typename Number_T>
  void nextWitnesses(WireListBuilder<Number_T>* builder, wtk::index_t num);
};

struct IterBounds
{
  std::string iter = "";
  wtk::index_t first = 0;
  wtk::index_t last = 0;
};

enum class IterThreshold
{
  lessthan,
  straddle,
  greaterthan
};

template<typename Number_T>
class Multiplex
{
  TreePools<Number_T> pool;

  Number_T characteristic;

  bool isBoolean;

  std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*> functions;
public:
  Multiplex(Number_T c, bool ib);

  wtk::IRTree<Number_T>* transform(wtk::IRTree<Number_T>* original);

private:
  bool transform(
      wtk::DirectiveList<Number_T>* original,
      TreeBuilder<Number_T>* builder,
      std::vector<IterBounds>& bounds);

  bool transformAdjust(
      wtk::DirectiveList<Number_T>* original,
      TreeBuilder<Number_T>* builder,
      WireAdjuster* adjuster,
      std::vector<IterBounds>& bounds);

  void generateCaseChecker();

  // sets up the mux by creating selector bits.
  // returns the index of the first selector wire.
  // next_ephemeral generates ephemeral wires.
  // cond_wire is the switch-condition wire.
  wtk::index_t muxStart(wtk::SwitchStatement<Number_T>* switch_stmt,
      wtk::index_t* const next_ephemeral,
      wtk::index_t const cond_wire,
      TreeBuilder<Number_T>* const builder);

  void muxFinish(wtk::SwitchStatement<Number_T>* switch_stmt,
      wtk::index_t* const next_ephemeral,
      wtk::index_t const first_selector,
      wtk::index_t const first_output,
      wtk::index_t const num_outputs,
      wtk::index_t const* const outer_selector,
    TreeBuilder<Number_T>* const builder);

  IterThreshold iterExprStraddlesThreshold(
      wtk::IterExpr* expr,
      std::vector<IterBounds>& bounds,
      wtk::index_t threshold);

  void iterExprStraddlesThresholdHelper(
      wtk::IterExpr* expr,
      std::vector<IterBounds>& bounds,
      std::unordered_map<std::string, wtk::index_t>& table,
      size_t bounds_place,
      wtk::index_t* max, wtk::index_t* min);

  // Returns false if overflow occurs (invalidating the fast result)
  bool fastIterExprStraddlesThresholdHelper(
      wtk::IterExpr* expr,
      std::vector<IterBounds>& bounds,
      std::unordered_map<std::string, wtk::index_t>& table,
      size_t bounds_place,
      wtk::index_t* max, wtk::index_t* min);

  IterThreshold iterExprRangeStraddlesThreshold(
      wtk::IterExpr* first, wtk::IterExpr* last,
      std::vector<IterBounds>& bounds,
      wtk::index_t threshold);

  void iterExprRangeStraddlesThresholdHelper(
      wtk::IterExpr* first, wtk::IterExpr* last,
      std::vector<IterBounds>& bounds,
      std::unordered_map<std::string, wtk::index_t>& table,
      size_t bounds_place,
      wtk::index_t* max, wtk::index_t* min);

  // Returns false if overflow occurs (invalidating the fast result)
  bool fastIterExprRangeStraddlesThresholdHelper(
      wtk::IterExpr* first, wtk::IterExpr* last,
      std::vector<IterBounds>& bounds,
      std::unordered_map<std::string, wtk::index_t>& table,
      size_t bounds_place,
      wtk::index_t* max, wtk::index_t* min);

  wtk::index_t iterExprRangeSize(
      wtk::IterExpr* first, wtk::IterExpr* last,
      std::vector<IterBounds>& bounds);
};

} } // namespace wtk::converters

#define LOG_IDENTIFIER "multiplex"
#include <stealth_logging.h>

#include <wtk/converters/Multiplex.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_CONVERTERS_MULTIPLEX_H_
