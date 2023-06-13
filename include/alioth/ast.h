#ifndef __ALIOTH_AST_H__
#define __ALIOTH_AST_H__

#include <map>
#include <memory>
#include <type_traits>
#include <vector>

#include "alioth/ast.h"
#include "alioth/document.h"
#include "alioth/generic.h"
#include "alioth/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {

struct ASTNode;
using AST = std::shared_ptr<ASTNode>;

struct ASTTermNode;
using ASTTerm = std::shared_ptr<ASTTermNode>;

struct ASTNtrmNode;
using ASTNtrm = std::shared_ptr<ASTNtrmNode>;

struct ASTRootNode;
using ASTRoot = std::shared_ptr<ASTRootNode>;

struct Skeleton;

/**
 * 语法树节点
 */
struct ASTNode : public std::enable_shared_from_this<ASTNode> {
  struct StoreOptions;

  std::weak_ptr<ASTRootNode> root{};  // 语法树根节点
  SymbolID id{};                      // 终结符或非终结符的唯一标识

  virtual ~ASTNode() = default;

  /**
   * 尝试将语法树节点转换为终结符，失败则返回nullptr
   */
  ASTNtrm AsNtrm();

  /**
   * 尝试将语法树节点转换为非终结符，失败则返回nullptr
   */
  ASTTerm AsTerm();

  /**
   * 获取符号的第一个终结符
   */
  ASTTerm First();

  /**
   * 获取符号的最后一个终结符
   */
  ASTTerm Last();

  /**
   * 获取语法树节点的原始产生式
   *
   * 若节点不是非终结符则返回 -1UL
   * 若语法结构的产生式为完全展开产生式
   * 则递归查找产生式体中符号的第一次规约时使用的产生式
   */
  FormulaID OriginFormula();

  /**
   * 获取语法树节点的某个属性的第一个值，若节点不是非终结符或属性不存在则返回空
   *
   * @param key 属性名称
   */
  AST Attr(std::string const& key);

  /**
   * 获取语法树节点的某个属性所有的值，若节点不是非终结符或属性不存在则返回空
   *
   * @param key 属性名称
   */
  std::vector<AST> Attrs(std::string const& key);

  /**
   * 计算符号在文本中的范围
   */
  alioth::Range Range();

  /**
   * 获取符号的名称
   */
  std::string Name();

  /**
   * 获取符号的文本
   */
  std::string Text();

  /**
   * 获取语法树节点的指定属性的文本内容
   *
   * 若节点不是非终结符或属性不存在则返回空
   *
   * @param key 属性名称
   */
  std::optional<std::string> TextOf(std::string const& key);

  /**
   * 获取符号的位置
   */
  std::string Location();

  /**
   * 获取符号的属性树
   *
   * @param node 语法树节点
   * @param options 存储选项
   */
  nlohmann::json Store(StoreOptions const& options);
};

/**
 * 语法树节点存储选项
 */
struct ASTNode::StoreOptions {
  /**
   * 是否紧凑存储
   *
   * 紧凑存储时只保存属性树
   * 非紧凑存储时保存语法树
   */
  bool compact{true};

  /**
   * 是否输出仅包含终结符的扁平列表
   */
  bool flatten{false};

  /**
   * 是否由骨架指导属性结构
   *
   * 若指定语法骨架，则参考骨架规范化输出结果
   * 只影响紧凑存储
   */
  Skeleton const* skeleton{nullptr};

  /**
   * 是否展开终结符属性表
   *
   * 若不展开终结符属性表，则终结符被打印为对应文本
   * 若展开终结符属性表，则终结符被打印为对应的属性表
   * 若展开终结符属性表，但终结符属性表为空，则终结符被打印为对应文本
   */
  bool unfold{false};

  /**
   * 指定终结符文本在符号属性表中的名称
   *
   * 省略则不输出终结符文本
   */
  std::optional<std::string> text{};

  /**
   * 指定终结符和非终结符id在符号属性表中的名称
   *
   * 省略则不输出终结符和非终结符id
   */
  std::optional<std::string> id{};

  /**
   * 指定终结符和非终结符名称在符号属性表中的名称
   *
   * 省略则不输出终结符和非终结符名称
   */
  std::optional<std::string> name{};

  /**
   * 指定非终结符句型范围在符号属性表中的名称
   *
   * 省略则不输出非终结符句型范围
   */
  std::optional<std::string> range{};

  /**
   * 指定非终结符句型在符号属性表中的名称
   *
   * 省略则不输出非终结符句型
   */
  std::optional<std::string> form{};

  /**
   * 指定非终结符产生式在符号属性表中的名称
   *
   * 省略则不输出非终结符产生式
   */
  std::optional<std::string> formula{};

  /**
   * 指定非终结符原始产生式在符号属性表中的名称
   *
   * 省略则不输出非终结符原始产生式
   */
  std::optional<std::string> origin{};
};

/**
 * 终结符
 */
struct ASTTermNode : public ASTNode {
  size_t offset{};  // 词法单元的偏移量
  size_t length{};  // 词法单元的长度

  /**
   * 由词法规则或产生式设置的属性
   */
  std::map<std::string, nlohmann::json> attributes{};
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
struct ASTRootNode : public ASTNtrmNode {
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

/**
 * 语法树属性封装
 */
struct ASTAttr : public AST {
  using AST::AST;
  ASTAttr(AST const& node) : AST(node) {}
  ASTAttr(AST&& node) : AST(std::move(node)) {}

  ASTAttr() = default;
  ASTAttr(ASTAttr const&) = default;
  ASTAttr(ASTAttr&&) = default;
  ~ASTAttr() = default;
  ASTAttr& operator=(ASTAttr const& node) = default;
  ASTAttr& operator=(ASTAttr&& node) = default;

  template <typename T, std::enable_if_t<!is_shared_ptr_v<T>>* = nullptr>
  std::shared_ptr<T> As() const {
    return std::dynamic_pointer_cast<T>(*this);
  }

  template <typename T, std::enable_if_t<is_shared_ptr_v<T>>* = nullptr>
  T As() const {
    return std::dynamic_pointer_cast<typename T::element_type>(*this);
  }
};

/**
 * 语法树属性列表
 */
using ASTAttrs = std::vector<ASTAttr>;

}  // namespace alioth

#endif