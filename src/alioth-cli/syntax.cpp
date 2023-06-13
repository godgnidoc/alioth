#include "alioth-cli/syntax.h"

#include <iostream>

#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/skeleton.h"

int Syntax::Run() {
  auto syntax = ::Syntax::Load(gpath->Value());

  auto json = alioth::StoreSyntax(syntax);
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
    auto syntax = alioth::LoadSyntax(json);
    return syntax;
  } catch (nlohmann::json::parse_error const&) {
    auto grammar = alioth::Grammar::Load(doc);
    auto syntax = grammar.Compile();
    return syntax;
  }
}