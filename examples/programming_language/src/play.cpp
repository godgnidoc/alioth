
#include "alioth/inspect.h"
#include "cli/cli.h"
#include "play/syntax.h"

struct Compile : public cli::Command {
  int Run() override {
    auto doc = alioth::Document::Read(source->Value(), "-");

    auto play = ::play::ParsePlay(doc);

    for (auto stmt : play->stmts) {
      fmt::println("statement: {}", alioth::NameOf(stmt));
    }

    auto stmt = play->stmts.front().As<play::StmtNode>();
    // fmt::println("{}", alioth::StoreNode(play).dump(2));
    auto fn = stmt->function.As<play::FunctionNode>();
    fmt::println("function name: {}", alioth::TextOf(fn->name));

    return 0;
  }

  cli::Arg source = Named("source");
};

int main(int argc, char** argv) {
  cli::Application::Name("Play Compiler");
  cli::Application::Version("0.1");
  cli::Application::Command(std::make_shared<Compile>());
  return cli::Application::Run(argc, argv);
}