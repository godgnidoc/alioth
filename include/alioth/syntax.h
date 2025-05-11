#ifndef __ALIOTH_SYNTAX_H__
#define __ALIOTH_SYNTAX_H__

#include <map>
#include <memory>
#include <optional>
#include <set>

#include "alioth/error.h"
#include "alioth/generic.h"
#include "alioth/lexicon.h"
#include "nlohmann/json.hpp"

namespace alioth {

/**
 * 语法规则
 */
class Syntactic;
using Syntax = std::shared_ptr<Syntactic>;

/**
 * 获取指定语言的语法规则
 *
 * @tparam Lang 语言类型
 */
template <typename Lang>
Syntax SyntaxOf();

struct Syntactic {
  struct LR0Item;
  struct LR1Item;
  struct State;
  struct Formula;
  class Builder;

  Lex lex;
  std::vector<std::string> ntrms;           // 非终结符，ID从最大词法单元ID开始
  std::vector<Formula> formulas;            // 产生式，ID与下标一致
  std::vector<State> states;                // 语法分析状态机
  std::set<SymbolID> ignores;               // 语法分析过程中应当忽略的符号
  std::map<SymbolID, FormulaID> externals;  // 导入符号与导入产生式映射

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
   * 判断符号是否为终结符
   *
   * @param id 符号ID
   */
  bool IsTerm(SymbolID id) const;

  /**
   * 判断符号是否代表导入的语言
   *
   * @param id 符号ID
   */
  bool IsImported(SymbolID id) const;

  /**
   * 判断符号是否被忽略
   *
   * @param id 符号ID
   */
  bool IsIgnored(SymbolID id) const;

  /**
   * 打印语法规则
   * 可以使用 https://jsmachines.sourceforge.net/machines/lalr1.html 查看
   */
  std::string Print() const;

  /**
   * 打印语法规则的一个状态
   *
   * @param state 状态ID
   */
  std::string PrintState(StateID state) const;

  /**
   * 打印产生式
   *
   * @param formula 产生式ID
   * @param point 产生式中的点
   */
  std::string PrintFormula(FormulaID formula,
                           std::optional<size_t> point = std::nullopt) const;

  /**
   * 将语法规则保存为JSON格式
   */
  nlohmann::json Store() const;

  /**
   * 从JSON格式加载语法规则
   *
   * @param json JSON格式的语法规则
   */
  static Syntax Load(nlohmann::json const& json);
};

struct Syntactic::LR0Item {
  FormulaID formula{};
  size_t point{};

  bool operator<(LR0Item const& other) const;
  bool operator==(LR0Item const& other) const;
};

struct Syntactic::LR1Item {
  FormulaID formula{};
  size_t point{};
  SymbolID ahead{};

  bool operator<(LR1Item const& other) const;
  bool operator==(LR1Item const& other) const;
};

/**
 * 语法分析器状态
 */
struct Syntactic::State {
  std::map<SymbolID, StateID> shift{};     // 移进规则
  std::map<SymbolID, FormulaID> reduce{};  // 规约规则
  std::set<ContextID> contexts{};          // 可能的上下文
  std::set<SymbolID> externals{};          // 当前状态可能期待的外部符号
};

/**
 * 产生式
 */
struct Syntactic::Formula {
  struct Symbol;

  SymbolID head{};                    // 目标非终结符ID
  std::vector<Symbol> body{};         // 产生式体
  std::optional<std::string> form{};  // 产生式所属句式
  std::optional<std::string> lang{};  // 导入语法

  /**
   * 符号属性注解表
   *
   * 产生式归约成功后，依据符号属性表向指定符号写入属性
   *
   * 产生式归约获得的属性可能超过产生式直接包含的符号
   * 所以符号属性注解不能定义在产生式体的符号定义上
   *
   * attr -> key -> value
   */
  std::map<std::string, std::map<std::string, nlohmann::json>> attributes{};

  /**
   * 判断产生式是否完全展开
   *
   * 完全展开产生式用于继承句型
   * 完全展开产生式没有句型名，仅拥有一个展开符号
   */
  bool Unfolded() const;

  /**
   * 判断产生式是否导入了其他语言
   *
   * 导入产生式没有产生式体，使用 lang 字段指定目标语言
   */
  bool Imported() const;
};

/**
 * 产生式体中的符号
 */
struct Syntactic::Formula::Symbol {
  /**
   * 符号ID
   */
  SymbolID id{};

  /**
   * 接收此符号的属性名
   *
   * "..." 表示将当前符号携带的属性表展开到目标符号
   */
  std::optional<std::string> attr{};

  /**
   * 判断符号是否应当被展开
   */
  bool Unfolded() const;
};

/**
 * 语法规则构建器
 */
class Syntactic::Builder {
  class FormulaBuilder;
  friend class FormulaBuilder;
  struct NtrmDef;

 public:
  /**
   * 构造语法规则构建器
   *
   * @param lex 词法规则
   */
  Builder(Lex lex);
  Builder(Builder const&) = delete;
  Builder(Builder&&) = delete;
  Builder& operator=(Builder const&) = delete;
  Builder& operator=(Builder&&) = delete;
  ~Builder() = default;

