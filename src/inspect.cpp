#include "alioth/inspect.h"

#include <stack>

#include "fmt/format.h"

namespace alioth {

Point PointAt(size_t offset, Doc doc) {
  Point point{};
  for (auto i = 0UL; i < offset; ++i) {
    if (doc->content[i] == '\n') {
      ++point.line;
      point.column = 1;
    } else {
      ++point.column;
    }
  }
  return point;
}

ASTNtrm AsNtrm(AST node) {
  return std::dynamic_pointer_cast<ASTNtrmNode>(node);
}

ASTTerm AsTerm(AST node) {
  return std::dynamic_pointer_cast<ASTTermNode>(node);
}

AST AttrOf(AST node, std::string const& key) {
  auto ntrm = AsNtrm(node);
  if (!ntrm) return nullptr;

  auto it = ntrm->attributes.find(key);
  if (it == ntrm->attributes.end()) return nullptr;
  if (it->second.empty()) return nullptr;

  return it->second.front();
}

std::vector<AST> AttrsOf(AST node, std::string const& key) {
  auto ntrm = AsNtrm(node);
  if (!ntrm) return {};

  auto it = ntrm->attributes.find(key);
  if (it == ntrm->attributes.end()) return {};

  return it->second;
}

Range RangeOf(AST node) {
  auto root = node->root.lock();

  auto first = FirstOf(node);
  auto last = LastOf(node);

  auto start = PointAt(first->offset, root->doc);
  auto end = PointAt(last->offset + last->length, root->doc);

  return Range{start, end};
}

std::string NameOf(AST node) {
  auto root = node->root.lock();
  return NameOf(node->id, root->syntax);
}

std::string TextOf(AST node) {
  auto root = node->root.lock();

  if (auto term = AsTerm(node))
    return root->doc->content.substr(term->offset, term->length);

  auto ntrm = AsNtrm(node);
  auto first = FirstOf(ntrm);
  auto last = LastOf(ntrm);

  return root->doc->content.substr(first->offset,
                                   last->offset + last->length - first->offset);
}

std::optional<std::string> TextOf(AST node, std::string const& key) {
  auto attr = AttrOf(node, key);
  if (!attr) return std::nullopt;

  return TextOf(attr);
}

std::string LocationOf(AST node) {
  auto root = node->root.lock();

  if (auto ntrm = AsNtrm(node)) return LocationOf(FirstOf(ntrm));

  auto term = AsTerm(node);
  auto point = PointAt(term->offset, root->doc);
  return fmt::format("{}:{}:{}",
                     root->doc->path.value_or("<unknown-path>").string(),
                     point.line, point.column);
}

ASTTerm FirstOf(AST node) {
  if (auto term = AsTerm(node)) return term;

  auto ntrm = AsNtrm(node);
  for (auto symbol : ntrm->sentence) {
    auto term = FirstOf(symbol);
    if (term) return term;
  }
  return nullptr;
}

ASTTerm LastOf(AST node) {
  if (auto term = AsTerm(node)) return term;

  auto ntrm = AsNtrm(node);
  for (auto it = ntrm->sentence.rbegin(); it != ntrm->sentence.rend(); ++it) {
    auto symbol = *it;
    auto term = LastOf(symbol);
    if (term) return term;
  }
  return nullptr;
}

std::vector<ASTTerm> Tokenize(AST node) {
  if (auto term = AsTerm(node)) return {term};

  using S = decltype(ASTNtrmNode::sentence);
  using I = typename S::const_iterator;
  std::stack<std::pair<S const*, I>> stack;

  auto ntrm = AsNtrm(node);
  stack.push({&ntrm->sentence, ntrm->sentence.begin()});

  std::vector<ASTTerm> tokens;
  while (!stack.empty()) {
    auto& [sentence, it] = stack.top();
    if (it == sentence->end()) {
      stack.pop();
      continue;
    }

    while (it != sentence->end()) {
      auto symbol = *it++;
      if (auto term = AsTerm(symbol)) {
        tokens.push_back(term);
        continue;
      }
      auto ntrm = AsNtrm(symbol);
      stack.push({&ntrm->sentence, ntrm->sentence.begin()});
      break;
    }
  }

  return tokens;
}

nlohmann::json AttrsOf(AST node) {
  if (auto term = AsTerm(node)) return TextOf(node);

  auto ntrm = AsNtrm(node);
  nlohmann::json attrs;
  for (auto const& [name, values] : ntrm->attributes) {
    if (values.size() == 1) {
      attrs[name] = AttrsOf(values.front());
      continue;
    }

    nlohmann::json attr;
    for (auto const& value : values) {
      attr.push_back(AttrsOf(value));
    }
    attrs[name] = attr;
  }

  return attrs;
}
std::string NameOf(SymbolID symbol, std::variant<Lex, Syntax> alphabeta) {
  if (symbol == Lexicon::kERR)
    return "<ERR>";
  else if (symbol == Lexicon::kEOF)
    return "<EOF>";

  Lex lex{};
  Syntax syntax{};
  if (std::holds_alternative<Lex>(alphabeta)) {
    lex = std::get<Lex>(alphabeta);
  } else {
    syntax = std::get<Syntax>(alphabeta);
    lex = syntax->lex;
  }

  if (symbol < lex->terms.size()) {
    return lex->terms.at(symbol).name;
  }

  return syntax->ntrms.at(symbol - lex->terms.size());
}

SymbolID SymbolIdOf(std::string const& name,
                    std::variant<Lex, Syntax> alphabeta) {
  if (name == "<ERR>") return Lexicon::kERR;
  if (name == "<EOF>") return Lexicon::kEOF;

  Lex lex{};
  Syntax syntax{};
  if (std::holds_alternative<Lex>(alphabeta)) {
    lex = std::get<Lex>(alphabeta);
  } else {
    syntax = std::get<Syntax>(alphabeta);
    lex = syntax->lex;
  }

  auto id = 0UL;
  for (auto const& term : lex->terms) {
    if (term.name == name) return id;
    ++id;
  }

  if (syntax) {
    for (auto const& ntrm : syntax->ntrms) {
      if (ntrm == name) return id;
      ++id;
    }
  }

  throw std::runtime_error(fmt::format("Unknown symbol {}", name));
}

std::string PrintFormula(FormulaID formula, Syntax syntax,
                         std::optional<size_t> point) {
  auto const& f = syntax->formulas.at(formula);
  auto result = fmt::format("{} -> ", NameOf(f.head, syntax));
  auto off = 0UL;
  for (auto const& symbol : f.body) {
    if (point && off++ == *point) {
      result += "• ";
    }
    result += fmt::format("{} ", NameOf(symbol.id, syntax));
  }
  if (point && off == *point) {
    result += "• ";
  }
  return result;
}

std::string PrintState(StateID state, Syntax syntax) {
  auto const& st = syntax->states.at(state);

  std::string result = fmt::format("State {}\n", state);
  for (auto const& reduce : st.reduce) {
    result +=
        fmt::format("  Reduce {} : {}\n", PrintFormula(reduce.second, syntax),
                    NameOf(reduce.first, syntax));
  }
  for (auto const& shift : st.shift) {
    result += fmt::format("  Shift {} : {}\n", shift.second,
                          NameOf(shift.first, syntax));
  }

  return result;
}

std::string PrintSyntax(Syntax syntax) {
  std::string result;
  for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
    result += PrintFormula(formula, syntax) + "\n";
  }
  return result;
}

