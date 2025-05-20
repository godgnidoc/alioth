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

using namespace alioth;

inline auto json() {
  return [](auto&, auto const& args) { return args[0].ToJson().dump(); };
}

inline auto nameOf(alioth::Syntax syntax) {
  return [syntax](auto&, auto const& args) {
    return syntax->NameOf(args[0].GetInteger());
  };
}

inline auto onlyOne() {
  return [](auto&, auto const& args) {
    auto const& arg = args[0];
    if (!arg.IsArray() || arg.GetArray().size() != 1)
      return Template::Value{nullptr};
    return arg.GetArray().front();
  };
}

inline auto isNtrm(alioth::Syntax syntax) {
  return [syntax](auto&, auto const& args) -> Template::Value {
    auto const& node = args[0];
    if (node.IsString()) {
      return !syntax->IsTerm(syntax->FindSymbol(node.GetString()));
    } else if (node.IsInteger()) {
      return !syntax->IsTerm(node.GetInteger());
    }
    return false;
  };
}

inline auto isImported(alioth::Syntax syntax) {
  return [syntax](auto&, auto const& args) -> Template::Value {
    auto const& node = args[0];
    if (node.IsString()) {
      return syntax->IsImported(syntax->FindSymbol(node.GetString()));
    } else if (node.IsInteger()) {
      return syntax->IsImported(node.GetInteger());
    }
    return false;
  };
}

int Framework::Run() {
  auto syntax = ::Syntax::Load(gpath->Value());

  Template::Map model;
  auto lang = syntax->Lang();
  model["lang"] = lang;
  auto jsyntax = syntax->Store();
  model["syntax"] = Template::Value::FromJson(jsyntax[0]);
  auto jsyntaxs = nlohmann::json::object();
  for (auto const& syntax : jsyntax) {
    jsyntaxs[syntax["lang"]] = syntax;
  }

  auto skeleton = alioth::Skeleton::Deduce(syntax);
  auto jskeleton = skeleton.Store();
  jskeleton.erase("S'");
  model["skeleton"] = Template::Value::FromJson(jskeleton);

  Template::Map meta;
  meta["uppercase"] = Template::Pipe(&Strings::Uppercase);
  meta["lowercase"] = Template::Pipe(&Strings::Lowercase);
  meta["camelcase"] = Template::Pipe(&Strings::Camelcase);
  meta["titlecase"] = Template::Pipe(&Strings::Titlecase);
  meta["json"] = Template::Filter{json()};
  meta["nameOf"] = Template::Filter{nameOf(syntax)};
  meta["onlyOne"] = Template::Filter{onlyOne()};
  meta["isNtrm"] = Template::Filter{isNtrm(syntax)};
  meta["isImported"] = Template::Filter{isImported(syntax)};

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

  return 0;
}