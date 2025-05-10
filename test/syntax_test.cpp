#include "alioth/syntax.h"

#include "alioth/regex.h"
#include "aliox/grammar.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Syntactic, Simple) {
  auto lex = Lexicon::Builder("test")
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
                    .Symbol("prog")
                    .Symbol("stmt")
                    .Commit()
                    .Formula("stmt")
                    .Symbol("INT")
                    .Symbol("ID")
                    .Symbol("SEMI")
                    .Commit()
                    .Formula("stmt")
                    .Symbol("INT")
                    .Symbol("ID")
                    .Symbol("EQ")
                    .Symbol("NUM")
                    .Symbol("SEMI")
                    .Commit()
                    .Build();
}

TEST(Syntactic, ReduceShift) {
  auto lex = Lexicon::Builder("test")
                 .Define("X", "x"_regex)
                 .Define("Y", "y"_regex)
                 .Define("Z", "z"_regex)
                 .Define("SPACE", "\\s+"_regex)
                 .Build();

  auto syntax = Syntactic::Builder(lex)
                    .Ignore("SPACE")
                    .Formula("s")
                    .Symbol("y")
                    .Symbol("z")
                    .Commit()
                    .Formula("s")
                    .Symbol("z")
                    .Commit()
                    .Formula("y")
                    .Symbol("X")
                    .Symbol("Y")
                    .Commit()
                    .Formula("y")
                    .Symbol("y")
                    .Symbol("X")
                    .Symbol("Y")
                    .Commit()
                    .Formula("z")
                    .Symbol("X")
                    .Symbol("Z")
                    .Commit()
                    .Formula("z")
                    .Symbol("z")
                    .Symbol("X")
                    .Symbol("Z")
                    .Commit()
                    .Build();
}

}  // namespace test
}  // namespace alioth