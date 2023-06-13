#ifndef __ALIOTH_SYNTAX_BUILDER_H__
#define __ALIOTH_SYNTAX_BUILDER_H__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/syntax/syntax.h"
#include "alioth/syntax/builder-fwd.h"
#include "alioth/logging.h"
#include "alioth/utils/dict.h"

namespace alioth::syntax {

struct UnfoldHintType {};

constexpr UnfoldHintType kUnfold{};

/**
 * 产生式
 */
struct Formula {
  /** 头部: 非终结符id */
  int head;

  /** 体: 符号id序列 */
  std::vector<int> body;

  /** 属性表 <符号在产生式中下标，属性名下标> */
  std::vector<std::pair<int, int>> attrs{};

  /** 要展开的符号下标 */
  std::vector<int> unfolds{};
};

/**
 * 非终结符描述
 */
struct NtrmDef {
  /**
   * 非终结符名称
   */
  std::string name;

  /** 产生式编号集合 */
  std::set<int> formulas{};

  /** 是否可空 */
  bool nullable{false};

  /** First集 */
  std::set<int> first{};

  /** Follow集 */
  std::set<int> follow{};
};

/** 项目 formula_id -> point(start from 0) */
using Item = std::pair<int, int>;

class Builder;

/**
 * 产生式构造器
 */
class FormulaBuilder {
 public:
  FormulaBuilder(Builder& builder, std::string const& head);

  /**
   * 添加一个符号
   *
   * @param name 符号名称
   * @param attr 接收此符号的属性名
   */
  FormulaBuilder& Symbol(std::string const& name,
                         std::optional<std::string> const& attr = std::nullopt);

  /**
   * 添加一个被展开的符号
   *
   * @param name 符号名称
   */
  FormulaBuilder& Symbol(std::string const& name, UnfoldHintType const&);

  /**
   * 提交产生式
   */
  Builder& Commit();

 private:
  Builder& builder_;
  Formula formula_;
};

/**
 * Syntax构造器
 */
class Builder {
 public:
  Builder(LexCRef lex);

  Builder(Builder const&) = delete;
  Builder(Builder&&) = delete;

  Builder& operator=(Builder const&) = delete;
  Builder& operator=(Builder&&) = delete;

  /**
   * 开始定义非终结符
   *
   * 在开始添加非终结符前必须先添加所有终结符
   * 因为推导式中使用但未定义的符号被视为非终结符
   *
   * @param name 非终结符名称
   */
  FormulaBuilder StartFormula(std::string const& head);

  /**
   * 添加一个产生式
   *
   * @param formula 产生式
   */
  Builder& AddFormula(Formula const& formula);

  /**
   * 在语法分析阶段忽略一种终结符
   * 多次调用可以忽略多个终结符
   *
   * 被忽略的终结符不参与移进和归约判定，也不占用归约长度
   * @param name 终结符名称
   */
  Builder& Ignore(std::string const& name);

  /**
   * 构建Syntax
   */
  std::shared_ptr<Syntax> Build();

  /**
   * 获取符号ID
   *
   * 必要时创建新符号
   * @param name 符号名称
   */
  int GetSymbolId(std::string const& name);

  /**
   * 获取属性ID
   *
   * 必要时创建新属性
   * @param name 属性名
   */
  int GetAttrId(std::string const& name);

  /**
   * 获取符号名称
   *
   * @param id 符号ID
   */
  std::string GetSymbolName(int id);

 private:
  /**
   * 计算并标记全部可空的非终结符
   */
  void CalculateNullable();

  /**
   * 为每个非终结符计算First集
   */
  void CalculateFirst();

  /**
   * 为每个非终结符计算Follow集
   */
  void CalculateFollow();

  /**
   * 计算文法状态
   */
  void CalculateStates();

  /**
   * 计算项集的闭包
   *
   * @param items 项目集
   */
  void Closure(std::set<Item>& items);

  /**
   * 计算项集的输入符号集
   *
   * @param items 项目集
   */
  std::set<int> Alphabet(std::set<Item> const& items);

  /**
   * 计算项集在输入指定符号后的转移
   *
   * @param items 项目集
   * @param symbol 符号
   */
  std::set<Item> Goto(std::set<Item> const& items, int symbol_id);

  /**
   * 判断是否为终结符
   */
  bool IsTerm(int id);

  /**
   * 将项打印为字符串
   */
  std::string DumpItem(Item const& item);

  /**
   * 将产生式打印为字符串
   */
  std::string DumpFormula(int formula_id);

 private:
  /**
   * 词法规则
   */
  alioth::LexCRef const lex_;

  /**
   * 非终结符描述表
   */
  std::map<int, NtrmDef> ntrms_;

  /**
   * 产生式表，产生式下标即为产生式编号
   */
  std::vector<Formula> formulas_;

  /**
   * 状态表
   */
  std::vector<State> states_;

  /**
   * 项目集族，项集与状态下标的映射
   */
  alioth::Dict<Item, size_t> state_dict_;

  /**
   * 属性名表
   */
  std::vector<std::string> attrs_;

  /**
   * 忽略的终结符
   */
  std::set<int> ignores_;

  /**
   * 报错计数
   */
  size_t errors_{0};

  /**
   * 日志器
   */
  Logger logger_{"syntax"};
};

}  // namespace alioth::syntax

#endif