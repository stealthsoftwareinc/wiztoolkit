/**
 * Copyright (C) 2022, Stealth Software Technologies, Inc.
 */

#ifndef WTK_CONVERTER_H_
#define WTK_CONVERTER_H_

#include <cstddef>

namespace wtk {

/**
 * The Converter abstract class takes responsibility for performing a
 * conversion.
 */
template<typename OutWire_T, typename InWire_T>
class Converter
{
public:

  // Length of the output vectors
  size_t const outLength;
  // Length of the input vectors
  size_t const inLength;

  Converter(size_t const ol, size_t const il) : outLength(ol), inLength(il) { }

  /**
   * The lineNum will be set by callers of the convert() method so that
   * error reporting may refer to the line number.
   */
  size_t lineNum = 0;

  /**
   * This function must convert the in_wires to the output type and stored
   * in out_wires.
   *
   * Each list of wires is given as a pointer, with the length specified
   * by outLength and inLength.
   *
   * The modulus flag indicates whether or not to use @modulus or
   * @no_modulus semantics. If a failure occurs in the @no_modulus mode,
   * then when check() is called at the end of the proof, it should
   * return false.
   */
  virtual void convert(OutWire_T* const out_wires,
      InWire_T const* const in_wires, bool modulus) = 0;

  /**
   * Indicate if a failure was encountered in the @no_modulus mode of
   * a convert gate.
   */
  virtual bool check() = 0;

  virtual ~Converter() = default;
};

} // namespace wtk

#endif//WTK_CONVERTER_H_
