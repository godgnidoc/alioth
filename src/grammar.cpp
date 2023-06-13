#include "alioth/grammar.h"

#include "alioth/alioth.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "nlohmann/json.hpp"

namespace alioth {

Syntax Grammar::Compile() const {
  auto lang = options.at("lang").get<std::string>();

  auto lex = Lexicon::Builder(lang);
  for (auto const& term : terms) {
    auto src = term.regex;
    auto regex = RegexTree::Compile(src);
    lex.Define(term.name, regex, term.contexts);
  }

  auto builder = Syntactic::Builder(lex.Build());
  for (auto const& term : terms) {
    if (term.ignore) {
      builder.Ignore(term.name);
    }
  }

  for (auto const& ntrm : ntrms) {
    for (auto const& f : ntrm.formulas) {
      auto obits = std::accumulate(
          f.symbols.begin(), f.symbols.end(), 0UL,
          [](auto acc, auto const& s) { return acc + (s.optional ? 1 : 0); });
      auto omax = 1UL << obits;
      for (auto oflags = 0UL; oflags < omax; ++oflags) {
        auto fbuilder = builder.Formula(ntrm.name, ntrm.form);
        auto obit = 0UL;
        for (auto const& symbol : f.symbols) {
          if (!symbol.optional) {
            fbuilder.Symbol(symbol.name, symbol.attr);
            continue;
          }

          if (0 != (oflags & (1UL << obit++))) {
            fbuilder.Symbol(symbol.name, symbol.attr);
          }
        }
        fbuilder.Commit();
      }
    }
  }
  return builder.Build();
}

Grammar Grammar::Load(Doc grammar) {
  static auto syntax = [] {
    auto home = AliothHome();
    auto path = home / "grammar" / "grammar-syntax.json";
    if (!std::filesystem::is_regular_file(path)) return SyntaxOf();

    auto jdoc = Document::Read(path);
    auto json = nlohmann::json::parse(jdoc->content);
    return LoadSyntax(json);
  }();
  auto parser = Parser(syntax, grammar);
  auto ast = parser.Parse();

  return Load(ast);
}

Grammar Grammar::Load(ASTRoot root) {
  Grammar g{};

  auto gnode = AsNtrm(root->attributes.at("grammar").front());

  for (auto& option : AttrsOf(gnode, "options")) {
    g.options[*TextOf(option, "key")] = ExtractJson(option);
  }

  for (auto& term : AttrsOf(gnode, "terms")) {
    Term t{};
    t.name = *TextOf(term, "name");
    if (AttrOf(term, "ignore")) t.ignore = true;
    t.regex = *TextOf(term, "regex");
    t.regex = t.regex.substr(1, t.regex.size() - 2);
    for (auto& context : AttrsOf(term, "contexts")) {
      t.contexts.insert(TextOf(context));
    }
    g.terms.push_back(t);
  }

  for (auto& ntrm : AttrsOf(gnode, "ntrms")) {
    Ntrm n{};
    n.name = *TextOf(ntrm, "name");
    n.form = TextOf(ntrm, "form");

    for (auto& formula : AttrsOf(ntrm, "formulas")) {
      Formula f{};

      for (auto& symbol : AttrsOf(formula, "symbols")) {
        Symbol s{};
        s.name = *TextOf(symbol, "name");
        if (auto attr = AttrOf(symbol, "attr"); attr) {
          s.attr = TextOf(attr);
        }
        if (AttrOf(symbol, "optional")) s.optional = true;
        f.symbols.push_back(s);
      }
      n.formulas.push_back(f);
    }
    g.ntrms.push_back(n);
  }

  return g;
}

nlohmann::json Grammar::ExtractJson(AST node) {
  for (auto const& [key, value] : AsNtrm(node)->attributes) {
    if (key == "string" || key == "number")
      return nlohmann::json::parse(TextOf(value.front()));

    if (key == "true") return true;
    if (key == "false") return false;
    if (key == "null") return nullptr;

    if (key == "empty_array") return nlohmann::json::array();
    if (key == "empty_object") return nlohmann::json::object();

    if (key == "array") {
      nlohmann::json result = nlohmann::json::array();
      for (auto const& item : value) {
        result.push_back(ExtractJson(item));
      }
      return result;
    }

    if (key == "object") {
      nlohmann::json result = nlohmann::json::object();

      for (auto const& field : value) {
        result[nlohmann::json::parse(*TextOf(field, "key"))] =
            ExtractJson(field);
      }
      return result;
    }
  }

  throw std::runtime_error("invalid json");
}

Syntax Grammar::SyntaxOf() {
  static auto lex =
      Lexicon::Builder("grammar")
          .Define("LEAD", R"(->)"_regex)
          .Define("LT", R"(<)"_regex)
          .Define("GT", R"(>)"_regex)
          .Define("UNION", R"(\|)"_regex)
          .Define("DEFINE", R"(=)"_regex)
          .Define("IGNORE", R"(\?)"_regex)
          .Define("AT", R"(@)"_regex)
          .Define("SEMICOLON", R"(;)"_regex)
          .Define("COLON", R"(:)"_regex)
          .Define("COMMA", R"(,)"_regex)
          .Define("DOT", R"(\.)"_regex)
          .Define("UNFOLD", R"(\.\.\.)"_regex)
          .Define("LBRACE", R"({)"_regex)
          .Define("RBRACE", R"(})"_regex)
          .Define("LBRACKET", R"(\[)"_regex)
          .Define("RBRACKET", R"(])"_regex)
          .Define("EMPTY", R"(%empty)"_regex)
          .Define("NULL", R"(null)"_regex, {"json"})
          .Define("TRUE", R"(true)"_regex, {"json"})
          .Define("FALSE", R"(false)"_regex, {"json"})
          .Define("STRING", R"(\"([^\"\n\\]|\\[^\n])*\")"_regex, {"json"})
          .Define("NUMBER", R"(-?(0|[1-9]\d*)(\.\d+)?([eE][+-]?\d+)?)"_regex,
                  {"json"})
          .Define("ID", R"([a-zA-Z_]\w*)"_regex)
          .Define("REGEX", R"(\/([^\\\/]|\\[^\n])+\/)"_regex)
          .Define("COMMENT", R"(#[^\n]*\n)"_regex)
          .Define("SPACE", R"(\s+)"_regex)
          .Build();

  static auto syntax = Syntactic::Builder(lex)
                           .Ignore("COMMENT")
                           .Ignore("SPACE")
                           .Formula("grammar")
                           .Symbol("options", "...")
                           .Symbol("terms", "...")
                           .Symbol("ntrms", "...")
                           .Commit()
                           .Formula("grammar")
                           .Symbol("terms", "...")
                           .Symbol("ntrms", "...")
                           .Commit()
                           .Formula("options")
                           .Symbol("option", "options")
                           .Commit()
                           .Formula("options")
                           .Symbol("options", "...")
                           .Symbol("option", "options")
                           .Commit()
                           .Formula("option")
                           .Symbol("ID", "key")
                           .Symbol("COLON")
                           .Symbol("json", "...")
                           .Commit()
                           .Formula("terms")
                           .Symbol("term", "terms")
                           .Commit()
                           .Formula("terms")
                           .Symbol("terms", "...")
                           .Symbol("term", "terms")
                           .Commit()
                           .Formula("term")
                           .Symbol("ID", "name")
                           .Symbol("contexts", "...")
                           .Symbol("DEFINE")
                           .Symbol("REGEX", "regex")
                           .Commit()
                           .Formula("term")
                           .Symbol("ID", "name")
                           .Symbol("contexts", "...")
                           .Symbol("IGNORE", "ignore")
                           .Symbol("DEFINE")
                           .Symbol("REGEX", "regex")
                           .Commit()
                           .Formula("term")
                           .Symbol("ID", "name")
                           .Symbol("DEFINE")
                           .Symbol("REGEX", "regex")
                           .Commit()
                           .Formula("term")
                           .Symbol("ID", "name")
                           .Symbol("IGNORE", "ignore")
                           .Symbol("DEFINE")
                           .Symbol("REGEX", "regex")
                           .Commit()
                           .Formula("contexts")
                           .Symbol("LT")
                           .Symbol("context_list", "...")
                           .Symbol("GT")
                           .Commit()
                           .Formula("context_list")
                           .Symbol("ID", "contexts")
                           .Commit()
                           .Formula("context_list")
                           .Symbol("context_list", "...")
                           .Symbol("COMMA")
                           .Symbol("ID", "contexts")
                           .Commit()
                           .Formula("ntrms")
                           .Symbol("ntrm", "ntrms")
                           .Commit()
                           .Formula("ntrms")
                           .Symbol("ntrms", "...")
                           .Symbol("ntrm", "ntrms")
                           .Commit()
                           .Formula("ntrm")
                           .Symbol("ID", "name")
                           .Symbol("LEAD")
                           .Symbol("formulas", "...")
                           .Symbol("SEMICOLON")
                           .Commit()
                           .Formula("ntrm")
                           .Symbol("ID", "name")
                           .Symbol("DOT")
                           .Symbol("ID", "form")
                           .Symbol("LEAD")
                           .Symbol("formulas", "...")
                           .Symbol("SEMICOLON")
                           .Commit()
                           .Formula("formulas")
                           .Symbol("formula", "formulas")
                           .Commit()
                           .Formula("formulas")
                           .Symbol("empty_formula", "formulas")
                           .Commit()
                           .Formula("formulas")
                           .Symbol("formulas", "...")
                           .Symbol("UNION")
                           .Symbol("formula", "formulas")
                           .Commit()
                           .Formula("formulas")
                           .Symbol("formula", "formulas")
                           .Symbol("UNFOLD")
                           .Symbol("empty_formula", "formulas")
                           .Commit()
                           .Formula("formula")
                           .Symbol("symbol", "symbols")
                           .Commit()
                           .Formula("formula")
                           .Symbol("formula", "...")
                           .Symbol("symbol", "symbols")
                           .Commit()
                           .Formula("empty_formula")
                           .Symbol("EMPTY", "empty")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("ID", "name")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("ID", "name")
                           .Symbol("IGNORE", "optional")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("ID", "name")
                           .Symbol("AT")
                           .Symbol("ID", "attr")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("ID", "name")
                           .Symbol("IGNORE", "optional")
                           .Symbol("AT")
                           .Symbol("ID", "attr")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("UNFOLD", "attr")
                           .Symbol("ID", "name")
                           .Commit()
                           .Formula("symbol")
                           .Symbol("UNFOLD", "attr")
                           .Symbol("ID", "name")
                           .Symbol("IGNORE", "optional")
                           .Commit()
                           .Formula("json")
                           .Symbol("object", "...")
                           .Commit()
                           .Formula("json")
                           .Symbol("array", "...")
                           .Commit()
                           .Formula("json")
                           .Symbol("string", "...")
                           .Commit()
                           .Formula("json")
                           .Symbol("number", "...")
                           .Commit()
                           .Formula("json")
                           .Symbol("boolean", "...")
                           .Commit()
                           .Formula("json")
                           .Symbol("null", "...")
                           .Commit()
                           .Formula("object")
                           .Symbol("LBRACE", "empty_object")
                           .Symbol("RBRACE")
                           .Commit()
                           .Formula("object")
                           .Symbol("LBRACE")
                           .Symbol("fields", "...")
                           .Symbol("RBRACE")
                           .Commit()
                           .Formula("fields")
                           .Symbol("field", "object")
                           .Commit()
                           .Formula("fields")
                           .Symbol("fields", "...")
                           .Symbol("COMMA")
                           .Symbol("field", "object")
                           .Commit()
                           .Formula("field")
                           .Symbol("STRING", "key")
                           .Symbol("COLON")
                           .Symbol("json", "...")
                           .Commit()
                           .Formula("array")
                           .Symbol("LBRACKET", "empty_array")
                           .Symbol("RBRACKET")
                           .Commit()
                           .Formula("array")
                           .Symbol("LBRACKET")
                           .Symbol("elements", "...")
                           .Symbol("RBRACKET")
                           .Commit()
                           .Formula("elements")
                           .Symbol("json", "array")
                           .Commit()
                           .Formula("elements")
                           .Symbol("elements", "...")
                           .Symbol("COMMA")
                           .Symbol("json", "array")
                           .Commit()
                           .Formula("string")
                           .Symbol("STRING", "string")
                           .Commit()
                           .Formula("number")
                           .Symbol("NUMBER", "number")
                           .Commit()
                           .Formula("boolean")
                           .Symbol("TRUE", "true")
                           .Commit()
                           .Formula("boolean")
                           .Symbol("FALSE", "false")
                           .Commit()
                           .Formula("null")
                           .Symbol("NULL", "null")
                           .Commit()
                           .Build();

  return syntax;
}

}  // namespace alioth