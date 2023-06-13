#ifndef __ALIOTH_AST_H__
#define __ALIOTH_AST_H__

#include <map>
#include <memory>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/syntax/syntax-fwd.h"
#include "alioth/source.h"
#include "alioth/token.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace ast {

struct Root;

struct Node {
  std::weak_ptr<Node> parent;  // 父节点
  int id;                      // 终结符或非终结符的ID

  std::string GetName() const;
  virtual std::string GetText() const = 0;

  virtual std::shared_ptr<Root> GetRoot();
  virtual std::shared_ptr<Root const> GetRoot() const;
  virtual nlohmann::json DumpRaw() const = 0;
  virtual nlohmann::json DumpAbs() const = 0;
  virtual std::string GetLocation() const = 0;

  virtual ~Node() = default;
};

using NodeRef = std::shared_ptr<Node>;

/**
 * 终结符
 */
struct Term : public Node {
  size_t start_line;    // 词法单元的起始行
  size_t start_column;  // 词法单元的起始列
  size_t end_line;      // 词法单元的结束行
  size_t end_column;    // 词法单元的结束列
  size_t offset;        // 词法单元的偏移量
  size_t length;        // 词法单元的长度

  std::string GetText() const override;
  nlohmann::json DumpRaw() const override;
  nlohmann::json DumpAbs() const override;
  std::string GetLocation() const override;
};

/**
 * 非终结符
 */
struct Ntrm : public Node {
  /**
   * 句型，即产生式id
   */
  int formula;

  /**
   * 当前非终结符在语法分析树中的全部直接子节点
   */
  std::vector<NodeRef> sentence;

  /**
   * 文法要求提取的属性，其内容可能不只有直接子节点还有子孙节点
   */
  std::map<std::string, std::vector<NodeRef>> attributes;

  /**
   * 获取当前非终结符的文本
   */
  std::string GetText() const override;

  nlohmann::json DumpRaw() const override;
  nlohmann::json DumpAbs() const override;
  std::string GetLocation() const override;
};

/**
 * 根节点
 */
struct Root : public Ntrm, public std::enable_shared_from_this<Root> {
  SourceRef source;  // 源码
  SyntaxCRef syntax;   // 语法规则

  std::shared_ptr<Root> GetRoot() override;
  std::shared_ptr<Root const> GetRoot() const override;

  /**
   * 创建一个终结符节点，使用根节点作为临时父节点
   *
   * @param token 词法单元
   */
  std::shared_ptr<Term> MakeTerm(Token const& token);
};

}  // namespace ast

/**
 * 语法分析树
 */
using AST = std::shared_ptr<ast::Node>;

}  // namespace alioth

#endif