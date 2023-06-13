#ifndef __ALIOX_SKELETON_H__
#define __ALIOX_SKELETON_H__

#include <filesystem>
#include <map>
#include <memory>
#include <set>

#include "alioth/generic.h"
#include "alioth/syntax.h"

namespace alioth {

/**
 * 语言骨架结构
 */
struct Skeleton {
  struct Attribute;
  struct Structure;
  struct Form;
  using Attributes = std::map<std::string, Attribute>;

  /**
   * 语言的语法规则
   */
  Syntax syntax{};

  /**
   * 每种语法结构的骨架
   */
  std::map<SymbolID, Structure> structures{};

  /**
   * 将语言骨架保存为JSON格式
   */
  nlohmann::json Store() const;

  /**
   * 推导语言骨架
   *
   * 分析语法结构的句型，为每种句型分析的属性结构
   *
   * @param syntax 语法规则
   * @return 语言骨架
   */
  static Skeleton Deduce(Syntax const& syntax);

  /**
   * 推导全量属性结构
   *
   * @param lang 语言骨架
   */
  static void DeduceStructures(Skeleton& lang);

  /**
   * 依据句型分组推导句型属性结构
   *
   * @param lang 语言骨架
   */
  static void DeduceForms(Skeleton& lang);

  /**
   * 依据匿名且完全展开的产生式传递句型定义
   * ntrm -> ...another; 为匿名完全展开产生式
   * 先找到各语法结构的展开产生式
   *
   * @param lang 语言骨架
   */
  static void PropagateForms(Skeleton& lang);

  /**
   * 归纳公共属性结构
   *
   * @param lang 语言骨架
   */
  static void DeduceCommon(Skeleton& lang);

  /**
   * 依据匿名完全展开的产生式将等价的属性候选替换为最高抽象
   *
   * 例如 higher -> ...lower;
   *
   * 则 ntrm -> lower@attr; 转换为 ntrm -> higher@attr;
   *
   * 此替换仅体现在属性树，不应当影响语法分析过程
   *
   * @param lang 语言骨架
   */
  static void ReplaceEquivalentCandidates(Skeleton& lang);

  /**
   * 去除中间属性结构
   *
   * 从语法树根沿着属性结构向下遍历，永远不能抵达的语法结构为中间语法结构
   *
   * @param lang 语言骨架
   */
  static void StripIntermediate(Skeleton& lang);
};

/**
 * 属性骨架结构
 */
struct Skeleton::Attribute {
  /**
   * 候选语法结构
   */
  std::set<SymbolID> candidates{};

  /**
   * 属性是否最多只有一个值
   */
  bool is_single{true};

  /**
   * 属性是否是可选的
   */
  bool is_optional{false};
};

/**
 * 句型分组抽象
 */
struct Skeleton::Form {
  /**
   * 句型专有的属性集
   */
  Attributes attributes;

  /**
   * 产生此句型的产生式集
   */
  std::set<FormulaID> formulas;
};

/**
 * 语法结构骨架
 */
struct Skeleton::Structure {
  /**
   * 以语法结构为单位分析的属性结构
   */
  Attributes attributes{};

  /**
   * 按句型归纳的属性，未命名句型不归纳
   */
  std::map<std::string, Form> forms{};

  /**
   * 全部句型均包含且结构相同的属性
   */
  Attributes common_attributes{};
};

}  // namespace alioth

#endif