nlohmann::json StoreSkeleton(Skeleton const& lang) {
  nlohmann::json json;
  auto syntax = lang.syntax;

  for (auto const& [symbol, structure] : lang.structures) {
    nlohmann::json s;
    for (auto const& [name, attr] : structure.attributes) {
      nlohmann::json a;
      a["optional"] = attr.is_optional;
      a["single"] = attr.is_single;
      for (auto const& id : attr.candidates) {
        a["candidates"].push_back(NameOf(id, syntax));
      }
      s["attributes"][name] = a;
    }
    for (auto const& [name, attr] : structure.common_attributes) {
      nlohmann::json a;
      a["optional"] = attr.is_optional;
      a["single"] = attr.is_single;
      for (auto const& id : attr.candidates) {
        a["candidates"].push_back(NameOf(id, syntax));
      }
      s["common_attributes"][name] = a;
    }
    for (auto const& [form, formed] : structure.formed_attributes) {
      nlohmann::json f;
      for (auto const& [name, attr] : formed) {
        nlohmann::json a;
        a["optional"] = attr.is_optional;
        a["single"] = attr.is_single;
        for (auto const& id : attr.candidates) {
          a["candidates"].push_back(NameOf(id, syntax));
        }
        f["attributes"][name] = a;
      }
      s["formed_attributes"][form] = f;
    }
    auto symbol_name = NameOf(symbol, syntax);
    json[symbol_name] = s;
  }

  return json;
}

