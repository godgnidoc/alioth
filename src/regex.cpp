#include "alioth/regex.h"

#include <map>
#include <tuple>
#include <vector>

#include "alioth/utils/hex.h"
#include "spdlog/fmt/fmt.h"

namespace alioth::regex {

namespace {

/** 字符集 */
std::set<char> char_range_digit = {'0', '1', '2', '3', '4',
                                   '5', '6', '7', '8', '9'};
std::set<char> char_range_lower = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                                   'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                                   's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
std::set<char> char_range_upper = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
                                   'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                   'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
std::set<char> char_range_punct = {'!', '"', '#', '$', '%', '&', '\'', '(',
                                   ')', '*', '+', ',', '-', '.', '/',  ':',
                                   ';', '<', '=', '>', '?', '@', '[',  '\\',
                                   ']', '^', '_', '`', '{', '|', '}',  '~'};
std::set<char> char_range_space = {'\t', '\n', '\v', '\f', '\r', ' '};
std::set<char> char_range_word = {
    '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
    'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/** 转义字符映射表 */
std::map<char, std::string> str_escape_table = {
    {'\a', "\\a"},  {'\b', "\\b"},  {'\f', "\\f"}, {'\n', "\\n"},
    {'\r', "\\r"},  {'\t', "\\t"},  {'\v', "\\v"}, {'\\', "\\\\"},
    {'\'', "\\\'"}, {'\"', "\\\""}, {'?', "\\?"},  {'*', "\\*"},
    {'+', "\\+"},   {'|', "\\|"},   {'(', "\\("},  {')', "\\)"},
    {'[', "\\["},   {']', "\\]"},   {'\0', "\\0"}};

}  // namespace

Node::Node(Type type) : type_(type) {}
LeafNode::LeafNode(Type type) : Node(type) {}
void LeafNode::CalcFollowpos() { /* do nothing */
}

AcceptNode::AcceptNode(int token_id)
    : LeafNode(Node::Type::kAccept), token_id_(token_id) {}
bool AcceptNode::GetNullable() {
  throw std::logic_error("AcceptNode::GetNullable()");
}
LeafNodes AcceptNode::GetFirstpos() {
  throw std::logic_error("AcceptNode::GetFirstpos()");
}
LeafNodes AcceptNode::GetLastpos() {
  throw std::logic_error("AcceptNode::GetLastpos()");
}
bool AcceptNode::Match(char input) {
  (void)input;
  return false;
}

CharNode::CharNode() : LeafNode(Node::Type::kChar) {}
bool CharNode::GetNullable() { return false; }
LeafNodes CharNode::GetFirstpos() { return {shared_from_this()}; }
LeafNodes CharNode::GetLastpos() { return {shared_from_this()}; }
bool CharNode::Match(char ch) { return ch_ == ch; }

RangeNode::RangeNode() : LeafNode(Node::Type::kRange) {}
bool RangeNode::GetNullable() { return false; }
LeafNodes RangeNode::GetFirstpos() { return {shared_from_this()}; }
LeafNodes RangeNode::GetLastpos() { return {shared_from_this()}; }
bool RangeNode::Match(char ch) {
  if (dir_ == kNegative)
    return !set_.Contains(ch);
  else
    return set_.Contains(ch);
}

ConcatNode::ConcatNode() : Node(Node::Type::kConcat) {}
bool ConcatNode::GetNullable() {
  return left_->GetNullable() && right_->GetNullable();
}
LeafNodes ConcatNode::GetFirstpos() {
  auto set = left_->GetFirstpos();
  if (left_->GetNullable()) set.merge(right_->GetFirstpos());
  return set;
}
LeafNodes ConcatNode::GetLastpos() {
  auto set = right_->GetLastpos();
  if (right_->GetNullable()) set.merge(left_->GetLastpos());
  return set;
}
void ConcatNode::CalcFollowpos() {
  left_->CalcFollowpos();
  right_->CalcFollowpos();

  for (auto& node : left_->GetLastpos()) {
    node->followpos_.merge(right_->GetFirstpos());
  }
}

UnionNode::UnionNode() : Node(Node::Type::kUnion) {}
bool UnionNode::GetNullable() {
  return left_->GetNullable() || right_->GetNullable();
}
LeafNodes UnionNode::GetFirstpos() {
  auto set = left_->GetFirstpos();
  set.merge(right_->GetFirstpos());
  return set;
}
LeafNodes UnionNode::GetLastpos() {
  auto set = left_->GetLastpos();
  set.merge(right_->GetLastpos());
  return set;
}
void UnionNode::CalcFollowpos() {
  left_->CalcFollowpos();
  right_->CalcFollowpos();
}

KleeneNode::KleeneNode() : Node(Node::Type::kKleene) {}
bool KleeneNode::GetNullable() { return true; }
LeafNodes KleeneNode::GetFirstpos() { return child_->GetFirstpos(); }
LeafNodes KleeneNode::GetLastpos() { return child_->GetLastpos(); }
void KleeneNode::CalcFollowpos() {
  child_->CalcFollowpos();

  for (auto& node : child_->GetLastpos()) {
    node->followpos_.merge(child_->GetFirstpos());
  }
}

PositiveNode::PositiveNode() : Node(Node::Type::kPositive) {}
bool PositiveNode::GetNullable() { return child_->GetNullable(); }
LeafNodes PositiveNode::GetFirstpos() { return child_->GetFirstpos(); }
LeafNodes PositiveNode::GetLastpos() { return child_->GetLastpos(); }
void PositiveNode::CalcFollowpos() {
  child_->CalcFollowpos();

  for (auto& node : child_->GetLastpos()) {
    node->followpos_.merge(child_->GetFirstpos());
  }
}

OptionalNode::OptionalNode() : Node(Node::Type::kOptional) {}
bool OptionalNode::GetNullable() { return true; }
LeafNodes OptionalNode::GetFirstpos() { return child_->GetFirstpos(); }
LeafNodes OptionalNode::GetLastpos() { return child_->GetLastpos(); }
void OptionalNode::CalcFollowpos() { child_->CalcFollowpos(); }

std::map<char, std::tuple<RangeNode::Direction, std::set<char>>>
    char_class_table = {
        {'a', {RangeNode::kPositive, {'\a'}}},
        {'b', {RangeNode::kPositive, {'\b'}}},
        {'D', {RangeNode::kNegative, char_range_digit}},
        {'d', {RangeNode::kPositive, char_range_digit}},
        {'f', {RangeNode::kPositive, {'\f'}}},
        {'L', {RangeNode::kNegative, char_range_lower}},
        {'l', {RangeNode::kPositive, char_range_lower}},
        {'n', {RangeNode::kPositive, {'\n'}}},
        {'P', {RangeNode::kNegative, char_range_punct}},
        {'p', {RangeNode::kPositive, char_range_punct}},
        {'r', {RangeNode::kPositive, {'\r'}}},
        {'S', {RangeNode::kNegative, char_range_space}},
        {'s', {RangeNode::kPositive, char_range_space}},
        {'t', {RangeNode::kPositive, {'\t'}}},
        {'U', {RangeNode::kNegative, char_range_upper}},
        {'u', {RangeNode::kPositive, char_range_upper}},
        {'W', {RangeNode::kNegative, char_range_word}},
        {'w', {RangeNode::kPositive, char_range_word}},
};

std::set<char> operator_table = {'*', '+', '?', '|', '(', ')'};

/** 泛型输入单元 */
struct Unit {
  enum Type { kChar, kNode };

