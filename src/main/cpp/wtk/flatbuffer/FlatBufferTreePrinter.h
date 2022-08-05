/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_FLATBUFFER_TREE_PRINTER_H_
#define WTK_FLATBUFFER_FLATBUFFER_TREE_PRINTER_H_

#include <cstdlib>
#include <cstddef>
#include <cinttypes>
#include <string>

#include <wtk/utils/NumUtils.h>

#include <wtk/flatbuffer/sieve_ir_generated.h>
#include <wtk/flatbuffer/FlatNumberHelper.t.h>

namespace wtk {
namespace flatbuffer {

using namespace wtk_gen_flatbuffer;

template<typename Number_T>
struct FlatBufferTreePrinter : public FlatBufferPrinter<Number_T>
{
  FlatBufferTreePrinter(FILE* ofile);

  void printBinaryGate(wtk::BinaryGate* gate,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printBinaryConstGate(wtk::BinaryConstGate<Number_T>* gate,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printUnaryGate(wtk::UnaryGate* gate,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printInput(wtk::Input* input,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printAssign(wtk::Assign<Number_T>* assign,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printAssertZero(wtk::Terminal* assertZero,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printDeleteSingle(wtk::Terminal* del,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printDeleteRange(wtk::WireRange* del,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  flatbuffers::Offset<wtk_gen_flatbuffer::WireList>
    printWireList(wtk::WireList* list);

  void printFunctionInvoke(wtk::FunctionInvoke* invoke,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printAnonFunction(wtk::AnonFunction<Number_T>* anon,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  flatbuffers::Offset<IterExprWireNumber> printIterExpr(wtk::IterExpr* expr);

  flatbuffers::Offset<wtk_gen_flatbuffer::IterExprWireList>
    printIterExprWireList(wtk::IterExprWireList* list);

  void printForLoop(wtk::ForLoop<Number_T>* loop,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printSwitchStatement(wtk::SwitchStatement<Number_T>* switch_stmt,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printDirectiveList(wtk::DirectiveList<Number_T>* list,
      std::vector<flatbuffers::Offset<Directive>>* directives);

  void printTree(wtk::IRTree<Number_T>* tree);
};

} } // namespace wtk::flatbuffer

#include <wtk/flatbuffer/FlatBufferTreePrinter.t.h>

#endif//WTK_FLATBUFFER_FLATBUFFER_TREE_PRINTER_H_
