#include "alioth/lexicon.h"

namespace alioth {

Lexicon::Builder::Builder(std::string const& lang) {
  lex_ = std::shared_ptr<Lexicon>(new Lexicon{});
  lex_->contexts.push_back(lang);
  lex_->terms.push_back({"<EOF>"});
}

Lexicon::Builder& Lexicon::Builder::Define(
    std::string const& name, Regex pattern,
    std::set<std::string> const& context) {
  std::set<char> entries;
  for (auto const& ctx : context) {
    auto it = std::find(lex_->contexts.begin(), lex_->contexts.end(), ctx);
    if (it != lex_->contexts.end()) {
      entries.insert(std::distance(lex_->contexts.begin(), it));
    } else {
      if (lex_->contexts.size() >= 256) throw TooManyContexts{};
      lex_->contexts.push_back(ctx);
      entries.insert(lex_->contexts.size() - 1);
    }
  }

  for (auto const& term : lex_->terms) {
    if (term.name == name) throw DuplicateTerm(name);
  }

  auto id = lex_->terms.size();
  lex_->terms.push_back({name, pattern, entries});
  RegexTree::AcceptNode::On(pattern, id);

  if (regex_ == nullptr) {
    regex_ = pattern;
  } else {
    regex_ = RegexTree::UnionNode::Of(regex_, pattern);
  }

  return *this;
}

Lex Lexicon::Builder::Build() {
  regex_->CalcFollowpos();

  std::vector<StateID> pending_states;
  std::map<StateID, RegexTree::Leafs> state_pos_map;

  {
    // 起始状态
    lex_->states.push_back({});

    // 每个上下文拥有一个首位置状态，包含当前上下文能接受的全部首位置
    auto ctxend = static_cast<char>(lex_->contexts.size());
    for (char ctxid = 0; ctxid < ctxend; ctxid++) {
      // 创建首状态
      auto const stateid = lex_->states.size();
      pending_states.push_back(stateid);
      lex_->states.push_back({});

      // 对起始状态来说上下文id被用作输入
      lex_->states.front().transitions.emplace(ctxid, stateid);

      // 计算当前上下文能接受的首位置
      for (auto const& term : lex_->terms) {
        if (!term.pattern) continue;
        if (!term.entries.empty() && term.entries.count(ctxid) == 0) continue;

        auto pos = term.pattern->GetFirstpos();
        state_pos_map[stateid].insert(pos.begin(), pos.end());
      }
    }
  }

  // 处理尚未处理的状态
  while (!pending_states.empty()) {
    // 收集当前状态信息
    auto const state_id = pending_states.back();
    auto const& current_pos = state_pos_map.at(state_id);

    // 将状态标记为已处理
    pending_states.pop_back();

    // 计算当前状态接受的词法记号
    for (auto const& posit : current_pos) {
      if (auto accept = std::dynamic_pointer_cast<RegexTree::AcceptNode>(posit);
          accept != nullptr) {
        auto& state = lex_->states.at(state_id);
        // 词法记号ID越小，优先级越高
        if (!state.accepts || accept->term_ < state.accepts) {
          state.accepts = accept->term_;
        }
      }
    }

    // 计算当前状态的出度转移
    for (auto c = 1; c <= 255; c++) {
      auto const ch = static_cast<char>(c);
      RegexTree::Leafs followpos;

      // 收集当前输入字符能到达的所有位置
      for (auto it : current_pos) {
        if (!it->Match(ch)) continue;

        followpos.insert(it->followpos_.begin(), it->followpos_.end());
      }

      // 若当前输入字符不能到达任何位置，则跳过
      if (followpos.empty()) continue;

      // 计算当前输入字符能到达的状态
      StateID next_state_id = 0;
      for (auto const& [candidate_state_id, candidate_state_pos] :
           state_pos_map) {
        if (candidate_state_pos != followpos) continue;

        next_state_id = candidate_state_id;
        break;
      }

      // 若当前输入字符能到达的状态尚未创建，则创建之
      if (next_state_id == 0) {
        next_state_id = lex_->states.size();
        pending_states.push_back(next_state_id);
        lex_->states.push_back({});
        state_pos_map.emplace(next_state_id, followpos);
      }

      // 添加转移
      lex_->states.at(state_id).transitions.emplace(ch, next_state_id);
    }
  }

  return lex_;
}

}  // namespace alioth