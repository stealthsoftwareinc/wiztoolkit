/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_FIREALARM_CONVERTER_H_
#define WTK_FIREALARM_CONVERTER_H_

#include <cstddef>

#include <wtk/Converter.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/firealarm/Wire.h>
#include <wtk/firealarm/Counters.h>

namespace wtk {
namespace firealarm {

template<typename Number_T, typename OutWire_T, typename InWire_T>
class Converter : public wtk::Converter<Wire<OutWire_T>, Wire<InWire_T>>
{
public:

  Number_T outPrime;
  Number_T inPrime;

  // The output type's counter
  TypeCounter* const outTypeCounter;

  // Number of times the conversion gate is invoked.
  size_t* const count = 0;

  // File name for error reporting.
  char const* const fileName;

  Converter(Number_T op, Number_T ip, size_t const ol, size_t const il,
      TypeCounter* const ofc, size_t* const cc, char const* const fn)
    : wtk::Converter<Wire<OutWire_T>, Wire<InWire_T>>(ol, il), outPrime(op),
      inPrime(ip), outTypeCounter(ofc), count(cc), fileName(fn) { }

  void convert( Wire<OutWire_T>* const out_wires,
      Wire<InWire_T> const* const in_wires, bool modulus) override;

  bool ok = true;
  bool check() override;
};

} } // namespace wtk::firealarm

#include <stealth_logging.h>
#define LOG_IDENTIFIER "wtk::firealarm"

#include <wtk/firealarm/Converter.t.h>

#define LOG_UNINCLUDE
#include <stealth_logging.h>

#endif//WTK_FIREALARM_CONVERTER_H_
