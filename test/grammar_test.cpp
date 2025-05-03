#include "aliox/grammar.h"

#include <filesystem>
#include <fstream>

#include "alioth/alioth.h"
#include "alioth/lexicon.h"
#include "alioth/parser.h"
#include "alioth/regex.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Grammar, Bootstrap) {
  auto path = AliothHome() / "grammar" / "grammar.grammar";

  auto gdoc = Document::Read(path);
  fmt::println("step 1: load grammar from source");
  auto grammar = Grammar::Parse(gdoc);
  fmt::println("step 2: compile grammar to syntax");
  auto syntax = grammar.Compile();
  fmt::println("step 3: use syntax to parse grammar");
  auto parser = Parser(syntax, gdoc);
  auto ast = parser.Parse();
  fmt::println("step 4: load grammar from ast");
  auto grammar2 = Grammar::Parse(ast);
  fmt::println("step 5: compile grammar to syntax");
  auto syntax2 = grammar2.Compile();
  fmt::println("step 6: use syntax to parse grammar");
  auto parser2 = Parser(syntax2, gdoc);
  auto ast2 = parser2.Parse();
  EXPECT_EQ(ast->Store({}), ast2->Store({}));
}

TEST(Grammar, Form) {
  auto gdoc = Document::Create(R"(
    lang: "formed"

    ASSIGN = /=/
    INT = /int/
    NUM = /\d+/
    SEMI = /;/
    ID = /[a-zA-Z_]\w*/
    SPACE = /\s+/

    formed -> ...formed? stmt;
    stmt.uninited -> INT ID@name SEMI;
    stmt.assigned -> INT ID@name ASSIGN NUM@value SEMI;
  )");

  auto source = Document::Create(R"(
    int a;
    int b = 1;
  )");

  auto syntax = Grammar::Parse(gdoc).Compile();
  ASSERT_EQ(syntax->formulas.at(3).form, "uninited");
  ASSERT_EQ(syntax->formulas.at(4).form, "assigned");
}

}  // namespace test
}  // namespace alioth