  Type type;
  char ch;
  std::shared_ptr<Node> node = nullptr;

  bool operator==(char ch) { return type == kChar && this->ch == ch; }

  bool operator!=(char ch) { return !(*this == ch); }
};

/**
 * 从start位置分析一个由方括号引领的字符类表达式，成功则原地归约，失败则抛出异常
 */
void ParseRange(std::vector<Unit>& input, size_t const start) {
  int state = 1;
  char range_left = 0;
  const auto node = std::make_shared<RangeNode>();

  if (input[start] != '[') {
    throw std::runtime_error("ParseRange: invalid start char");
  } else {
    input.erase(input.begin() + start);
  }

  if (input[start] == '^') {
    node->dir_ = RangeNode::kNegative;
    input.erase(input.begin() + start);
  } else {
    node->dir_ = RangeNode::kPositive;
  }

  while (state > 0) {
    if (input[start].type != Unit::kChar) {
      throw std::runtime_error("ParseRange: invalid input in range");
    }

    switch (state) {
      case 1: {
        if (input[start] == ']') {
          state = 0;
        } else if (input[start] == '-') {
          if (range_left == 0)
            node->set_.Insert('-');
          else
            state = 3;
        } else if (input[start] == '\\') {
          state = 2;
        } else {
          range_left = input[start].ch;
          node->set_.Insert(range_left);
        }
      } break;
      case 2: {  // 转义字符
        if (auto it = char_class_table.find(input[start].ch);
            it != char_class_table.end()) {
          const auto dir = std::get<0>(it->second);
          if (dir == RangeNode::kNegative) {
            throw std::runtime_error(
                "ParseRange: negative char class in range");
          }

          for (auto c : std::get<1>(it->second)) {
            node->set_.Insert(c);
          }
          state = 1;
        } else {
          node->set_.Insert(input[start].ch);
          state = 1;
        }
      } break;
      case 3: {  // 字符范围
        if (input[start] == ']') {
          node->set_.Insert('-');
          state = 0;
        } else if (input[start].ch < range_left) {
          node->set_.Insert(input[start].ch, range_left);
          range_left = 0;
          state = 1;
        } else {
          node->set_.Insert(range_left, input[start].ch);
          range_left = 0;
          state = 1;
        }
      } break;
    }

    input.erase(input.begin() + start);
  }

  if (state != 0) {
    throw std::runtime_error("ParseRange: endless range");
  }

  if (node->set_.IsEmpty()) {
    throw std::runtime_error("ParseRange: empty range");
  }

  input.insert(input.begin() + start, Unit{Unit::kNode, 0, node});
}

/**
 * 做词法分析，将字符和转义字符都归约成节点
 */
void ParseChars(std::vector<Unit>& input) {
  size_t i = 0;
  while (i < input.size()) {
    if (input[i].type == Unit::kChar) {
      if (input[i].ch == '[') {
        ParseRange(input, i);
      } else if (operator_table.count(input[i].ch)) {
        // skip
      } else if (input[i].ch == '\\') {
        if (i + 1 >= input.size()) {
          throw std::runtime_error("invalid escape sequence");
        }

        input.erase(input.begin() + i);
        input[i].type = Unit::kNode;
        if (auto it = char_class_table.find(input[i].ch);
            it != char_class_table.end()) {
          auto const set = std::get<1>(it->second);
          auto const dir = std::get<0>(it->second);

          if (set.size() == 1) {
            auto const node = std::make_shared<CharNode>();
            node->ch_ = *set.begin();
            input[i].node = node;
          } else {
            auto const node = std::make_shared<RangeNode>();
            node->dir_ = dir;
            for (auto c : set) node->set_.Insert(c);
            input[i].node = node;
          }
        } else {
          auto const node = std::make_shared<CharNode>();
          node->ch_ = input[i].ch;
          input[i].node = node;
          input[i].type = Unit::kNode;
        }
      } else if (input[i].ch == '.') {
        auto const node = std::make_shared<RangeNode>();
        node->dir_ = RangeNode::kNegative;
        node->set_ = {};
        input[i].node = node;
        input[i].type = Unit::kNode;
      } else {
        auto const node = std::make_shared<CharNode>();
        node->ch_ = input[i].ch;
        input[i].node = node;
        input[i].type = Unit::kNode;
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
void Parse(std::vector<Unit>& base, size_t const start) {
  // 1. 递归处理括号，将分析完毕的表达式在原地归约
  for (size_t i = start; i < base.size() && base[i] != ')'; ++i) {
    if (base[i] == '(') {
      base.erase(base.begin() + i);
      Parse(base, i);
      if (i + 1 >= base.size() || base[i + 1] != ')') {
        throw std::runtime_error("Parse: missing ')'");
      }
      base.erase(base.begin() + i + 1);
    }
  }

  // 2. 处理后缀运算符，将分析完毕的表达式在原地归约
  for (size_t i = start; i < base.size() && base[i] != ')'; ++i) {
    if (base[i] == '*') {
      if (i == start || base[i - 1].type != Unit::kNode) {
        throw std::runtime_error("Parse: missing operand");
      }

      auto const node = std::make_shared<KleeneNode>();
      node->child_ = base[--i].node;
      base.erase(base.begin() + i);
      base[i].node = node;
      base[i].type = Unit::kNode;
    } else if (base[i] == '+') {
      if (i == start || base[i - 1].type != Unit::kNode) {
        throw std::runtime_error("Parse: missing operand");
      }

      auto const node = std::make_shared<PositiveNode>();
      node->child_ = base[--i].node;
      base.erase(base.begin() + i);
      base[i].node = node;
      base[i].type = Unit::kNode;
    } else if (base[i] == '?') {
      if (i == start || base[i - 1].type != Unit::kNode) {
        throw std::runtime_error("Parse: missing operand");
      }

      auto const node = std::make_shared<OptionalNode>();
      node->child_ = base[--i].node;
      base.erase(base.begin() + i);
      base[i].node = node;
      base[i].type = Unit::kNode;
    }
  }

  // 3. 处理连接，将分析完毕的表达式在原地归约
  for (size_t i = start; i < base.size() && base[i] != ')'; ++i) {
    if (i <= start || base[i].type != Unit::kNode ||
        base[i - 1].type != Unit::kNode)
      continue;

    auto const node = std::make_shared<ConcatNode>();
    node->right_ = base[i].node;
    node->left_ = base[--i].node;
    base[i].node = node;
    base.erase(base.begin() + i + 1);
  }

  // 4. 处理或运算符，将分析完毕的表达式在原地归约
  for (size_t i = start; i < base.size() && base[i] != ')'; ++i) {
    if (base[i] != '|') continue;

    if (i == start || base[i - 1].type != Unit::kNode || i >= base.size() ||
        base[i + 1].type != Unit::kNode) {
      throw std::runtime_error("Parse: missing operand");
    }

    auto const node = std::make_shared<UnionNode>();
    base.erase(base.begin() + i);
    node->right_ = base[i].node;
    node->left_ = base[--i].node;
    base[i].node = node;
    base[i].type = Unit::kNode;
    base.erase(base.begin() + i + 1);
  }
}

Regex Compile(std::string const& expression) {
  std::vector<Unit> input;
  for (auto const ch : expression) {
    input.push_back(Unit{Unit::kChar, ch});
  }

  ParseChars(input);
  Parse(input, 0);

  if (input.size() != 1 || input[0].type != Unit::kNode) {
    throw std::runtime_error("invalid expression");
  }

  return input[0].node;
}

Regex Union(Regex const& left, Regex const& right) {
  auto const node = std::make_shared<UnionNode>();
  node->left_ = left;
  node->right_ = right;
  return node;
}

std::shared_ptr<AcceptNode> Accept(Regex const& regex, int token_id) {
  auto const node = std::make_shared<AcceptNode>(token_id);
  for (auto it : regex->GetLastpos()) it->followpos_.insert(node);
  return node;
}

}  // namespace alioth::regex