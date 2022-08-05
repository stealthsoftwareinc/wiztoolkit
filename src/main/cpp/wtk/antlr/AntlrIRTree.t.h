/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_TEXT_IR_TREE_H_
#define WTK_ANTLR_TEXT_IR_TREE_H_

#include <cstddef>

#include <wtk/IRTree.h>

#include <wtk/irregular/TextIRTree.t.h>

// This is largely inherited from irregular, simply redefining lineNum
// functions to give real line numbers.

namespace wtk {
namespace antlr {

struct AntlrBinaryGate : public wtk::irregular::TextBinaryGate
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrUnaryGate : public wtk::irregular::TextUnaryGate
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrBinaryConstGate
: public wtk::irregular::TextBinaryConstGate<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrInput : public wtk::irregular::TextInput
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrAssign : public wtk::irregular::TextAssign<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrTerminal : public wtk::irregular::TextTerminal
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrWireRange : public wtk::irregular::TextWireRange
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrWireList : public wtk::irregular::TextWireList
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrFunctionDeclare
: public wtk::irregular::TextFunctionDeclare<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrFunctionInvoke : public wtk::irregular::TextFunctionInvoke
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrAnonFunction : public wtk::irregular::TextAnonFunction<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrIterExpr : public wtk::irregular::TextIterExpr
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrIterExprWireRange : public wtk::irregular::TextIterExprWireRange
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrIterExprWireList : public wtk::irregular::TextIterExprWireList
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrIterExprFunctionInvoke
: public wtk::irregular::TextIterExprFunctionInvoke
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrIterExprAnonFunction
: public wtk::irregular::TextIterExprAnonFunction<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrForLoop : public wtk::irregular::TextForLoop<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

struct AntlrCaseFunctionInvoke : public wtk::irregular::TextCaseFunctionInvoke
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrCaseAnonFunction
: public wtk::irregular::TextCaseAnonFunction<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrCaseBlock : public wtk::irregular::TextCaseBlock<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrSwitchStatement
: public wtk::irregular::TextSwitchStatement<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrDirectiveList : public wtk::irregular::TextDirectiveList<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

template<typename Number_T>
struct AntlrIRTree : public wtk::irregular::TextIRTree<Number_T>
{
  size_t line;

  size_t lineNum() override { return this->line; }
};

} } // namespace wtk::antlr

#endif // WTK_ANTLR_TEXT_IR_TREE_H_
