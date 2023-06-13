#ifndef __ALIOTH_STRINGS_H__
#define __ALIOTH_STRINGS_H__

#include <set>
#include <string>

#include "alioth/error.h"

namespace alioth {

struct Chars {
  static std::string Range(char from, char to);

  static std::string const& Any();
  static std::string const& Digit();
  static std::string const& Lower();
  static std::string const& Upper();
  static std::string const& Punct();
  static std::string const& Space();
  static std::string const& Word();

  static std::string const& Escape(char c);
  struct EscapeError : public Error {
    EscapeError(char escape)
        : Error("charactor {} has no escape sequence", escape) {}
  };

  /**
   *
   * \d 数字 == [0-9]
   * \D 非数字 == [^0-9]
   * \l 小写字母 == [a-z]
   * \L 非小写字母 == [^a-z]
   * \p 标点符号 == [!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~]
   * \s 空白字符 == [\t\n\v\f\r ]
   * \u 大写字母 == [A-Z]
   * \U 非大写字母 == [^A-Z]
   * \w 单词字符 == [a-zA-Z0-9_]
   * \W 非单词字符 == [^a-zA-Z0-9_]
   *
   * @returns tuple<range, includes>
   */
  static std::tuple<std::string, bool> const& Extract(char escape);
  struct ExtractError : public Error {
    ExtractError(char escape)
        : Error("sequence \\{} is not a valid escape sequence", escape) {}
  };
};

struct Strings {
  static std::string Trim(std::string const& str, bool trim_start,
                          bool trim_end);

  static std::string Uppercase(std::string const& str);
  static std::string Lowercase(std::string const& str);
  static std::string Camelcase(std::string const& str);
  static std::string Titlecase(std::string const& str);
};

std::set<char>& operator+=(std::set<char>& lhs, std::string const& rhs);
std::set<char> operator+(std::set<char> lhs, std::string const& rhs);

}  // namespace alioth

#endif