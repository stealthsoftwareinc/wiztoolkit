/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#ifndef WTK_IRREGULAR_AUTOMATA_CTX_H_
#define WTK_IRREGULAR_AUTOMATA_CTX_H_

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cerrno>
#include <cstring>

#include <string>

namespace wtk {
namespace irregular {

class AutomataCtx
{
public:
  // The read buffer (caller or subclass has responsibility for freeing)
  char* const buffer;

  // A mark in the buffer indicating the start of the current token
  size_t mark = 0;

  // The scan's current place in the buffer.
  size_t place = 0;

  // The last valid character in the buffer (rather than the buffer length)
  size_t last = 0;

  // Indicates that the last character is the end of file. (rather than
  // being able to load more characters)
  bool eof = true;

  // A file name for error reporting.
  char const* name = "<ctx>";

  // The current line number
  size_t lineNum = 1;

  // Constructor with a buffer pointer.
  AutomataCtx(char* const b);

  // Update the buffer (if possible). Returns false on failure, true otherwise.
  virtual bool update() = 0;

  // Indicates that the end of file has been parsed.
  bool atEnd();

  virtual ~AutomataCtx() = default;
};

// An AutomataCtx for working with files
class FileAutomataCtx : public AutomataCtx
{
private:
  // The file from which to read.
  FILE* file;

  // The total length of the buffer.
  size_t const bufLen;

public:
  // Constructor with optional buffer-length and update threshold.
  FileAutomataCtx(size_t const bl = 65536);

  // Open the context when given a file name
  // returns false on failure
  bool open(char const* const n);

  // Open the context with an already open FILE*, name is for error reporting
  // returns false on failure
  bool open(FILE* const f, char const* const n);

  // Updates by reading from file.
  bool update() override;

  // frees the buffer and closes the file.
  virtual ~FileAutomataCtx() override;
};

// An AutomataCtx for working with strings.
class StringAutomataCtx : public AutomataCtx
{
public:

  StringAutomataCtx(std::string& str, char const* const n = "<string>");

  // doesn't update
  bool update() override;
};

// An AutomataCtx for working with char*s
class CharStarAutomataCtx : public AutomataCtx
{
public:
  CharStarAutomataCtx(char const* const str, char const* const n = "<char*>");

  // doesn't update
  bool update() override;
};

} } // namespace wtk::irregular

#endif//WTK_IRREGULAR_AUTOMATA_CTX_H_
