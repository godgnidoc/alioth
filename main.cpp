#include <fstream>
#include <iostream>

#include "alioth/alioth.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/skeleton.h"
#include "alioth/strings.h"
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

struct Skeleton : public cli::Command {
  int Run() override {
    alioth::Doc gdoc;
    if (gpath->Value() == "-") {
      gdoc = alioth::Document::Read();
    } else {
      gdoc = alioth::Document::Read(gpath->Value());
    }

    auto grammar = alioth::Grammar::Load(gdoc);
    auto syntax = grammar.Compile();
    auto skeleton = alioth::Skeleton::Deduce(syntax);
    fmt::println("{}", alioth::StoreSkeleton(skeleton).dump(2));

    return 0;
  }

  cli::Arg gpath = Named("grammar-path");
};

struct Framework : public cli::Command {
  int Run() override {
    using namespace alioth;

    alioth::Doc gdoc;
    if (gpath->Value() == "-") {
      gdoc = Document::Read();
    } else {
      gdoc = Document::Read(gpath->Value());
    }

    auto grammar = Grammar::Load(gdoc);
    auto syntax = grammar.Compile();
    auto skeleton = alioth::Skeleton::Deduce(syntax);

    Template::Map meta;
    Template::Map model;

    meta["uppercase"] = Template::Pipe(&Strings::Uppercase);
    meta["lowercase"] = Template::Pipe(&Strings::Lowercase);
    meta["camelcase"] = Template::Pipe(&Strings::Camelcase);
    meta["titlecase"] = Template::Pipe(&Strings::Titlecase);
    model["skeleton"] = Template::Value::FromJson(StoreSkeleton(skeleton));
    model["syntax"] = Template::Value::FromJson(StoreSyntax(syntax));
    model["lang"] = NameOf(syntax);

    auto home = AliothHome();
    auto root = home / "templates" / "skeleton" / "cpp";
    auto output_dir = std::filesystem::path(opath->Value());
    std::filesystem::create_directories(output_dir);

    auto text = Template::Render(root / "syntax.h.tmpl", model, meta);

    std::ofstream ofs(output_dir / "include" / "syntax.h");
    ofs << text;

    return 0;
  }

  cli::Arg gpath = Named("grammar-path");
  cli::Opt opath = Option({"-o", "--output"})
                       ->Required()
                       ->Argument("output-dir")
                       ->Brief("output directory");
};

struct Tokenize : public cli::Command {
  int Run() override {
    alioth::Doc doc;
    if (path->Value() == "-") {
      doc = alioth::Document::Read();
    } else {
      doc = alioth::Document::Read(path->Value());
    }

    auto gdoc = alioth::Document::Read(gpath->Value());
    auto grammar = alioth::Grammar::Load(gdoc);
    auto syntax = grammar.Compile();

    auto parser = alioth::Parser(syntax, doc);
    auto root = parser.Parse();
    nlohmann::json tokens;
    for (auto const& token : alioth::Tokenize(root)) {
      nlohmann::json t;
      t["id"] = token->id;
      t["name"] = alioth::NameOf(token);
      // t["text"] = alioth::TextOf(token);
      t["range"] = alioth::StoreRange(alioth::RangeOf(token));
      tokens.push_back(t);
      continue;
    }

    if (isatty(fileno(stdout))) {
      fmt::println("{}", tokens.dump(2));
    } else {
      fmt::print("{}", tokens.dump());
    }

    return 0;
  }

  cli::Arg path = Named("source-path");
  cli::Opt gpath = Option({"-g", "--grammar"})
                       ->Required()
                       ->Argument("grammar-path")
                       ->Brief("grammar file path")
                       ->Doc("specify the path to the grammar file");
};

int main(int argc, char** argv) {
  cli::Application::Command(std::make_shared<Syntax>(), {"syntax"});
  cli::Application::Command(std::make_shared<Compile>(), {"compile"});
  cli::Application::Command(std::make_shared<Render>(), {"render"});
  cli::Application::Command(std::make_shared<Skeleton>(), {"skeleton"});
  cli::Application::Command(std::make_shared<Framework>(), {"framework"});
  cli::Application::Command(std::make_shared<Tokenize>(), {"tokenize"});
  cli::Application::Name("alioth");
  cli::Application::Brief("compiler utils");
  cli::Application::Version("0.0.0");
  cli::Application::Author("GodGnidoc");
  return cli::Application::Run(argc, argv);
}