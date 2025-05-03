#include "alioth/regex.h"

#include <optional>

#include "alioth/strings.h"

namespace alioth {

void RegexTree::LeafNode::CalcFollowpos() { /** 无动作 */ }

bool RegexTree::AcceptNode::GetNullable() { return {}; }
RegexTree::Leafs RegexTree::AcceptNode::GetFirstpos() { return {}; }
RegexTree::Leafs RegexTree::AcceptNode::GetLastpos() { return {}; }
bool RegexTree::AcceptNode::Match(char) { return {}; }
std::string RegexTree::AcceptNode::Print() const { return ""; }

bool RegexTree::CharNode::GetNullable() { return false; }
RegexTree::Leafs RegexTree::CharNode::GetFirstpos() {
  return {shared_from_this()};
}
RegexTree::Leafs RegexTree::CharNode::GetLastpos() {
  return {shared_from_this()};
}
bool RegexTree::CharNode::Match(char ch) { return ch_ == ch; }
std::string RegexTree::CharNode::Print() const { return pattern_; };

bool RegexTree::RangeNode::GetNullable() { return false; }
RegexTree::Leafs RegexTree::RangeNode::GetFirstpos() {
  return {shared_from_this()};
}
RegexTree::Leafs RegexTree::RangeNode::GetLastpos() {
  return {shared_from_this()};
}
bool RegexTree::RangeNode::Match(char ch) {
  return includes_ == (0 != set_.count(ch));
}
std::string RegexTree::RangeNode::Print() const { return pattern_; };

bool RegexTree::ConcatNode::GetNullable() {
  return left_->GetNullable() && right_->GetNullable();
}
RegexTree::Leafs RegexTree::ConcatNode::GetFirstpos() {
  auto set = left_->GetFirstpos();
  if (left_->GetNullable()) set.merge(right_->GetFirstpos());
  return set;
}
RegexTree::Leafs RegexTree::ConcatNode::GetLastpos() {
  auto set = right_->GetLastpos();
  if (right_->GetNullable()) set.merge(left_->GetLastpos());
  return set;
}
void RegexTree::ConcatNode::CalcFollowpos() {
  left_->CalcFollowpos();
  right_->CalcFollowpos();

  for (auto& node : left_->GetLastpos()) {
    node->followpos_.merge(right_->GetFirstpos());
  }
}
std::string RegexTree::ConcatNode::Print() const {
  auto lhs = left_->Print();
  auto rhs = right_->Print();
  if (std::dynamic_pointer_cast<UnionNode>(left_)) {
    lhs = "(" + lhs + ")";
  } else if (std::dynamic_pointer_cast<UnionNode>(right_)) {
    rhs = "(" + rhs + ")";
  }
  return lhs + rhs;
};

bool RegexTree::UnionNode::GetNullable() {
  return left_->GetNullable() || right_->GetNullable();
}
RegexTree::Leafs RegexTree::UnionNode::GetFirstpos() {
  auto set = left_->GetFirstpos();
  set.merge(right_->GetFirstpos());
  return set;
}
RegexTree::Leafs RegexTree::UnionNode::GetLastpos() {
  auto set = left_->GetLastpos();
  set.merge(right_->GetLastpos());
  return set;
}
void RegexTree::UnionNode::CalcFollowpos() {
  left_->CalcFollowpos();
  right_->CalcFollowpos();
}
std::string RegexTree::UnionNode::Print() const {
  auto lhs = left_->Print();
  auto rhs = right_->Print();
  return lhs + "|" + rhs;
};

bool RegexTree::KleeneNode::GetNullable() { return true; }
RegexTree::Leafs RegexTree::KleeneNode::GetFirstpos() {
  return child_->GetFirstpos();
}
RegexTree::Leafs RegexTree::KleeneNode::GetLastpos() {
  return child_->GetLastpos();
}
void RegexTree::KleeneNode::CalcFollowpos() {
  child_->CalcFollowpos();

  for (auto& node : child_->GetLastpos()) {
    node->followpos_.merge(child_->GetFirstpos());
  }
}
std::string RegexTree::KleeneNode::Print() const {
  auto sub = child_->Print();
  if (std::dynamic_pointer_cast<BinaryNode>(child_)) {
    sub = "(" + sub + ")";
  }
  return sub + "*";
};

bool RegexTree::PositiveNode::GetNullable() { return child_->GetNullable(); }
RegexTree::Leafs RegexTree::PositiveNode::GetFirstpos() {
  return child_->GetFirstpos();
}
RegexTree::Leafs RegexTree::PositiveNode::GetLastpos() {
  return child_->GetLastpos();
}
void RegexTree::PositiveNode::CalcFollowpos() {
  child_->CalcFollowpos();

  for (auto& node : child_->GetLastpos()) {
    node->followpos_.merge(child_->GetFirstpos());
  }
}
std::string RegexTree::PositiveNode::Print() const {
  auto sub = child_->Print();
  if (std::dynamic_pointer_cast<BinaryNode>(child_)) {
    sub = "(" + sub + ")";
  }
  return sub + "+";
};

bool RegexTree::OptionalNode::GetNullable() { return true; }
RegexTree::Leafs RegexTree::OptionalNode::GetFirstpos() {
  return child_->GetFirstpos();
}
RegexTree::Leafs RegexTree::OptionalNode::GetLastpos() {
  return child_->GetLastpos();
}
void RegexTree::OptionalNode::CalcFollowpos() { child_->CalcFollowpos(); }
std::string RegexTree::OptionalNode::Print() const {
  auto sub = child_->Print();
  if (std::dynamic_pointer_cast<BinaryNode>(child_)) {
    sub = "(" + sub + ")";
  }
  return sub + "?";
};

std::string const& RegexTree::Operators() {
  static std::string str = "+*?|()";
  return str;
}

/**
 * 从start位置分析一个由方括号引领的字符类表达式，成功则原地归约，失败则抛出异常
 */
void RegexTree::ParseRange(std::vector<Unit>& input, size_t const start) {
  int state = 1;
  std::optional<char> range_left;
  const auto node = std::make_shared<RangeNode>();

  if (input[start].Char() != '[') {
    throw InvalidRange{};
  } else {
    node->pattern_ += input.at(start).Char();
    input.erase(input.begin() + start);
  }

  if (input[start].Char() == '^') {
    node->includes_ = false;
    node->pattern_ += input.at(start).Char();
    input.erase(input.begin() + start);
  } else {
    node->includes_ = true;
  }

  while (state > 0) {
    if (!input[start].IsChar()) {
      throw InvalidRange{};
    }

    switch (state) {
      case 1: {
        if (input[start].Char() == ']') {
          state = 0;
        } else if (input[start].Char() == '-') {
          if (!range_left)
            node->set_.insert('-');
          else
            state = 3;
        } else if (input[start].Char() == '\\') {
          state = 2;
        } else {
          range_left = input[start].Char();
          node->set_.insert(*range_left);
        }
      } break;
      case 2: {  // 转义字符
        auto [range, includes] = Chars::Extract(input[start].Char());
        if (!includes) throw InvalidRange{};
        for (auto const& c : range) {
          node->set_.insert(c);
        }
        if (range.size() == 1) range_left = range.front();
        state = 1;
      } break;
      case 3: {  // 字符范围
        if (input[start].Char() == ']') {
          node->set_.insert('-');
          state = 0;
        } else {
          node->set_ += Chars::Range(*range_left, input[start].Char());
          range_left.reset();
          state = 1;
        }
      } break;
    }

    node->pattern_ += input.at(start).Char();
    input.erase(input.begin() + start);
  }

  if (state != 0) throw InvalidRange{};

  input.insert(input.begin() + start, node);
}

/**
 * 做词法分析，将字符和转义字符都归约成节点
 */
void RegexTree::ParseChars(std::vector<Unit>& input) {
  size_t i = 0;
  while (i < input.size()) {
    if (input[i].IsChar()) {
      if (input[i].Char() == '[') {
        ParseRange(input, i);
      } else if (Operators().find(input[i].Char()) != std::string::npos) {
        // skip
      } else if (input[i].Char() == '\\') {
        if (i + 1 >= input.size()) throw InvalidEscape{};

        input.erase(input.begin() + i);
        auto p = std::string({'\\', input.at(i).Char()});
        auto [range, includes] = Chars::Extract(input[i].Char());
        if (range.size() == 1) {
          auto const node = std::make_shared<CharNode>();
          node->pattern_ = p;
          node->ch_ = range[0];
          input[i] = node;
        } else {
          auto const node = std::make_shared<RangeNode>();
          node->pattern_ = p;
          node->includes_ = includes;
          for (auto c : range) node->set_.insert(c);
          input[i] = node;
        }
      } else if (input[i].Char() == '.') {
        auto const node = std::make_shared<RangeNode>();
        node->pattern_ = ".";
        node->includes_ = false;
        node->set_ = {};
        input[i] = node;
      } else {
        auto const node = std::make_shared<CharNode>();
        node->pattern_ = input[i].Char();
        node->ch_ = input[i].Char();
        input[i] = node;
      }
    }
    ++i;
  }
}

/**
 * 解析正则表达式
 *  1. 递归处理括号，将分析完毕的表达式在原地归约
 *  2. 处理后缀运算符，将分析完毕的表达式在原地归约
 *  3. 处理连接，将分析完毕的表达式在原地归约
 *  4. 处理或运算符，将分析完毕的表达式在原地归约
 */
void RegexTree::Parse(std::vector<Unit>& base, size_t const start) {
  // 1. 递归处理括号，将分析完毕的表达式在原地归约
  for (size_t i = start;
       i < base.size() && (!base[i].IsChar() || base[i].Char() != ')'); ++i) {
    if (base[i].IsChar() && base[i].Char() == '(') {
      base.erase(base.begin() + i);
      Parse(base, i);
      if (i + 1 >= base.size() || base[i + 1].Char() != ')') {
        throw InvalidPattern{};
      }
      base.erase(base.begin() + i + 1);
    }
  }

  // 2. 处理后缀运算符，将分析完毕的表达式在原地归约
  for (size_t i = start;
       i < base.size() && (!base[i].IsChar() || base[i].Char() != ')'); ++i) {
    if (base[i].IsChar() && base[i].Char() == '*') {
      if (i == start || !base[i - 1].IsNode()) {
        throw InvalidPattern{};
      }

      auto const node = std::make_shared<KleeneNode>();
      node->child_ = base[--i].Node();
      base.erase(base.begin() + i);
      base[i] = node;
    } else if (base[i].IsChar() && base[i].Char() == '+') {
      if (i == start || !base[i - 1].IsNode()) {
        throw InvalidPattern{};
      }

      auto const node = std::make_shared<PositiveNode>();
      node->child_ = base[--i].Node();
      base.erase(base.begin() + i);
      base[i] = node;
    } else if (base[i].IsChar() && base[i].Char() == '?') {
      if (i == start || !base[i - 1].IsNode()) {
        throw InvalidPattern{};
      }

      auto const node = std::make_shared<OptionalNode>();
      node->child_ = base[--i].Node();
      base.erase(base.begin() + i);
      base[i] = node;
    }
  }

  // 3. 处理连接，将分析完毕的表达式在原地归约
  for (size_t i = start;
       i < base.size() && (!base[i].IsChar() || base[i].Char() != ')'); ++i) {
    if (i <= start || !base[i].IsNode() || !base[i - 1].IsNode()) continue;

    auto const node = std::make_shared<ConcatNode>();
    node->right_ = base[i].Node();
    node->left_ = base[--i].Node();
    base[i] = node;
    base.erase(base.begin() + i + 1);
  }

  // 4. 处理或运算符，将分析完毕的表达式在原地归约
  for (size_t i = start;
       i < base.size() && (!base[i].IsChar() || base[i].Char() != ')'); ++i) {
    if (!base[i].IsChar() || base[i].Char() != '|') continue;

    if (i == start || !base[i - 1].IsNode() || i >= base.size() ||
        !base[i + 1].IsNode()) {
      throw InvalidPattern{};
    }

    auto const node = std::make_shared<UnionNode>();
    base.erase(base.begin() + i);
    node->right_ = base[i].Node();
    node->left_ = base[--i].Node();
    base[i] = node;
    base.erase(base.begin() + i + 1);
  }
}

Regex RegexTree::Compile(std::string const& pattern) {
  std::vector<Unit> input{pattern.begin(), pattern.end()};

  ParseChars(input);
  Parse(input, 0);

  if (input.size() != 1 || !input[0].IsNode()) {
    throw InvalidPattern{};
  }

  return input[0].Node();
}

std::shared_ptr<RegexTree::AcceptNode> RegexTree::AcceptNode::On(
    Regex const& regex, SymbolID term) {
  auto const node = std::make_shared<AcceptNode>();
  node->term_ = term;
  for (auto it : regex->GetLastpos()) it->followpos_.insert(node);
  return node;
}

Regex RegexTree::UnionNode::Of(Regex const& left, Regex const& right) {
  auto const node = std::make_shared<UnionNode>();
  node->left_ = left;
  node->right_ = right;
  return node;
}

}  // namespace alioth