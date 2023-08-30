/**
 * Copyright (C) 2020-2023 Stealth Software Technologies, Inc.
 */

namespace wtk {
namespace utils {

// TODO: how to do signed and unsigned?

char const* const DEC_DIGITS = "0123456789";

template<typename Number_T>
std::string dec(Number_T num)
{
  if(num == 0) { return std::string("0"); }

  std::string ret;
  while(num != 0)
  {
    // Ignore static analysis false positives when negative sst::bignums may
    // occur. This should be impossible because WizToolKit treats all numerics
    // as unsigned, even though sst::bignum is signed.

    // NOLINTNEXTLINE
    ret += DEC_DIGITS[cast_size(num % 10)];
    num = Number_T(num / 10);
  }

  std::reverse(ret.begin(), ret.end());
  return ret;
}

char const* const HEX_DIGITS = "0123456789ABCDEF";

template<typename Number_T>
std::string hex(Number_T num)
{
  if(num == 0) { return std::string("0x0"); }

  std::string ret;
  while(num != 0)
  {
    ret += HEX_DIGITS[cast_size(num & 0x0F)];
    num = Number_T(num >> 4);
  }

  ret.append("x0");
  std::reverse(ret.begin(), ret.end());
  return ret;
}

template <typename Number_T>
std::string short_str(Number_T num)
{
  if(num > 1000000000000) { return hex(num); }
  else                    { return dec(num); }
}

/**
 * A list of conversions of ascii digits to numeric values.
 *  - 0-9 is 0-9
 *  - a-f is 10-15
 *  - A-F is 10-15
 */
extern const unsigned int NUMERIC_VALS[128];

template<typename Number_T>
ALWAYS_INLINE static inline void hex_to_uint(
    char const* start, char const* end, Number_T& num)
{
  num = 0;
  for(char const* place = start; place < end; ++place) {
    num = Number_T(num << 4);
    num = Number_T(num + Number_T(NUMERIC_VALS[(size_t) *place]));
  }
}

template<typename Number_T>
ALWAYS_INLINE static inline void dec_to_uint(
    char const* start, char const* end, Number_T& num)
{
  num = 0;
  for(char const* place = start; place < end; ++place) {
    num = Number_T(num * 10);
    num = Number_T(num + Number_T(NUMERIC_VALS[(size_t) *place]));
  }
}

template<typename Number_T>
ALWAYS_INLINE static inline void oct_to_uint(
    char const* start, char const* end, Number_T& num)
{
  num = 0;
  for(char const* place = start; place < end; ++place) {
    num = Number_T(num << 3);
    num = Number_T(num + Number_T(NUMERIC_VALS[(size_t) *place]));
  }
}

template<typename Number_T>
ALWAYS_INLINE static inline void bin_to_uint(
    char const* start, char const* end, Number_T& num)
{
  num = 0;
  for(char const* place = start; place < end; ++place) {
    num = Number_T(num << 1);
    num = Number_T(num + Number_T(NUMERIC_VALS[(size_t) *place]));
  }
}

template<typename Number_T>
ALWAYS_INLINE static inline size_t cast_size(Number_T const& number)
{
  return static_cast<size_t>(number);
}
template<typename Number_T>
ALWAYS_INLINE static inline wtk::wire_idx cast_wire(Number_T const& number)
{
  return static_cast<wtk::wire_idx>(number);
}
template<typename Number_T>
ALWAYS_INLINE static inline wtk::type_idx cast_type(Number_T const& number)
{
  return static_cast<wtk::type_idx>(number);
}

} } // namespace wtk::utils
