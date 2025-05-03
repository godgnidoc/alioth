#include "alioth-cli/parse.h"

#include "alioth-cli/syntax.h"
#include "alioth/document.h"
#include "alioth/parser.h"
#include "aliox/grammar.h"
#include "aliox/skeleton.h"

int Parse::Run() {
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
  std::shared_ptr<alioth::Skeleton> skeleton;

  alioth::ASTNode::StoreOptions options{};

  if (disable_compact->HasValue()) options.compact = false;
  if (hint_with_skeleton->HasValue()) {
    skeleton =
        std::make_shared<alioth::Skeleton>(alioth::Skeleton::Deduce(syntax));
    options.skeleton = skeleton.get();
  }
  if (unfold_terminals->HasValue()) options.unfold = true;
  if (flatten_output->HasValue()) options.flatten = true;

  if (text_key->HasValue()) options.text = text_key->Value();
  if (id_key->HasValue()) options.id = id_key->Value();
  if (name_key->HasValue()) options.name = name_key->Value();
  if (range_key->HasValue()) options.range = range_key->Value();
  if (form_key->HasValue()) options.form = form_key->Value();
  if (formula_key->HasValue()) options.formula = formula_key->Value();
  if (origin_key->HasValue()) options.origin = origin_key->Value();

  fmt::println("{}", root->Store(options).dump(2));

  return 0;
}