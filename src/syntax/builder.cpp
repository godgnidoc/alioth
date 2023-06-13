#include "alioth/syntax/builder.h"

#include <stdexcept>

#include "alioth/lex/lex.h"

namespace alioth::syntax {

namespace {

/**
 * 开始符号
 */
constexpr auto kStart = "S'";

}  // namespace

FormulaBuilder::FormulaBuilder(Builder& builder, std::string const& head)
    : builder_(builder) {
  formula_.head = builder_.GetSymbolId(head);
}

FormulaBuilder& FormulaBuilder::Symbol(std::string const& name,
                                       std::optional<std::string> const& attr) {
  if (attr) {
    formula_.attrs.push_back({formula_.body.size(), builder_.GetAttrId(*attr)});
  }

  formula_.body.push_back(builder_.GetSymbolId(name));
  return *this;
}

FormulaBuilder& FormulaBuilder::Symbol(std::string const& name,
                                       UnfoldHintType const&) {
  formula_.unfolds.push_back(formula_.body.size());
  formula_.body.push_back(builder_.GetSymbolId(name));
  return *this;
}

Builder& FormulaBuilder::Commit() { return builder_.AddFormula(formula_); }

Builder::Builder(LexCRef lex) : lex_(lex) { GetSymbolId(kStart); }

FormulaBuilder Builder::StartFormula(std::string const& head) {
  if (0 < lex_->GetTokenId(head))
    throw std::runtime_error("symbol name conflict with token name");

  return FormulaBuilder(*this, head);
}

Builder& Builder::AddFormula(Formula const& formula) {
  auto& ntrm = ntrms_.at(formula.head);
  ntrm.formulas.insert(formulas_.size());
  formulas_.push_back(formula);

  for (auto offset : formula.unfolds) {
    auto symbol = offset;
    if (IsTerm(symbol)) continue;

    logger_->error("cannot unfold nonterminal symbol {} in {}",
                   GetSymbolName(symbol), DumpFormula(formulas_.size() - 1));
    errors_ += 1;
  }

  return *this;
}

Builder& Builder::Ignore(std::string const& name) {
  auto id = lex_->GetTokenId(name);
  if (id <= 0) {
    logger_->error("unknown token: {}", name);
    errors_++;
    return *this;
  }
  ignores_.insert(id);
  return *this;
}

std::shared_ptr<Syntax> Builder::Build() {
  /**
   * 添加开始符号的产生式，升级为增广文法
   */
  auto start = GetSymbolId(kStart);
  AddFormula(Formula{.head = start, .body = {start + 1, Token::kEOF}});

  CalculateNullable();
  CalculateFirst();
  CalculateFollow();

  for (auto const& [id, ntrm] : ntrms_) {
    std::vector<std::string> first;
    std::vector<std::string> follow;
    for (auto id : ntrm.first) first.push_back(lex_->GetTokenName(id));
    for (auto id : ntrm.follow) follow.push_back(lex_->GetTokenName(id));

    std::string formulas;
    for (auto formula_id : ntrm.formulas) {
      formulas += "  " + DumpFormula(formula_id) + "\n";
    }

    SPDLOG_LOGGER_TRACE(
        logger_, "{} {}:\n\tnullable = {}\n\tfirst = {}\n\tfollow = {}\n{}", id,
        ntrm.name, ntrm.nullable, fmt::join(first, ", "),
        fmt::join(follow, ", "), formulas);
  }

  CalculateStates();

  if (errors_ > 0) return nullptr;

  auto syntax = std::shared_ptr<Syntax>(new Syntax{});

  /**
   * 传递词法规则
   */
  syntax->lex_ = lex_;

  /**
   * 传递非终结符定义
   */
  syntax->ntrms_.reserve(ntrms_.size());
  for (auto const& [_, ntrm] : ntrms_) {
    syntax->ntrms_.push_back(ntrm.name);
  }

  /**
   * 传递归约规则
   */
  syntax->reduces_.reserve(formulas_.size());
  for (auto const& formula : formulas_) {
    syntax->reduces_.push_back(
        Reduce{.ntrm = formula.head,
               .length = static_cast<int>(formula.body.size()),
               .attrs = formula.attrs,
               .unfolds = formula.unfolds});
  }

  /**
   * 传递状态机
   */
  syntax->states_ = states_;

  /**
   * 传递属性名表
   */
  syntax->attributes_ = attrs_;

  /**
   * 传递忽略规则
   */
  syntax->ignores_ = ignores_;

  /**
   * 创建完毕
   */
  return syntax;
}

int Builder::GetSymbolId(std::string const& name) {
  for (auto const& [id, def] : ntrms_) {
    if (def.name == name) return id;
  }

  if (auto id = lex_->GetTokenId(name); id > 0) return id;

  auto base = lex_->CountTokens() + 1;
  auto id = base + ntrms_.size();
  ntrms_.emplace(id, NtrmDef{.name = name});
  return id;
}

int Builder::GetAttrId(std::string const& name) {
  for (auto i = 0UL; i < attrs_.size(); i++) {
    auto const& attr = attrs_.at(i);
    if (attr == name) return i;
  }

  attrs_.push_back(name);
  return attrs_.size() - 1;
}

std::string Builder::GetSymbolName(int id) {
  if (id <= lex_->CountTokens()) return lex_->GetTokenName(id);
  return ntrms_.at(id).name;
}

void Builder::CalculateNullable() {
  std::set<int> nullables;

  /** 先统计能够直接判断的可空符号 */
  for (auto const& [id, def] : ntrms_) {
    for (auto const& formula_id : def.formulas) {
      auto const& formula = formulas_.at(formula_id);
      if (formula.body.empty()) nullables.insert(id);
    }
  }

  while (true) {
    auto snapshot = nullables.size();
    if (snapshot == 0 || snapshot == ntrms_.size()) break;

    /** 遍历所有非终结符 */
    for (auto& [id, ntrm] : ntrms_) {
      if (nullables.count(id)) continue;

      /** 遍历所有产生式 */
      for (auto const& formula_id : ntrm.formulas) {
        auto const& formula = formulas_.at(formula_id);

        /** 如果产生式右部所有符号都可空，则产生式左部也可空 */
        bool all_nullable = true;
        for (auto const& symbol : formula.body) {
          if (IsTerm(symbol)) {
            all_nullable = false;
            break;
          }

          if (!nullables.count(symbol)) {
            all_nullable = false;
            break;
          }
        }
        if (all_nullable) {
          nullables.insert(id);
          break;
        }
      }
    }

    /** 如果没有新的可空符号产生，则结束 */
    if (snapshot == nullables.size()) break;
  }

  for (auto const& id : nullables) {
    ntrms_.at(id).nullable = true;
  }
}

void Builder::CalculateFirst() {
  std::set<int> inprogress;
  std::set<int> calculated;

  /**
   * 前缀关系表，键为前缀非终结符，值为包含该前缀的非终结符集合
   * prefix -> {ntrm}
   */
  std::map<int, std::set<int>> prefix_ntrms;

  for (auto& [id, ntrm] : ntrms_) {
    bool all_terminal = true;

    /** 从所有产生式收集FIRST */
    for (auto const& formula_id : ntrm.formulas) {
      auto const& formula = formulas_.at(formula_id);

      for (auto const& symbol : formula.body) {
        if (symbol != id) ntrm.first.insert(symbol);
        if (IsTerm(symbol)) break;

        if (symbol != id) {
          prefix_ntrms[symbol].insert(id);  // 登记前缀关系
          all_terminal = false;
        }

        if (!ntrms_.at(symbol).nullable) break;
      }
    }

    /** 若FIRST均为终结符，则该非终结符计算完成 */
    if (all_terminal)
      calculated.insert(id);
    else
      inprogress.insert(id);
  }

  /** 归纳计算所有非终结符的FIRST */
  while (!inprogress.empty()) {
    auto const snapshot = inprogress.size();
    auto const known = std::move(calculated);

    /** 基于已知的FIRST，计算新的FIRST */
    for (auto const& prefix_id : known) {
      const auto& prefix = ntrms_.at(prefix_id);

      for (auto const ntrm_id : prefix_ntrms[prefix_id]) {
        auto& ntrm = ntrms_.at(ntrm_id);

        ntrm.first.erase(prefix_id);
        ntrm.first.insert(prefix.first.begin(), prefix.first.end());

        /**
         * 若first集中最大的id是终结符，则该非终结符的first集已不再包含非终结符
         * 判定为已计算完成
         */
        if (IsTerm(*ntrm.first.rbegin())) {
          calculated.insert(ntrm_id);
          inprogress.erase(ntrm_id);
        }
      }
    }

    if (snapshot == inprogress.size()) {
      std::vector<std::string> inprogress_names;
      for (auto id : inprogress) inprogress_names.push_back(GetSymbolName(id));

      logger_->error("circular reference in FIRST: {}",
                     fmt::join(inprogress_names, ", "));
      errors_++;
      break;
    }
  }

  /** 去除FIRST中的EOF */
  // for (auto& [id, ntrm] : ntrms_) {
  //   ntrm.first.erase(Token::kEOF);
  // }
}

void Builder::CalculateFollow() {
  /** ntrm -> {suffix} */
  std::map<int, std::set<int>> ntrm_suffixes;

  /**
   * 依据产生式统计后缀关系和部分FOLLOW
   */
  for (auto formula : formulas_) {
    bool suffix = true;
    auto const body_size = formula.body.size();

    /**
     * 倒序遍历产生式体
     * 在遇到终结符或不可空非终结符前的符号均为当前产生式头的后缀
     */
    for (auto i = body_size; i > 0; --i) {
      auto const& symbol = formula.body.at(i - 1);

      /** 统计后缀关系 */
      if (suffix && !IsTerm(symbol) && symbol != formula.head) {
        ntrm_suffixes[formula.head].insert(symbol);
      }

      /**
       * 遇到终结符时，后缀关系结束
       */
      if (IsTerm(symbol)) {
        suffix = false;
        continue;
      }

      /**
       * 聚焦当前非终结符
       */
      auto& focus = ntrms_.at(symbol);

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
        if (IsTerm(follow_symbol)) {
          focus.follow.insert(follow_symbol);
          break;  // 终结符结束后继关系
        }

        auto const& follow_ntrm = ntrms_.at(follow_symbol);
        focus.follow.insert(follow_ntrm.first.begin(), follow_ntrm.first.end());
        if (!follow_ntrm.nullable) break;  // 不可空非终结符结束后继关系
      }
    }
  }

