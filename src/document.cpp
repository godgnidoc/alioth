#include "alioth/document.h"

#include <fstream>
#include <iostream>

#include "fmt/format.h"

namespace alioth {

Doc Document::Create(std::string const& content,
                     std::optional<std::filesystem::path> const& path) {
  return std::make_shared<Document>(Document{content, path});
}

Doc Document::Read(std::filesystem::path const& path) {
  auto size = std::filesystem::file_size(path);
  std::string text(size, '\0');
  std::ifstream file(path);
  file.read(text.data(), size);
  return Create(std::move(text), path.string());
}

Doc Document::Read() {
  std::string text;
  std::getline(std::cin, text, '\0');
  return Create(std::move(text), "<stdin>");
}

}  // namespace alioth