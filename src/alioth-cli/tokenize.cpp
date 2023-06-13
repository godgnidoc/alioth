
#include "alioth-cli/tokenize.h"

#include "alioth/document.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth-cli/syntax.h"

int Tokenize::Run() {
  alioth::Doc doc;
  if (path->Value() == "-") {
    doc = alioth::Document::Read();
  } else {
    doc = alioth::Document::Read(path->Value());
  }

  auto syntax = ::Syntax::Load(gpath->Value());
  auto parser = alioth::Parser(syntax, doc);
  auto root = parser.Parse();
  nlohmann::json tokens;
  for (auto const& token : alioth::Tokenize(root)) {
    nlohmann::json t;
    t["id"] = token->id;
    t["name"] = alioth::NameOf(token);
    // t["text"] = alioth::TextOf(token);
    t["range"] = alioth::StoreRange(alioth::RangeOf(token));
    tokens.push_back(t);
    continue;
  }

  if (isatty(fileno(stdout))) {
    fmt::println("{}", tokens.dump(2));
  } else {
    fmt::print("{}", tokens.dump());
  }

  return 0;
}