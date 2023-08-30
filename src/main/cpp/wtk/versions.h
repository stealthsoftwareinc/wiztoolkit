#ifndef WTK_VERSION_H_
#define WTK_VERSION_H_

#include <cstddef>

namespace wtk
{

// WizToolKit Version
size_t constexpr WTK_VERSION_MAJOR = 2;
size_t constexpr WTK_VERSION_MINOR = 1;
size_t constexpr WTK_VERSION_PATCH = 0;
char const* const WTK_VERSION_EXTRA = "";

// SIEVE IR Version
size_t constexpr IR_VERSION_MAJOR = 2;
size_t constexpr IR_VERSION_MINOR = 1;
size_t constexpr IR_VERSION_PATCH = 0;
char const* const IR_VERSION_EXTRA = "";

} // namespace wtk

#endif//WTK_VERSION_H_
