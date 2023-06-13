#include <filesystem>
#include <fstream>

#include "alioth/alioth.h"
#include "alioth/grammar.h"
#include "alioth/inspect.h"
#include "alioth/lexicon.h"
#include "alioth/parser.h"
#include "alioth/regex.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Storage, Json) {
  auto path = AliothHome() / "grammar" / "grammar.grammar";
  auto gdoc = Document::Read(path);
  auto syntax = Grammar::SyntaxOf();

  {
    auto parser = Parser(syntax, gdoc);
    auto root = parser.Parse();
  }

  auto json = StoreSyntax(syntax);
  auto loaded = LoadSyntax(json);
  auto parser = Parser(loaded, gdoc);
  auto again = parser.Parse();
}

}  // namespace test
}  // namespace alioth