nlohmann::json StoreSyntax(Syntax syntax) {
  nlohmann::json json;
  json["lex"] = StoreLex(syntax->lex);
  json["ntrms"] = syntax->ntrms;
  for (auto const& formula : syntax->formulas) {
    nlohmann::json f;
    f["head"] = formula.head;
    if (formula.form) f["form"] = *formula.form;
    for (auto const& symbol : formula.body) {
      nlohmann::json s;
      s["id"] = symbol.id;
      if (symbol.attr) s["attr"] = *symbol.attr;
      f["body"].push_back(s);
    }
    json["formulas"].push_back(f);
  }

  for (auto const& state : syntax->states) {
    nlohmann::json s;
    for (auto const& shift : state.shift) {
      s["shift"][std::to_string(shift.first)] = shift.second;
    }
    for (auto const& reduce : state.reduce) {
      s["reduce"][std::to_string(reduce.first)] = reduce.second;
    }
    for (auto const& context : state.contexts) {
      s["contexts"].push_back(context);
    }
    json["states"].push_back(s);
  }

  json["ignores"] = syntax->ignores;

  return json;
}

Syntax LoadSyntax(nlohmann::json const& json) {
  auto syntax = std::make_shared<Syntactic>();
  syntax->lex = LoadLex(json["lex"]);
  syntax->ntrms = json["ntrms"].get<std::vector<std::string>>();

  for (auto const& f : json["formulas"]) {
    auto formula = Syntactic::Formula{};
    formula.head = f["head"];
    if (f.contains("form")) formula.form = f["form"];
    if (f.contains("body"))
      for (auto const& s : f["body"]) {
        Syntactic::Formula::Symbol symbol;
        symbol.id = s["id"];
        if (s.contains("attr")) symbol.attr = s["attr"];
        formula.body.push_back(symbol);
      }
    syntax->formulas.push_back(formula);
  }

  for (auto const& s : json["states"]) {
    auto state = Syntactic::State{};
    if (s.contains("shift"))
      for (auto const& [symbol, id] : s["shift"].items()) {
        state.shift[std::stoul(symbol)] = id;
      }
    if (s.contains("reduce"))
      for (auto const& [symbol, id] : s["reduce"].items()) {
        state.reduce[std::stoul(symbol)] = id;
      }
    if (s.contains("contexts"))
      for (auto const& context : s["contexts"]) {
        state.contexts.insert(context.get<ContextID>());
      }
    syntax->states.push_back(state);
  }

  syntax->ignores = json["ignores"].get<std::set<SymbolID>>();

  return syntax;
}

nlohmann::json StoreLex(Lex lex) {
  nlohmann::json json;
  auto& terms = json["terms"];
  for (auto const& term : lex->terms) {
    nlohmann::json t;
    t["name"] = term.name;
    // TODO 尚未实现正则表达式转文本，暂时不保存正则表达式
    for (auto const& entry : term.entries) {
      t["entries"].push_back(entry);
    }
    terms.push_back(t);
  }

  json["contexts"] = lex->contexts;
  for (auto const& state : lex->states) {
    nlohmann::json s;
    if (state.accepts) s["accepts"] = *state.accepts;
    for (auto const& [ch, st] : state.transitions) {
      s["transitions"][std::to_string(ch)] = st;
    }
    json["states"].push_back(s);
  }
  return json;
}

Lex LoadLex(nlohmann::json const& json) {
  auto lex = std::make_shared<Lexicon>();
  for (auto const& t : json["terms"]) {
    Lexicon::Term term{};
    term.name = t["name"];
    if (t.contains("entries"))
      for (auto const& entry : t["entries"]) {
        term.entries.insert(entry.get<char>());
      }
    lex->terms.push_back(term);
  }

  lex->contexts = json["contexts"];
  for (auto const& s : json["states"]) {
    Lexicon::State state{};
    if (s.contains("accepts")) state.accepts = s["accepts"];
    if (s.contains("transitions"))
      for (auto const& [input, st] : s["transitions"].items()) {
        state.transitions[std::stoi(input)] = st;
      }
    lex->states.push_back(state);
  }

  return lex;
}

}  // namespace alioth