#ifndef __ALIOTH_ERROR_H__
#define __ALIOTH_ERROR_H__

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "fmt/format.h"
#include "fmt/ranges.h"

namespace alioth {

struct Error : public std::runtime_error {
  template <typename... Args>
  Error(std::string const& fmt, Args&&... args)
      : std::runtime_error(
            fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...)) {}
};

}  // namespace alioth

#endif