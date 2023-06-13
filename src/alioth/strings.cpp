#include "alioth/strings.h"

#include <cctype>
#include <map>
#include <stdexcept>
#include <type_traits>

#include "alioth/error.h"
#include "alioth/strings.h"

namespace alioth {

namespace {
template <typename F>
std::string Includes(size_t reserve, F&& pick) {
  static_assert(std::is_invocable_r_v<bool, F, char>, "F must be char -> bool");

  std::string str;
  str.reserve(reserve);
  for (auto c = 1; c < 256; ++c) {
    auto ch = static_cast<char>(c);
    if (pick(ch)) str.push_back(ch);
  }

  return str;
}
}  // namespace

std::string Chars::Range(char from, char to) {
  if (from > to) std::swap(from, to);
  std::string str;
  str.reserve(to - from + 1);
  for (char c = from; c <= to; ++c) {
    str.push_back(c);
  }
  return str;
}

std::string const& Chars::Any() {
  static std::string const str = Includes(255, [](char) { return true; });
  return str;
}

std::string const& Chars::Digit() {
  static std::string const str = Includes(10, isdigit);
  return str;
}
std::string const& Chars::Lower() {
  static std::string const str = Includes(26, islower);
  return str;
}
std::string const& Chars::Upper() {
  static std::string const str = Includes(26, isupper);
  return str;
}
std::string const& Chars::Punct() {
  static std::string const str = Includes(32, ispunct);
  return str;
}
std::string const& Chars::Space() {
  static std::string const str = Includes(6, isspace);
  return str;
}
std::string const& Chars::Word() {
  static std::string const str =
      Includes(63, [](char c) { return isalnum(c) || c == '_'; });
  return str;
}

std::string const& Chars::Escape(char c) try {
  static std::map<char, std::string> const table = {
      {'\a', "\\a"}, {'\b', "\\b"},  {'\f', "\\f"},  {'\n', "\\n"},
      {'\r', "\\r"}, {'\t', "\\t"},  {'\v', "\\v"},  {'\\', "\\\\"},
      {'.', "\\."},  {'\'', "\\\'"}, {'\"', "\\\""}, {'?', "\\?"},
      {'*', "\\*"},  {'+', "\\+"},   {'|', "\\|"},   {'(', "\\("},
      {')', "\\)"},  {'[', "\\["},   {']', "\\]"},   {'/', "\\/"}};
  return table.at(c);
} catch (std::out_of_range const&) {
  throw Chars::EscapeError(c);
}

std::tuple<std::string, bool> const& Chars::Extract(char escape) try {
  static std::map<char, std::tuple<std::string, bool>> const table = {
      {'a', {"\a", true}},     {'b', {"\b", true}},     {'d', {Digit(), true}},
      {'D', {Digit(), false}}, {'f', {"\f", true}},     {'l', {Lower(), true}},
      {'L', {Lower(), false}}, {'n', {"\n", true}},     {'p', {Punct(), true}},
      {'P', {Punct(), false}}, {'r', {"\r", true}},     {'s', {Space(), true}},
      {'S', {Space(), false}}, {'t', {"\t", true}},     {'v', {"\v", true}},
      {'u', {Upper(), true}},  {'U', {Upper(), false}}, {'w', {Word(), true}},
      {'W', {Word(), false}},  {'\\', {"\\", true}},    {'.', {".", true}},
      {'\'', {"\'", true}},    {'\"', {"\"", true}},    {'?', {"?", true}},
      {'*', {"*", true}},      {'+', {"+", true}},      {'|', {"|", true}},
      {'(', {"(", true}},      {')', {")", true}},      {'[', {"[", true}},
      {']', {"]", true}},      {'/', {"/", true}}};
  return table.at(escape);
} catch (std::out_of_range const&) {
  throw Chars::ExtractError(escape);
}

std::string Strings::Trim(std::string const& str, bool trim_start,
                          bool trim_end) {
  auto start = str.begin();
  auto end = str.end();

  if (trim_start) {
    while (start != end && std::isspace(*start)) ++start;
  }
  if (trim_end) {
    while (end != start && std::isspace(*(end - 1))) --end;
  }

  return std::string(start, end);
}

std::string Strings::Uppercase(std::string const& str) {
  std::string result;
  result.reserve(str.size());
  for (auto const c : str) {
    result.push_back(std::toupper(c));
  }
  return result;
}
std::string Strings::Lowercase(std::string const& str) {
  std::string result;
  result.reserve(str.size());
  for (auto const c : str) {
    result.push_back(std::tolower(c));
  }
  return result;
}

std::string Strings::Camelcase(std::string const& str) {
  std::string result;
  result.reserve(str.size());
  bool upper = true;
  for (auto const c : str) {
    if (std::isspace(c) || c == '_' || c == '-') {
      upper = true;
      continue;
    }
    if (upper) {
      result.push_back(std::toupper(c));
      upper = false;
    } else {
      result.push_back(std::tolower(c));
    }
  }
  return result;
}

std::string Strings::Titlecase(std::string const& str) {
  std::string result;
  auto first = true;
  result.reserve(str.size());
  for (auto const c : str) {
    if (first) {
      result.push_back(std::toupper(c));
      first = false;
    } else {
      result.push_back(std::tolower(c));
    }
  }
  return result;
}

std::set<char>& operator+=(std::set<char>& lhs, std::string const& rhs) {
  for (auto const c : rhs) {
    lhs.insert(c);
  }
  return lhs;
}

std::set<char> operator+(std::set<char> lhs, std::string const& rhs) {
  lhs += rhs;
  return lhs;
}

}  // namespace alioth