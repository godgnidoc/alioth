#include "abnf.h"

#include "alioth/lex/builder.h"
#include "alioth/logging.h"
#include "alioth/syntax/builder.h"
#include "alioth/syntax/parser.h"

void ABNF::PopulateFormulas(nlohmann::json& formulas) {
  auto origin_size = formulas.size();
  for (auto i = 0UL; i < origin_size; ++i) {
    auto& formula = formulas[i];
    if (formula.count("empty")) continue;
    auto symbols = formula["symbols"];
    if (symbols.is_object()) {
      if (symbols.count("optional")) {
        alioth::Logger {
          "ABNF"
        } -> error("The only symbol in a formula can't be optional");
      }
      continue;
    }

    std::vector<size_t> optional_indexes;
    for (auto j = 0UL; j < symbols.size(); ++j) {
      auto& symbol = symbols[j];
      if (symbol.count("optional")) {
        optional_indexes.push_back(j);
      }
    }

    if (optional_indexes.empty()) continue;
    std::reverse(optional_indexes.begin(), optional_indexes.end());
    auto library = nlohmann::json::array();

    for (auto idx : optional_indexes) {
      auto snapshot = library.size();
      library.push_back(formula);
      library.back()["symbols"].erase(idx);
      for (auto k = 0UL; k < snapshot; k++) {
        library.push_back(library[k]);
        library.back()["symbols"].erase(idx);
      }
    }

    formulas.insert(formulas.end(), library.begin(), library.end());
  }
}

alioth::SyntaxCRef ABNF::Compile(alioth::SourceRef source) {
  auto ast = alioth::syntax::Parser::Parse(GetSyntax(), source);
  if (!ast) return nullptr;

  auto abs = ast->DumpAbs();
  std::set<std::string> ignores;

  alioth::lex::Builder lex_builder{};
  auto tokens = abs["tokens"];
  if (tokens.is_object()) tokens = {tokens};
  for (auto& token : tokens) {
    auto name = token["name"].get<std::string>();
    auto src = token["regex"].get<std::string>();
    auto regex = alioth::regex::Compile(src.substr(1, src.size() - 2));
    lex_builder.AddToken(name, regex);
    if (token.count("ignore")) ignores.insert(name);
  }

  alioth::syntax::Builder syntax_builder{lex_builder.Build()};
  auto ntrms = abs["ntrms"];
  if (ntrms.is_object()) ntrms = {ntrms};
  for (auto& ntrm : ntrms) {
    auto name = ntrm["name"].get<std::string>();
    auto formulas = ntrm["formulas"];
    if (formulas.is_object()) formulas = {formulas};
    PopulateFormulas(formulas);

    for (auto& formula : formulas) {
      auto formula_bulder = syntax_builder.StartFormula(name);

      if (formula.count("empty")) {
        formula_bulder.Commit();
        continue;
      }

      auto symbols = formula["symbols"];
      if (symbols.is_object()) symbols = {symbols};
      for (auto& symbol : symbols) {
        auto name = symbol["name"].get<std::string>();
        if (symbol.count("attr")) {
          auto attr = symbol["attr"].get<std::string>();
          formula_bulder.Symbol(name, attr);
        } else if (symbol.count("unfold")) {
          formula_bulder.Symbol(name, alioth::syntax::kUnfold);
        } else {
          formula_bulder.Symbol(name);
        }
      }

      formula_bulder.Commit();
    }
  }

  for (auto& ignore : ignores) syntax_builder.Ignore(ignore);

  return syntax_builder.Build();
}

alioth::LexCRef ABNF::GetLex() {
  using namespace alioth;

  return lex::Builder()
      .AddToken("EMPTY", regex::Compile(R"(%empty)"))
      .AddToken("ID", regex::Compile(R"([a-zA-Z_]\w*)"))
      .AddToken("REGEX", regex::Compile(R"(\/([^\\\/]|\\[^\n])+\/)"))
      .AddToken("LEAD", regex::Compile(R"(->)"))
      .AddToken("LT", regex::Compile(R"(<)"))
      .AddToken("GT", regex::Compile(R"(>)"))
      .AddToken("UNION", regex::Compile(R"(\|)"))
      .AddToken("DEFINE", regex::Compile(R"(=)"))
      .AddToken("IGNORE", regex::Compile(R"(\?)"))
      .AddToken("AT", regex::Compile(R"(@)"))
      .AddToken("SEMICOLON", regex::Compile(R"(;)"))
      .AddToken("COMMA", regex::Compile(R"(,)"))
      .AddToken("UNFOLD", regex::Compile(R"(\.\.\.)"))
      .AddToken("SPACE", regex::Compile(R"(\s+)"))
      .AddToken("COMMENT", regex::Compile(R"(#[^\n]*\n)"))
      .Build();
}

