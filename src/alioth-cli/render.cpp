#include "alioth-cli/render.h"

#include "alioth/alioth.h"
#include "alioth/document.h"
#include "alioth/parser.h"
#include "alioth/strings.h"
#include "aliox/template.h"

int Render::Run() {
  auto tdoc = alioth::Document::Read(tpath->Value());

  alioth::Doc sdoc;
  if (source->Value() == "-") {
    sdoc = alioth::Document::Read();
  } else {
    sdoc = alioth::Document::Read(source->Value());
  }

  auto json = nlohmann::json::parse(sdoc->content);
  auto value = alioth::Template::Value::FromJson(json);
  auto model = std::get<alioth::Template::Map>(value);

  alioth::Template::Map meta;
  if (!disable_builtin_pipe->HasValue()) {
    meta["uppercase"] = alioth::Template::Pipe(&alioth::Strings::Uppercase);
    meta["lowercase"] = alioth::Template::Pipe(&alioth::Strings::Lowercase);
    meta["camelcase"] = alioth::Template::Pipe(&alioth::Strings::Camelcase);
    meta["titlecase"] = alioth::Template::Pipe(&alioth::Strings::Titlecase);
  }

  auto text = alioth::Template::Render(tdoc, model, meta);
  fmt::print("{}", text);

  return 0;
}