#include "alioth/source.h"

#include <fstream>

namespace alioth {
std::shared_ptr<Source> Source::Create(std::string const& content,
                                       std::string const& path) {
  return std::make_shared<Source>(Source{.content = content, .path = path});
}

std::shared_ptr<Source> Source::Load(std::string const& path) {
  std::ifstream file{path};
  if (!file.is_open()) {
    return nullptr;
  }

  std::string content{std::istreambuf_iterator<char>{file},
                      std::istreambuf_iterator<char>{}};

  return Create(content, path);
}
}  // namespace alioth