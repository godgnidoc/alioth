#include "aliox/grammar.h"

#include "alioth/alioth.h"
#include "alioth/parser.h"
#include "alioth/strings.h"
#include "grammar/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {
using namespace generic;

namespace {

/**
 * 文法语义结构
 */
struct G {
  /**
   * 当前文法原文语法树根
   */
  ASTNtrm root;

  /**
   * 闭包后文法的终结符定义
   */
  std::vector<grammar::Term> terms;

  /**
   * 闭包后文法的非终结符定义
   */
  std::vector<grammar::Ntrm> ntrms;

  /**
   * 从文档中创建文法语义结构
   *
   * @param grammar 文档
   */
  static G From(Doc grammar);

  /**
   * 获取所有通过using导入的文法路径
   */
  std::vector<std::filesystem::path> Usings() const;

  /**
   * 获取所有通过外部非终结符定义导入的文法路径
   */
  std::vector<std::filesystem::path> Externs() const;

  /**
   * 获取所有依赖的文法路径
   */
  std::set<std::filesystem::path> Dependencies() const;
  grammar::Grammar View() const;
};

struct Compiling {
  std::map<std::filesystem::path, G> grammers;
  std::map<std::string, Syntax> syntaxes;
  std::map<std::string, std::filesystem::path> paths;
};

G G::From(Doc grammar) {
  auto syntax = SyntaxOf<grammar::Grammar>();
  return {Parser(syntax, grammar).Parse()};
}

std::vector<std::filesystem::path> G::Usings() const {
  auto g = ViewOf<grammar::Grammar>(root);

  return collect<multiple>(g.usings(), [p = *root->doc->path](auto const& u) {
    return Paths::RelativeTo(p, Strings::Parse(u.grammar()->Text()));
  });
}

std::vector<std::filesystem::path> G::Externs() const {
  auto g = ViewOf<grammar::Grammar>(root);

  return collect<multiple>(g.ntrms(), [p = *root->doc->path](auto const& ntrm) {
    auto external = ntrm->As<grammar::Ntrm::External>();
    if (!external) throw skip{};
    return Paths::RelativeTo(p, Strings::Parse(external.grammar()->Text()));
  });
}

std::set<std::filesystem::path> G::Dependencies() const {
  std::set<std::filesystem::path> deps;

  auto usings = Usings();
  auto externals = Externs();

  deps.insert(usings.begin(), usings.end());
  deps.insert(externals.begin(), externals.end());

  return deps;
}

grammar::Grammar G::View() const { return ViewOf<grammar::Grammar>(root); }

/**
 * 收集全部相关文法建立分析上下文，按照自底向上的顺序返回路径表
 *
 * @param compiling 编译上下文
 */
std::vector<std::filesystem::path> Collect(Compiling& compiling) {
  using Unit =
      std::pair<std::filesystem::path, std::set<std::filesystem::path>>;

  std::vector<std::filesystem::path> result;
  std::vector<Unit> pendings;

  auto const& [path, g] = *compiling.grammers.begin();
  pendings.emplace_back(path, g.Dependencies());
  compiling.paths.emplace(g.View().lang()->Text(), path);

  while (!pendings.empty()) {
    auto& [path, deps] = pendings.back();
    if (deps.empty()) {
      pendings.pop_back();
      result.push_back(path);
      continue;
    }

    auto it = deps.begin();
    auto dep = *it;
    deps.erase(it);

    if (some(pendings, [dep](auto const& unit) { return unit.first == dep; })) {
      throw std::runtime_error(
          fmt::format("circular dependency detected: {}", dep.string()));
    }

    if (compiling.grammers.count(dep)) {
      continue;
    }

    auto g = G::From(Document::Read(dep));
    compiling.grammers.emplace(dep, g);
    pendings.emplace_back(dep, g.Dependencies());
    compiling.paths.emplace(g.View().lang()->Text(), dep);
  }

  return result;
}

/**
 * 闭包文法，处理using和外部非终结符定义
 *
 * @param compiling 编译上下文
 * @param path 文法路径
 */
void Closure(Compiling& compiling, std::filesystem::path const& path) {
  auto& g = compiling.grammers.at(path);

  std::map<std::string, AST> terms;

  /**
   * 处理全部词法定义语句，将复用语句替换为原始定义语句
   */
  for (auto stmt : g.View().terms()) {
    auto name = stmt.name()->Text();
    if (terms.count(name)) {
      throw std::runtime_error(fmt::format("duplicate term name: {}", name));
    }

    auto use = stmt->As<grammar::Term::Using>();
    if (!use) {
      g.terms.push_back(stmt);
      terms.emplace(name, stmt);
      continue;
    }

    auto lang = use.lang()->Text();
    auto lg = compiling.grammers.at(compiling.paths.at(lang));
    for (auto const& term : lg.terms) {
      if (term.name()->Text() != name) continue;
      terms.emplace(name, term);
      g.terms.push_back(term);
      break;
    }

    if (!terms.count(name)) {
      throw std::runtime_error(
          fmt::format("unknown term {} in language {}", name, lang));
    }
  }

  /**
   * 按次序将using文法中的终结符添加到当前文法中
   */
  for (auto dep : g.Usings()) {
    auto const& dg = compiling.grammers.at(dep);
    auto lang = dg.View().lang()->Text();
    for (auto const& term : dg.terms) {
      if (terms.count(term.name()->Text())) {
        if (terms.at(term.name()->Text()) != term) {
          throw std::runtime_error(fmt::format(
              "duplicate term {} in language {}", term.name()->Text(), lang));
        }
        continue;
      }
      g.terms.push_back(term);
      terms.emplace(term.name()->Text(), term);
    }
  }

  std::set<std::string> ntrms;

  for (auto stmt : g.View().ntrms()) {
    auto use = stmt->As<grammar::Ntrm::Using>();
    if (!use) {
      ntrms.insert(stmt.name()->Text());
      g.ntrms.push_back(stmt);
      continue;
    }

    std::vector<std::pair<std::string, std::string>> pendings;
    pendings.emplace_back(use.lang()->Text(), use.name()->Text());

    while (!pendings.empty()) {
      auto [lang, name] = pendings.back();
      pendings.pop_back();
    }
  }
}

Syntax Translate(Compiling& compiling, std::filesystem::path const& path) {}

}  // namespace

Syntax Grammar::Compile(Doc grammar) {
  Compiling compiling{};
  compiling.grammers.emplace(*grammar->path, G::From(grammar));

  auto collected = Collect(compiling);
  for (auto const& path : collected) {
    auto& g = compiling.grammers.at(path);
    Closure(compiling, path);
  }

  return Translate(compiling, *grammar->path);

  auto root = Parser(SyntaxOf<grammar::Grammar>(), grammar).Parse();
  auto g = ViewOf<grammar::Grammar>(root);

  auto lang = g.lang()->Text();
  {
    auto it = known.find(lang);
    if (it != known.end()) return it->second;
  }

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

  auto lex = Lexicon::Builder(lang);
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
    std::filesystem::path path = nlohmann::json::parse(imported.from()->Text());
    if (path.is_relative()) {
      path = grammar->path->parent_path() / path;
    }
    auto external = Compile(Document::Read(path), known);
    if (!known.count(external->Lang())) {
      known[external->Lang()] = external;
    }
    auto alias = text::maybe(imported.alias());
    syntax.Import(external, alias);
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