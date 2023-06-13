#ifndef __ALIOTH_LEX_REGEX_H__
#define __ALIOTH_LEX_REGEX_H__

#include <memory>
#include <set>
#include <stdexcept>
#include <string>

#include "alioth/utils/clan.h"

namespace alioth {

namespace regex {

struct LeafNode;
using LeafNodes = std::set<std::shared_ptr<LeafNode>,
                           std::owner_less<std::shared_ptr<LeafNode>>>;

/** 正则表达式节点 */
struct Node {
  enum class Type {
    /** 接受 */
    kAccept,

    /** 字符 */
    kChar,

    /** 字符类 */
    kRange,

    /** 拼接 */
    kConcat,

    /** 交集 */
    kUnion,

    /** Kleene闭包 */
    kKleene,

    /** 正闭包 */
    kPositive,

    /** 可选 */
    kOptional,
  };

  Type const type_;

  Node(Type type);
  virtual ~Node() = default;

  virtual bool GetNullable() = 0;
  virtual LeafNodes GetFirstpos() = 0;
  virtual LeafNodes GetLastpos() = 0;
  virtual void CalcFollowpos() = 0;
};

struct LeafNode : public Node, public std::enable_shared_from_this<LeafNode> {
  /**
   * 后继位置集合
   */
  LeafNodes followpos_;

  LeafNode(Type type);
  void CalcFollowpos() final;
  virtual bool Match(char input) = 0;
};

struct AcceptNode : public LeafNode {
  int token_id_;

  AcceptNode(int token_id);
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  bool Match(char input) override;
};

/**
 * 单个字符节点
 */
struct CharNode : public LeafNode {
  char ch_;

  CharNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
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
struct RangeNode : public LeafNode {
  enum Direction { kNegative, kPositive };

  Direction dir_;
  Clan set_;

  RangeNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  bool Match(char input) override;
};

struct ConcatNode : public Node {
  std::shared_ptr<Node> left_;
  std::shared_ptr<Node> right_;

  ConcatNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  void CalcFollowpos() override;
};

struct UnionNode : public Node {
  std::shared_ptr<Node> left_;
  std::shared_ptr<Node> right_;

  UnionNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  void CalcFollowpos() override;
};

struct KleeneNode : public Node {
  std::shared_ptr<Node> child_;

  KleeneNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  void CalcFollowpos() override;
};

struct PositiveNode : public Node {
  std::shared_ptr<Node> child_;

  PositiveNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  void CalcFollowpos() override;
};

struct OptionalNode : public Node {
  std::shared_ptr<Node> child_;

  OptionalNode();
  bool GetNullable() override;
  LeafNodes GetFirstpos() override;
  LeafNodes GetLastpos() override;
  void CalcFollowpos() override;
};
}  // namespace regex

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
using Regex = std::shared_ptr<regex::Node>;

namespace regex {

/**
 * 编译正则表达式
 *
 * @param expression 正则表达式
 */
Regex Compile(std::string const& expression);

/**
 * 将两个正则表达式使用或运算连接
 *
 * @param lhs 左正则表达式
 * @param rhs 右正则表达式
 */
Regex Union(Regex const& lhs, Regex const& rhs);

/**
 * 为正则表达式标记接受节点
 * @param regex 正则表达式
 * @param token_id 接受的token id
 */
std::shared_ptr<regex::AcceptNode> Accept(Regex const& regex, int token_id);

}  // namespace regex

}  // namespace alioth

#endif