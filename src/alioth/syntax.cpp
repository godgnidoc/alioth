#include "alioth/syntax.h"

#include <tuple>

namespace alioth {

bool Syntactic::IsTerm(SymbolID id) const {
  return id < lex->terms.size() || id == Lexicon::kERR;
}

bool Syntactic::IsIgnored(SymbolID id) const { return ignores.count(id); }

std::string Syntactic::Lang() const { return lex->Lang(); }

std::string Syntactic::NameOf(SymbolID symbol) const {
  if (symbol == Lexicon::kERR) return "<ERR>";
  if (symbol == Lexicon::kEOF) return "<EOF>";

  if (symbol < lex->terms.size()) {
    return lex->terms.at(symbol).name;
  }

  return ntrms.at(symbol - lex->terms.size());
}

SymbolID Syntactic::FindSymbol(std::string const& name) const {
  auto id = 0UL;

  for (auto const& term : lex->terms) {
    if (term.name == name) return id;
    ++id;
  }

  for (auto const& ntrm : ntrms) {
    if (ntrm == name) return id;
    ++id;
  }

  throw std::runtime_error(fmt::format("Unknown symbol {}", name));
}

std::string Syntactic::Print() const {
  std::string result;
  for (auto formula = 0UL; formula < formulas.size(); ++formula) {
    result += PrintFormula(formula) + "\n";
  }
  return result;
}

std::string Syntactic::PrintState(StateID state) const {
  auto const& st = states.at(state);

  std::string result = fmt::format("State {}\n", state);
  for (auto const& reduce : st.reduce) {
    result += fmt::format("  Reduce {} : {}\n", PrintFormula(reduce.second),
                          NameOf(reduce.first));
  }
  for (auto const& shift : st.shift) {
    result +=
        fmt::format("  Shift {} : {}\n", shift.second, NameOf(shift.first));
  }

  return result;
}

std::string Syntactic::PrintFormula(FormulaID formula,
                                    std::optional<size_t> point) const {
  auto const& f = formulas.at(formula);
  auto result = fmt::format("{} -> ", NameOf(f.head));
  auto off = 0UL;
  for (auto const& symbol : f.body) {
    if (point && off++ == *point) {
      result += "• ";
    }
    result += fmt::format("{} ", NameOf(symbol.id));
  }
  if (point && off == *point) {
    result += "• ";
  }
  return result;
}

nlohmann::json Syntactic::Store() const {
  nlohmann::json json;
  json["lex"] = lex->Store();
  json["ntrms"] = ntrms;
  for (auto const& formula : formulas) {
    nlohmann::json f;
    f["head"] = formula.head;
    if (formula.form) f["form"] = *formula.form;
    for (auto const& symbol : formula.body) {
      nlohmann::json s;
      s["id"] = symbol.id;
      if (symbol.attr) s["attr"] = *symbol.attr;
      f["body"].push_back(s);
    }
    if (!formula.attributes.empty()) f["attrs"] = formula.attributes;
    json["formulas"].push_back(f);
  }

  for (auto const& state : states) {
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

  json["ignores"] = ignores;

  return json;
}

Syntax Syntactic::Load(nlohmann::json const& json) {
  auto syntax = std::make_shared<Syntactic>();
  syntax->lex = Lexicon::Load(json["lex"]);
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
    if (f.contains("attrs")) formula.attributes = f["attrs"];
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

bool Syntactic::LR0Item::operator<(LR0Item const& other) const {
  return std::tie(formula, point) < std::tie(other.formula, other.point);
}
bool Syntactic::LR0Item::operator==(LR0Item const& other) const {
  return std::tie(formula, point) == std::tie(other.formula, other.point);
}

bool Syntactic::LR1Item::operator<(LR1Item const& other) const {
  return std::tie(formula, point, ahead) <
         std::tie(other.formula, other.point, other.ahead);
}
bool Syntactic::LR1Item::operator==(LR1Item const& other) const {
  return std::tie(formula, point, ahead) ==
         std::tie(other.formula, other.point, other.ahead);
}

bool Syntactic::Formula::Unfolded() const {
  if (form) return false;
  if (body.size() != 1) return false;
  if (body.front().attr.value_or("") != "...") return false;
  return true;
}

bool Syntactic::Formula::Symbol::Unfolded() const {
  return attr && *attr == "...";
}

Syntactic::Builder::Builder(Lex lex) {
  syntax_ = Syntax{new Syntactic{}};
  syntax_->lex = lex;
  auto start = TouchNtrm("S'");
  Formula(start).Symbol(start + 1, lex->Lang()).Commit();
}

Syntactic::Builder::FormulaBuilder Syntactic::Builder::Formula(
    std::string const& head, std::optional<std::string> const& form) {
  auto id = TouchNtrm(head);
  if (syntax_->IsTerm(id)) throw TermHeadError{head};
  return Formula(id, form);
}

Syntactic::Builder::FormulaBuilder Syntactic::Builder::Formula(
    SymbolID head, std::optional<std::string> const& form) {
  ntrms_.at(head).formulas.insert(syntax_->formulas.size());
  syntax_->formulas.push_back({.head = head, .form = form});
  return FormulaBuilder{*this, syntax_->formulas.back()};
}

Syntactic::Builder& Syntactic::Builder::Ignore(std::string const& name) {
  for (auto id = 0UL; id < syntax_->lex->terms.size(); ++id) {
    if (syntax_->lex->terms.at(id).name == name) {
      syntax_->ignores.insert(id);
      return *this;
    }
  }
  throw UnknownTermError{name};
}

Syntax Syntactic::Builder::Build() {
  CalculateNullable();
  CalculateFirst();
  CalculateFollow();
  CalculateStates();

  return syntax_;
}

void Syntactic::Builder::CalculateNullable() {
  std::set<SymbolID> nullables;

  /** 先统计能够直接判断的可空符号 */
  for (auto const& formula : syntax_->formulas) {
    if (formula.body.empty()) nullables.insert(formula.head);
  }

  /** 推导出所有可空符号 */
  while (true) {
    auto snapshot = nullables.size();
    if (snapshot == 0 || snapshot == ntrms_.size()) break;

    /** 遍历所有非终结符 */
    for (auto const& formula : syntax_->formulas) {
      if (nullables.count(formula.head)) continue;

      /** 如果产生式右部所有符号都可空，则产生式左部也可空 */
      bool all_nullable = true;
      for (auto const& symbol : formula.body) {
        if (syntax_->IsTerm(symbol.id) || !nullables.count(symbol.id)) {
          all_nullable = false;
          break;
        }
      }
      if (all_nullable) {
        nullables.insert(formula.head);
        break;
      }
    }

    /** 如果没有新的可空符号产生，则结束 */
    if (snapshot == nullables.size()) break;
  }

  for (auto const& id : nullables) {
    ntrms_.at(id).nullable = true;
  }
}

void Syntactic::Builder::CalculateFirst() {
  std::set<SymbolID> inprogress;
  std::set<SymbolID> calculated;

  /**
   * 前缀关系表，键为前缀非终结符，值为包含该前缀的非终结符集合
   * prefix -> {ntrm}
   */
  std::map<SymbolID, std::set<SymbolID>> prefix_ntrms;

  /**
   * 为每个非终结符迭代全部产生式初步收集前缀集合
   *
   * 从左到右逐个将产生式体的符号加入前缀集合
   * 遇到终结符或不可空非终结符时结束
   *
   * 若全部前缀符号均为终结符，则该非终结符的FIRST集合已计算完成
   */
  for (auto& [id, ntrm] : ntrms_) {
    for (auto const& formula_id : ntrm.formulas) {
      auto const& formula = syntax_->formulas.at(formula_id);

      for (auto const& symbol : formula.body) {
        if (symbol.id != id) ntrm.first.insert(symbol.id);
        if (syntax_->IsTerm(symbol.id)) break;

        if (symbol.id != id) {
          prefix_ntrms[symbol.id].insert(id);
        }

        if (!ntrms_.at(symbol.id).nullable) break;
      }
    }

    if (syntax_->IsTerm(*ntrm.first.rbegin())) {
      calculated.insert(id);
    } else {
      inprogress.insert(id);
    }
  }

  /**
   * 基于已知的前缀关推广FIRST集合
   */
  while (!inprogress.empty()) {
    auto const snapshot = inprogress.size();
    auto const known = std::move(calculated);

    /** 基于已知的FIRST，计算新的FIRST */
    for (auto const& prefix_id : known) {
      const auto& prefix = ntrms_.at(prefix_id);

      for (auto const& ntrm_id : prefix_ntrms[prefix_id]) {
        auto& ntrm = ntrms_.at(ntrm_id);

        ntrm.first.erase(prefix_id);
        ntrm.first.insert(prefix.first.begin(), prefix.first.end());

        if (ntrm.first.empty()) throw EmptyFirstError{ntrm.name};

        if (syntax_->IsTerm(*ntrm.first.rbegin())) {
          calculated.insert(ntrm_id);
          inprogress.erase(ntrm_id);
        }
      }
    }

    if (snapshot == inprogress.size()) {
      std::vector<std::string> inprogress_names;
      for (auto id : inprogress) inprogress_names.push_back(ntrms_.at(id).name);

      throw CircularFirstError{inprogress_names};
    }
  }
}

void Syntactic::Builder::CalculateFollow() {
  /**
   * 非终结符与其后缀关系表，键为非终结符，值为后缀符号集合
   * ntrm -> {suffix}
   */
  std::map<SymbolID, std::set<SymbolID>> ntrm_suffixes;

  /**
   * 依据产生式统计后缀关系和部分FOLLOW
   */
  for (auto formula : syntax_->formulas) {
    bool suffix = true;
    auto const body_size = formula.body.size();

    /**
     * 倒序遍历产生式体
     * 在遇到终结符或不可空非终结符前的符号均为当前产生式头的后缀
     */
    for (auto i = body_size; i > 0; --i) {
      auto const& symbol = formula.body.at(i - 1);

      /** 统计后缀关系 */
      if (suffix && !syntax_->IsTerm(symbol.id) && symbol.id != formula.head) {
        ntrm_suffixes[formula.head].insert(symbol.id);
      }

      /**
       * 遇到终结符时，后缀关系结束
       */
      if (syntax_->IsTerm(symbol.id)) {
        suffix = false;
        continue;
      }

      /**
       * 聚焦当前非终结符
       */
      auto& focus = ntrms_.at(symbol.id);

      /**
       * 若当前非终结符不可空，则后缀关系结束
       */
      if (!focus.nullable) suffix = false;

      /**
       * 当前非终结符的FOLLOW集包含其后继符号的FIRST集
       * 终结符或不可空的非终结符会结束后继关系
       */
      for (auto j = i; j < body_size; ++j) {
        auto const& follow_symbol = formula.body.at(j);
        if (syntax_->IsTerm(follow_symbol.id)) {
          focus.follow.insert(follow_symbol.id);
          break;  // 终结符结束后继关系
        }

        auto const& follow_ntrm = ntrms_.at(follow_symbol.id);
        focus.follow.insert(follow_ntrm.first.begin(), follow_ntrm.first.end());
        if (!follow_ntrm.nullable) break;  // 不可空非终结符结束后继关系
      }
    }
  }

  /**
   * 需要向自己的后缀传播FOLLOW集的非终结符
   */
  std::set<SymbolID> active;
  for (auto const& [id, _] : ntrm_suffixes) {
    active.insert(id);
  }

  /** 利用后缀关系补全FOLLOW */
  while (!active.empty()) {
    auto const known = std::move(active);

    for (auto ntrm_id : known) {
      auto& ntrm = ntrms_.at(ntrm_id);

      /**
       * 当前非终结符的全部后缀符号的FOLLOW集均包含当前非终结符的FOLLOW集
       */
      for (auto const& suffix_id : ntrm_suffixes[ntrm_id]) {
        auto& suffix = ntrms_.at(suffix_id);

        /**
         * 若FOLLOW集发生变化，则激活后缀非终结符，以便传播变化
         */
        auto const snapshot = suffix.follow.size();
        suffix.follow.insert(ntrm.follow.begin(), ntrm.follow.end());
        if (snapshot != suffix.follow.size()) active.insert(suffix_id);
      }
    }
  }
}

void Syntactic::Builder::CalculateStates() {
  std::map<StateID, std::set<LR1Item>> itemsets;
  itemsets.emplace(0, Closure({{}}));
  syntax_->states.push_back({});
  std::vector<StateID> tasks{0};

  while (!tasks.empty()) {
    auto const state_id = tasks.back();
    tasks.pop_back();
    auto itemset = itemsets.at(state_id);
    for (auto x : Alphabet(itemset)) {
      auto next = Goto(itemset, x);
      if (next.empty()) continue;

      std::optional<StateID> next_state;
      for (auto const& [s, I] : itemsets) {
        if (I != next) continue;
        next_state = s;
        break;
      }
      if (!next_state) {
        next_state = syntax_->states.size();
        syntax_->states.push_back({});
        itemsets.emplace(*next_state, next);
        tasks.push_back(*next_state);
      }

      auto& state = syntax_->states.at(state_id);
      state.shift.emplace(x, *next_state);
    }

    for (auto const& item : itemset) {
      auto const& formula = syntax_->formulas.at(item.formula);
      if (item.point != formula.body.size()) continue;

      auto& state = syntax_->states.at(state_id);
      if (state.shift.count(item.ahead)) {
        fmt::println(stderr, "shift reduce conflict: {}",
                     syntax_->NameOf(item.ahead));
        fmt::println(stderr, "Reduce: {}", syntax_->PrintFormula(item.formula));
        fmt::println(stderr, "Shift:");
        for (auto const& it : itemset) {
          if (it.ahead != item.ahead) continue;

          fmt::println(stderr, "  {}",
                       syntax_->PrintFormula(it.formula, it.point));
        }
        fmt::println(stderr, "state: {}", syntax_->PrintState(state_id));
        throw ShiftReduceConflict{};
      }

      if (state.reduce.count(item.ahead)) {
        fmt::println(stderr, "reduce-reduce conflict: {}",
                     syntax_->NameOf(item.ahead));
        fmt::println(stderr, "Reduce: {}", syntax_->PrintFormula(item.formula));
        fmt::println(stderr, "Reduce: {}",
                     syntax_->PrintFormula(state.reduce.at(item.ahead)));
        fmt::println(stderr, "state: {}", syntax_->PrintState(state_id));
        throw ReduceReduceConflict{};
      }

      state.reduce.emplace(item.ahead, item.formula);
    }
  }

  /** 为每个状态计算所处上下文 */
  for (auto& state : syntax_->states) {
    std::set<SymbolID> terms;
    for (auto const& [symbol, next] : state.shift) {
      if (!syntax_->IsTerm(symbol)) continue;
      terms.insert(symbol);
    }
    for (auto const& [symbol, formula] : state.reduce) {
      terms.insert(symbol);
    }

    for (auto const term : terms) {
      auto const& entries = syntax_->lex->terms.at(term).entries;
      state.contexts.insert(entries.begin(), entries.end());
    }
  }
}

std::set<Syntactic::LR1Item> Syntactic::Builder::Closure(
    std::set<LR1Item> items) {
  while (true) {
    auto const snapshot = items.size();

    for (auto const& item : items) {
      auto const& formula = syntax_->formulas.at(item.formula);
      if (item.point == formula.body.size()) continue;

      auto const& symbol = formula.body.at(item.point);
      if (syntax_->IsTerm(symbol.id)) continue;

      auto all_nullable = true;
      std::set<SymbolID> aheads;
      for (auto it = formula.body.begin() + item.point + 1;
           it != formula.body.end(); ++it) {
        if (syntax_->IsTerm(it->id)) {
          all_nullable = false;
          aheads.insert(it->id);
          break;
        }
        auto const& first = ntrms_.at(it->id).first;
        aheads.insert(first.begin(), first.end());
        if (!ntrms_.at(it->id).nullable) {
          all_nullable = false;
          break;
        }
      }
      if (all_nullable) aheads.insert(item.ahead);

      for (auto const formula_id : ntrms_.at(symbol.id).formulas) {
        for (auto ahead : aheads) {
          items.insert({.formula = formula_id, .point = 0, .ahead = ahead});
        }
      }
    }

    if (snapshot == items.size()) break;
  }

  return items;
}

std::set<Syntactic::LR1Item> Syntactic::Builder::Goto(
    std::set<LR1Item> const& items, SymbolID symbol) {
  std::set<LR1Item> result;
  for (auto const& item : items) {
    auto const& formula = syntax_->formulas.at(item.formula);
    if (item.point == formula.body.size()) continue;

    auto const& next = formula.body.at(item.point);
    if (next.id == symbol) {
      result.insert({.formula = item.formula,
                     .point = item.point + 1,
                     .ahead = item.ahead});
    }
  }
  return Closure(result);
}

std::set<SymbolID> Syntactic::Builder::Alphabet(
    std::set<LR1Item> const& items) {
  std::set<SymbolID> alphabet;
  for (auto const& item : items) {
    auto const& formula = syntax_->formulas.at(item.formula);
    if (item.point == formula.body.size()) continue;

    auto const& symbol = formula.body.at(item.point);
    alphabet.insert(symbol.id);
  }
  return alphabet;
}

SymbolID Syntactic::Builder::TouchNtrm(std::string const& name) {
  for (auto id = 0UL; id < syntax_->lex->terms.size(); ++id) {
    if (syntax_->lex->terms.at(id).name == name) return id;
  }
  for (auto const& [id, ntrm] : ntrms_) {
    if (ntrm.name == name) return id;
  }

  auto const id = syntax_->lex->terms.size() + syntax_->ntrms.size();
  syntax_->ntrms.push_back(name);
  ntrms_.emplace(id, NtrmDef{.name = name});
  return id;
}

Syntactic::Builder::FormulaBuilder::FormulaBuilder(Builder& builder,
                                                   Syntactic::Formula& formula)
    : builder_{builder}, formula_{formula} {}

Syntactic::Builder::FormulaBuilder& Syntactic::Builder::FormulaBuilder::Symbol(
    std::string const& name, std::optional<std::string> const& attr) {
  return Symbol(builder_.TouchNtrm(name), attr);
}

Syntactic::Builder::FormulaBuilder& Syntactic::Builder::FormulaBuilder::Symbol(
    SymbolID symbol, std::optional<std::string> const& attr) {
  formula_.body.push_back({.id = symbol, .attr = attr});
  return *this;
}

Syntactic::Builder::FormulaBuilder&
Syntactic::Builder::FormulaBuilder::Annotate(std::string const& attr,
                                             std::string const& key,
                                             nlohmann::json const& value) {
  formula_.attributes[attr][key] = value;
  return *this;
}

Syntactic::Builder& Syntactic::Builder::FormulaBuilder::Commit() {
  return builder_;
}

}  // namespace alioth