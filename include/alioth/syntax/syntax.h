#ifndef __ALIOTH_SYNTAX_H__
#define __ALIOTH_SYNTAX_H__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/syntax/syntax-fwd.h"
#include "alioth/syntax/builder-fwd.h"
#include "nlohmann/json.hpp"

namespace alioth {

namespace syntax {

/**
 * 归约规则，与产生式一一对应
 */
struct Reduce {
  /**
   * 目标非终结符ID
   */
  int ntrm;

  /**
   * 归约单词数
   */
  int length;

  /**
   * 属性接收规则 <符号偏移量，属性名偏移量>
   */
  std::vector<std::pair<int, int>> attrs;

  /**
   * 要展开属性的符号的下标
   */
  std::vector<int> unfolds;
};

/**
 * 项目集，即状态
 */
struct State {
  /** 项目集转移 symbol_id -> state_id */
  std::map<int, int> shift;

  /** 规约项 term_id -> reduce_id */
  std::map<int, int> reduce;
};

}  // namespace syntax

/**
 * 语法规则抽象
 */
class Syntax {
  friend class syntax::Builder;

 private:
  Syntax() = default;

 public:
  Syntax(Syntax const&) = delete;
  Syntax(Syntax&&) = delete;

  Syntax& operator=(Syntax const&) = delete;
  Syntax& operator=(Syntax&&) = delete;

  /**
   * 获取接受符号ID
   */
  int GetAcceptSymbol() const;

  /**
   * 获取符号名称
   *
   * @param id 终结符或非终结符的ID
   */
  std::string GetSymbolName(int id) const;

  /**
   * 获取属性名称
   */
  std::string GetAttrName(int id) const;

  /**
   * 获取词法规则
   */
  LexCRef GetLex() const;

  /**
   * 获取状态
   *
   * @param id 状态ID
   */
  syntax::State const& GetState(int id) const;

  /**
   * 获取归约规则
   *
   * @param id 归约规则ID
   */
  syntax::Reduce const& GetReduce(int id) const;

  /**
   * 判断符号是否应当被忽略
   */
  bool IsIgnored(int id) const;

  /**
   * 保存语法规则
   */
  nlohmann::json Save() const;

  /**
   * 加载语法规则
   */
  static std::shared_ptr<Syntax> Load(nlohmann::json const& json);

 private:
  /**
   * 获取属性名id，若不存在则添加
   */
  int GetOrAddAttr(std::string const& name);

 private:
  /**
   * 词法规则
   */
  LexCRef lex_;

  /**
   * 非终结符名表
   * 第一个元素是起始符号
   * (下标+lex->CountTokens()) 是非终结符的ID
   */
  std::vector<std::string> ntrms_;

  /**
   * 属性名表
   */
  std::vector<std::string> attributes_;

  /**
   * 归约规则
   */
  std::vector<syntax::Reduce> reduces_;

  /**
   * 状态表
   */
  std::vector<syntax::State> states_;

  /**
   * 语法分析过程中忽略的符号
   */
  std::set<int> ignores_;
};

}  // namespace alioth

#endif