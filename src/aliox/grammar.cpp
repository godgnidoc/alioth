#include "aliox/grammar.h"

#include <numeric>

#include "alioth/alioth.h"
#include "alioth/parser.h"
#include "alioth/strings.h"
#include "grammar/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {
using namespace generic;

namespace {

/**
 * 获取非终结符的formulas
 * 
 * 根据不同的句型(define/formed)获取产生式列表
 */
std::vector<AST> GetFormulas(grammar::Ntrm const& ntrm) {
  auto define = ntrm->As<grammar::Ntrm::Define>();
  if (define) return define.formulas();
  
  auto formed = ntrm->As<grammar::Ntrm::Formed>();
  if (formed) return formed.formulas();
  
  return {};
}

/**
 * 获取非终结符的form名称
 */
std::optional<std::string> GetForm(grammar::Ntrm const& ntrm) {
  auto formed = ntrm->As<grammar::Ntrm::Formed>();
  if (formed) return formed.form()->Text();
  return std::nullopt;
}

}  // namespace

Syntax Grammar::Compile(Doc grammar) {
  auto root = Parser(SyntaxOf<grammar::Grammar>(), grammar).Parse();
  auto g = ViewOf<grammar::Grammar>(root);

  auto lang = g.lang()->Text();

  auto lex = Lexicon::Builder(lang);
  
  // 处理终结符定义
  for (auto const& term : g.terms()) {
    // 跳过 term.using，它是复用其他语言的终结符
    // 在简化版中，我们不支持这个功能
    auto use = term->As<grammar::Term::Using>();
    if (use) {
      continue;
    }
    
    auto define = term->As<grammar::Term::Define>();
    if (!define) continue;
    
    auto src = define.regex()->Text();
    auto regex = RegexTree::Compile(src.substr(1, src.size() - 2));

    lex.Define(define.name()->Text(), regex, collect(define.contexts(), text()));
  }

  auto syntax = Syntactic::Builder(lex.Build());

  // 标记可忽略的终结符
  for (auto const& term : g.terms()) {
    auto define = term->As<grammar::Term::Define>();
    if (!define) continue;
    
    if (define.optional()) {
      syntax.Ignore(define.name()->Text());
    }
  }

  // 处理非终结符定义
  for (auto const& ntrm : g.ntrms()) {
    // 跳过 ntrm.external，它是引用外部文法
    // 保留对外部符号机制的支持，但在这个简化版中只是跳过
    auto external = ntrm->As<grammar::Ntrm::External>();
    if (external) {
      // TODO: 外部非终结符需要从其他语言加载
      continue;
    }
    
    // 跳过 ntrm.using，它是复用其他语言的非终结符
    // 在简化版中，我们不支持这个功能
    auto use = ntrm->As<grammar::Ntrm::Using>();
    if (use) {
      continue;
    }
    
    auto formulas = GetFormulas(ntrm);
    auto form = GetForm(ntrm);
    
    for (auto const& f : formulas) {
      if (f->As<grammar::EmptyFormula>()) {
        syntax.Formula(ntrm.name()->Text(), form).Commit();
        continue;
      }
      
      auto formula = f->As<grammar::Formula>();
      if (!formula) continue;
      
      auto symbols = formula.symbols();
      auto obits = std::accumulate(
          symbols.begin(), symbols.end(), 0UL,
          [](auto acc, auto const& s) { return acc + (s.optional() ? 1 : 0); });
      auto omax = 1UL << obits;
      
      for (auto oflags = 0UL; oflags < omax; ++oflags) {
        auto builder = syntax.Formula(ntrm.name()->Text(), form);
        auto obit = 0UL;
        
        for (auto const& symbol : symbols) {
          if (symbol.optional() && 0 == (oflags & (1UL << obit++))) continue;
          builder.Symbol(symbol.name()->Text(), text::maybe(symbol.attr()));
        }
        
        builder.Commit();
      }
    }
  }

  return syntax.Build();
}

}  // namespace alioth
