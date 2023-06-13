#ifndef __ALIOTH_ERROR_HANDLE_H__
#define __ALIOTH_ERROR_HANDLE_H__

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace alioth {

template <typename Exception>
[[noreturn]] inline void throw_nested(Exception const& e) {
  if (std::current_exception())
    std::throw_with_nested(e);
  else
    throw e;
}

template <>
[[noreturn]] inline void throw_nested(std::exception_ptr const& ptr) {
  if (std::current_exception()) {
    auto cur = std::current_exception();
    try {
      std::rethrow_exception(ptr);
    } catch (std::exception& e) {
      try {
        std::rethrow_exception(cur);
      } catch (...) {
        // TODO: 最好能不丢失原始的异常信息
        std::throw_with_nested(std::runtime_error(e.what()));
      }
    }
  } else {
    std::rethrow_exception(ptr);
  }
}

/**
 * 将嵌套的异常转换为字符串
 */
std::vector<std::string> dump_errors(const std::exception& e);

}  // namespace alioth

#endif