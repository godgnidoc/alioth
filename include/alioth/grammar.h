#ifndef __ALIOTH_GRAMMAR_H__
#define __ALIOTH_GRAMMAR_H__

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "alioth/ast.h"
#include "alioth/document.h"
#include "alioth/error.h"
#include "alioth/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {

/**
 * 语法定义
 */
struct Grammar {
  struct Term;
  struct Ntrm;
  struct Formula;
  struct Symbol;

  nlohmann::json options{};
  std::vector<Term> terms{};
  std::vector<Ntrm> ntrms{};

  /**
   * 编译文法定义为语法规则
   */
  Syntax Compile() const;

  /**
   * 从文法源码加载文法定义
   *
   * @param grammar 文法源码
   */
  static Grammar Load(Doc grammar);

  /**
   * 从 AST 加载文法定义
   *
   * @param root 文法 AST
   */
  static Grammar Load(ASTRoot root);

  /**
   * 获取文法定义的语法规则
   */
  static Syntax SyntaxOf();

  /**
   * 从JSON语法结构的属性树提取json变量
   *
   * @param node JSON语法结构
   */
  static nlohmann::json ExtractJson(AST node);

 protected:
};

struct Grammar::Term {
  std::string name{};
  bool ignore{};
  std::set<std::string> contexts{};
  std::string regex{};
};

struct Grammar::Ntrm {
  std::string name{};
  std::optional<std::string> form{};
  std::vector<Formula> formulas{};
};

struct Grammar::Formula {
  std::vector<Symbol> symbols{};
};

struct Grammar::Symbol {
  std::string name{};
  std::optional<std::string> attr{};
  bool optional{};
};

}  // namespace alioth

#endif