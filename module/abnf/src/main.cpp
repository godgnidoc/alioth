#include "abnf.h"
#include "alioth/lex/builder.h"
#include "alioth/logging.h"
#include "alioth/syntax/builder.h"
#include "alioth/syntax/parser.h"
#include "cli/cli.h"
#include "cli/help.h"
#include "cli/version.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace cli;

class Compile : public CommandDefinition<Compile> {
 public:
  Compile()
      : CommandDefinition{
            cmd::defaults(), cmd::global(),
            cmd::brief(
                "Compile the source to an syntax tree using specified syntax"),
            cmd::keyword("--compile"), cmd::args("<syntax> <source>")} {}

  Option abstract_{
      opt::brief("output abstract syntax tree instead of raw syntax tree"),
      opt::keyword("--abstract"), opt::keyword("-a"), opt::bind(this)};

  alioth::Logger logger_{"compile"};

  int Execute(std::list<std::string> const& args) {
    if (args.size() != 2) {
      logger_->error("missing required arguments");
      return 1;
    }

    auto syntax_source = alioth::Source::Load(args.front());
    auto syntax = ABNF::Compile(syntax_source);
    if (syntax == nullptr) {
      logger_->error("Failed to compile syntax");
      return 1;
    }

    auto source = alioth::Source::Load(args.back());
    auto ast = alioth::syntax::Parser::Parse(syntax, source);

    if (ast == nullptr) {
      logger_->error("Failed to parse source");
      return 1;
    }

    if (abstract_) {
      fmt::print("{}\n", ast->DumpAbs().dump(2));
    } else {
      fmt::print("{}\n", ast->DumpRaw().dump(2));
    }

    return 0;
  }
};

int main(int argc, char** argv) {
  alioth::InitLogging(
      {std::make_shared<spdlog::sinks::stderr_color_sink_st>()});
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("%^%l%$ (%n) %v");

  App::SetBrief("ABNF syntax compiler");
  App::SetVersion(PROJECT_VERSION);

  DefaultHelp help{cmd::global(), cmd::keyword("--help"), cmd::keyword("-h")};
  DefaultVersion version{cmd::global(), cmd::keyword("--version"),
                         cmd::keyword("-v")};

  Option logging{
      opt::global(), opt::brief("set logging level"), opt::keyword("--logging"),
      opt::arg({"trace", "debug", "info", "warn", "error", "critical"})};

  try {
    return App::Execute(argc, argv, [&logging] {
      if (!logging) return 0;

      if (*logging == "trace") {
        spdlog::set_level(spdlog::level::trace);
      } else if (*logging == "debug") {
        spdlog::set_level(spdlog::level::debug);
      } else if (*logging == "info") {
        spdlog::set_level(spdlog::level::info);
      } else if (*logging == "warn") {
        spdlog::set_level(spdlog::level::warn);
      } else if (*logging == "error") {
        spdlog::set_level(spdlog::level::err);
      } else if (*logging == "critical") {
        spdlog::set_level(spdlog::level::critical);
      }

      return 0;
    });
  } catch (std::exception const& e) {
    alioth::PrintErrors(e);
    return 1;
  }
}