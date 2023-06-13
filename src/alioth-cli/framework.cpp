#include "alioth-cli/framework.h"

#include <fstream>

#include "alioth-cli/syntax.h"
#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/parser.h"
#include "alioth/strings.h"
#include "aliox/grammar.h"
#include "aliox/skeleton.h"
#include "aliox/template.h"

int Framework::Run() {
  using namespace alioth;

  auto syntax = ::Syntax::Load(gpath->Value());

  Template::Map model;
  auto lang = syntax->Lang();
  model["lang"] = lang;

  auto jsyntax = syntax->Store();
  auto binary = nlohmann::json::to_cbor(jsyntax);
  jsyntax["binary"] = binary;
  model["syntax"] = Template::Value::FromJson(jsyntax);

  auto skeleton = alioth::Skeleton::Deduce(syntax);
  auto jskeleton = skeleton.Store();
  jskeleton.erase("S'");
  model["skeleton"] = Template::Value::FromJson(jskeleton);

  Template::Map meta;
  meta["uppercase"] = Template::Pipe(&Strings::Uppercase);
  meta["lowercase"] = Template::Pipe(&Strings::Lowercase);
  meta["camelcase"] = Template::Pipe(&Strings::Camelcase);
  meta["titlecase"] = Template::Pipe(&Strings::Titlecase);
  meta["eq"] = Template::Filter{[](auto&, auto const& args) {
    return Template::Value{args[0].TextOf() == args[1].TextOf()};
  }};
  meta["model"] =
      Template::Filter{[](auto& ctx, auto const&) { return ctx.Model(); }};

  auto home = AliothHome();
  auto root = home / "templates" / "skeleton" / "cpp";
  auto output_dir = std::filesystem::path(opath->Value());

  {
    auto text = Template::Render(root / "syntax.h.template", model, meta);
    auto dir = output_dir / "include" / lang;
    std::filesystem::create_directories(dir);
    std::ofstream ofs(dir / "syntax.h");
    ofs << text;
  }

  {
    auto text = Template::Render(root / "syntax.cpp.template", model, meta);
    auto dir = output_dir / "src";
    std::filesystem::create_directories(dir);
    std::ofstream ofs(dir / "syntax.cpp");
    ofs << text;
  }

  return 0;
}