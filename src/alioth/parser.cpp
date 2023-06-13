#include "alioth/parser.h"

namespace alioth {

Parser::Parser(Syntax syntax, Doc doc) : root_{new ASTRootNode} {
  root_->doc = doc;
  root_->syntax = syntax;
  root_->root = root_;
}

ASTRoot Parser::Parse() {
  auto syntax = root_->syntax;
  threads_.push_back(Thread{
      .stack = {0},
  });

  while (!threads_.empty()) {
    std::vector<size_t> failed;
    std::vector<size_t> accepted;

    /**
     * 扫描新的输入，必要时产生新的分析线路
     */
    ScanAndFork();

    /**
     * 处理所有分析线路
     */
    for (auto i = 0UL; i < threads_.size(); i++) {
      auto& thread = threads_[i];

      /**
       * 归约当前输入符号
       * 若当前线路接受了归约结果，则将其加入接受列表
       */
      auto snap = candidates_.size();
      if (ReduceOrFalse(thread)) {
        if (candidates_.size() > snap) accepted.push_back(i);
        continue;
      }

      /**
       * 移进当前输入符号
       */
      if (ShiftOrFalse(thread)) continue;

      /**
       * 忽略当前输入符号
       */
      if (IgnoreOrFalse(thread)) continue;

      /**
       * 以上操作均失败，当前线路也失败
       */
      failed.push_back(&thread - threads_.data());
    }

    /**
     * 清除所有失败和已接受的分析线路
     * 若所有线路均失败，则抛出异常
     */
    ClearOrCrash(failed, accepted);
  }

  /**
   * 获取接受结果
   * 若有多个接受结果，则抛出异常
   */
  return AcceptOrAmbiguous();
}

bool Parser::IgnoreOrFalse(Thread& thread) {
  auto syntax = root_->syntax;
  auto input = thread.inputs.front();

  if (!syntax->IsIgnored(input->id)) return false;

  if (auto term = input->AsTerm()) thread.ignores.push_back(term);
  thread.inputs.erase(thread.inputs.begin());
  return true;
}

bool Parser::ReduceOrFalse(Thread& thread) {
  auto syntax = root_->syntax;
  auto input = thread.inputs.front();
  auto state = &syntax->states.at(thread.stack.back());
  auto reduce = state->reduce.find(input->id);

  if (reduce == state->reduce.end()) return false;

  auto const formula = reduce->second;

  /**
   * 触发0号产生式，接受解析结果
   */
  if (formula == 0) {
    auto accept = thread.seens.back();
    candidates_.push_back({accept->AsNtrm(), thread.ignores});
    return true;
  }

  auto const& f = root_->syntax->formulas.at(formula);
  auto cost = f.body.size();
  auto begin = thread.seens.begin() + thread.seens.size() - cost;
  auto end = thread.seens.end();
  auto ntrm = root_->Ntrm(formula, begin, end);

  /**
   * 将被忽略的符号填回句子单词串
   */
  if (!ntrm->sentence.empty()) {
    auto from = ntrm->First()->offset;
    auto to = ntrm->Last()->offset;
    auto itx = 0UL;
    auto igx = 0UL;
    while (igx < thread.ignores.size()) {
      auto ignored = thread.ignores.at(igx);
      if (ignored->offset >= to) break;
      if (ignored->offset <= from) {
        ++igx;
        continue;
      }

      while (itx < ntrm->sentence.size()) {
        auto symbol = ntrm->sentence.at(itx);
        if (ignored->offset < symbol->First()->offset) break;
        ++itx;
      }

      ntrm->sentence.insert(ntrm->sentence.begin() + itx, ignored);
      thread.ignores.erase(thread.ignores.begin() + igx);
    }
  }

  /**
   * 依据产生式长度，从栈中弹出相应数量的状态和符号
   */
  thread.stack.resize(thread.stack.size() - cost);
  thread.seens.resize(thread.seens.size() - cost);

  /**
   * 将归约结果用作下一个输入
   */
  thread.inputs.insert(thread.inputs.begin(), ntrm);
  return true;
}

bool Parser::ShiftOrFalse(Thread& thread) {
  auto syntax = root_->syntax;
  auto input = thread.inputs.front();
  auto state = &syntax->states.at(thread.stack.back());
  auto shift = state->shift.find(input->id);

  if (shift == state->shift.end()) return false;

  thread.stack.push_back(shift->second);
  thread.seens.push_back(input);
  thread.inputs.erase(thread.inputs.begin());
  return true;
}

void Parser::ClearOrCrash(std::vector<size_t> const& failed,
                          std::vector<size_t> const& accepted) {
  Clean(accepted);
  if (threads_.empty()) return;

  if (failed.size() == threads_.size()) {
    Crash();
  } else {
    Clean(failed);
  }
}

ASTRoot Parser::AcceptOrAmbiguous() {
  if (candidates_.size() != 1) {
    fmt::println(stderr, "error: Ambiguous results");
    for (auto const& [candidate, _] : candidates_) {
      fmt::println(stderr, "\033[1;34mCandate:\033[0m");
      fmt::println(stderr, "  {}", candidate->Store({}).dump(2));
    }

    throw ParseError{};
  }

  auto syntax = root_->syntax;
  auto const& [candidate, ignores] = candidates_.back();
  root_->id = syntax->formulas.front().head;
  root_->formula = 0;
  root_->attributes[syntax->Lang()].push_back(candidate);

  /**
   * 将源码首末被忽略的单词填回句子
   */
  auto from = candidate->First()->offset;
  auto feeded = false;
  for (auto ignored : ignores) {
    if (ignored->offset > from) {
      if (!feeded) {
        root_->sentence.push_back(candidate);
        feeded = true;
      }
      root_->sentence.push_back(ignored);
    } else {
      root_->sentence.push_back(ignored);
    }
  }
  if (!feeded) {
    root_->sentence.push_back(candidate);
  }

  return root_;
}

void Parser::Crash() {
  if (threads_.size() != 1) {
    fmt::println(stderr, "error: Parse error on multiple routes");
  }

  for (auto const& thread : threads_) {
    auto const& input = thread.inputs.front();

    fmt::println(stderr,
                 "\033[1;31merror:\033[0m {}: Unexpected symbol {} ({})",
                 input->Location(), input->Name(), input->Text());
    if (!thread.seens.empty()) {
      auto const& last = thread.seens.back();
      fmt::println(stderr, "note: last symbol was {} ({})", last->Name(),
                   last->Text());
    }
    std::vector<std::string> expected;
    auto const& state = root_->syntax->states.at(thread.stack.back());
    for (auto const& reduce : state.reduce)
      expected.push_back(root_->syntax->NameOf(reduce.first));
    for (auto const& shift : state.shift)
      expected.push_back(root_->syntax->NameOf(shift.first));
    fmt::println(stderr, "note: expected one of {}", fmt::join(expected, ", "));
  }
  throw ParseError{};
}

void Parser::Clean(std::vector<size_t> const& according) {
  for (auto it = according.rbegin(); it != according.rend(); ++it) {
    threads_.erase(threads_.begin() + *it);
  }
}

void Parser::ScanAndFork() {
  auto syntax = root_->syntax;
  auto lex = syntax->lex;

  for (auto i = 0UL; i < threads_.size(); i++) {
    if (!threads_[i].inputs.empty()) continue;

    auto& state = syntax->states.at(threads_[i].stack.back());
    if (state.contexts.empty() || state.contexts.size() == 1) {
      auto& thread = threads_[i];
      auto context = state.contexts.empty() ? 0 : *state.contexts.begin();
      auto term = Scan(thread, context);
      thread.inputs.push_back(term);
      continue;
    }

    auto const offset = threads_[i].offset;

    std::set<std::tuple<SymbolID, size_t, size_t>> seen;  // id, offset, length
    for (auto const& context : state.contexts) {
      if (seen.empty()) {
        auto term = Scan(threads_[i], context);
        threads_[i].inputs.push_back(term);
        seen.insert({term->id, term->offset, term->length});
        continue;
      }

      Thread th{.offset = offset};
      auto term = Scan(th, context);
      auto tuple = std::make_tuple(term->id, term->offset, term->length);
      if (!seen.insert(tuple).second) continue;

      threads_.push_back(Thread{
          .offset = th.offset,
          .stack = threads_[i].stack,
          .seens = threads_[i].seens,
          .inputs = {term},
      });
    }
  }
}

ASTTerm Parser::Scan(Thread& thread, ContextID context) {
  auto doc = root_->doc;
  auto syntax = root_->syntax;
  auto lex = syntax->lex;

  auto term = root_->Term(Lexicon::kEOF, thread.offset);
  if (thread.offset >= doc->content.size()) return term;

  auto start = lex->states.front();
  auto state = start.transitions.at(context);

  while (state != 0 && thread.offset <= doc->content.size()) {
    auto const ch = doc->content[thread.offset];
    auto const& st = lex->states.at(state);
    auto next = st.transitions.find(ch);
    if (next == st.transitions.end()) {
      state = 0;
      if (st.accepts) {
        term->id = *st.accepts;
        break;
      }
    } else {
      state = next->second;
    }

    term->id = Lexicon::kERR;
    term->length++;
    thread.offset++;
  }

  /**
   * 为终结符指定初始属性
   */
  if (term->id != Lexicon::kEOF && term->id != Lexicon::kERR) {
    term->attributes = lex->terms.at(term->id).attributes;
  }

  return term;
}

}  // namespace alioth