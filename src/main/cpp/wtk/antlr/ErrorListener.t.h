/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_ANTLR_ERROR_LISTENERS_H_
#define WTK_ANTLR_ERROR_LISTENERS_H_

#include <antlr4-runtime.h>

#define LOG_IDENTIFIER "antlr"
#include <stealth_logging.h>

/**
 * These are error listeners for ANTLR to share output format with
 * the rest of wiztoolkit.
 */

namespace wtk {
namespace antlr {

struct ErrorListener : public antlr4::BaseErrorListener
{
private:
  std::string filename = "";

public:
  ErrorListener(std::string const& f) : filename(f) { }

  void syntaxError(antlr4::Recognizer*, antlr4::Token*, size_t line, size_t col,
      std::string const& msg, std::exception_ptr)
  {
    log_error("%s:%zu:%zu %s", this->filename.c_str(), line, col, msg.c_str());
  }

  char const* name() { return this->filename.c_str(); }
};

} } // namespace wtk::antlr

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_ANTLR_ERROR_LISTENERS_H_
