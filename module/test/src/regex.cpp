#include "alioth/regex.h"

#include "gtest/gtest.h"

using namespace alioth::regex;

template <typename T, typename U>
std::shared_ptr<T> is(std::shared_ptr<U> const& ptr) {
    return std::dynamic_pointer_cast<T>(ptr);
}

TEST(REGEX, Case0) {
    auto const pattern = R"---(a(b|c)*d)---";
    auto const regex = Compile(pattern);

    auto const root = is<ConcatNode>(regex);
    ASSERT_TRUE(root);

    auto const a_b_c = is<ConcatNode>(root->left_);
    ASSERT_TRUE(a_b_c);

    auto const d = is<CharNode>(root->right_);
    ASSERT_TRUE(d);

    auto const a = is<CharNode>(a_b_c->left_);
    ASSERT_TRUE(a);

    auto const b_c_ = is<KleeneNode>(a_b_c->right_);
    ASSERT_TRUE(b_c_);

    auto const b_c = is<UnionNode>(b_c_->child_);
    ASSERT_TRUE(b_c);

    auto const b = is<CharNode>(b_c->left_);
    ASSERT_TRUE(b);

    auto const c = is<CharNode>(b_c->right_);
    ASSERT_TRUE(c);
}

TEST(REGEX, Case1) {
    auto const pattern = R"---(a(b|[\nA-C\r\\r_-])*d?)---";
    auto const regex = Compile(pattern);

    auto const root = is<ConcatNode>(regex);
    ASSERT_TRUE(root);

    auto const a_b_range = is<ConcatNode>(root->left_);
    ASSERT_TRUE(a_b_range);

    auto const d_opt = is<OptionalNode>(root->right_);
    ASSERT_TRUE(d_opt);

    auto const a = is<CharNode>(a_b_range->left_);
    ASSERT_TRUE(a);

    auto const b_range_ = is<KleeneNode>(a_b_range->right_);
    ASSERT_TRUE(b_range_);

    auto const d = is<CharNode>(d_opt->child_);
    ASSERT_TRUE(d);

    auto const b_range = is<UnionNode>(b_range_->child_);
    ASSERT_TRUE(b_range);

    auto const b = is<CharNode>(b_range->left_);
    ASSERT_TRUE(b);

    auto const range = is<RangeNode>(b_range->right_);
    ASSERT_TRUE(range);

    ASSERT_EQ(range->dir_, RangeNode::Direction::kPositive);
    ASSERT_EQ(range->set_.GetChars(),
              std::set<unsigned char>(
                  {'\n', 'A', 'B', 'C', '\r', '\\', 'r', '_', '-'}));
}

TEST(REGEX, Case2) {
    auto const pattern = R"---(true)---";
    auto const regex = Compile(pattern);

    auto const root = is<ConcatNode>(regex);
    ASSERT_TRUE(root);

    auto const tru = is<ConcatNode>(root->left_);
    ASSERT_TRUE(tru);

    auto const e = is<CharNode>(root->right_);
    ASSERT_TRUE(e);

    auto const tr = is<ConcatNode>(tru->left_);
    ASSERT_TRUE(tr);

    auto const t = is<CharNode>(tr->left_);
    ASSERT_TRUE(t);

    auto const r = is<CharNode>(tr->right_);
    ASSERT_TRUE(r);
}

TEST(REGEX, Case3) {
    auto const pattern = R"---([a-zA-Z_][a-zA-Z0-9_]*)---";
    auto const regex = Compile(pattern);

    auto const root = is<ConcatNode>(regex);
    ASSERT_TRUE(root);

    auto const a_z = is<RangeNode>(root->left_);
    ASSERT_TRUE(a_z);

    ASSERT_EQ(a_z->dir_, RangeNode::Direction::kPositive);
    ASSERT_EQ(a_z->set_.GetChars(),
              std::set<unsigned char>(
                  {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
                   'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                   'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
                   'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                   'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_'}));

    auto const a_z_0_9_ = is<KleeneNode>(root->right_);
    ASSERT_TRUE(a_z_0_9_);

    auto const a_z_0_9 = is<RangeNode>(a_z_0_9_->child_);
    ASSERT_TRUE(a_z_0_9);

    ASSERT_EQ(a_z_0_9->dir_, RangeNode::Direction::kPositive);
    ASSERT_EQ(
        a_z_0_9->set_.GetChars(),
        std::set<unsigned char>(
            {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
             'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
             'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
             'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
             '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}));
}

TEST(REGEX, Case4) {
    auto const pattern = R"---(/\*([^*]|\*[^/])*\*/)---";
    auto const regex = Compile(pattern);
}

TEST(REGEX, Case5) {
    auto const pattern = R"---("([^"\\]|\\.)*")---";
    auto const regex = Compile(pattern);
}

TEST(REGEX, Case6) {
    auto const pattern = R"---('([^'\\]|\\.)*')---";
    auto const regex = Compile(pattern);
}

TEST(REGEX, Case7) {
    auto const pattern = R"---([0-9]+(\.[0-9]+(e[0-9]+)?)?)---";
    auto const regex = Compile(pattern);
}
