/**
 * Copyright (C) 2020-2021 Stealth Software Technologies, Inc.
 */

#ifndef WTK_UTILS_HINTS_H_
#define WTK_UTILS_HINTS_H_

#if (defined(__GNUC__) || defined (__clang__))  && (!defined(__APPLE__))

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define ALWAYS_INLINE [[ gnu::always_inline ]]
#define RESTRICT __restrict__

#else

#define UNLIKELY(x) (x)
#define LIKELY(x) (x)
#define ALWAYS_INLINE
#define RESTRICT

#endif

#endif // WTK_UTILS_HINTS_
