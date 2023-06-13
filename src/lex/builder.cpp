#include "alioth/lex/builder.h"

#include <vector>

#include "alioth/lex/lex.h"

namespace alioth::lex {

Builder::Builder() { (void)GetOrAddContext(kDefaultContext); }

Builder& Builder::AddToken(std::string const& name, Regex const& pattern,
                           std::set<std::string> context) {
  auto ctx_ids = GetOrAddContexts(context);
  int token_id = AddToken(name);

  (void)regex::Accept(pattern, token_id);
  if (!ctx_ids.empty())
    for (auto const& it : pattern->GetFirstpos())
      firstpos_ctx_map_[it] = ctx_ids;

  if (regex_ == nullptr) {
    regex_ = pattern;
  } else {
    regex_ = regex::Union(regex_, pattern);
  }

  return *this;
}

std::shared_ptr<Lex> Builder::Build() {
  auto lex = std::shared_ptr<Lex>(new Lex());
  regex_->CalcFollowpos();

  lex->tokens_ = tokens_;
  lex->contexts_ = contexts_;

  std::vector<int> pending_states;
  std::map<int, regex::LeafNodes> state_pos_map;

  {
    // 起始状态
    auto const start = std::make_shared<lex::State>();
    lex->states_.push_back(start);

    // 首位置
    auto firstpos = regex_->GetFirstpos();

    // 每个上下文拥有一个首位置状态，包含当前上下文能接受的全部首位置
    for (size_t ctxid = 0; ctxid < contexts_.size(); ctxid++) {
      // 创建首状态
      auto const stateid = lex->states_.size();
      pending_states.push_back(stateid);
      lex->states_.push_back(std::make_shared<lex::State>());

      // 对起始状态来说上下文id被用作输入
      start->transitions.emplace(ctxid, stateid);

      // 计算当前上下文能接受的首位置
      for (auto const& posit : firstpos) {
        auto it = firstpos_ctx_map_.find(posit);
        if (it != firstpos_ctx_map_.end() && 0 == it->second.count(ctxid))
          continue;

        state_pos_map[stateid].insert(posit);
      }
    }
  }

  // 处理尚未处理的状态
  while (!pending_states.empty()) {
    // 收集当前状态信息
    const auto state_id = pending_states.back();
    auto const state = lex->states_.at(state_id);
    const auto& current_pos = state_pos_map.at(state_id);

    // 将状态标记为已处理
    pending_states.pop_back();

    // 计算当前状态接受的词法记号
    for (auto const& posit : current_pos) {
      if (auto accept = std::dynamic_pointer_cast<regex::AcceptNode>(posit);
          accept != nullptr) {
        // 词法记号ID越小，优先级越高
        if (state->accepts == 0 || accept->token_id_ < state->accepts) {
          state->accepts = accept->token_id_;
        }
      }
    }

    // 计算当前状态的出度转移
    for (auto ch = 1; ch <= 255; ch++) {
      regex::LeafNodes followpos;

      // 收集当前输入字符能到达的所有位置
      for (auto it : current_pos) {
        if (!it->Match(ch)) continue;

        followpos.insert(it->followpos_.begin(), it->followpos_.end());
      }

      // 若当前输入字符不能到达任何位置，则跳过
      if (followpos.empty()) continue;

      // 计算当前输入字符能到达的状态
      int next_state_id = 0;
      for (auto const& [candidate_state_id, candidate_state_pos] :
           state_pos_map) {
        if (candidate_state_pos != followpos) continue;

        next_state_id = candidate_state_id;
        break;
      }

      // 若当前输入字符能到达的状态尚未创建，则创建之
      if (next_state_id == 0) {
        next_state_id = lex->states_.size();
        pending_states.push_back(next_state_id);
        lex->states_.push_back(std::make_shared<lex::State>());
        state_pos_map.emplace(next_state_id, followpos);
      }

      // 添加转移
      state->transitions.emplace(ch, next_state_id);
    }
  }

  return lex;
}

int Builder::GetOrAddContext(std::string const& name) {
  for (size_t id = 0; id < contexts_.size(); id++)
    if (contexts_.at(id) == name) return id;

  contexts_.push_back(name);
  return contexts_.size() - 1;
}

std::set<int> Builder::GetOrAddContexts(std::set<std::string> const& names) {
  std::set<int> ids;
  for (auto const& name : names) {
    ids.insert(GetOrAddContext(name));
  }
  return ids;
}

int Builder::AddToken(std::string const& name) {
  for (auto& it : tokens_)
    if (it == name) throw std::runtime_error("Token already exists");
  tokens_.push_back(name);
  return tokens_.size();
}

}  // namespace alioth::lex