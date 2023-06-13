#include "alioth/alioth.h"

namespace alioth {

std::filesystem::path AliothHome() {
  static auto const alioth_home = [] {
    auto env = getenv("ALIOTH_HOME");
    if (env) {
      return std::filesystem::path(env);
    }

    return std::filesystem::current_path();
  }();

  return alioth_home;
}

}  // namespace alioth