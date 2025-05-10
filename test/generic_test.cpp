#include <filesystem>
#include <fstream>

#include "alioth/lexicon.h"
#include "alioth/parser.h"
#include "alioth/regex.h"
#include "aliox/grammar.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Generic, Ambiguous) {
  auto gdoc = Document::Create(R"(
    # Ambiguous grammar

    lang: "lang"
    
    SEMI = /;/
    COMMA = /,/
    FOR<control> = /for/
    IN<control> = /in/
    ID = /[a-zA-Z_]\w*/
    SPACE ?= /\s+/

    lang -> ...lang? stmt@stmts;

    stmt -> expr@expr SEMI 
        | loop@loop SEMI;
    expr -> ID@var;
    expr -> ID@func ID@arg;
    loop -> FOR ID@value IN expr@range
        | FOR ID@value COMMA ID@key IN expr@range;
  )");
  auto syntax = Grammar::Compile(gdoc);

  auto doc = Document::Create(R"(
        x;
        for i in z;
        fn arg;
        for i, j in z;
    )");

  auto parser = Parser{syntax, doc};
  auto result = parser.Parse();

  auto expected = nlohmann::json::parse(R"({
  "lang": {
    "stmts": [
      {
        "expr": {
          "var": "x"
        }
      },
      {
        "loop": {
          "range": {
            "var": "z"
          },
          "value": "i"
        }
      },
      {
        "expr": {
          "arg": "arg",
          "func": "fn"
        }
      },
      {
        "loop": {
          "key": "j",
          "range": {
            "var": "z"
          },
          "value": "i"
        }
      }
    ]
  }
})");

  ASSERT_EQ(result->Store({}), expected);
}

}  // namespace test
}  // namespace alioth