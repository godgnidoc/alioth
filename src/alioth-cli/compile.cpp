#include "alioth-cli/compile.h"

#include "alioth-cli/syntax.h"
#include "alioth/document.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"

int Compile::Run() {
  alioth::Doc gdoc;
  auto syntax = ::Syntax::Load(gpath->Value());

  alioth::Doc sdoc;
  if (source->Value() == "-") {
    sdoc = alioth::Document::Read();
  } else {
    sdoc = alioth::Document::Read(source->Value());
  }

  auto parser = alioth::Parser(syntax, sdoc);
  auto root = parser.Parse();

  if (!disable_skeleton_hint->HasValue()) {
    auto skeleton = alioth::Skeleton::Deduce(syntax);
    fmt::println("{}", alioth::AttrsOf(root, &skeleton).dump(2));
  } else {
    fmt::println("{}", alioth::AttrsOf(root).dump(2));
  }

  return 0;
}