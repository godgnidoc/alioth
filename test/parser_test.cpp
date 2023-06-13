#include "alioth/parser.h"

#include "alioth/inspect.h"
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
  auto prog = AttrOf(root, "prog");
  ASSERT_TRUE(prog);
  auto stmts = AttrsOf(prog, "stmts");
  ASSERT_EQ(stmts.size(), 2);
  auto stmt1 = stmts[0];
  auto stmt2 = stmts[1];
  auto type1 = AttrOf(stmt1, "type");
  auto name1 = AttrOf(stmt1, "name");
  auto type2 = AttrOf(stmt2, "type");
  auto name2 = AttrOf(stmt2, "name");
  auto value2 = AttrOf(stmt2, "value");
  ASSERT_TRUE(type1);
  ASSERT_TRUE(name1);
  ASSERT_TRUE(type2);
  ASSERT_TRUE(name2);
  ASSERT_TRUE(value2);
  ASSERT_EQ(TextOf(type1), "int");
  ASSERT_EQ(TextOf(name1), "x");
  ASSERT_EQ(TextOf(type2), "int");
  ASSERT_EQ(TextOf(name2), "y");
  ASSERT_EQ(TextOf(value2), "1");

  auto tokens = alioth::Tokenize(root);
  std::string tokenized;
  for (auto const& token : tokens) {
    tokenized += TextOf(token);
  }

  ASSERT_EQ(tokenized, source);
}

}  // namespace test
}  // namespace alioth