alioth::SyntaxCRef ABNF::GetSyntax() {
  using namespace alioth;

  return syntax::Builder(GetLex())
      .Ignore("SPACE")
      .Ignore("COMMENT")
      .StartFormula("prog")
      .Symbol("lex", syntax::kUnfold)
      .Symbol("syntax", syntax::kUnfold)
      .Commit()
      .StartFormula("lex")
      .Commit()
      .StartFormula("lex")
      .Symbol("lex", syntax::kUnfold)
      .Symbol("token", "tokens")
      .Commit()
      .StartFormula("token")
      .Symbol("ID", "name")
      .Symbol("contexts", syntax::kUnfold)
      .Symbol("DEFINE")
      .Symbol("REGEX", "regex")
      .Commit()
      .StartFormula("token")
      .Symbol("ID", "name")
      .Symbol("contexts", syntax::kUnfold)
      .Symbol("IGNORE", "ignore")
      .Symbol("DEFINE")
      .Symbol("REGEX", "regex")
      .Commit()
      .StartFormula("contexts")
      .Commit()
      .StartFormula("contexts")
      .Symbol("LT")
      .Symbol("context_list", syntax::kUnfold)
      .Symbol("GT")
      .Commit()
      .StartFormula("context_list")
      .Symbol("ID", "contexts")
      .Commit()
      .StartFormula("context_list")
      .Symbol("context_list", syntax::kUnfold)
      .Symbol("COMMA")
      .Symbol("ID", "contexts")
      .Commit()
      .StartFormula("syntax")
      .Symbol("ntrm", "ntrms")
      .Commit()
      .StartFormula("syntax")
      .Symbol("syntax", syntax::kUnfold)
      .Symbol("ntrm", "ntrms")
      .Commit()
      .StartFormula("syntax")
      .Commit()
      .StartFormula("ntrm")
      .Symbol("ID", "name")
      .Symbol("LEAD")
      .Symbol("body_group", syntax::kUnfold)
      .Symbol("SEMICOLON")
      .Commit()
      .StartFormula("body_group")
      .Symbol("formula", "formulas")
      .Commit()
      .StartFormula("body_group")
      .Symbol("empty_formula", "formulas")
      .Commit()
      .StartFormula("body_group")
      .Symbol("body_group", syntax::kUnfold)
      .Symbol("UNION")
      .Symbol("formula", "formulas")
      .Commit()
      .StartFormula("body_group")
      .Symbol("body_group", syntax::kUnfold)
      .Symbol("UNION")
      .Symbol("empty_formula", "formulas")
      .Commit()
      .StartFormula("formula")
      .Symbol("symbol", "symbols")
      .Commit()
      .StartFormula("formula")
      .Symbol("formula", syntax::kUnfold)
      .Symbol("symbol", "symbols")
      .Commit()
      .StartFormula("empty_formula")
      .Symbol("EMPTY", "empty")
      .Commit()
      .StartFormula("symbol")
      .Symbol("ID", "name")
      .Commit()
      .StartFormula("symbol")
      .Symbol("ID", "name")
      .Symbol("IGNORE", "optional")
      .Commit()
      .StartFormula("symbol")
      .Symbol("ID", "name")
      .Symbol("AT")
      .Symbol("ID", "attr")
      .Commit()
      .StartFormula("symbol")
      .Symbol("ID", "name")
      .Symbol("IGNORE", "optional")
      .Symbol("AT")
      .Symbol("ID", "attr")
      .Commit()
      .StartFormula("symbol")
      .Symbol("UNFOLD", "unfold")
      .Symbol("ID", "name")
      .Commit()
      .StartFormula("symbol")
      .Symbol("UNFOLD", "unfold")
      .Symbol("ID", "name")
      .Symbol("IGNORE", "optional")
      .Commit()
      .Build();
}