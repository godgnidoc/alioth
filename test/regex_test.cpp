#include "alioth/regex.h"

#include "fmt/ranges.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace alioth {
namespace test {

TEST(Regex, Compile) {
  auto const regex = "a(b|c)*d"_regex;
  auto abcd = std::dynamic_pointer_cast<RegexTree::ConcatNode>(regex);
  EXPECT_TRUE(abcd);

  auto abc = std::dynamic_pointer_cast<RegexTree::ConcatNode>(abcd->left_);
  EXPECT_TRUE(abc);

  auto a = std::dynamic_pointer_cast<RegexTree::CharNode>(abc->left_);
  EXPECT_TRUE(a);
  EXPECT_EQ(a->ch_, 'a');

  auto bcK = std::dynamic_pointer_cast<RegexTree::KleeneNode>(abc->right_);
  EXPECT_TRUE(bcK);

  auto bc = std::dynamic_pointer_cast<RegexTree::UnionNode>(bcK->child_);
  EXPECT_TRUE(bc);

  auto b = std::dynamic_pointer_cast<RegexTree::CharNode>(bc->left_);
  EXPECT_TRUE(b);
  EXPECT_EQ(b->ch_, 'b');

  auto c = std::dynamic_pointer_cast<RegexTree::CharNode>(bc->right_);
  EXPECT_TRUE(c);
  EXPECT_EQ(c->ch_, 'c');

  auto d = std::dynamic_pointer_cast<RegexTree::CharNode>(abcd->right_);
  EXPECT_TRUE(d);
  EXPECT_EQ(d->ch_, 'd');
}

TEST(Regex, Match) {
  auto const digit =
      std::dynamic_pointer_cast<RegexTree::RangeNode>("\\d"_regex);
  EXPECT_TRUE(digit);
  for (auto i = '0'; i <= '9'; i++) {
    EXPECT_TRUE(digit->Match(i));
  }
  auto const letter =
      std::dynamic_pointer_cast<RegexTree::RangeNode>("[a-zA-Z]"_regex);
  EXPECT_TRUE(letter);
  for (auto i = 'a'; i <= 'z'; i++) {
    EXPECT_TRUE(letter->Match(i));
  }
  for (auto i = 'A'; i <= 'Z'; i++) {
    EXPECT_TRUE(letter->Match(i));
  }
  auto const word =
      std::dynamic_pointer_cast<RegexTree::RangeNode>("\\w"_regex);
  EXPECT_TRUE(word);
  for (auto i = '0'; i <= '9'; i++) {
    EXPECT_TRUE(word->Match(i));
  }
  for (auto i = 'a'; i <= 'z'; i++) {
    EXPECT_TRUE(word->Match(i));
  }
  for (auto i = 'A'; i <= 'Z'; i++) {
    EXPECT_TRUE(word->Match(i));
  }
  EXPECT_TRUE(word->Match('_'));
  auto const any = std::dynamic_pointer_cast<RegexTree::RangeNode>("."_regex);
  EXPECT_TRUE(any);
  for (auto i = 0; i < 256; i++) {
    EXPECT_TRUE(any->Match(i));
  }
}

}  // namespace test
}  // namespace alioth