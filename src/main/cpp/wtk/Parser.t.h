/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

namespace wtk {

template<typename Number_T>
size_t InputStream<Number_T>::lineNum() { return 0; }

template<typename Number_T>
bool Parser<Number_T>::parseHdrResParams()
{
  if(this->parseHeader() && this->parseResource())
  {
    if(this->resource == Resource::relation)
    {
      return this->parseParameters();
    }
    else
    {
      return true;
    }
  }
  else
  {
    return false;
  }
}

} // namespace wtk
