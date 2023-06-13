#include "alioth/document.h"

#include <fstream>
#include <iostream>

#include "fmt/format.h"

namespace alioth {

Point Document::PointAt(size_t offset) const {
  Point point{};
  for (auto i = 0UL; i < offset; ++i) {
    if (content[i] == '\n') {
      ++point.line;
      point.column = 1;
    } else {
      auto ch = content[i];
      ++point.column;
      if (0xC0 == (ch & 0xE0)) {
        i += 1;  // 2 bytes
      } else if (0xE0 == (ch & 0xF0)) {
        i += 2;  // 3 bytes
      } else if (0xF0 == (ch & 0xF8)) {
        i += 3;  // 4 bytes
      }
    }
  }
  return point;
}

Doc Document::Create(std::string const& content,
                     std::optional<std::filesystem::path> const& path) {
  return std::make_shared<Document>(Document{content, path});
}

Doc Document::Read(std::filesystem::path const& path,
                   std::optional<std::string> const& stdin) {
  if (stdin && path == *stdin) return Read();

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

nlohmann::json Point::Store() const {
  nlohmann::json json;
  json["line"] = line;
  json["column"] = column;
  return json;
}

nlohmann::json Range::Store() const {
  nlohmann::json json;
  json["start"] = start.Store();
  json["end"] = end.Store();
  return json;
}

}  // namespace alioth