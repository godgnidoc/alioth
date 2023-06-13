#include "aliox/template.h"

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

TEST(Template, Expressions) {
  Template::Map model{};
  model["var"] = Template::Value{"hello"};
  model["obj"] = Template::Map{{"name", "World"}};
  model["arr"] = Template::Array{{"a", "b", "c"}};

  Template::Map meta{};
  meta["func"] = Template::Filter{[](auto&, auto const& args) {
    std::vector<std::string> strings;
    for (auto const& arg : args) {
      strings.push_back(arg.TextOf());
    }

    return fmt::format("{}", fmt::join(strings, "---"));
  }};

  meta["upper"] = Template::Filter{[](auto&, auto const& args) {
    auto text = args.front().TextOf();
    for (auto& c : text) {
      c = std::toupper(c);
    }
    return text;
  }};

  auto path = AliothHome() / "test" / "template_test" / "expr.template";
  auto result = Template::Render(path, model, meta);

  auto expected = R"(具名变量 hello
成员运算 World
下标运算1 World
下标运算2 b
函数调用 1---2---hello
管道运算 HELLO)";

  ASSERT_EQ(result, expected);
}

TEST(Template, Branch) {
  Template::Map model{};
  model["obj"] = Template::Map{{"name", "World"}};

  Template::Map meta{};
  meta["isobject"] = Template::Filter{
      [](auto&, auto const& args) { return args.front().IsMap(); }};

  meta["isstring"] = Template::Filter{
      [](auto&, auto const& args) { return args.front().IsString(); }};

  auto path = AliothHome() / "test" / "template_test" / "branch.template";
  auto result = Template::Render(path, model, meta);

  ASSERT_EQ(result, "外层分支成立块内层分支备选块{}内层分支不成立块2");
}

TEST(Template, Iter) {
  Template::Map model{};
  model["arr"] = Template::Array{"Hello", "World", "MyFriend"};
  model["obj"] = Template::Map{{"Hello", "World"}, {"Welcome", "MyFriend"}};

  auto path = AliothHome() / "test" / "template_test" / "iter.template";
  auto result = Template::Render(path, model);

  auto expected = R"(
- value: Hello;
  key: 0;
  index: 0;
  first: true;
  last: false;
  nonfirst: false;
  nonlast: true;

- value: World;
  key: 1;
  index: 1;
  first: false;
  last: false;
  nonfirst: true;
  nonlast: true;

- value: MyFriend;
  key: 2;
  index: 2;
  first: false;
  last: true;
  nonfirst: true;
  nonlast: false;

- value: World;
  key: Hello;
  index: 0;
  first: true;
  last: false;
  nonfirst: false;
  nonlast: true;

- value: MyFriend;
  key: Welcome;
  index: 1;
  first: false;
  last: true;
  nonfirst: true;
  nonlast: false;
)";

  ASSERT_EQ(result, expected);
}

TEST(Template, Extends) {
  auto path = AliothHome() / "test" / "template_test" / "final.template";
  auto result = Template::Render(path, {});

  auto expected = R"(第一行原始文本

第二行最终文本

第三行原始文本

第四行覆盖文本

追加行最终文本


第五行原始文本

第六行原始文本
)";

  ASSERT_EQ(result, expected);
}

TEST(Template, Call) {
  auto path = AliothHome() / "test" / "template_test" / "caller.template";
  auto result = Template::Render(
      path, {
                {"world",
                 Template::Map{
                     {"people", Template::Array{"你", "我", "他"}},
                 }},
            });

  auto expected = R"(Hello 你
Hello 我
Hello 他
)";

  ASSERT_EQ(result, expected);
}
}  // namespace test
}  // namespace alioth