  /**
   * 需要向自己的后缀传播FOLLOW集的非终结符
   */
  std::set<int> active;
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
      for (auto const suffix_id : ntrm_suffixes[ntrm_id]) {
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

  /** 去除FOLLOW中的空符号 */
  // for (auto& [id, ntrm] : ntrms_) {
  // ntrm.follow.erase(Token::kEmpty);
  // }
}

void Builder::CalculateStates() {
  std::vector<size_t> tasks;
  {
    /** 计算初始项集 */
    std::set<Item> start_items;
    for (auto formula_id : ntrms_.at(GetSymbolId(kStart)).formulas) {
      auto formula = formulas_.at(formula_id);
      start_items.insert({formula_id, 0});
    }
    Closure(start_items);

    /**
     * 登记初始状态
     */
    tasks.push_back(0);
    state_dict_.Set(start_items, 0);
    states_.emplace_back();
  }

  while (!tasks.empty()) {
    /** 找到要处理的状态 */
    auto state_id = tasks.back();
    tasks.pop_back();

    /** 获取状态对应的项集 */
    std::set<Item> items;
    state_dict_.ForKeys(state_id, [&items](auto const& item) {
      items.insert(item);
      return true;
    });

    /** 计算状态的转移 */
    auto const alphabet = Alphabet(items);
    for (auto symbol_id : alphabet) {
      auto next = Goto(items, symbol_id);
      if (next.empty()) continue;

      auto& state = states_.at(state_id);
      auto next_state_id = state.shift[symbol_id] =
          state_dict_.Touch(next, states_.size());

      /** 按需新增状态，并加入任务队列 */
      if (next_state_id != static_cast<int>(states_.size())) continue;

      tasks.push_back(next_state_id);
      states_.emplace_back();
    }

    /** 计算归约 */
    for (auto [formula_id, point] : items) {
      auto& state = states_.at(state_id);
      auto const& formula = formulas_.at(formula_id);
      if (point < static_cast<int>(formula.body.size())) continue;

      auto const& ntrm = ntrms_.at(formula.head);
      for (auto term : ntrm.follow) {
        /**
         * 移进-归约冲突
         *
         * 空归约优先级低，不会产生冲突
         */
        if (state.shift.count(term)) {
          /**
           * 有移进方案时，空归约不采纳
           */
          if (formula.body.empty()) continue;

          /**
           * 报告冲突
           */
          for (auto [fm, pt] : items) {
            if (fm == formula_id && pt == point) continue;
            auto const& f = formulas_.at(fm);
            if (pt >= static_cast<int>(f.body.size())) continue;
            auto input = f.body.at(pt);
            if (input != term) continue;

            logger_->error(
                "shift/reduce conflict:({})\n  reduce: {}\n  shift: {}",
                lex_->GetTokenName(term), DumpItem({formula_id, point}),
                DumpItem({fm, pt}));
          }
          errors_++;
          continue;
        }

        /**
         * 归约-归约冲突
         */
        if (state.reduce.count(term)) {
          logger_->error("reduce/reduce conflict: {}/{} ({})",
                         DumpItem({formula_id, point}),
                         DumpFormula(state.reduce.at(term)),
                         lex_->GetTokenName(term));
          errors_++;
          continue;
        }

        state.reduce[term] = formula_id;
      }
    }
  }
}

void Builder::Closure(std::set<Item>& items) {
  while (true) {
    auto const snapshot = items.size();

    /**
     * 不断将新的项加入项集直到项集不再变化
     */
    for (auto [formula_id, point] : items) {
      auto const& formula = formulas_.at(formula_id);
      if (point >= static_cast<int>(formula.body.size())) continue;

      auto const& symbol = formula.body.at(point);
      if (IsTerm(symbol)) continue;

      auto const& ntrm = ntrms_.at(symbol);
      for (auto const& formula_id : ntrm.formulas) {
        items.insert({formula_id, 0});
      }
    }

    if (snapshot == items.size()) break;
  }
}

std::set<int> Builder::Alphabet(std::set<Item> const& items) {
  std::set<int> alphabet;

  for (auto& [formula_id, point] : items) {
    auto const& formula = formulas_.at(formula_id);
    if (point >= static_cast<int>(formula.body.size())) continue;

    auto symbol = formula.body.at(point);
    if (symbol >= 0) alphabet.insert(symbol);
  }

  return alphabet;
}

std::set<Item> Builder::Goto(std::set<Item> const& items, int symbol_id) {
  std::set<Item> result;

  for (auto [formula_id, point] : items) {
    auto const& formula = formulas_.at(formula_id);
    if (point >= static_cast<int>(formula.body.size())) continue;

    auto const& symbol = formula.body.at(point);
    if (symbol == symbol_id) result.insert({formula_id, point + 1});
  }

  Closure(result);
  return result;
}

bool Builder::IsTerm(int id) { return id <= lex_->CountTokens(); }

std::string Builder::DumpItem(Item const& item) {
  auto const& formula = formulas_.at(item.first);
  auto const& ntrm = ntrms_.at(formula.head);
  auto s = fmt::format("{}: {} ->", item.first, ntrm.name);

  if (formula.body.empty()) return s + "%empty .;";

  for (auto i = 0; i < static_cast<int>(formula.body.size()); ++i) {
    if (i == item.second) s += " .";
    s += " " + GetSymbolName(formula.body.at(i));
  }

  if (item.second == static_cast<int>(formula.body.size())) s += " .";
  return s + ";";
}

std::string Builder::DumpFormula(int formula_id) {
  auto const& formula = formulas_.at(formula_id);
  auto const& ntrm = ntrms_.at(formula.head);
  auto s = fmt::format("{}: {} ->", formula_id, ntrm.name);

  if (formula.body.empty()) return s + " %empty;";

  for (auto i = 0; i < static_cast<int>(formula.body.size()); ++i) {
    s += " " + GetSymbolName(formula.body.at(i));
  }

  return s + ";";
}

}  // namespace alioth::syntax