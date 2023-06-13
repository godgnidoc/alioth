#include "alioth/error_handle.h"

namespace alioth {

void dump_errors(const std::exception& e, std::vector<std::string>& result) {
  result.push_back(e.what());
  try {
    std::rethrow_if_nested(e);
  } catch (const std::exception& nestedException) {
    dump_errors(nestedException, result);
  }
}

std::vector<std::string> dump_errors(const std::exception& e) {
  std::vector<std::string> result;
  dump_errors(e, result);
  return result;
}
}  // namespace alioth