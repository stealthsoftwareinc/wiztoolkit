/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/IRTree.h>

namespace wtk {

size_t BinaryGate::lineNum() { return 0; }

size_t UnaryGate::lineNum() { return 0; }

size_t Input::lineNum() { return 0; }

size_t Terminal::lineNum() { return 0; }

size_t WireRange::lineNum() { return 0; }

size_t WireList::lineNum() { return 0; }

size_t FunctionInvoke::lineNum() { return 0; }

size_t IterExpr::lineNum() { return 0; }

size_t IterExprWireRange::lineNum() { return 0; }

size_t IterExprWireList::lineNum() { return 0; }

size_t IterExprFunctionInvoke::lineNum() { return 0; }

size_t CaseFunctionInvoke::lineNum() { return 0; }

} // namespace wtk
