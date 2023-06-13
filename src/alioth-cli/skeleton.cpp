#include "alioth-cli/skeleton.h"

#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/skeleton.h"
#include "alioth/strings.h"
#include "alioth/template.h"
#include "alioth-cli/syntax.h"

int Skeleton::Run() {
  auto syntax = ::Syntax::Load(gpath->Value());

  auto skeleton = alioth::Skeleton::Deduce(syntax);
  fmt::println("{}", alioth::StoreSkeleton(skeleton).dump(2));

  return 0;
}