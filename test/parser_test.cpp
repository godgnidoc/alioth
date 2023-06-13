#include "alioth/parser.h"

#include "alioth/regex.h"
#include "alioth/strings.h"
#include "alioth/syntax.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Parser, Simple) {
  auto lex = Lexicon::Builder("prog")
                 .Define("INT", "int"_regex)
                 .Define("EQ", "="_regex)
                 .Define("ID", "[a-zA-Z_][a-zA-Z0-9_]*"_regex)
                 .Define("NUM", "-?\\d+(\\.\\d+)?([eE]-?\\d+)?"_regex)
                 .Define("SPACE", "\\s+"_regex)
                 .Define("SEMI", ";"_regex)
                 .Build();
  auto syntax = Syntactic::Builder(lex)
                    .Ignore("SPACE")
                    .Formula("prog")
                    .Commit()
                    .Formula("prog")
                    .Symbol("prog", "...")
                    .Symbol("stmt", "stmts")
                    .Commit()
                    .Formula("stmt")
                    .Symbol("INT", "type")
                    .Symbol("ID", "name")
                    .Symbol("SEMI")
                    .Commit()
                    .Formula("stmt")
                    .Symbol("INT", "type")
                    .Symbol("ID", "name")
                    .Symbol("EQ")
                    .Symbol("NUM", "value")
                    .Symbol("SEMI")
                    .Commit()
                    .Build();
  auto source = R"(
        int x;
        int y = 1;
  )";
  auto doc = Document::Create(source);
  auto parser = Parser(syntax, doc);

  auto root = parser.Parse();
  auto prog = root->Attr("prog");
  ASSERT_TRUE(prog);
  auto stmts = prog->Attrs("stmts");
  ASSERT_EQ(stmts.size(), 2);
  auto stmt1 = stmts[0];
  auto stmt2 = stmts[1];
  auto type1 = stmt1->Attr("type");
  auto name1 = stmt1->Attr("name");
  auto type2 = stmt2->Attr("type");
  auto name2 = stmt2->Attr("name");
  auto value2 = stmt2->Attr("value");
  ASSERT_TRUE(type1);
  ASSERT_TRUE(name1);
  ASSERT_TRUE(type2);
  ASSERT_TRUE(name2);
  ASSERT_TRUE(value2);
  ASSERT_EQ(type1->Text(), "int");
  ASSERT_EQ(name1->Text(), "x");
  ASSERT_EQ(type2->Text(), "int");
  ASSERT_EQ(name2->Text(), "y");
  ASSERT_EQ(value2->Text(), "1");

  auto tokens = root->Store({.flatten = true});
  std::string tokenized;
  for (auto const& token : tokens) {
    tokenized += token.get<std::string>();
  }

  ASSERT_EQ(tokenized, source);
}

}  // namespace test
}  // namespace alioth