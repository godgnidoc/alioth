#ifndef __ALIOTH_LEXICON_H__
#define __ALIOTH_LEXICON_H__

#include <map>
#include <memory>
#include <vector>

#include "alioth/error.h"
#include "alioth/generic.h"
#include "alioth/regex.h"
#include "nlohmann/json.hpp"

namespace alioth {

/**
 * 记录单词表和词法规则
 */
class Lexicon;
using Lex = std::shared_ptr<Lexicon>;

struct Lexicon {
  struct Term;
  struct State;
  class Builder;

  static constexpr SymbolID kEOF = 0;
  static constexpr SymbolID kERR = -1UL;

  std::vector<Term> terms{};            // 单词表，第一个元素ID为0
  std::vector<std::string> contexts{};  // 上下文表，第一个元素ID为0
  std::vector<State> states{};          // 状态表，第一个元素ID为0

  /**
   * 获取语言名称
   *
   * 约定使用词法规则的第一个上下文名称作为语言名称
   */
  std::string Lang() const;

  /**
   * 获取符号名
   *
   * @param symbol 符号ID
   */
  std::string NameOf(SymbolID symbol) const;

  /**
   * 获取符号的ID
   *
   * @param symbol 符号名
   */
  SymbolID FindSymbol(std::string const& name) const;

  /**
   * 将词法规则保存为JSON格式
   */
  nlohmann::json Store() const;

  /**
   * 从JSON格式加载词法规则
   *
   * @param json JSON格式的词法规则
   */
  static Lex Load(nlohmann::json const& json);
};

struct Lexicon::Term {
  std::string name{};             // 单词名称
  Regex pattern{};                // 单词正则表达式
  std::set<ContextID> entries{};  // 单词入口上下文，空表示任何上下文

  /**
   * 单词属性表
   */
  std::map<std::string, nlohmann::json> attributes{};
};

/**
 * 词法规则构建器
 */
class Lexicon::Builder {
 public:
  /**
   * 构造词法规则构建器
   *
   * @param lang 词法规则语言，用作默认上下文名
   */
  Builder(std::string const& lang);
  Builder(Builder const&) = delete;
  Builder(Builder&&) = delete;
  Builder& operator=(Builder const&) = delete;
  Builder& operator=(Builder&&) = delete;
  ~Builder() = default;

  /**
   * 添加一个单词
   *
   * @param name 单词名称
   * @param pattern 正则表达式
   * @param context 单词上下文，空表示任何上下文
   */
  Builder& Define(std::string const& name, Regex pattern,
                  std::set<std::string> const& context = {});

  /**
   * 为单词添加一个属性注解
   *
   * @param term 单词名称
   * @param key 属性名
   * @param value 属性值
   */
  Builder& Annotate(std::string const& term, std::string const& key,
                    nlohmann::json const& value);

  /**
   * 构建词法规则
   */
  Lex Build();

  struct DuplicateTerm : public Error {
    DuplicateTerm(std::string const& name)
        : Error("Duplicated terminal definition of {}", name) {}
  };

  struct TooManyContexts : public Error {
    TooManyContexts() : Error("TooManyContexts") {}
  };

 protected:
  Lex lex_;
  Regex regex_;
};

/**
 * 词法规则状态
 */
struct Lexicon::State {
  /**
   * 记录当前状态可以接受的单词
   */
  std::optional<SymbolID> accepts{};

  /**
   * 记录从当前状态到其他状态的转移 <ch/ctx, state>
   */
  std::map<char, StateID> transitions{};
};

}  // namespace alioth

#endif