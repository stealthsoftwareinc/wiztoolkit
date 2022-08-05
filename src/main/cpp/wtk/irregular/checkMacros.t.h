/**
 * Copyright (C) 2021 Stealth Software Technologies, Inc.
 */

#include <wtk/utils/hints.h>
#include <wtk/utils/NumUtils.h>

#include <wtk/irregular/AutomataCtx.h>

#ifndef WTK_IRREGULAR_CHECK_MACROS_H_
#define WTK_IRREGULAR_CHECK_MACROS_H_
namespace wtk {
namespace irregular {

/**
 * convert a string known to be in correct numeric form to an integer.
 * returns true on success, but only fails on an invalid base type.
 */
template<typename Number_T, typename Base_T>
ALWAYS_INLINE static inline bool to_uint(
    Base_T base, Number_T& num, AutomataCtx* ctx)
{
  switch(base)
  {
  case Base_T::dec:
    wtk::utils::dec_to_uint(
        &ctx->buffer[ctx->mark], &ctx->buffer[ctx->place], num);
    break;
  case Base_T::hex:
    wtk::utils::hex_to_uint(
        &ctx->buffer[ctx->mark + 2], &ctx->buffer[ctx->place], num);
    break;
  case Base_T::bin:
    wtk::utils::bin_to_uint(
        &ctx->buffer[ctx->mark + 2], &ctx->buffer[ctx->place], num);
    break;
  case Base_T::oct:
    wtk::utils::oct_to_uint(
        &ctx->buffer[ctx->mark + 2], &ctx->buffer[ctx->place], num);
    break;
  default:
    return false;
  }
  return true;
}

} } // namespace wtk::irregular

#endif // WTK_IRREGULAR_CHECK_MACROS_H_

#ifndef UNINCLUDE_CHECK_MACROS

/*
 * These macros are for sequencing single tokens quickly.
 *
 * BEGIN opens a sequence of tokens, then CHECK(token) can run an DFA
 * function and check it succeeded. WSPACE, WSPACE_AROUND(token) and
 * WSPACE_AFTER(token) are helpers to check for whitespace between
 * tokens. END and END_SEQ 
 */
#define BEGIN { bool check_var = true;

#define CHECK(token) check_var = (token(&this->ctx)) & check_var;
#define CHECK_EXPR(expr) check_var = (expr) & check_var;

#define WSPACE CHECK(whitespace)
#define WSPACE_AROUND(token) WSPACE CHECK(token) WSPACE
#define WSPACE_AFTER(token) CHECK(token) WSPACE

#define END if(UNLIKELY(!check_var)) { return false; } } this->ctx.updateMark();
#define END_SEQ if(UNLIKELY(!check_var)) { return false; } }

#define END_NULL \
  if(UNLIKELY(!check_var)) { return nullptr; } } this->ctx.updateMark();
#define END_NULL_SEQ if(UNLIKELY(!check_var)) { return nullptr; } }

#define NUMBER(num) do { \
  NumericBase base = numericLiteral(&this->ctx); \
  if(UNLIKELY(!to_uint(base, (num), &this->ctx))) { return false; } \
} while (false)

#define NUMBER_NULL(num) do { \
  NumericBase base = numericLiteral(&this->ctx); \
  if(UNLIKELY(!to_uint(base, (num), &this->ctx))) { return nullptr; } \
} while (false)

#else

#undef BEGIN

#undef CHECK
#undef CHECK_EXPR

#undef WSPACE
#undef WSPACE_AROUND
#undef WSPACE_AFTER

#undef END
#undef END_SEQ

#undef END_NULL
#undef END_NULL_SEQ

#undef NUMBER
#undef NUMBER_NULL

#undef UNINCLUDE_CHECK_MACROS

#endif //UNINCLUDE_CHECK_MACROS
