#include "alioth/variable.h"

#include "gtest/gtest.h"

namespace alioth {
namespace test {

TEST(Variable, Eval) {
  Variable a;
  Variable b;

  a.Set<std::string>("123");
  ASSERT_EQ(a.Get<std::string>(), "123");
  a.Set(123);
  ASSERT_EQ(a.Get<int>(), 123);

  ASSERT_FALSE(b.HasValue());

  {
    Variable c;

    c.Eval([&]() { return a.Get<int>() + b.Get<int>(); });
    ASSERT_FALSE(c.HasValue());

    b.Set(456);
    ASSERT_EQ(c.Get<int>(), 123 + 456);

    a.Set(789);
    ASSERT_EQ(c.Get<int>(), 789 + 456);
  }

  a.Set(nullptr);
  ASSERT_TRUE(a.HasValue());
}

}  // namespace test
}  // namespace alioth