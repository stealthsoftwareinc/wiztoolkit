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
  FILE* const file;
  size_t const bufferLen;


  /** The place where the automata is currently reading */
  size_t place = 0;

// readonly:
  char* const buffer;

  /** The start character of the string the automata is currently generating */
  size_t mark = 0;

  /** The expected maximum length of a token */
  size_t maxTknLen;
  /** A number of extra nulls to padd the end of the buffer and cause an
   * automata to fail gracefully in the case of an overrun */
  size_t const extraNulls;
  /** The place in the buffer that is the end of the buffer. */
  size_t last = 0;

  /** indicates if the last place is an EOF character */
  bool eof = false;

  /**
   * Constructor for an AutomataCtx.
   *
   * @param an open FILE pointer.
   * @param Maximum number of characters to read from the buffer at once.
   */
  AutomataCtx(FILE* const f,
      size_t bufLen = 65536, size_t maxTkn = 1024, size_t extraNulls = 64);

  AutomataCtx(AutomataCtx const&) = delete;
  AutomataCtx& operator=(AutomataCtx const&) = delete;

  ~AutomataCtx();

  /**
   * Update the mark position for the next automata.
   */
  void updateMark();

  /**
   * read the next portion of the file. (called automatically by updateMark
   * when the remaining characters in the buffer are less than the maxTknLen).
   */
  void updateBuffer();
};

} } // namespace wtk::irregular

#endif//WTK_IRREGULAR_AUTOMATA_CTX_H_
