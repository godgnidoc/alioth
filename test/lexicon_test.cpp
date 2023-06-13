#include "alioth/lexicon.h"

#include "alioth/parser.h"
#include "alioth/regex.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

namespace {

class MyParser : public Parser {
 public:
  using Parser::Parser;

 public:
  auto Scan() { return Parser::Scan(th); }

  Thread th{};
};

}  // namespace

TEST(Lexicon, Simple) {
  auto lex = Lexicon::Builder("test")
                 .Define("INT", "int"_regex)
                 .Define("IF", "if"_regex)
                 .Build();
  auto syntax = Syntax{new Syntactic{}};
  syntax->lex = lex;
  auto source = R"(intifisint)";
  auto doc = Document::Create(source);
  auto parser = MyParser(syntax, doc);

  std::string terms[] = {
      "INT", "IF", "<ERR>", "INT", "<EOF>",
  };
  for (auto name : terms) {
    auto term = parser.Scan();
    ASSERT_EQ(term->Name(), name);
  }
}

TEST(Lexicon, Complex) {
  auto lex = Lexicon::Builder("test")
                 .Define("FN", "fn"_regex)
                 .Define("FLOAT", "float"_regex)
                 .Define("INT", "int"_regex)
                 .Define("IF", "if"_regex)
                 .Define("ELSE", "else"_regex)
                 .Define("ENUM", "enum"_regex)
                 .Define("OPEN_BRACE", "{"_regex)
                 .Define("CLOSE_BRACE", "}"_regex)
                 .Define("OPEN_PAREN", "\\("_regex)
                 .Define("CLOSE_PAREN", "\\)"_regex)
                 .Define("SEMICOLON", ";"_regex)
                 .Define("COMMA", ","_regex)
                 .Define("ASSIGN", "="_regex)
                 .Define("ADD", "\\+"_regex)
                 .Define("SUB", "-"_regex)
                 .Define("GT", ">"_regex)
                 .Define("ID", "[a-zA-Z_][a-zA-Z0-9_]*"_regex)
                 .Define("NUM", "-?\\d+(\\.\\d+)?([eE]-?\\d+)?"_regex)
                 .Define("SPACE", "\\s+"_regex)
                 .Define("COMMENT", "//[^\\n]*"_regex)
                 .Define("DOCUMENT", "/\\*([^\\*]|\\*[^/])*\\*/"_regex)
                 .Build();
  auto source = R"(
      // This is a comment
      fn main() {
        /**
         * This is a document
         */
        float x = 1.0;
        int y = 1;
        if (x > y) {
          x = x + y;
        } else {
          x = x - y;
        }
        enum Color {
          RED,
          GREEN,
          BLUE
        }
      }
    )";
  auto syntax = Syntax{new Syntactic{}};
  syntax->lex = lex;
  auto doc = Document::Create(source);
  auto parser = MyParser(syntax, doc);

  std::string names[] = {
      "SPACE",                 // line 1
      "COMMENT",     "SPACE",  // line 2
      "FN",          "SPACE",      "ID",         "OPEN_PAREN",  "CLOSE_PAREN",
      "SPACE",       "OPEN_BRACE", "SPACE",  // line 3
      "DOCUMENT",    "SPACE",                // line 4 5 6
      "FLOAT",       "SPACE",      "ID",         "SPACE",       "ASSIGN",
      "SPACE",       "NUM",        "SEMICOLON",  "SPACE",  // line 7
      "INT",         "SPACE",      "ID",         "SPACE",       "ASSIGN",
      "SPACE",       "NUM",        "SEMICOLON",  "SPACE",  // line 8
      "IF",          "SPACE",      "OPEN_PAREN", "ID",          "SPACE",
      "GT",          "SPACE",      "ID",         "CLOSE_PAREN", "SPACE",
      "OPEN_BRACE",  "SPACE",  // line 9
      "ID",          "SPACE",      "ASSIGN",     "SPACE",       "ID",
      "SPACE",       "ADD",        "SPACE",      "ID",          "SEMICOLON",
      "SPACE",  // line 10
      "CLOSE_BRACE", "SPACE",      "ELSE",       "SPACE",       "OPEN_BRACE",
      "SPACE",  // line 11
      "ID",          "SPACE",      "ASSIGN",     "SPACE",       "ID",
      "SPACE",       "SUB",        "SPACE",      "ID",          "SEMICOLON",
      "SPACE",                 // line 12
      "CLOSE_BRACE", "SPACE",  // line 13
      "ENUM",        "SPACE",      "ID",         "SPACE",       "OPEN_BRACE",
      "SPACE",                               // line 14
      "ID",          "COMMA",      "SPACE",  // line 15
      "ID",          "COMMA",      "SPACE",  // line 16
      "ID",          "SPACE",                // line 17
      "CLOSE_BRACE", "SPACE",                // line 18
      "CLOSE_BRACE", "SPACE",                // line 19
  };

  for (auto name : names) {
    auto term = parser.Scan();
    ASSERT_EQ(term->Name(), name);
  }
}

}  // namespace test
}  // namespace alioth