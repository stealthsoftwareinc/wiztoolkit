/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace printers {

template <typename Number_T>
ArithmeticTextStreamPrinter<Number_T>::ArithmeticTextStreamPrinter(FILE* ofile)
  : outFile(ofile)
{
  fprintf(this->outFile, "@begin\n");
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleInstance(wtk::index_t const idx)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@instance;\n", idx);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleShortWitness(wtk::index_t const idx)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@short_witness;\n", idx);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleAdd(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@add($%" PRIu64 ",$%" PRIu64 ");\n",
      out, left, right);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleMul(wtk::index_t const out,
    wtk::index_t const left, wtk::index_t const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@mul($%" PRIu64 ",$%" PRIu64 ");\n",
      out, left, right);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleAddC(wtk::index_t const out,
    wtk::index_t const left, Number_T const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@addc($%" PRIu64 ",<%s>);\n", out, left,
      wtk::utils::short_str(right).c_str());
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleMulC(wtk::index_t const out,
    wtk::index_t const left, Number_T const right)
{
  fprintf(this->outFile, "$%" PRIu64 "<-@mulc($%" PRIu64 ",<%s>);\n", out, left,
      wtk::utils::short_str(right).c_str());
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleCopy(wtk::index_t const out,
    wtk::index_t const left)
{
  fprintf(this->outFile, "$%" PRIu64 "<-$%" PRIu64 ";\n", out, left);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleAssign(wtk::index_t const out,
    Number_T const left)
{
  fprintf(this->outFile, "$%" PRIu64 "<-<%s>;\n", out,
      wtk::utils::short_str(left).c_str());
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleAssertZero(
    wtk::index_t const in)
{
  fprintf(this->outFile, "@assert_zero($%" PRIu64 ");\n", in);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleDeleteSingle(
    wtk::index_t const in)
{
  fprintf(this->outFile, "@delete($%" PRIu64 ");\n", in);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleDeleteRange(
    wtk::index_t const first, wtk::index_t const last)
{
  fprintf(this->outFile, "@delete($%" PRIu64 ", $%" PRIu64 ");\n", first, last);
}

template <typename Number_T>
void ArithmeticTextStreamPrinter<Number_T>::handleEnd()
{
  fprintf(this->outFile, "@end\n");
}

} } // namespace wtk::printers
