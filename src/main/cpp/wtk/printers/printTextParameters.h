/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRINTERS_PRINT_TEXT_PARAMATERS_H_
#define WTK_PRINTERS_PRINT_TEXT_PARAMATERS_H_

#include <cstdio>

#include <wtk/IRParameters.h>
#include <wtk/Parser.h>
#include <wtk/Version.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace printers {

/**
 * Prints the header elements named by the given parser.
 */
template<typename Number_T>
void printTextHeaderParser(FILE* out, wtk::Parser<Number_T>* parser);

/**
 * Prints the header with given characteristics.
 */
template<typename Number_T = uint8_t>
void printTextHeader(FILE* out, Number_T characteristic = Number_T(2),
    size_t degree = 1, size_t major = IR_MAJOR_INT,
    size_t minor = IR_MINOR_INT, size_t patch = IR_PATCH_INT);

/**
 * Prints the resource type from the given parser.
 */
template<typename Number_T>
void printTextResourceParser(FILE* out, wtk::Parser<Number_T>* parser);

/**
 * Prints the resource type.
 */
void printTextResource(FILE* out, wtk::Resource resource);

/**
 * Prints the GateSet from the given parser.
 */
template<typename Number_T>
void printTextGateSetParser(FILE* out, wtk::Parser<Number_T>* parser);

/**
 * Prints the GateSet.
 */
void printTextGateSet(FILE* out, wtk::GateSet gateset);

/**
 * Prints the FeatureToggles from the given parser.
 */
template<typename Number_T>
void printTextFeatureTogglesParser(FILE* out, wtk::Parser<Number_T>* parser);

/**
 * Prints the Feature Toggles.
 */
void printTextFeatureToggles(FILE* out, wtk::FeatureToggles toggles);

} } // namespace wtk::printers

#include <wtk/printers/printTextParameters.t.h>

#endif // WTK_PRINTERS_PRINT_TEXT_PARAMETERS_H_
