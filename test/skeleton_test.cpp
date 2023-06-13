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

TEST(Skeleton, Deduce) {
  auto home = AliothHome();
  auto path = home / "grammar" / "template.grammar";
  auto gdoc = Document::Read(path);
  auto syntax = Grammar::Load(gdoc).Compile();
  auto skeleton = Skeleton::Deduce(syntax);

  for (auto const& [symbol, structure] : skeleton.structures) {
    if (NameOf(symbol, syntax) != "prim") continue;

    EXPECT_TRUE(structure.formed_attributes.count("var"));
    EXPECT_TRUE(structure.formed_attributes.count("string"));
    EXPECT_TRUE(structure.formed_attributes.count("number"));
    EXPECT_TRUE(structure.formed_attributes.count("invoke"));
    EXPECT_TRUE(structure.formed_attributes.count("field"));
    EXPECT_TRUE(structure.formed_attributes.count("index"));
  }
}

}  // namespace test
}  // namespace alioth