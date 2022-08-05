/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cinttypes>

#include <wtk/printers/BooleanTextStreamPrinter.h>

namespace wtk {
namespace printers {

BooleanTextStreamPrinter::BooleanTextStreamPrinter(FILE* f) : outFile(f)
{
  fprintf(this->outFile, "@begin\n");
}

void BooleanTextStreamPrinter::handleInstance(wtk::index_t const idx)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@instance;\n", idx);
}

void BooleanTextStreamPrinter::handleShortWitness(wtk::index_t const idx)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@short_witness;\n", idx);
}

void BooleanTextStreamPrinter::handleXor(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@xor($%" PRIu64 ",$%" PRIu64 ");\n",
      out, left, right);
}

void BooleanTextStreamPrinter::handleAnd(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@and($%" PRIu64 ",$%" PRIu64 ");\n",
      out, left, right);
}

void BooleanTextStreamPrinter::handleNot(wtk::index_t const out,
    wtk::index_t const left)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@not($%" PRIu64 ");\n", out, left);
}

void BooleanTextStreamPrinter::handleCopy(wtk::index_t const out,
    wtk::index_t const left)
{
  fprintf(this->outFile, "$%" PRIu64 "<-$%" PRIu64 ";\n", out, left);
}

void BooleanTextStreamPrinter::handleAssign(wtk::index_t const out,
    uint8_t const left)
{
  fprintf(this->outFile, "$%" PRIu64 "<-<%" PRIu8 ">;\n", out, left);
}

void BooleanTextStreamPrinter::handleAssertZero(wtk::index_t const in)
{
  fprintf(this->outFile, "@assert_zero($%" PRIu64 ");\n", in);
}

void BooleanTextStreamPrinter::handleDeleteSingle(wtk::index_t const in)
{
  fprintf(this->outFile, "@delete($%" PRIu64 ");\n", in);
}

void BooleanTextStreamPrinter::handleDeleteRange(wtk::index_t const first,
    wtk::index_t const last)
{
  fprintf(this->outFile, "@delete($%" PRIu64 ",$%" PRIu64 ");\n", first, last);
}

void BooleanTextStreamPrinter::handleEnd()
{
  fprintf(this->outFile, "@end\n");
}

} } // namespace wtk::printers
