#include "aliox/skeleton.h"

#include <filesystem>
#include <fstream>

#include "alioth/alioth.h"
#include "alioth/lexicon.h"
#include "alioth/parser.h"
#include "alioth/regex.h"
#include "aliox/grammar.h"
#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Skeleton, Deduce) {
  auto home = AliothHome();
  auto path = home / "grammar" / "template.grammar";
  auto gdoc = Document::Read(path);
  auto syntax = Grammar::Load(gdoc).Compile();
  auto skeleton = Skeleton::Deduce(syntax);

  for (auto const& [symbol, structure] : skeleton.structures) {
    if (syntax->NameOf(symbol) != "expr") continue;

    EXPECT_TRUE(structure.forms.count("var"));
    EXPECT_TRUE(structure.forms.count("string"));
    EXPECT_TRUE(structure.forms.count("number"));
    EXPECT_TRUE(structure.forms.count("invoke"));
    EXPECT_TRUE(structure.forms.count("field"));
    EXPECT_TRUE(structure.forms.count("index"));
  }
}

}  // namespace test
}  // namespace alioth