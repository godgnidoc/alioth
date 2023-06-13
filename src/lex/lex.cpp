#include "alioth/lex/lex.h"

#include <stdexcept>

#include "alioth/lex/builder.h"
#include "alioth/lex/scanner.h"
#include "alioth/logging.h"
#include "alioth/utils/clan.h"
#include "alioth/utils/hex.h"

namespace alioth {

std::string Lex::GetContextName(int id) const { return contexts_.at(id); }
int Lex::GetContextId(std::string const& name) const {
  for (size_t id = 0; id < contexts_.size(); id++) {
    if (contexts_.at(id) == name) return id;
  }
  throw std::runtime_error("context not found");
}

std::vector<std::string> Lex::GetContexts() const { return contexts_; }

std::string Lex::GetTokenName(int token) const {
  if (token == 0)
    return "<EOF>";
  else if (token < 0)
    return "<ERR>";
  else
    return tokens_.at(token - 1);
}

int Lex::GetTokenId(std::string const& name) const {
  for (size_t off = 0; off < tokens_.size(); off++) {
    if (tokens_.at(off) == name) return off + 1;
  }

  return Token::kError;
}

std::vector<std::string> Lex::GetTokens() const { return tokens_; }

int Lex::CountTokens() const { return tokens_.size(); }

std::shared_ptr<lex::State> Lex::GetState(int state) const {
  return states_.at(state);
}

nlohmann::json Lex::Save() const {
  nlohmann::json json;

  json["tokens"] = tokens_;
  json["contexts"] = contexts_;

  nlohmann::json states;
  for (auto const& state : states_) {
    std::map<int, Clan> state_inputs;
    for (auto const& [token, next] : state->transitions) {
      state_inputs[next].Insert(token);
    }
    std::map<std::string, int> transitiions;
    for (auto const& [next, inputs] : state_inputs) {
      transitiions[inputs.Format()] = next;
    }

    states.push_back(
        {{"accepts", state->accepts}, {"transitions", transitiions}});
  }
  json["states"] = states;

  return json;
}

std::shared_ptr<Lex> Lex::Load(nlohmann::json const& json) {
  auto lex = std::shared_ptr<Lex>(new Lex{});

  lex->tokens_ = json["tokens"].get<std::vector<std::string>>();
  lex->contexts_ = json["contexts"].get<std::vector<std::string>>();

  for (auto const& state : json["states"]) {
    auto s = std::make_shared<lex::State>();
    s->accepts = state["accepts"].get<int>();
    auto transitions = state["transitions"].get<std::map<std::string, int>>();
    for (auto const& [inputs, next] : transitions) {
      auto clan = Clan::Parse(inputs);
      for (auto const& token : clan) {
        s->transitions[token] = next;
      }
    }
    lex->states_.push_back(s);
  }

  return lex;
}

}  // namespace alioth