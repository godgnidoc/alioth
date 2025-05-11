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

TEST(Parser, Importing) {
  auto mark = [] {
    auto lex = Lexicon::Builder("mark");
    lex.Define("TEXT", "\\w+"_regex);
    lex.Define("CODE_START", "{"_regex);
    lex.Define("CODE_END", "}"_regex);
    lex.Define("SPACES", "\\s+"_regex);
    auto syntax = Syntactic::Builder(lex.Build());
    syntax.Ignore("SPACES");
    syntax.Import("json");
    syntax.Formula("mark").Symbol("TEXT", "frags").Commit();
    syntax.Formula("mark")
        .Symbol("CODE_START")
        .Symbol("json", "frags")
        .Symbol("CODE_END")
        .Commit();
    syntax.Formula("mark")
        .Symbol("mark", "...")
        .Symbol("TEXT", "frags")
        .Commit();
    syntax.Formula("mark")
        .Symbol("mark", "...")
        .Symbol("CODE_START")
        .Symbol("json", "frags")
        .Symbol("CODE_END")
        .Commit();
    return syntax.Build();
  }();

  auto json = [] {
    auto lex = Lexicon::Builder("json");
    lex.Define("STRING", "\"[^\"]*\""_regex);
    lex.Define("NUMBER", "-?\\d+(\\.\\d+)?([eE]-?\\d+)?"_regex);
    lex.Define("TRUE", "true"_regex);
    lex.Define("FALSE", "false"_regex);
    lex.Define("NULL", "null"_regex);
    lex.Define("COMMA", ","_regex);
    lex.Define("COLON", ":"_regex);
    lex.Define("LBRACE", "{"_regex);
    lex.Define("RBRACE", "}"_regex);
    lex.Define("LBRACKET", "\\["_regex);
    lex.Define("RBRACKET", "\\]"_regex);
    lex.Define("SPACES", "\\s+"_regex);
    auto syntax = Syntactic::Builder(lex.Build());
    syntax.Ignore("SPACES");
    syntax.Formula("json").Symbol("object", "...").Commit();
    syntax.Formula("json").Symbol("array", "...").Commit();
    syntax.Formula("json").Symbol("string", "...").Commit();
    syntax.Formula("json").Symbol("number", "...").Commit();
    syntax.Formula("json").Symbol("boolean", "...").Commit();
    syntax.Formula("json").Symbol("null", "...").Commit();
    syntax.Formula("object")
        .Symbol("LBRACE", "empty_object")
        .Symbol("RBRACE")
        .Commit();
    syntax.Formula("object")
        .Symbol("LBRACE")
        .Symbol("fields", "...")
        .Symbol("RBRACE")
        .Commit();
    syntax.Formula("fields").Symbol("field", "object").Commit();
    syntax.Formula("fields")
        .Symbol("fields", "...")
        .Symbol("COMMA")
        .Symbol("field", "object")
        .Commit();
    syntax.Formula("field")
        .Symbol("STRING", "key")
        .Symbol("COLON")
        .Symbol("json", "value")
        .Commit();
    syntax.Formula("array")
        .Symbol("LBRACKET", "empty_array")
        .Symbol("RBRACKET")
        .Commit();
    syntax.Formula("array")
        .Symbol("LBRACKET")
        .Symbol("elements", "...")
        .Symbol("RBRACKET")
        .Commit();
    syntax.Formula("elements").Symbol("json", "array").Commit();
    syntax.Formula("elements")
        .Symbol("elements", "...")
        .Symbol("COMMA")
        .Symbol("json", "array")
        .Commit();
    syntax.Formula("string").Symbol("STRING", "string").Commit();
    syntax.Formula("number").Symbol("NUMBER", "number").Commit();
    syntax.Formula("boolean").Symbol("TRUE", "boolean").Commit();
    syntax.Formula("boolean").Symbol("FALSE", "boolean").Commit();
    syntax.Formula("null").Symbol("NULL", "null").Commit();
    return syntax.Build();
  }();

  auto source = R"(
        outer text prefix{
          {
            "name": "alioth",
            "version": 0.1,
            "author": ["GodGnidoc"],
            "license": "MIT",
            "commands": [
              {"name": "parse", "desc": "Parse source using specified syntax"},
              {"name": "framework", "desc": "Generate framework for specified language"}
            ]
          }
        }outer text suffix
  )";

  auto doc = Document::Create(source);
  auto parser = Parser(mark, doc,
                       Parser::ParseOptions{
                           .syntaxes{
                               {"json", json},
                           },
                       });
  auto root = parser.Parse();
  fmt::println("{}", root->Store({}).dump(2));
}

}  // namespace test
}  // namespace alioth