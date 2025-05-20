#ifndef __ALIOX_TEMPLATE_H__
#define __ALIOX_TEMPLATE_H__

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "alioth/ast.h"
#include "alioth/document.h"
#include "nlohmann/json.hpp"

namespace alioth {

struct Template;
using Tmpl = std::shared_ptr<Template>;

/**
 * 模板结构
 */
struct Template {
  struct Fragment;
  struct TextFragment;
  struct CallFragment;
  struct EvalFragment;
  struct IterFragment;
  struct BranchFragment;
  struct BlockFragment;
  struct ExtendsFragment;
  struct OverwriteFragment;
  struct Expression;
  struct VariableExpr;
  struct FieldExpr;
  struct IndexExpr;
  struct IntegerExpr;
  struct NumberExpr;
  struct StringExpr;
  struct InvokeExpr;
  struct PipeExpr;
  struct Context;
  using Expr = std::shared_ptr<Expression>;
  using Frag = std::shared_ptr<Fragment>;

  struct Value;
  using Null = std::nullptr_t;
  using String = std::string;
  using Integer = int64_t;
  using Number = double;
  using Boolean = bool;
  using Array = std::vector<Value>;
  using Map = std::map<std::string, Value>;
  using Filter = std::function<Value(Context&, Array const&)>;

  std::vector<Frag> fragments;

  /**
   * 从模板源码加载模板语法结构
   *
   * @param source 模板源码
   */
  static Tmpl Load(Doc source);

  /**
   * 从模板语法结构加载模板
   *
   * @param node 模板语法结构
   */
  static Frag LoadFrag(AST node);

  /**
   * 从表达式语法结构加载表达式
   *
   * @param node 表达式语法结构
   */
  static Expr LoadExpr(AST node);

  /**
   * 将模板语法结构编译为可执行的过滤器
   *
   * @param tmpl 模板语法结构
   */
  static Filter Compile(Tmpl tmpl);

  /**
   * 将模板语法结构编译为可执行的过滤器
   *
   * @param frag 模板语法结构
   */
  static Filter Compile(Frag frag);

  /**
   * 将表达式语法结构编译为可执行的过滤器
   *
   * @param expr 表达式语法结构
   */
  static Filter Compile(Expr expr);

  static Filter Compile(std::shared_ptr<TextFragment> frag);
  static Filter Compile(std::shared_ptr<CallFragment> frag);
  static Filter Compile(std::shared_ptr<EvalFragment> frag);
  static Filter Compile(std::shared_ptr<IterFragment> frag);
  static Filter Compile(std::shared_ptr<BranchFragment> frag);
  static Filter Compile(std::shared_ptr<BlockFragment> frag);
  static Filter Compile(std::shared_ptr<ExtendsFragment> frag);
  static Filter Compile(std::shared_ptr<OverwriteFragment> frag);
  static Filter Compile(std::shared_ptr<VariableExpr> expr);
  static Filter Compile(std::shared_ptr<FieldExpr> expr);
  static Filter Compile(std::shared_ptr<IndexExpr> expr);
  static Filter Compile(std::shared_ptr<IntegerExpr> expr);
  static Filter Compile(std::shared_ptr<NumberExpr> expr);
  static Filter Compile(std::shared_ptr<StringExpr> expr);
  static Filter Compile(std::shared_ptr<InvokeExpr> expr);
  static Filter Compile(std::shared_ptr<PipeExpr> expr);

  template <typename T>
  static Filter Pipe(T const& pipe);

  /**
   * 渲染模板
   *
   * @param tmpl 模板源码
   * @param model 数据模型
   * @param meta 元数据，跨模板共享可用于注入过滤器
   */
  static std::string Render(Doc tmpl, Map const& model, Map const& meta = {});

  /**
   * 渲染模板
   *
   * @param path 模板路径
   * @param model 数据模型
   * @param meta 元数据，跨模板共享可用于注入过滤器
   */
  static std::string Render(std::filesystem::path const& path, Map const& model,
                            Map const& meta = {});
};

/**
 * 支持函数式编程的模板变量
 */
struct Template::Value : public std::variant<Null, String, Integer, Number,
                                             Boolean, Array, Map, Filter> {
  using variant::variant;

  /**
   * 判断变量是否为真值
   *
   * 真值的定义为：
   * - true 为真
   * - 非空字符串为真
   * - 非零数字为真
   * - 非空数组为真
   * - 非空映射为真
   * - 过滤器为真
   */
  bool Truthy() const;

  /**
   * 尽可能将变量转换为字符串
   */
  std::string TextOf() const;

  bool IsNull() const;
  bool IsString() const;
  bool IsInteger() const;
  bool IsNumber() const;
  bool IsBoolean() const;
  bool IsArray() const;
  bool IsMap() const;
  bool IsFilter() const;

  String const& GetString() const;
  Integer const& GetInteger() const;
  Number const& GetNumber() const;
  Boolean const& GetBoolean() const;
  Array const& GetArray() const;
  Map const& GetMap() const;
  Filter const& GetFilter() const;

  nlohmann::json ToJson() const;
  static Value FromJson(nlohmann::json const& json);
};

/**
 * 过滤器上下文
 */
struct Template::Context {
  struct Frame;

