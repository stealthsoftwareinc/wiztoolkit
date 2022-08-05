/**
 * Copyright (C) 2020 Stealth Software Technologies, Inc.
 */

#include <wtk/BooleanStreamHandler.h>

namespace wtk {

void BooleanStreamHandler::setLineNum(size_t const line)
{
  (void) line;
}

void BooleanStreamHandler::handleInstance(wtk::index_t const idx)
{
  (void) idx;
}

void BooleanStreamHandler::handleShortWitness(wtk::index_t const idx)
{
  (void) idx;
}

void BooleanStreamHandler::handleXor(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  (void) out;
  (void) left;
  (void) right;
}

void BooleanStreamHandler::handleAnd(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  (void) out;
  (void) left;
  (void) right;
}

void BooleanStreamHandler::handleNot(wtk::index_t const out,
    wtk::index_t const in)
{
  (void) out;
  (void) in;
}

void BooleanStreamHandler::handleAssign(wtk::index_t const out, uint8_t const val)
{
  (void) out;
  (void) val;
}

void BooleanStreamHandler::handleCopy(wtk::index_t const out,
    wtk::index_t const in)
{
  (void) out;
  (void) in;
}

void BooleanStreamHandler::handleAssertZero(wtk::index_t const in)
{
  (void) in;
}

void BooleanStreamHandler::handleDeleteSingle(wtk::index_t const in)
{
  (void) in;
}

void BooleanStreamHandler::handleDeleteRange(wtk::index_t const first,
    wtk::index_t const last)
{
  (void) first;
  (void) last;
}

void BooleanStreamHandler::handleEnd() { }

} // namespace wtk
