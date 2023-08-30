/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#include <wtk/utils/hints.h>
#include <wtk/irregular/AutomataCtx.h>

#define LOG_IDENTIFIER "wtk::irregular"
#include <stealth_logging.h>

namespace wtk {
namespace irregular {

AutomataCtx::AutomataCtx(char* const b) : buffer(b) { /* empty */ }

bool AutomataCtx::atEnd()
{
  return this->eof & (this->last < this->place);
}

FileAutomataCtx::FileAutomataCtx(size_t const bl)
  : AutomataCtx((char*) malloc(sizeof(char) * bl)),
    bufLen(bl)
{
  this->eof = false;
}

bool FileAutomataCtx::open(char const* const n)
{
  this->name = n;

  if(this->buffer == nullptr)
  {
    log_error("failed to allocate buffer of size %zu", this->bufLen);
    return false;
  }

  this->file = fopen(this->name, "r");
  if(this->file == nullptr)
  {
    log_perror();
    log_error("could not open file %s", this->name);
    return false;
  }

  return this->update();
}

bool FileAutomataCtx::open(FILE* const f, char const* const n)
{
  this->name = n;

  if(this->buffer == nullptr)
  {
    log_error("failed to allocate buffer of size %zu", this->bufLen);
    return false;
  }

  this->file = f;
  if(this->file == nullptr)
  {
    log_error("File %s is not open", this->name);
    return false;
  }

  return this->update();
}

bool FileAutomataCtx::update()
{
  if(this->last >= this->mark && LIKELY(this->last != 0))
  {
    memmove(
        this->buffer, this->buffer + this->mark, 1 + this->last - this->mark);
    this->place = this->place - this->mark;
    this->last = 1 + this->last - this->mark;
    this->mark = 0;
  }
  else
  {
    this->place = 0;
    this->last = 0;
    this->mark = 0;
  }

  size_t const to_read = this->bufLen - this->last;
  size_t const n_read = fread(
      this->buffer + this->last, sizeof(char), to_read, this->file);

  if(n_read != to_read)
  {
    if(ferror(this->file) != 0)
    {
      log_perror();
      return false;
    }
    else if(feof(this->file) != 0) 
    {
      this->eof = true;
    }
  }

  this->last += n_read - 1;
  return true;
}

FileAutomataCtx::~FileAutomataCtx()
{
  free((void*) this->buffer);
  fclose(this->file);
}

StringAutomataCtx::StringAutomataCtx(std::string& str, char const* const n)
  : AutomataCtx(&str[0])
{
  this->last = str.size() - 1;
  this->name = n;
}

bool StringAutomataCtx::update() { return true; }

CharStarAutomataCtx::CharStarAutomataCtx(
    char const* const str, char const* const n)
  : AutomataCtx((char*) str)
{
  this->last = strlen(str);
  this->name = n;
}

bool CharStarAutomataCtx::update() { return true; }

} } // namespace wtk::irregular
