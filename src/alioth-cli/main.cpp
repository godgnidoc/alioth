#include <fstream>
#include <iostream>

#include "alioth/alioth.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/parser.h"
#include "alioth/skeleton.h"
#include "alioth/strings.h"
#include "alioth/template.h"
#include "alioth-cli/compile.h"
#include "alioth-cli/framework.h"
#include "alioth-cli/render.h"
#include "alioth-cli/skeleton.h"
#include "alioth-cli/syntax.h"
#include "alioth-cli/tokenize.h"
#include "cli/cli.h"

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