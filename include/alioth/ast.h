#ifndef __ALIOTH_AST_H__
#define __ALIOTH_AST_H__

#include <map>
#include <memory>
#include <vector>

#include "alioth/document.h"
#include "alioth/syntax.h"

namespace alioth {

struct ASTNode;
using AST = std::shared_ptr<ASTNode>;

struct ASTTermNode;
using ASTTerm = std::shared_ptr<ASTTermNode>;

struct ASTNtrmNode;
using ASTNtrm = std::shared_ptr<ASTNtrmNode>;

struct ASTRootNode;
using ASTRoot = std::shared_ptr<ASTRootNode>;

/**
 * 语法树节点
 */
struct ASTNode {
  std::weak_ptr<ASTRootNode> root{};  // 语法树根节点
  SymbolID id{};                      // 终结符或非终结符的唯一标识

  virtual ~ASTNode() = default;
};

/**
 * 终结符
 */
struct ASTTermNode : public ASTNode {
  size_t offset{};  // 词法单元的偏移量
  size_t length{};  // 词法单元的长度
};

/**
 * 非终结符
 */
struct ASTNtrmNode : public ASTNode {
  /**
   * 句型，即产生式id
   */
  FormulaID formula;

  /**
   * 当前非终结符在语法分析树中的全部直接子节点
   */
  std::vector<AST> sentence;

  /**
   * 文法要求提取的属性，其内容可能不只有直接子节点还有子孙节点
   */
  std::map<std::string, std::vector<AST>> attributes;
};

/**
 * 根节点
 */
struct ASTRootNode : public ASTNtrmNode,
                     public std::enable_shared_from_this<ASTRootNode> {
  Syntax syntax;  // 语法规则
  Doc doc;        // 源码

  /**
   * 创建一个终结符节点
   *
   * @param id 终结符ID
   * @param offset 词法单元的偏移量
   * @param length 词法单元的长度
   */
  ASTTerm Term(SymbolID id, size_t offset, size_t length = 0);

  /**
   * 创建一个非终结符节点
   *
   * @param formula 句型产生式
   * @param begin 子节点起始迭代器
   * @param end 子节点结束迭代器
   */
  ASTNtrm Ntrm(FormulaID formula, std::vector<AST>::const_iterator begin,
               std::vector<AST>::const_iterator end);
};

}  // namespace alioth

#endif