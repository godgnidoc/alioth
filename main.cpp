#include <iostream>

#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/template.h"
#include "cli/cli.h"

struct Syntax : public cli::Command {
  int Run() override {
    alioth::Doc gdoc;
    if (gpath->Value() == "-") {
      gdoc = alioth::Document::Read();
    } else {
      gdoc = alioth::Document::Read(gpath->Value());
    }

    auto grammar = alioth::Grammar::Load(gdoc);
    auto syntax = grammar.Compile();
    auto json = alioth::StoreSyntax(syntax);
    std::cout << json.dump(2) << std::endl;
    return 0;
  }

  cli::Arg gpath = Named("grammar");
};

struct Compile : public cli::Command {
  int Run() override {
    alioth::Doc gdoc;
    if (gpath->Value() == "-") {
      gdoc = alioth::Document::Read();
    } else {
      gdoc = alioth::Document::Read(gpath->Value());
    }

    auto grammar = alioth::Grammar::Load(gdoc);
    auto syntax = grammar.Compile();

    alioth::Doc sdoc;
    if (source->Value() == "-") {
      sdoc = alioth::Document::Read();
    } else {
      sdoc = alioth::Document::Read(source->Value());
    }

    auto parser = alioth::Parser(syntax, sdoc);
    auto root = parser.Parse();
    fmt::println("{}", alioth::AttrsOf(root).dump(2));

    return 0;
  }

  cli::Opt gpath = Option({"-g", "--grammar"})
                       ->Required()
                       ->Argument("grammar-path")
                       ->Brief("grammar file path")
                       ->Doc(
                           "specify the path to the grammar file,"
                           " '-' represents stdin");
  cli::Arg source = Named("source-path");
};

struct Render : public cli::Command {
  int Run() override {
    alioth::Doc tdoc;
    if (tpath->Value() == "-") {
      tdoc = alioth::Document::Read();
    } else {
      tdoc = alioth::Document::Read(tpath->Value());
    }

    alioth::Doc sdoc;
    if (source->Value() == "-") {
      sdoc = alioth::Document::Read();
    } else {
      sdoc = alioth::Document::Read(source->Value());
    }

    auto json = nlohmann::json::parse(sdoc->content);
    auto value = alioth::Template::Value::FromJson(json);
    auto model = std::get<alioth::Template::Map>(value);

    auto text = alioth::Template::Render(tdoc, model);
    fmt::print("{}", text);

    return 0;
  }

  cli::Opt tpath = Option({"-t", "--template"})
                       ->Required()
                       ->Argument("template-path")
                       ->Brief("tempalte file path")
                       ->Doc(
                           "specify the path to the template file,"
                           " '-' represents stdin");
  cli::Arg source = Named("model-path");
};

int main(int argc, char** argv) {
  cli::Application::Command(std::make_shared<Syntax>(), {"syntax"});
  cli::Application::Command(std::make_shared<Compile>(), {"compile"});
  cli::Application::Command(std::make_shared<Render>(), {"render"});
  cli::Application::Name("alioth");
  cli::Application::Brief("compiler utils");
  cli::Application::Version("0.0.0");
  cli::Application::Author("GodGnidoc");
  return cli::Application::Run(argc, argv);
}