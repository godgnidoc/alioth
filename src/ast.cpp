#include "alioth/ast.h"

#include <stdexcept>

#include "alioth/lex/lex.h"
#include "alioth/syntax/syntax.h"
#include "spdlog/fmt/fmt.h"

namespace alioth {

namespace ast {

std::string Node::GetName() const {
  return GetRoot()->syntax->GetSymbolName(id);
}
std::shared_ptr<Root> Node::GetRoot() { return parent.lock()->GetRoot(); }
std::shared_ptr<Root const> Node::GetRoot() const {
  return parent.lock()->GetRoot();
}

std::string Term::GetText() const {
  return GetRoot()->source->content.substr(offset, length);
}

nlohmann::json Term::DumpRaw() const {
  nlohmann::json json;
  json["id"] = id;
  json["start_line"] = start_line;
  json["start_column"] = start_column;
  json["end_line"] = end_line;
  json["end_column"] = end_column;
  json["offset"] = offset;
  json["length"] = length;
  json["text"] = GetText();
  json["name"] = GetName();
  return json;
}

nlohmann::json Term::DumpAbs() const { return GetText(); }

std::string Term::GetLocation() const {
  return fmt::format("{}:{}:{}", GetRoot()->source->path, start_line,
                     start_column);
}

std::string Ntrm::GetText() const {
  auto _first = sentence.front();
  while (auto ntrm = std::dynamic_pointer_cast<Ntrm>(_first)) {
    _first = ntrm->sentence.front();
  }

  auto _last = sentence.back();
  while (auto ntrm = std::dynamic_pointer_cast<Ntrm>(_last)) {
    _last = ntrm->sentence.back();
  }

  auto first = std::dynamic_pointer_cast<Term>(_first);
  auto last = std::dynamic_pointer_cast<Term>(_last);
  return GetRoot()->source->content.substr(
      first->offset, last->offset + last->length - first->offset);
}

nlohmann::json Ntrm::DumpRaw() const {
  nlohmann::json json;
  json["id"] = id;
  json["name"] = GetName();

  auto sentence = nlohmann::json::array();
  for (auto const& node : this->sentence) {
    sentence.push_back(node->DumpRaw());
  }
  json["sentence"] = sentence;

  return json;
}

nlohmann::json Ntrm::DumpAbs() const {
  nlohmann::json json;
  json["$"] = GetName();
  for (auto const& [name, nodes] : attributes) {
    if (nodes.size() == 1) {
      json[name] = nodes.front()->DumpAbs();
      continue;
    }

    nlohmann::json attr;
    for (auto const& node : nodes) {
      attr.push_back(node->DumpAbs());
    }
    json[name] = attr;
  }
  return json;
}

std::string Ntrm::GetLocation() const {
  auto _first = sentence.front();
  while (auto ntrm = std::dynamic_pointer_cast<Ntrm>(_first)) {
    _first = ntrm->sentence.front();
  }
  auto first = std::dynamic_pointer_cast<Term>(_first);
  return first->GetLocation();
}

std::shared_ptr<Root> Root::GetRoot() { return shared_from_this(); }
std::shared_ptr<Root const> Root::GetRoot() const { return shared_from_this(); }

std::shared_ptr<Term> Root::MakeTerm(Token const& token) {
  if (token.source != GetRoot()->source) {
    throw std::runtime_error("token does not belong to the source");
  }

  auto term = std::make_shared<Term>();
  term->parent = shared_from_this();
  term->id = token.id;
  term->start_line = token.start_line;
  term->start_column = token.start_column;
  term->end_line = token.end_line;
  term->end_column = token.end_column;
  term->offset = token.offset;
  term->length = token.length;
  return term;
}

}  // namespace ast

}  // namespace alioth