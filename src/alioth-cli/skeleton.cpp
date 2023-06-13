#include "alioth-cli/skeleton.h"

#include "alioth-cli/syntax.h"
#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/parser.h"
#include "alioth/strings.h"
#include "aliox/grammar.h"
#include "aliox/skeleton.h"
#include "aliox/template.h"

int Skeleton::Run() {
  auto syntax = ::Syntax::Load(gpath->Value());

  auto skeleton = alioth::Skeleton::Deduce(syntax);
  fmt::println("{}", skeleton.Store().dump(2));

  return 0;
}