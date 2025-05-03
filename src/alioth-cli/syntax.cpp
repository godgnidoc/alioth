#include "alioth-cli/syntax.h"

#include <iostream>

#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/parser.h"
#include "aliox/grammar.h"
#include "aliox/skeleton.h"

int Syntax::Run() {
  auto syntax = ::Syntax::Load(gpath->Value());

  auto json = syntax->Store();
  std::cout << json.dump(2) << std::endl;
  return 0;
}

alioth::Syntax Syntax::Load(std::string const& path) {
  alioth::Doc doc;
  if (path == "-") {
    doc = alioth::Document::Read();
  } else {
    doc = alioth::Document::Read(path);
  }

  try {
    auto json = nlohmann::json::parse(doc->content);
    auto syntax = alioth::Syntactic::Load(json);
    return syntax;
  } catch (nlohmann::json::parse_error const&) {
    auto grammar = alioth::Grammar::Parse(doc);
    auto syntax = grammar.Compile();
    return syntax;
  }
}