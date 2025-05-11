#include "aliox/grammar.h"

#include "alioth/alioth.h"
#include "alioth/parser.h"
#include "annotate/syntax.h"
#include "grammar/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {
using namespace generic;

Syntax Grammar::Compile(Doc grammar) {
  // std::map<std::string, std::vector<annotate::Annotate>> annotates;

  auto root = Parser(SyntaxOf<grammar::Grammar>(), grammar).Parse();
  auto g = ViewOf<grammar::Grammar>(root);

  auto options = associate(collect<multiple>(g.options(), [](auto const& it) {
    return std::pair{
        it.key()->Text(),
        nlohmann::json::parse(it.value()->Text()),
    };
  }));

  auto annotations = associate<multiple>(
      collect<multiple>(g.annotations(), [](auto const& it) {
        return
            [selectors = it.selectors(), annotation = it, i = 0UL]() mutable {
              if (i == selectors.size()) throw nomore{};
              auto selector = selectors[i++];
              return std::pair{
                  selector.symbol()->Text(),
                  std::pair(selector, annotation),
              };
            };
      }));

  auto lex = Lexicon::Builder(options.at("lang"));
  for (auto const& term : g.terms()) {
    auto src = term.regex()->Text();
    auto regex = RegexTree::Compile(src.substr(1, src.size() - 2));

    lex.Define(term.name()->Text(), regex, collect(term.contexts(), text()));
    for (auto const& attr : term.attributes()) {
      if (attr.of()) {
        throw std::runtime_error("Cannot annotate attribute of term");
      }
      lex.Annotate(term.name()->Text(), attr.key()->Text(),
                   nlohmann::json::parse(attr.value()->Text()));
    }
    for (auto const& [symbol, it] : annotations) {
      if (symbol != term.name()->Text()) continue;
      for (auto const& [selector, annotation] : it) {
        if (selector.form()) {
          throw std::runtime_error("Cannot annotate term with form");
        }

        for (auto const& attr : annotation.attributes()) {
          if (attr.of()) {
            throw std::runtime_error("Cannot annotate attribute of term");
          }
          lex.Annotate(term.name()->Text(), attr.key()->Text(),
                       nlohmann::json::parse(attr.value()->Text()));
        }
      }
    }
  }

  auto syntax = Syntactic::Builder(lex.Build());

  for (auto const& term : g.terms()) {
    if (term.ignore()) {
      syntax.Ignore(term.name()->Text());
    }
  }

  for (auto const& imported : g.imports()) {
    auto lang = imported.lang()->Text();
    auto alias = text::maybe(imported.alias());
    syntax.Import(lang, alias);
  }

  for (auto const& ntrm : g.ntrms()) {
    for (auto const& f : ntrm.formulas()) {
      if (f->As<grammar::EmptyFormula>()) {
        syntax.Formula(ntrm.name()->Text(), ntrm->TextOf("form")).Commit();
        continue;
      }
      auto formula = f->As<grammar::Formula>();
      auto symbols = formula.symbols();
      auto obits = std::accumulate(
          symbols.begin(), symbols.end(), 0UL,
          [](auto acc, auto const& s) { return acc + (s.optional() ? 1 : 0); });
      auto omax = 1UL << obits;
      for (auto oflags = 0UL; oflags < omax; ++oflags) {
        auto builder =
            syntax.Formula(ntrm.name()->Text(), text::maybe(ntrm.form()));
        auto obit = 0UL;
        for (auto const& symbol : formula.symbols()) {
          if (symbol.optional() && 0 == (oflags & (1UL << obit++))) continue;
          builder.Symbol(symbol.name()->Text(), text::maybe(symbol.attr()));
        }
        for (auto const& attr : formula.attributes()) {
          if (!attr.of()) {
            throw std::runtime_error("Cannot annotate ntrm directly");
          }
          builder.Annotate(attr.of()->Text(), attr.key()->Text(),
                           nlohmann::json::parse(attr.value()->Text()));
        }
        for (auto const& [symbol, it] : annotations) {
          if (symbol != ntrm.name()->Text()) continue;

          for (auto const& [selector, annotation] : it) {
            if (text::maybe(selector.form()) != text::maybe(ntrm.form()))
              continue;

            for (auto const& attr : annotation.attributes()) {
              if (!attr.of()) {
                throw std::runtime_error("Cannot annotate ntrm directly");
              }
              builder.Annotate(attr.of()->Text(), attr.key()->Text(),
                               nlohmann::json::parse(attr.value()->Text()));
            }
          }
        }
        builder.Commit();
      }
    }
  }

  return syntax.Build();
}

}  // namespace alioth