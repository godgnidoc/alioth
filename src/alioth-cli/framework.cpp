#include "alioth-cli/framework.h"

#include <fstream>

#include "alioth-cli/syntax.h"
#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/skeleton.h"
#include "alioth/strings.h"
#include "alioth/template.h"

int Framework::Run() {
  using namespace alioth;

  auto syntax = ::Syntax::Load(gpath->Value());

  Template::Map model;
  auto lang = NameOf(syntax);
  model["lang"] = lang;

  auto jsyntax = StoreSyntax(syntax);
  auto binary = nlohmann::json::to_cbor(jsyntax);
  jsyntax["binary"] = binary;
  model["syntax"] = Template::Value::FromJson(jsyntax);

  auto skeleton = alioth::Skeleton::Deduce(syntax);
  auto jskeleton = StoreSkeleton(skeleton);
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
    auto text = Template::Render(root / "syntax.h.tmpl", model, meta);
    auto dir = output_dir / "include" / lang;
    std::filesystem::create_directories(dir);
    std::ofstream ofs(dir / "syntax.h");
    ofs << text;
  }

  {
    auto text = Template::Render(root / "syntax.cpp.tmpl", model, meta);
    auto dir = output_dir / "src";
    std::filesystem::create_directories(dir);
    std::ofstream ofs(dir / "syntax.cpp");
    ofs << text;
  }

  return 0;
}