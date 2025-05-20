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
#include "grammar/syntax.h"

namespace alioth {
namespace test {

TEST(Grammar, Bootstrap) {
  auto path = AliothHome() / "grammar" / "grammar.grammar";

  auto gdoc = Document::Read(path);
  auto gs = Grammar::Compile(gdoc);
  auto native = SyntaxOf<grammar::Grammar>();
  EXPECT_EQ(gs->Store(), native->Store());
}

TEST(Grammar, Form) {
  auto gdoc = Document::Create(R"(
    lang: formed

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

  auto syntax = Grammar::Compile(gdoc);
  ASSERT_EQ(syntax->formulas.at(3).form, "uninited");
  ASSERT_EQ(syntax->formulas.at(4).form, "assigned");
}

}  // namespace test
}  // namespace alioth