  /**
   * 当前上下文已经编译过的模板缓冲
   */
  std::map<std::filesystem::path, Filter> templates;

  /**
   * 元信息跨模板共享，查找优先级低于调用栈
   *
   * 可以存储过滤器、函数、常量等
   */
  Map meta;

  /**
   * 上下文的调用栈
   *
   * 嵌套语法结构可能产生多个调用栈帧
   * 其中，调用模板时产生被称为逻辑栈底的特殊栈帧
   * 逻辑栈底的栈帧具有路径信息，查找操作不可越过逻辑栈底
   */
  std::vector<Frame> stack;

  /**
   * 沿着调用栈查找变量，若调用栈和过滤器表都没有找到，则返回空值
   */
  Value Variable(std::string const& name) const;

  /**
   * 查找当前逻辑栈底的变量表
   */
  Value Model() const;

  /**
   * 基于当前上下文解析模板绝对路径，必要时加载并编译模板
   *
   * - 若指定相对路径，则基于当前逻辑栈底路径查找模板
   * - 若指定绝对路径，则直接查找模板
   *
   * @param path 模板路径
   */
  std::filesystem::path Resolve(std::filesystem::path path);
};

/**
 * 模板引擎上下文中的调用栈帧
 */
struct Template::Context::Frame {
  /**
   * 当前栈帧上的具名变量表
   */
  Map variables{};

  /**
   * 具有路径的栈帧是逻辑栈底，查找操作不可越过逻辑栈顶
   */
  std::optional<std::filesystem::path> path{};

  /**
   * 若当前栈帧是逻辑栈底
   * transparent 可以允许查找操作从此跳跃到上一个逻辑栈底
   * 用于应对 `call` 语句直接复用当前模型的情况
   * transparent 不影响变量创建的栈帧，即，不允许污染上层调用栈
   */
  bool transparent{};
};

struct Template::Fragment {
  AST source{};
  bool trim_start{};
  bool trim_end{};

  virtual ~Fragment() = default;
};

struct Template::TextFragment : public Fragment {
  std::string text;
};

struct Template::CallFragment : public Fragment {
  std::string call;
  std::optional<Expr> model;
  std::map<std::string, Expr> these;
};

struct Template::EvalFragment : public Fragment {
  Expr eval;
};

struct Template::IterFragment : public Fragment {
  Expr iter;
  std::string value;
  std::optional<std::string> key;
  std::optional<std::string> index;
  std::vector<Frag> fragments;
  std::optional<std::string> anchor;
};

struct Template::BranchFragment : public Fragment {
  struct BranchBlock;
  using Branch = std::shared_ptr<BranchBlock>;

  std::vector<Branch> branch;
};

struct Template::BranchFragment::BranchBlock : public Fragment {
  std::vector<Frag> fragments;
  std::optional<Expr> cond;
};

struct Template::BlockFragment : public Fragment {
  std::string block;
  std::vector<Frag> fragments;
};

struct Template::ExtendsFragment : public Fragment {
  std::string extends;
  std::vector<Frag> overwrites;
};

struct Template::OverwriteFragment : public Fragment {
  std::string overwrite;
  std::vector<Frag> fragments;
};

struct Template::Expression {
  AST source{};
  virtual ~Expression() = default;
};

struct Template::VariableExpr : public Template::Expression {
  std::string variable;
  std::optional<std::string> anchor;
};

struct Template::FieldExpr : public Template::Expression {
  Expr of;
  std::string field;
};

struct Template::IndexExpr : public Template::Expression {
  Expr of;
  Expr index;
};

struct Template::IntegerExpr : public Template::Expression {
  Integer integer;
};

struct Template::NumberExpr : public Template::Expression {
  Number number;
};

struct Template::StringExpr : public Template::Expression {
  String string;
};

struct Template::InvokeExpr : public Template::Expression {
  Expr invoke;
  std::vector<Expr> params;
};

struct Template::PipeExpr : public Template::Expression {
  Expr pipe;
  Expr expr;
};

template <typename T>
inline Template::Filter Template::Pipe(T const& pipe) {
  return [pipe](Context& ctx, Array const& args) -> Value {
    return pipe(args.front().TextOf());
  };
}

}  // namespace alioth

#endif