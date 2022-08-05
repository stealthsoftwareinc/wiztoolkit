/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_FLATBUFFER_FLAT_NUMBER_HELPER_H_
#define WTK_FLATBUFFER_FLAT_NUMBER_HELPER_H_

#include <wtk/flatbuffer/sieve_ir_generated.h>

#define LOG_IDENTIFIER "wtk::flatbuffer"
#include <stealth_logging.h>

namespace wtk {
namespace flatbuffer {

inline bool checkFlatNumber(flatbuffers::Vector<uint8_t> const* vec)
{
  if(vec == nullptr)
  {
    log_error("Flat Number is null");
    return false;
  }

  return true;
}

template<typename Number_T>
Number_T flatToNumber(flatbuffers::Vector<uint8_t> const* vec)
{
  Number_T ret(0);
  for(size_t i = 0; i < vec->size(); i++)
  {
    ret = ret << 8;
    ret = ret + Number_T(vec->Get(vec->size() - 1 - i));
  }

  return ret;
}

template<typename Number_T>
flatbuffers::Offset<flatbuffers::Vector<uint8_t>> flattenNumber(
    flatbuffers::FlatBufferBuilder* builder, Number_T number)
{
  std::vector<uint8_t> buffer;
  if(number == 0)
  {
    buffer.push_back(0);
  }
  else
  {
    while(number != 0)
    {
      buffer.push_back((uint8_t) (number & 0xff));
      number = number >> 8;
    }
  }

  return builder->CreateVector(buffer);
}

} } // namespace wtk::flatbuffer

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FLATBUFFER_FLAT_NUMBER_HELPER_H_
