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
  return Compile(grammar, {});
}

Syntax Grammar::Compile(Doc grammar, ExternalLoader loader) {
  auto root = Parser(SyntaxOf<grammar::Grammar>(), grammar).Parse();
  auto g = ViewOf<grammar::Grammar>(root);

  auto lang = g.lang()->Text();

  auto lex = Lexicon::Builder(lang);
  
  // 处理终结符定义
  for (auto const& term : g.terms()) {
    auto src = term.regex()->Text();
    auto regex = RegexTree::Compile(src.substr(1, src.size() - 2));

    lex.Define(term.name()->Text(), regex, collect(term.contexts(), text()));
  }

  auto syntax = Syntactic::Builder(lex.Build());

  // 标记可忽略的终结符
  for (auto const& term : g.terms()) {
    if (term.optional()) {
      syntax.Ignore(term.name()->Text());
    }
  }

  // 处理非终结符定义
  for (auto const& ntrm : g.ntrms()) {
    // 处理外部符号: 导入其他语言作为外部符号
    auto external = ntrm->As<grammar::Ntrm::External>();
    if (external) {
      auto grammarPath = external.grammar()->Text();
      // 去除引号
      grammarPath = grammarPath.substr(1, grammarPath.size() - 2);
      
      if (loader) {
        auto externalSyntax = loader(grammarPath);
        if (externalSyntax) {
          syntax.Import(externalSyntax, external.name()->Text());
        }
      }
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
