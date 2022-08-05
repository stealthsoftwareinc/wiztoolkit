/**
 * Copyright 2021 Stealth Software Technologies, Inc.
 */

#include <string>

#ifndef WTK_UTILS_INDENT_H_
#define WTK_UTILS_INDENT_H_

namespace wtk {
namespace utils {

/**
 * This is a really dumb thing to print indent whitespace.
 */
class Indent
{
  std::string whitespace;
  size_t size;

public:
  Indent(size_t s = 0) : whitespace("              "), size(s) { }

  void inc(size_t n = 2)
  {
    char const* SPACES_32 = "                                "; // len=32
    while(this->whitespace.size() < this->size + n)
    {
      size_t space_needed =
        (this->size + n - this->whitespace.size() > 32)
          ? 32
          : this->size + n - this->whitespace.size();

      this->whitespace.append(SPACES_32, space_needed);
    }

    this->size = this->size + n;
  }

  void dec(size_t n = 2)
  {
    this->size = n > (this->size) ? 0 : this->size - n;
  }

  char const* get()
  {
    return this->whitespace.c_str() + (this->whitespace.size() - this->size);
  }

  void print(FILE* f)
  {
    fputs(this->get(), f);
  }
};


} } // namespace wtk::utils

#endif//WTK_UTILS_INDENT_H_

