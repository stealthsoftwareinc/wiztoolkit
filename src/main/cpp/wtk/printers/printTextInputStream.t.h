/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace printers {

template<typename Number_T>
bool printTextInputStream(FILE* out, wtk::InputStream<Number_T>* stream)
{
  if(stream == nullptr) { return false; }

  fputs("@begin\n", out);

  Number_T num(0);
  wtk::StreamStatus status = stream->next(&num);

  while(status == wtk::StreamStatus::success)
  {
    fprintf(out, "<%s>;\n", wtk::utils::short_str(num).c_str());
    status = stream->next(&num);
  }

  fputs("@end\n", out);

  return status == wtk::StreamStatus::end;
}

} } // namespace wtk::printers
