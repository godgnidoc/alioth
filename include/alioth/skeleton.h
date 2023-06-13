#ifndef __ALIOTH_SKELETON_H__
#define __ALIOTH_SKELETON_H__

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
   * 推导语言骨架
   *
   * 分析语法结构的句型，为每种句型分析的属性结构
   *
   * @param syntax 语法规则
   * @return 语言骨架
   */
  static Skeleton Deduce(Syntax const& syntax);
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
  std::map<std::string, Attributes> formed_attributes{};

  /**
   * 全部句型均包含且结构相同的属性
   */
  Attributes common_attributes{};
};

}  // namespace alioth

#endif