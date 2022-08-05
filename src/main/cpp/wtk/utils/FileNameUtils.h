/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_FILE_NAME_UTILS_H_
#define WTK_UTILS_FILE_NAME_UTILS_H_

#include <string>

namespace wtk {
namespace utils {

/**
 * Determines if a file name should be a relation based on having
 * the suffix ".rel"
 */
bool isRelation(std::string const& str);

/**
 * Determines if a file name should be a instance based on having
 * the suffix ".ins"
 */
bool isInstance(std::string const& str);

/**
 * Determines if a file name should be a witness based on having
 * the suffix ".wit"
 */
bool isWitness(std::string const& str);

/**
 * Determines if a file name should be bristol fashion based on having
 * the suffix ".btl"
 */
bool isBristol(std::string const& str);

} } // namespace wtk::utils

#endif // WTK_UTILS_FILE_NAME_UTILS_H_
