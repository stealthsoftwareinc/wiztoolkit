/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_CONVERTERS_TREE_HELPER_H_
#define WTK_CONVERTERS_TREE_HELPER_H_

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <wtk/index.h>
#include <wtk/IRTree.h>

#include <wtk/utils/Pool.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/irregular/TextIRTree.t.h>

namespace wtk {
namespace converters {

using wtk::utils::Pool;

/**
 * Holds all the memory pools for each element of the tree.
 */
template<typename Number_T>
struct TreePools
{
  wtk::irregular::TextIRTree<Number_T> tree;

  Pool<wtk::irregular::TextDirectiveList<Number_T>> directiveLists;
  Pool<wtk::irregular::TextBinaryGate> binaryGates;
  Pool<wtk::irregular::TextBinaryConstGate<Number_T>> binaryConstGates;
  Pool<wtk::irregular::TextUnaryGate> unaryGates;
  Pool<wtk::irregular::TextInput> inputs;
  Pool<wtk::irregular::TextTerminal> terminals;
  Pool<wtk::irregular::TextWireRange> wireRanges;
  Pool<wtk::irregular::TextAssign<Number_T>> assigns;
  Pool<wtk::irregular::TextWireList> wireLists;

  Pool<wtk::irregular::TextFunctionDeclare<Number_T>> functionDeclares;
  Pool<wtk::irregular::TextFunctionInvoke> functionInvokes;
  Pool<wtk::irregular::TextAnonFunction<Number_T>> anonFunctions;
  Pool<char> identifiers;

  Pool<wtk::irregular::TextIterExpr> iterExprs;
  Pool<wtk::irregular::TextIterExprWireRange> iterExprWireRanges;
  Pool<wtk::irregular::TextIterExprWireList> iterExprWireLists;

  Pool<wtk::irregular::TextForLoop<Number_T>> forLoops;
  Pool<wtk::irregular::TextIterExprFunctionInvoke> iterExprFunctionInvokes;
  Pool<wtk::irregular::TextIterExprAnonFunction<Number_T>>
    iterExprAnonFunctions;

  std::vector<std::pair<char const*, char*>> iterRegister;
  size_t iterRegisterNext = 0;

  // For unique loop-iterator names.
  void registerIter(char const* iter);
  void unregisterIter();
  char* findRegisteredIter(char const* iter);

  char* strdup(char const* str);
};

template<typename Number_T>
struct WireListBuilder
{
  TreePools<Number_T>* pools = nullptr;
  wtk::irregular::TextWireList* list = nullptr;

  WireListBuilder(TreePools<Number_T>* p);

  void single(wtk::index_t wire);
  void range(wtk::index_t first, wtk::index_t last);
  void range(wtk::WireRange* range);

  // duplicate/deep copy
  void duplicate(wtk::WireList* list);
};

template<typename Number_T>
struct IterExprBuilder
{
  TreePools<Number_T>* pools = nullptr;

  IterExprBuilder(TreePools<Number_T>* p);

  wtk::irregular::TextIterExpr* literal(wtk::index_t l);
  wtk::irregular::TextIterExpr* name(char const* n);

  wtk::irregular::TextIterExpr* add(
      wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r);
  wtk::irregular::TextIterExpr* sub(
      wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r);
  wtk::irregular::TextIterExpr* mul(
      wtk::irregular::TextIterExpr* l, wtk::irregular::TextIterExpr* r);
  wtk::irregular::TextIterExpr* div(
      wtk::irregular::TextIterExpr* l, wtk::index_t r);

  // duplicate/deep copy
  wtk::irregular::TextIterExpr* duplicate(IterExpr* e);
};

template<typename Number_T>
struct IterExprWireListBuilder
{
  TreePools<Number_T>* pools = nullptr;
  wtk::irregular::TextIterExprWireList* list = nullptr;

  IterExprWireListBuilder(TreePools<Number_T>* p);

  void single(wtk::irregular::TextIterExpr* expr);
  void range(
      wtk::irregular::TextIterExpr* first, wtk::irregular::TextIterExpr* last);

  // duplicate/deep copy
  void duplicate(
      wtk::IterExprWireList* list, IterExprBuilder<Number_T>* builder);
};

template<typename Number_T>
struct TreeBuilder
{
  // holder for this tree
  TreePools<Number_T>* pools;

  // this directive-list node.
  wtk::irregular::TextDirectiveList<Number_T>* node;

  TreeBuilder(TreePools<Number_T>* p);

  void binaryGate(wtk::BinaryGate* gate);
  void binaryGate(wtk::index_t out, wtk::BinaryGate::Calculation calc,
      wtk::index_t left, wtk::index_t right);
  void addGate(wtk::index_t out, wtk::index_t left, wtk::index_t right);
  void mulGate(wtk::index_t out, wtk::index_t left, wtk::index_t right);
  void andGate(wtk::index_t out, wtk::index_t left, wtk::index_t right);
  void xorGate(wtk::index_t out, wtk::index_t left, wtk::index_t right);

  void binaryConstGate(wtk::BinaryConstGate<Number_T>* gate);
  void binaryConstGate(wtk::index_t out,
      typename wtk::BinaryConstGate<Number_T>::Calculation calc,
      wtk::index_t left, Number_T right);
  void addcGate(wtk::index_t out, wtk::index_t left, Number_T right);
  void mulcGate(wtk::index_t out, wtk::index_t left, Number_T right);

  void unaryGate(wtk::UnaryGate* gate);
  void unaryGate(
      wtk::index_t out, wtk::UnaryGate::Calculation calc, wtk::index_t in);
  void notGate(wtk::index_t out, wtk::index_t in);
  void copy(wtk::index_t out, wtk::index_t in);

  void assign(wtk::Assign<Number_T>* assign);
  void assign(wtk::index_t output, Number_T value);

  void input(wtk::Input* input);
  void input(wtk::index_t out, wtk::Input::Stream stream);
  void instance(wtk::index_t out);
  void shortWitness(wtk::index_t out);

  void deleteSingle(wtk::Terminal* term);
  void deleteSingle(wtk::index_t wire);
  void deleteRange(wtk::index_t first, wtk::index_t last);
  void deleteRange(wtk::WireRange* range);

  void assertZero(wtk::Terminal* term);
  void assertZero(wtk::index_t wire);

  void functionInvoke(
      WireListBuilder<Number_T>* outputs,
      char const* name,
      WireListBuilder<Number_T>* inputs);

  void anonFunction(
      WireListBuilder<Number_T>* outputs,
      WireListBuilder<Number_T>* inputs,
      wtk::index_t instance_count,
      wtk::index_t witness_count,
      TreeBuilder<Number_T>* body);

  void forLoopInvoke(
      WireListBuilder<Number_T>* outputs,
      char const* iter_name,
      wtk::index_t first,
      wtk::index_t last,
      IterExprWireListBuilder<Number_T>* iter_outputs,
      char const* func_name,
      IterExprWireListBuilder<Number_T>* iter_inputs);

  void forLoopAnonymous(
      WireListBuilder<Number_T>* outputs,
      char const* iter_name,
      wtk::index_t first,
      wtk::index_t last,
      IterExprWireListBuilder<Number_T>* iter_outputs,
      IterExprWireListBuilder<Number_T>* iter_inputs,
      wtk::index_t num_instance,
      wtk::index_t num_witness,
      TreeBuilder<Number_T>* body);
};

} } // namespace wtk::converters

#include <wtk/converters/TreeHelper.t.h>

#endif//WTK_CONVERTERS_TREE_HELPER_H_
