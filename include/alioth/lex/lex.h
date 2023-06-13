#ifndef __ALIOTH_LEX_H__
#define __ALIOTH_LEX_H__

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/lex/builder-fwd.h"
#include "alioth/token.h"
#include "nlohmann/json.hpp"

namespace alioth {

namespace lex {

/**
 * 状态
 *
 * Lex 内部状态
 */
struct State {
  /**
   * 记录当前状态可以接受的词法记号
   * 0 表示不能在此状态接受单词
   */
  int accepts{0};

  /**
   * 记录从当前状态到其他状态的转移 <ch/ctx, state>
   */
  std::map<int, int> transitions;
};

using StateRef = std::shared_ptr<State>;

static constexpr char const* kDefaultContext = "default";
static constexpr int kDefaultContextID = 1;

}  // namespace lex

/**
 * Lex 词法
 *
 * Lex类是对一套词法规则的抽象
 * Lex 总是尽可能匹配最长的词法单元
 * 若有多个词法单元匹配同一段文本，则优先匹配先添加的词法单元
 * Lex 可以基于上下文提供不同的词法分析策略
 */
class Lex {
 public:
  friend class lex::Builder;

 protected:
  Lex() = default;

 public:
  Lex(Lex const&) = delete;
  Lex(Lex&&) = delete;

  Lex& operator=(Lex const&) = delete;
  Lex& operator=(Lex&&) = delete;

  /**
   * 获取上下文名称
   *
   * @param id 上下文ID
   */
  std::string GetContextName(int id) const;

  /**
   * 获取上下文ID
   *
   * @param name 上下文名称
   */
  int GetContextId(std::string const& name) const;

  /**
   * 获取所有上下文名称
   */
  std::vector<std::string> GetContexts() const;

  /**
   * 获取词法记号名称
   *
   * @param token 词法记号ID
   */
  std::string GetTokenName(int token) const;

  /**
   * 获取词法记号ID
   *
   * @param name 词法记号名称
   */
  int GetTokenId(std::string const& name) const;

  /**
   * 获取所有词法记号名称
   */
  std::vector<std::string> GetTokens() const;

  /**
   * 统计词法记号数量
   */
  int CountTokens() const;

  /**
   * 获取状态
   *
   * @param state 状态ID
   */
  lex::StateRef GetState(int state) const;

  /**
   * 保存到 JSON
  */
  nlohmann::json Save() const;

  /**
   * 从 JSON 加载
   */
  static std::shared_ptr<Lex> Load(nlohmann::json const& json);

 private:
  std::vector<std::string> tokens_;  // 词法记号表，第一个元素ID为1
  std::vector<std::string> contexts_;   // 上下文表，第一个元素ID为0
  std::vector<lex::StateRef> states_;  // 状态表，第一个元素ID为0
};

}  // namespace alioth

#endif