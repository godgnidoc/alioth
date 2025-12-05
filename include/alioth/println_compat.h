#ifndef __ALIOTH_PRINTLN_COMPAT_H__
#define __ALIOTH_PRINTLN_COMPAT_H__

#include <fmt/core.h>
#include <cstdio>

namespace fmt {
#if FMT_VERSION < 100000
template <typename... Args>
inline void println(FILE* f, format_string<Args...> format, Args&&... args) {
  print(f, format, std::forward<Args>(args)...);
  print(f, "\n");
}

template <typename... Args>
inline void println(format_string<Args...> format, Args&&... args) {
  print(format, std::forward<Args>(args)...);
  print("\n");
}
#endif
}

#endif
