#include "alioth/syntax/syntax.h"

#include "alioth/lex/lex.h"
#include "spdlog/fmt/fmt.h"

namespace alioth {
std::string Syntax::GetSymbolName(int id) const {
  if (id <= lex_->CountTokens()) return lex_->GetTokenName(id);

  return ntrms_.at(id - 1 - lex_->CountTokens());
}
std::string Syntax::GetAttrName(int id) const { return attributes_.at(id); }

int Syntax::GetAcceptSymbol() const { return lex_->CountTokens() + 1 + 1; }

LexCRef Syntax::GetLex() const { return lex_; }

syntax::State const& Syntax::GetState(int id) const { return states_.at(id); }
syntax::Reduce const& Syntax::GetReduce(int id) const { return reduces_.at(id); }

bool Syntax::IsIgnored(int id) const { return ignores_.count(id); }

int Syntax::GetOrAddAttr(std::string const& name) {
  for (auto i = 0UL; i < attributes_.size(); ++i) {
    if (attributes_[i] == name) return i;
  }

  attributes_.push_back(name);
  return attributes_.size() - 1;
}

nlohmann::json Syntax::Save() const {
  nlohmann::json json;

  json["lex"] = lex_->Save();
  json["ntrms"] = ntrms_;
  json["attributes"] = attributes_;

  nlohmann::json states;
  for (auto const& state : states_) {
    states.push_back({{"shift", state.shift}, {"reduce", state.reduce}});
  }
  json["states"] = states;

  nlohmann::json reduces;
  for (auto const& reduce : reduces_) {
    reduces.push_back({{"ntrm", reduce.ntrm},
                       {"length", reduce.length},
                       {"attrs", reduce.attrs},
                       {"unfolds", reduce.unfolds}});
  }
  json["reduces"] = reduces;

  json["ignores"] = ignores_;

  return json;
}

std::shared_ptr<Syntax> Syntax::Load(nlohmann::json const& json) {
  auto syntax = std::shared_ptr<Syntax>(new Syntax{});

  syntax->lex_ = Lex::Load(json["lex"]);
  syntax->ntrms_ = json["ntrms"].get<std::vector<std::string>>();
  syntax->attributes_ = json["attributes"].get<std::vector<std::string>>();

  for (auto const& state : json["states"]) {
    syntax->states_.push_back(
        {.shift = state["shift"], .reduce = state["reduce"]});
  }

  for (auto const& reduce : json["reduces"]) {
    syntax->reduces_.push_back({.ntrm = reduce["ntrm"],
                              .length = reduce["length"],
                              .attrs = reduce["attrs"],
                              .unfolds = reduce["unfolds"]});
  }

  syntax->ignores_ = json["ignores"].get<std::set<int>>();

  return syntax;
}

}  // namespace alioth