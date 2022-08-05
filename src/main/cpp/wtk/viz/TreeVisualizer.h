/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_VIZ_TREE_VISUALIZER_H_
#define WTK_VIZ_TREE_VISUALIZER_H_

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cinttypes>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/utils/IRTreeUtils.h>
#include <wtk/utils/Indent.h>

namespace wtk {
namespace viz {

struct LoopBound
{
  char const* const name;
  size_t const first;
  size_t const last;

  LoopBound(char const* n, size_t f, size_t l) : name(n), first(f), last(l) { }
};

/**
 * This is basically a wrapper for FILE*. Each method of this class will
 * print the entirety of it's argument to the output file using grapviz dot.
 */
template<typename Number_T>
struct TreeVisualizer
{
  FILE* outFile = nullptr;
  wtk::utils::Indent indent;
  size_t assertZeroNum = 0;
  size_t scopeNum = 0;
  size_t curScope = 0;

  std::string foreground;
  std::string background;
  bool useFg = false;
  bool useBg = false;

  std::vector<LoopBound> loopBounds;

  TreeVisualizer(FILE* out, std::string fg = "", std::string bg = "");

  TreeVisualizer(FILE* out, char const* const fg, char const* const bg);

  void printBinaryGate(BinaryGate* gate);

  void printBinaryConstGate(BinaryConstGate<Number_T>* gate);

  void printUnaryGate(UnaryGate* gate);

  void printInput(Input* input);

  void printAssign(Assign<Number_T>* assign);

  void printAssertZero(Terminal* assertZero);

  void printDeleteSingle(Terminal* del);

  void printDeleteRange(WireRange* del);

  size_t countWireList(WireList* list);

  void printInputListEdges(WireList* list,
      size_t const outer_scope, size_t const output_count);

  void printInputListNodes(WireList* list, size_t const output_count);

  void printOutputList(WireList* list, size_t const outer_scope);

  void printFunctionInvoke(FunctionInvoke* invoke);

  void printAnonFunction(AnonFunction<Number_T>* anon);

  void printIterExpr(IterExpr* expr);

  size_t printLoopOutputMappingNodes(
      IterExprWireList* list, size_t const loop_scope);

  void printLoopInputMappingNodes(
      IterExprWireList* list, size_t const loop_scope);

  size_t printLoopInputNodes(
      IterExprWireList* list, size_t const output_count);

  void printLoopInputEdges(IterExprWireList* list,
      size_t const output_count, size_t const loop_scope);

  void printLoopOutputEdges(IterExprWireList* list, size_t const loop_scope);

  void printLoopOutputMappingEdges(
      IterExprWireList* list, size_t const loop_scope);

  void printLoopInputMappingEdges(
      IterExprWireList* list, size_t const loop_scope);

  void printForLoop(ForLoop<Number_T>* loop);

  void printSwitchStatement(SwitchStatement<Number_T>* switch_stmt);

  void printDirectiveList(DirectiveList<Number_T>* list);

  void printTree(IRTree<Number_T>* tree);
};

} } //  namespace wtk::viz

#include <wtk/viz/TreeVisualizer.t.h>

#endif//WTK_VIZ_TREE_VISUALIZER_H_
