/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_PRINTERS_PRINT_TEXT_INPUT_STREAM_H_
#define WTK_PRINTERS_PRINT_TEXT_INPUT_STREAM_H_

#include <cstdio>

#include <wtk/Parser.h>
#include <wtk/utils/NumUtils.h>

namespace wtk {
namespace printers {

/**
 * Prints the input stream as text onto the given output file.
 */
template<typename Number_T>
bool printTextInputStream(FILE* out, wtk::InputStream<Number_T>* stream);

} } // namespace wtk::printers

#include <wtk/printers/printTextInputStream.t.h>

#endif//WTK_PRINTERS_PRINT_TEXT_INPUT_STREAM_H_
