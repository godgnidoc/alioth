#ifndef __ALIOTH_REGEX_H__
#define __ALIOTH_REGEX_H__

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "alioth/error.h"
#include "alioth/generic.h"

namespace alioth {

/**
 * Regular Expression
 *
 * 支持服务于词法分析器的正则表达式
 * 正则表达式内容格式：
 *  regexpr  := expr
 *  expr     := term { '|' term }
 *  term     := factor { factor }
 *  factor   := primary [ '*' | '+' | '?' ]
 *  primary  := '(' expr ')' | '[' range ']' | char
 */
struct RegexTree;
using Regex = std::shared_ptr<RegexTree>;

struct RegexTree {
  struct LeafNode;
  struct MonoNode;
  struct BinaryNode;
  struct AcceptNode;
  struct CharNode;
  struct RangeNode;
  struct ConcatNode;
  struct UnionNode;
  struct KleeneNode;
  struct PositiveNode;
  struct OptionalNode;
  struct Unit;

  using Leaf = std::shared_ptr<LeafNode>;
  using Leafs = std::set<Leaf, std::owner_less<Leaf>>;

  virtual ~RegexTree() = default;
  virtual bool GetNullable() = 0;
  virtual Leafs GetFirstpos() = 0;
  virtual Leafs GetLastpos() = 0;
  virtual void CalcFollowpos() = 0;

  /**
   * 编译正则表达式
   *
   * @param pattern 正则表达式
   */
  static Regex Compile(std::string const& pattern);

  struct ParseError : public Error {
    using Error::Error;
  };

  struct InvalidRange : public ParseError {
    InvalidRange() : ParseError("ParseRange: invalid range") {}
  };

  struct InvalidEscape : public ParseError {
    InvalidEscape() : ParseError("invalid escape sequence") {}
  };

  struct InvalidPattern : public ParseError {
    InvalidPattern() : ParseError("invalid pattern") {}
  };

 protected:
  static std::string const& Operators();
  static void ParseRange(std::vector<Unit>& input, size_t const start);
  static void ParseChars(std::vector<Unit>& base);
  static void Parse(std::vector<Unit>& base, size_t const start);
};

struct RegexTree::LeafNode : public RegexTree,
                             public std::enable_shared_from_this<LeafNode> {
  /**
   * 后继位置集合
   */
  Leafs followpos_{};

  void CalcFollowpos() final;
  virtual bool Match(char input) = 0;
};

struct RegexTree::MonoNode : public RegexTree {
  Regex child_{};
};

struct RegexTree::BinaryNode : public RegexTree {
  Regex left_{};
  Regex right_{};
};

/**
 * 正则表达式中的接受节点
 */
struct RegexTree::AcceptNode : public LeafNode {
  SymbolID term_{};

  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  bool Match(char input) override;

  /**
   * 为正则表达式标记接受节点
   * @param regex 正则表达式
   * @param term 接受的终结符 id
   */
  static std::shared_ptr<AcceptNode> On(Regex const& regex, SymbolID term);
};

/**
 * 单个字符节点
 */
struct RegexTree::CharNode : public LeafNode {
  char ch_{};

  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  bool Match(char input) override;
};

/**
 * 字符类节点
 *
 * 支持正向匹配和反向匹配
 * 例如：[a-z]和[^a-z]
 *
 * 支持简写
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
 */
struct RegexTree::RangeNode : public LeafNode {
  bool includes_{};
  std::set<char> set_{};

  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  bool Match(char input) override;
};

struct RegexTree::ConcatNode : public BinaryNode {
  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  void CalcFollowpos() override;
};

struct RegexTree::UnionNode : public BinaryNode {
  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  void CalcFollowpos() override;

  /**
   * 将两个正则表达式使用或运算连接
   *
   * @param lhs 左正则表达式
   * @param rhs 右正则表达式
   */
  static Regex Of(Regex const& lhs, Regex const& rhs);
};

struct RegexTree::KleeneNode : public MonoNode {
  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  void CalcFollowpos() override;
};

struct RegexTree::PositiveNode : public MonoNode {
  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  void CalcFollowpos() override;
};

struct RegexTree::OptionalNode : public MonoNode {
  bool GetNullable() override;
  Leafs GetFirstpos() override;
  Leafs GetLastpos() override;
  void CalcFollowpos() override;
};

/** 泛型输入单元 */
struct RegexTree::Unit : public std::variant<char, Regex> {
 public:
  using variant::variant;

  bool IsChar() const { return std::holds_alternative<char>(*this); }
  bool IsNode() const { return std::holds_alternative<Regex>(*this); }

  char Char() const { return std::get<char>(*this); }
  Regex Node() const { return std::get<Regex>(*this); }
};

/**
 * 正则表达式字面语法
 */
inline Regex operator"" _regex(const char* pattern, size_t) {
  return RegexTree::Compile(pattern);
}

}  // namespace alioth

#endif