#ifndef __ALIOTH_PRINTLN_COMPAT_H__
#define __ALIOTH_PRINTLN_COMPAT_H__

#include <fmt/core.h>
#include <cstdio>

namespace fmt {
// fmt::println was added in fmt 10.0.0 (FMT_VERSION >= 100000)
// This provides a compatibility implementation for older versions
#if FMT_VERSION < 100000
template <typename S, typename... Args>
inline void println(FILE* f, const S& format, Args&&... args) {
  print(f, fmt::runtime(format), std::forward<Args>(args)...);
  std::fputc('\n', f);
}

template <typename S, typename... Args>
inline void println(const S& format, Args&&... args) {
  print(fmt::runtime(format), std::forward<Args>(args)...);
  std::putchar('\n');
}
#endif
}

#endif
