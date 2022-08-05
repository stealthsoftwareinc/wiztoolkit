/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRINTERS_TEXT_TREE_PRINTER_H_
#define WTK_PRINTERS_TEXT_TREE_PRINTER_H_

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cinttypes>

#include <wtk/index.h>
#include <wtk/IRTree.h>
#include <wtk/utils/NumUtils.h>
#include <wtk/utils/Indent.h>

namespace wtk {
namespace printers {

/**
 * This is basically a wrapper for FILE*. Each method of this class will
 * print the entirety of it's argument to the output file.
 */
template<typename Number_T>
struct TextTreePrinter
{
  FILE* outFile = nullptr;
  wtk::utils::Indent indent;

  TextTreePrinter(FILE* out) : outFile(out) { }

  void printBinaryGate(BinaryGate* gate);

  void printBinaryConstGate(BinaryConstGate<Number_T>* gate);

  void printUnaryGate(UnaryGate* gate);

  void printInput(Input* input);

  void printAssign(Assign<Number_T>* assign);

  void printAssertZero(Terminal* assertZero);

  void printDeleteSingle(Terminal* del);

  void printDeleteRange(WireRange* del);

  void printWireList(WireList* list);

  void printFunctionInvoke(FunctionInvoke* invoke);

  void printAnonFunction(AnonFunction<Number_T>* anon);

  void printIterExpr(IterExpr* expr);

  void printIterExprWireList(IterExprWireList* list);

  void printForLoop(ForLoop<Number_T>* loop);

  void printSwitchStatement(SwitchStatement<Number_T>* switch_stmt);

  void printDirectiveList(DirectiveList<Number_T>* list);

  void printTree(IRTree<Number_T>* tree);
};

} } //  namespace wtk::printers

#include <wtk/printers/TextTreePrinter.t.h>

#endif // WTK_PRINTERS_TEXT_TREE_PRINTER_H_
