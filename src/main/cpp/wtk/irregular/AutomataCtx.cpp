/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#include <stdexcept>

#include <wtk/utils/hints.h>
#include <wtk/irregular/AutomataCtx.h>

namespace wtk {
namespace irregular {

AutomataCtx::AutomataCtx(FILE* const f,
    size_t bufLen, size_t maxTknLen, size_t extraNulls)
  : file(f),
    bufferLen(bufLen),
    buffer((char*) malloc(sizeof(char) * bufLen + extraNulls)),
    maxTknLen(maxTknLen),
    extraNulls(extraNulls)
{
  if(this->file == nullptr)
  {
    perror("error");
    throw std::runtime_error("Read error.");
  }

  if(this->maxTknLen > this->bufferLen) { this->maxTknLen = this->bufferLen; }
  this->updateBuffer();
}

AutomataCtx::~AutomataCtx()
{
  free((void*) this->buffer);
}

void AutomataCtx::updateMark()
{
  this->mark = this->place;

  if(UNLIKELY(this->place + this->maxTknLen > this->last))
  {
    this->updateBuffer();
  }
}

void AutomataCtx::updateBuffer()
{
  memmove(this->buffer, this->buffer + this->mark, this->last - this->mark);
  this->place = this->place - this->mark;
  this->last = this->last - this->mark;
  this->mark = 0;

  errno = 0;
  size_t const to_read = this->bufferLen - this->last;
  size_t const n_read =
    fread(this->buffer + this->last, sizeof(char), to_read, this->file);
  this->last += n_read;
  memset(this->buffer + this->last, '\0',
      this->bufferLen + this->extraNulls - this->last);

  if(n_read != to_read && feof(this->file) != 0)
  {
    if(n_read == 0)
    {
      this->eof = true;
    }
  }
  else if(n_read != to_read)
  {
    perror("error");
    throw std::runtime_error("Read error.");
  }
}

} } // namespace wtk::irregular