  /**
   * 开始定义非终结符
   *
   * 在开始添加非终结符前必须先添加所有终结符
   * 因为推导式中使用但未定义的符号被视为非终结符
   *
   * @param head 非终结符名，自动创建
   * @param form 产生式所属句式名，可选
   */
  FormulaBuilder Formula(std::string const& head,
                         std::optional<std::string> const& form = {});

  /**
   * 开始定义非终结符
   *
   * 在开始添加非终结符前必须先添加所有终结符
   * 因为推导式中使用但未定义的符号被视为非终结符
   *
   * @param head 非终结符ID，必须存在
   * @param form 产生式所属句式名，可选
   */
  FormulaBuilder Formula(SymbolID head,
                         std::optional<std::string> const& form = {});

  /**
   * 忽略一个单词
   *
   * 在语法分析阶段忽略一种终结符
   * 多次调用可以忽略多个终结符
   *
   * 被忽略的终结符不参与移进和归约判定，也不占用归约长度
   *
   * @param name 终结符名称
   */
  Builder& Ignore(std::string const& name);

  /**
   * 导入一种语言用作一个非终结符
   *
   * 默认情况下使用语言名作为非终结符名，可选地可以为其指定一个别名
   *
   * @param lang 语言名称
   * @param alias 导入非终结符的别名
   */
  Builder& Import(std::string const& lang,
                  std::optional<std::string> const& alias = {});

  /**
   * 创建LALR(1)语法规则
   */
  Syntax Build();

  struct UnknownTermError : public Error {
    UnknownTermError(std::string const& name)
        : Error("unknown terminal symbol {}", name) {}
  };

  struct TermHeadError : public Error {
    TermHeadError(std::string const& name)
        : Error("terminal symbol {} cannot be a head of formula", name) {}
  };

  struct ExternalHeadError : public Error {
    ExternalHeadError(std::string const& name)
        : Error("external symbol {} cannot be a head of formula", name) {}
  };

  struct AlreadyImportedError : public Error {
    AlreadyImportedError(std::string const& lang)
        : Error("language {} is already imported") {}
  };

  struct EmptyFirstError : public Error {
    EmptyFirstError(std::string const& ntrm)
        : Error("FIRST set of {} is empty", ntrm) {}
  };

  struct CircularFirstError : public Error {
    CircularFirstError(std::vector<std::string> const& ntrms)
        : Error("circular reference in FIRST set between : {}",
                fmt::join(ntrms, ", ")) {}
  };

  struct ReduceReduceConflict : public Error {
    ReduceReduceConflict() : Error("reduce-reduce conflict detected") {}
  };

  struct ShiftReduceConflict : public Error {
    ShiftReduceConflict() : Error("shift-reduce conflict detected") {}
  };

 protected:
  void CalculateNullable();
  void CalculateFirst();
  void CalculateFollow();
  void CalculateStates();

  std::set<LR1Item> Closure(std::set<LR1Item> items);
  std::set<LR1Item> Goto(std::set<LR1Item> const& items, SymbolID symbol);
  std::set<SymbolID> Alphabet(std::set<LR1Item> const& items);

  /**
   * 获取非终结符 ID，必要时创建新符号
   */
  SymbolID TouchNtrm(std::string const& name);

 protected:
  Syntax syntax_;
  std::map<SymbolID, NtrmDef> ntrms_;
};

struct Syntactic::Builder::NtrmDef {
  std::string name{};
  std::set<size_t> formulas{};
  bool nullable{};
  std::set<SymbolID> first{};
  std::set<SymbolID> follow{};
};

/**
 * 产生式构造器
 */
class Syntactic::Builder::FormulaBuilder {
 public:
  FormulaBuilder(Builder& builder, Syntactic::Formula& formula);
  FormulaBuilder(FormulaBuilder const&) = delete;
  FormulaBuilder(FormulaBuilder&&) = delete;
  FormulaBuilder& operator=(FormulaBuilder const&) = delete;
  FormulaBuilder& operator=(FormulaBuilder&&) = delete;
  ~FormulaBuilder() = default;

  /**
   * 添加一个符号
   *
   * @param name 符号名，自动创建
   * @param attr 符号属性名，"..." 表示将当前符号携带的属性表展开到目标符号
   */
  FormulaBuilder& Symbol(std::string const& name,
                         std::optional<std::string> const& attr = std::nullopt);

  /**
   * 添加一个符号
   *
   * @param symbol 符号ID，不必存在，不自动创建
   * @param attr 符号属性名，"..." 表示将当前符号携带的属性表展开到目标符号
   */
  FormulaBuilder& Symbol(SymbolID symbol,
                         std::optional<std::string> const& attr = std::nullopt);

  /**
   * 添加一个符号属性注解
   *
   * @param attr 符号属性名
   * @param key 属性名
   * @param value 属性值
   */
  FormulaBuilder& Annotate(std::string const& attr, std::string const& key,
                           nlohmann::json const& value);

  /**
   * 完成产生式构建
   */
  Builder& Commit();

 protected:
  Builder& builder_;
  Syntactic::Formula& formula_;
};

}  // namespace alioth

#endif