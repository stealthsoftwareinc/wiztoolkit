/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_IR_TREE_UTILS_H_
#define WTK_UTILS_IR_TREE_UTILS_H_

#include <cstdint>
#include <cinttypes>
#include <unordered_map>

#include <wtk/IRTree.h>
#include <wtk/utils/SkipList.h>
#include <wtk/utils/hints.h>

namespace wtk {
namespace utils {

/**
 * Total number of wires in a WireList
 */
wtk::index_t countWireList(wtk::WireList* list);

/**
 * maximum number of instance/witness usages in a switch-case.
 */
template<typename Number_T>
bool maxInsWit(
    wtk::SwitchStatement<Number_T>* switch_stmt,
    std::unordered_map<std::string, wtk::FunctionDeclare<Number_T>*>* func_map,
    wtk::index_t* max_ins,
    wtk::index_t* max_wit);

/**
 * Evaluate an iterator expression using given values for iterator names.
 */
wtk::index_t iterExprEval(
    wtk::IterExpr* expr, std::unordered_map<std::string, wtk::index_t>& iters);

/**
 * Evaluate an iterator expression using given values for iterator names.
 * However, the result is a pointer and it returns true/false to indicate
 * whether or not an overflow occured.
 */
bool iterExprEvalOverflow(wtk::index_t* ret,
    wtk::IterExpr* expr, std::unordered_map<std::string, wtk::index_t>& iters);

/**
 * Checks if an iterator expression consists only of constants
 */
bool iterExprConstant(wtk::IterExpr* expr);

/**
 * Checks if an iterator expression has a linear (or constant) relation to
 * iterator names.
 */
bool iterExprLinear(wtk::IterExpr* expr);

/**
 * Checks if an iterator expression depends only on the named iterator.
 *
 * iter must be null-terminated.
 */
bool iterExprSoleDependence(wtk::IterExpr* expr, char const* iter);

/**
 * Given an output wire list, this function adds each wire of the list
 * to a SkipList. It fails if the wire list duplicates a wire which was already
 * in the SkipList or previously indicated by the list. It may also fail if
 * the list contains an invalid range.
 */
bool listOutputWires(
    wtk::WireList* const wire_list, 
    wtk::utils::SkipList<wtk::index_t>* const list);

/**
 * Given a list of directives, this adds to the given list each wire assigned
 * by each directive. It may fail if a directive duplicates a wire already
 * in the list or assigned by a prior directive. It may also fail if an
 * invalid range is given by a directive.
 */
template<typename Number_T>
bool listAssignedWires(
    wtk::DirectiveList<Number_T>* const directives, 
    wtk::utils::SkipList<wtk::index_t>* const list);

} } // namespace wtk::utils

#define LOG_IDENTIFIER "utils"
#include <stealth_logging.h>

#include <wtk/utils/IRTreeUtils.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_UTILS_IR_TREE_UTILS_H_
