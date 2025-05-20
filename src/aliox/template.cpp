#include "aliox/template.h"

#include <alioth/strings.h>

#include "alioth/alioth.h"
#include "alioth/parser.h"
#include "aliox/grammar.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"

namespace alioth {

Tmpl Template::Load(Doc source) {
  static auto syntax = [] {
    auto home = AliothHome();

    auto path = home / "grammar" / "template.json";
    if (std::filesystem::exists(path)) {
      auto sdoc = Document::Read(path);
      auto json = nlohmann::json::parse(sdoc->content);
      return Syntactic::Load(json);
    }

    path = home / "grammar" / "template.grammar";
    auto gdoc = Document::Read(path);
    return Grammar::Compile(gdoc);
  }();

  auto parser = Parser(syntax, source);
  auto root = parser.Parse();

  auto tnode = root->Attr("template");
  auto tmpl = std::make_shared<Template>();
  for (auto fnode : tnode->Attrs("fragments")) {
    tmpl->fragments.push_back(LoadFrag(fnode));
  }

  return tmpl;
}

Template::Frag Template::LoadFrag(AST node) {
  Frag f;
  if (auto text = node->Attr("text"); text) {
    auto frag = std::make_shared<TextFragment>();
    frag->text = text->Text();
    f = frag;
  } else if (auto call = node->Attr("call"); call) {
    auto frag = std::make_shared<CallFragment>();
    frag->call = nlohmann::json::parse(call->Text()).get<String>();
    auto model = node->Attr("model");
    if (model) frag->model = LoadExpr(model);
    for (auto that : node->Attrs("these")) {
      auto name = *that->TextOf("key");
      auto value = that->Attr("value");
      frag->these[name] = LoadExpr(value);
    }
    f = frag;
  } else if (auto eval = node->Attr("eval"); eval) {
    auto frag = std::make_shared<EvalFragment>();
    frag->eval = LoadExpr(eval);
    f = frag;
  } else if (auto iter = node->Attr("iter"); iter) {
    auto frag = std::make_shared<IterFragment>();
    frag->iter = LoadExpr(iter);
    frag->value = *node->TextOf("value");
    if (auto key = node->Attr("key"); key) {
      frag->key = key->Text();
    }
    if (auto index = node->Attr("index"); index) {
      frag->index = index->Text();
    }
    for (auto fnode : node->Attrs("fragments")) {
      frag->fragments.push_back(LoadFrag(fnode));
    }
    if (auto anchor = node->Attr("anchor"); anchor) {
      frag->anchor = anchor->Text();
    }
    f = frag;
  } else if (auto branch = node->Attrs("branch"); !branch.empty()) {
    auto frag = std::make_shared<BranchFragment>();
    if (node->Attr("trim_start")) frag->trim_start = true;
    if (node->Attr("trim_end")) frag->trim_end = true;
    for (auto br : branch) {
      auto block = std::make_shared<BranchFragment::BranchBlock>();
      if (br->Attr("trim_start")) block->trim_start = true;
      if (br->Attr("trim_end")) block->trim_end = true;
      if (auto cond = br->Attr("cond"); cond) block->cond = LoadExpr(cond);
      for (auto fnode : br->Attrs("fragments")) {
        block->fragments.push_back(LoadFrag(fnode));
      }
      frag->branch.push_back(block);
    }
    auto next = frag->branch.begin();
    for (auto br : frag->branch) {
      ++next;
      if (next == frag->branch.end())
        br->trim_end = frag->trim_end;
      else
        br->trim_end = (*next)->trim_end;
    }
    f = frag;
  } else if (auto block = node->Attr("block"); block) {
    auto frag = std::make_shared<BlockFragment>();
    frag->block = block->Text();
    for (auto fnode : node->Attrs("fragments")) {
      frag->fragments.push_back(LoadFrag(fnode));
    }
    f = frag;
  } else if (auto extends = node->Attr("extends"); extends) {
    auto frag = std::make_shared<ExtendsFragment>();
    frag->extends = nlohmann::json::parse(extends->Text()).get<String>();
    for (auto overwrite : node->Attrs("overwrites")) {
      frag->overwrites.push_back(LoadFrag(overwrite));
    }
    f = frag;
  } else if (auto overwrite = node->Attr("overwrite"); overwrite) {
    auto frag = std::make_shared<OverwriteFragment>();
    frag->overwrite = overwrite->Text();
    for (auto fnode : node->Attrs("fragments")) {
      frag->fragments.push_back(LoadFrag(fnode));
    }
    f = frag;
  }

  f->source = node;
  if (node->Attr("trim_start")) f->trim_start = true;
  if (node->Attr("trim_end")) f->trim_end = true;
  return f;
}

Template::Expr Template::LoadExpr(AST node) {
  Expr e;

  if (auto variable = node->Attr("variable"); variable) {
    auto expr = std::make_shared<VariableExpr>();
    expr->variable = variable->Text();
    if (auto anchor = node->Attr("anchor"); anchor) {
      expr->anchor = anchor->Text();
    }
    e = expr;
  } else if (auto field = node->Attr("field"); field) {
    auto expr = std::make_shared<FieldExpr>();
    expr->of = LoadExpr(node->Attr("of"));
    expr->field = field->Text();
    e = expr;
  } else if (auto index = node->Attr("index"); index) {
    auto expr = std::make_shared<IndexExpr>();
    expr->of = LoadExpr(node->Attr("of"));
    expr->index = LoadExpr(node->Attr("index"));
    e = expr;
  } else if (auto number = node->Attr("number"); number) {
    auto text = number->Text();
    if (text.find('.') == std::string::npos &&
        text.find('e') == std::string::npos &&
        text.find('E') == std::string::npos) {
      auto expr = std::make_shared<IntegerExpr>();
      expr->integer = nlohmann::json::parse(text).get<Integer>();
      e = expr;
    } else {
      auto expr = std::make_shared<NumberExpr>();
      expr->number = nlohmann::json::parse(text).get<Number>();
      e = expr;
    }
  } else if (auto string = node->Attr("string"); string) {
    auto expr = std::make_shared<StringExpr>();
    expr->string = nlohmann::json::parse(string->Text()).get<String>();
    e = expr;
  } else if (auto invoke = node->Attr("invoke"); invoke) {
    auto expr = std::make_shared<InvokeExpr>();
    expr->invoke = LoadExpr(node->Attr("invoke"));
    for (auto param : node->Attrs("params")) {
      expr->params.push_back(LoadExpr(param));
    }
    e = expr;
  } else if (auto pipe = node->Attr("pipe"); pipe) {
    auto expr = std::make_shared<PipeExpr>();
    expr->pipe = LoadExpr(node->Attr("pipe"));
    expr->expr = LoadExpr(node->Attr("expr"));
    e = expr;
  } else {
    throw std::runtime_error("invalid expression");
  }

  e->source = node;
  return e;
}

Template::Filter Template::Compile(Tmpl tmpl) {
  std::vector<Filter> filters;
  for (auto frag : tmpl->fragments) {
    filters.push_back(Compile(frag));
  }

  return [filters](Context& ctx, Array const& args) {
    std::string result;
    for (auto filter : filters) {
      result += filter(ctx, args).TextOf();
    }
    return result;
  };
}

Template::Filter Template::Compile(Frag frag) {
  Filter f;
  if (auto text = std::dynamic_pointer_cast<TextFragment>(frag)) {
    f = Compile(text);
  } else if (auto call = std::dynamic_pointer_cast<CallFragment>(frag)) {
    f = Compile(call);
  } else if (auto eval = std::dynamic_pointer_cast<EvalFragment>(frag)) {
    f = Compile(eval);
  } else if (auto iter = std::dynamic_pointer_cast<IterFragment>(frag)) {
    f = Compile(iter);
  } else if (auto branch = std::dynamic_pointer_cast<BranchFragment>(frag)) {
    f = Compile(branch);
  } else if (auto block = std::dynamic_pointer_cast<BlockFragment>(frag)) {
    f = Compile(block);
  } else if (auto extends = std::dynamic_pointer_cast<ExtendsFragment>(frag)) {
    f = Compile(extends);
  } else if (auto ovwrt = std::dynamic_pointer_cast<OverwriteFragment>(frag)) {
    f = Compile(ovwrt);
  } else {
    throw std::runtime_error("invalid fragment");
  }

  return [f, frag](Context& ctx, Array const& args) {
    try {
      return f(ctx, args);
    } catch (...) {
      auto range = frag->source->Range();
      fmt::println(stderr, "error: failed to render fragment at {}:{}:{}",
                   frag->source->doc->path.value_or("<unknown-path>").string(),
                   range.start.line, range.start.column);
      throw;
    }
  };
}

Template::Filter Template::Compile(Expr expr) {
  Filter f;

  if (auto variable = std::dynamic_pointer_cast<VariableExpr>(expr)) {
    f = Compile(variable);
  } else if (auto field = std::dynamic_pointer_cast<FieldExpr>(expr)) {
    f = Compile(field);
  } else if (auto index = std::dynamic_pointer_cast<IndexExpr>(expr)) {
    f = Compile(index);
  } else if (auto integer = std::dynamic_pointer_cast<IntegerExpr>(expr)) {
    f = Compile(integer);
  } else if (auto number = std::dynamic_pointer_cast<NumberExpr>(expr)) {
    f = Compile(number);
  } else if (auto string = std::dynamic_pointer_cast<StringExpr>(expr)) {
    f = Compile(string);
  } else if (auto invoke = std::dynamic_pointer_cast<InvokeExpr>(expr)) {
    f = Compile(invoke);
  } else if (auto pipe = std::dynamic_pointer_cast<PipeExpr>(expr)) {
    f = Compile(pipe);
  } else {
    throw std::runtime_error("invalid expression");
  }

  return [f, expr](Context& ctx, Array const& args) {
    try {
      return f(ctx, args);
    } catch (...) {
      auto range = expr->source->Range();
      fmt::println(stderr, "error: failed to evaluate expression at {}:{}:{}",
                   expr->source->doc->path.value_or("<unknown-path>").string(),
                   range.start.line, range.start.column);
      throw;
    }
  };
}

Template::Filter Template::Compile(std::shared_ptr<TextFragment> frag) {
  return [=](Context&, Array const&) { return Value(frag->text); };
}

Template::Filter Template::Compile(std::shared_ptr<CallFragment> frag) {
  Filter expr;
  if (frag->model) expr = Compile(*frag->model);
  std::map<std::string, Filter> these;
  for (auto const& [name, value] : frag->these) {
    these[name] = Compile(value);
  }
  return [=](Context& ctx, Array const&) {
    auto path = ctx.Resolve(frag->call);
    if (expr) {
      auto value = expr(ctx, {});
      auto model = value.GetMap();
      ctx.stack.push_back(
          {.variables = model, .path = path, .transparent = false});
    } else if (!these.empty()) {
      Map model;
      for (auto const& [name, filter] : these) {
        model[name] = filter(ctx, {});
      }
      ctx.stack.push_back(
          {.variables = model, .path = path, .transparent = false});
    } else {
      ctx.stack.push_back({.variables = {}, .path = path, .transparent = true});
    }
    auto result = ctx.templates.at(path)(ctx, {}).TextOf();
    ctx.stack.pop_back();
    return Strings::Trim(result, frag->trim_start, frag->trim_end);
  };
}

Template::Filter Template::Compile(std::shared_ptr<EvalFragment> frag) {
  auto expr = Compile(frag->eval);
  return [=](Context& ctx, Array const&) {
    auto result = expr(ctx, {}).TextOf();
    return Value{Strings::Trim(result, frag->trim_start, frag->trim_end)};
  };
}

Template::Filter Template::Compile(std::shared_ptr<IterFragment> frag) {
  auto expr = Compile(frag->iter);
  std::vector<Filter> fragments;
  for (auto f : frag->fragments) fragments.push_back(Compile(f));

  std::function<void(Context&, size_t, size_t)> anchor;
  if (frag->anchor) {
    anchor = [n = *frag->anchor](Context& ctx, size_t offset, size_t total) {
      auto& frame = ctx.stack.back();
      frame.variables["first@" + n] = (offset == 0);
      frame.variables["last@" + n] = (offset == total - 1);
      frame.variables["nonfirst@" + n] = (offset != 0);
      frame.variables["nonlast@" + n] = (offset != total - 1);
    };
  }
  return [=](Context& ctx, Array const&) {
    auto range = expr(ctx, {});
    ctx.stack.push_back({});

    auto result = std::string{};
    if (range.IsArray()) {
      auto total = range.GetArray().size();
      auto offset = 0;
      for (auto const& value : range.GetArray()) {
        auto& frame = ctx.stack.back();
        frame.variables[frag->value] = value;
        if (frag->key) frame.variables[*frag->key] = offset;
        if (frag->index) frame.variables[*frag->index] = offset;
        if (anchor) anchor(ctx, offset, total);
        std::string part{};
        for (auto frag : fragments) part += frag(ctx, {}).TextOf();
        result += Strings::Trim(part, frag->trim_start, frag->trim_end);
        ++offset;
      }
    } else if (range.IsMap()) {
      auto total = range.GetMap().size();
      auto offset = 0;
      for (auto const& [key, value] : range.GetMap()) {
        auto& frame = ctx.stack.back();
        frame.variables[frag->value] = value;
        if (frag->key) frame.variables[*frag->key] = key;
        if (frag->index) frame.variables[*frag->index] = offset;
        if (anchor) anchor(ctx, offset, total);
        std::string part{};
        for (auto frag : fragments) part += frag(ctx, {}).TextOf();
        result += Strings::Trim(part, frag->trim_start, frag->trim_end);
        ++offset;
      }
    } else {
      throw std::runtime_error("invalid range");
    }

    ctx.stack.pop_back();
    return result;
  };
}

Template::Filter Template::Compile(std::shared_ptr<BranchFragment> frag) {
  std::vector<Filter> conds;
  std::vector<Filter> blocks;
  for (auto const& br : frag->branch) {
    if (br->cond) conds.push_back(Compile(*br->cond));
    std::vector<Filter> fragments;
    for (auto f : br->fragments) fragments.push_back(Compile(f));
    blocks.push_back([=](Context& ctx, Array const&) {
      auto result = std::string{};
      for (auto frag : fragments) result += frag(ctx, {}).TextOf();
      return Strings::Trim(result, br->trim_start, br->trim_end);
    });
  }

  return [=](Context& ctx, Array const&) -> Value {
    size_t i;
    for (i = 0; i < conds.size(); ++i) {
      auto cond = conds[i];
      if (cond(ctx, {}).Truthy()) break;
    }
    if (i >= blocks.size()) return "";

    auto block = blocks[i];
    auto result = block(ctx, {}).TextOf();
    auto br = frag->branch.at(i);
    return Strings::Trim(result, br->trim_start, br->trim_end);
  };
}

Template::Filter Template::Compile(std::shared_ptr<BlockFragment> frag) {
  auto id = "#" + frag->block;
  std::vector<Filter> fragments;
  for (auto f : frag->fragments) fragments.push_back(Compile(f));
  return [=](Context& ctx, Array const&) -> Value {
    auto overwrite = ctx.Variable(id);
    if (!overwrite.IsNull()) return overwrite;

    auto result = std::string{};
    for (auto frag : fragments) result += frag(ctx, {}).TextOf();
    return Strings::Trim(result, frag->trim_start, frag->trim_end);
  };
}

Template::Filter Template::Compile(std::shared_ptr<ExtendsFragment> frag) {
  auto path = frag->extends;
  std::map<std::string, Filter> overwrites;
  for (auto f : frag->overwrites) {
    auto ovwrt = std::dynamic_pointer_cast<OverwriteFragment>(f);
    auto id = "#" + ovwrt->overwrite;
    overwrites.emplace(id, Compile(ovwrt));
  }
  return [=](Context& ctx, Array const&) {
    for (auto const& [id, filter] : overwrites) {
      if (!ctx.Variable(id).IsNull()) continue;
      ctx.stack.back().variables[id] = filter(ctx, {});
    }
    auto path = ctx.Resolve(frag->extends);
    ctx.stack.back().path = path;
    return ctx.templates.at(path)(ctx, {});
  };
}

Template::Filter Template::Compile(std::shared_ptr<OverwriteFragment> frag) {
  std::vector<Filter> fragments;
  for (auto f : frag->fragments) fragments.push_back(Compile(f));
  return [=](Context& ctx, Array const&) {
    auto result = std::string{};
    for (auto frag : fragments) result += frag(ctx, {}).TextOf();
    return Strings::Trim(result, frag->trim_start, frag->trim_end);
  };
}

Template::Filter Template::Compile(std::shared_ptr<VariableExpr> expr) {
  auto name = expr->variable;
  if (expr->anchor) {
    name = name + "@" + *expr->anchor;
  }
  return [=](Context& ctx, Array const&) { return ctx.Variable(name); };
}

Template::Filter Template::Compile(std::shared_ptr<FieldExpr> expr) {
  auto of = Compile(expr->of);
  auto field = expr->field;
  return [=](Context& ctx, Array const&) {
    auto value = of(ctx, {});
    try {
      if (value.IsMap()) {
        return value.GetMap().at(field);
      } else if (value.IsArray()) {
        return value.GetArray().at(std::stoi(field));
      }
    } catch (std::out_of_range const&) {
      return Value{nullptr};
    }
    throw std::runtime_error("invalid field");
  };
}

Template::Filter Template::Compile(std::shared_ptr<IndexExpr> expr) {
  auto of = Compile(expr->of);
  auto index = Compile(expr->index);
  return [=](Context& ctx, Array const&) {
    auto value = of(ctx, {});
    auto idx = index(ctx, {});
    try {
      if (value.IsMap()) {
        return value.GetMap().at(idx.TextOf());
      } else if (value.IsArray()) {
        return value.GetArray().at(std::stoi(idx.TextOf()));
      }
    } catch (std::out_of_range const&) {
      return Value{nullptr};
    }
    throw std::runtime_error("invalid index");
  };
}

Template::Filter Template::Compile(std::shared_ptr<IntegerExpr> expr) {
  return [=](Context&, Array const&) { return Value(expr->integer); };
}

Template::Filter Template::Compile(std::shared_ptr<NumberExpr> expr) {
  return [=](Context&, Array const&) { return Value(expr->number); };
}

Template::Filter Template::Compile(std::shared_ptr<StringExpr> expr) {
  return [=](Context&, Array const&) { return Value(expr->string); };
}

Template::Filter Template::Compile(std::shared_ptr<InvokeExpr> expr) {
  auto invoke = Compile(expr->invoke);
  std::vector<Filter> params;
  for (auto param : expr->params) {
    params.push_back(Compile(param));
  }
  return [=](Context& ctx, Array const&) {
    auto func = invoke(ctx, {});
    auto args = Array{};
    for (auto param : params) {
      args.push_back(param(ctx, {}));
    }
    return func.GetFilter()(ctx, args);
  };
}

Template::Filter Template::Compile(std::shared_ptr<PipeExpr> expr) {
  auto rhs = Compile(expr->pipe);
  auto lhs = Compile(expr->expr);
  return [=](Context& ctx, Array const&) {
    auto value = lhs(ctx, {});
    auto pipe = rhs(ctx, {});
    return pipe.GetFilter()(ctx, {value});
  };
}

std::string Template::Render(Doc tmpl, Map const& model, Map const& meta) {
  Context ctx{};
  auto filter = Compile(Template::Load(tmpl));
  if (tmpl->path) {
    auto path = std::filesystem::path{*tmpl->path};
    ctx.templates.emplace(path, filter);
  }
  ctx.meta = meta;
  ctx.stack.push_back(
      {.variables = model, .path = tmpl->path, .transparent = false});
  return filter(ctx, {}).TextOf();
}

std::string Template::Render(std::filesystem::path const& path,
                             Map const& model, Map const& meta) {
  auto tdoc = Document::Read(path);
  return Render(tdoc, model, meta);
}

bool Template::Value::Truthy() const {
  return std::visit(
      [](auto const& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Null>) {
          return false;
        } else if constexpr (std::is_same_v<T, Boolean>) {
          return value;
        } else if constexpr (std::is_same_v<T, Number>) {
          return value != 0;
        } else if constexpr (std::is_same_v<T, String>) {
          return !value.empty();
        } else if constexpr (std::is_same_v<T, Array>) {
          return !value.empty();
        } else if constexpr (std::is_same_v<T, Map>) {
          return !value.empty();
        } else if constexpr (std::is_same_v<T, Filter>) {
          return true;
        }
        return false;
      },
      *this);
}

std::string Template::Value::TextOf() const {
  return std::visit(
      [](auto const& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Null>) {
          return "";
        } else if constexpr (std::is_same_v<T, Boolean>) {
          return value ? "true" : "false";
        } else if constexpr (std::is_same_v<T, Integer>) {
          return std::to_string(value);
        } else if constexpr (std::is_same_v<T, Number>) {
          return std::to_string(value);
        } else if constexpr (std::is_same_v<T, String>) {
          return value;
        } else if constexpr (std::is_same_v<T, Array>) {
          std::vector<std::string> elements;
          for (auto const& ele : value) elements.push_back(ele.TextOf());
          return fmt::format("[{}]", fmt::join(elements, ", "));
        } else if constexpr (std::is_same_v<T, Map>) {
          std::vector<std::string> fields;
          for (auto const& [k, v] : value)
            fields.push_back(fmt::format("{}: {}", k, v.TextOf()));
          return fmt::format("{{{}}}", fmt::join(fields, ", "));
        } else if constexpr (std::is_same_v<T, Filter>) {
          return "<filter>";
        }
        return std::string{};
      },
      *this);
}

bool Template::Value::IsNull() const {
  return std::holds_alternative<Null>(*this);
}
bool Template::Value::IsString() const {
  return std::holds_alternative<String>(*this);
}
bool Template::Value::IsInteger() const {
  return std::holds_alternative<Integer>(*this);
}
bool Template::Value::IsNumber() const {
  return std::holds_alternative<Number>(*this);
}
bool Template::Value::IsBoolean() const {
  return std::holds_alternative<Boolean>(*this);
}
bool Template::Value::IsArray() const {
  return std::holds_alternative<Array>(*this);
}
bool Template::Value::IsMap() const {
  return std::holds_alternative<Map>(*this);
}
bool Template::Value::IsFilter() const {
  return std::holds_alternative<Filter>(*this);
}

Template::String const& Template::Value::GetString() const {
  return std::get<String>(*this);
}
Template::Integer const& Template::Value::GetInteger() const {
  return std::get<Integer>(*this);
}
Template::Number const& Template::Value::GetNumber() const {
  return std::get<Number>(*this);
}
Template::Boolean const& Template::Value::GetBoolean() const {
  return std::get<Boolean>(*this);
}
Template::Array const& Template::Value::GetArray() const {
  return std::get<Array>(*this);
}
Template::Map const& Template::Value::GetMap() const {
  return std::get<Map>(*this);
}
Template::Filter const& Template::Value::GetFilter() const {
  return std::get<Filter>(*this);
}

nlohmann::json Template::Value::ToJson() const {
  if (IsNull()) return nullptr;
  if (IsString()) return GetString();
  if (IsInteger()) return GetInteger();
  if (IsNumber()) return GetNumber();
  if (IsBoolean()) return GetBoolean();
  if (IsFilter()) return "<filter>";
  if (IsArray()) {
    auto json = nlohmann::json::array();
    for (auto const& item : GetArray()) {
      json.push_back(item.ToJson());
    }
    return json;
  }
  if (IsMap()) {
    auto json = nlohmann::json::object();
    for (auto const& [key, value] : GetMap()) {
      json[key] = value.ToJson();
    }
    return json;
  }

  throw std::runtime_error("unreachable branch");
}

Template::Value Template::Value::FromJson(nlohmann::json const& json) {
  if (json.is_null()) return nullptr;
  if (json.is_boolean()) return json.get<Boolean>();
  if (json.is_number_integer()) return json.get<Integer>();
  if (json.is_number_float()) return json.get<Number>();
  if (json.is_string()) return json.get<String>();
  if (json.is_array()) {
    auto array = Array{};
    for (auto const& ele : json) array.push_back(FromJson(ele));
    return array;
  }
  if (json.is_object()) {
    auto map = Map{};
    for (auto const& [k, v] : json.items()) map[k] = FromJson(v);
    return map;
  }
  throw std::runtime_error("invalid JSON");
}

Template::Value Template::Context::Variable(std::string const& name) const {
  auto jumping = false;
  for (auto frame = stack.rbegin(); frame != stack.rend(); ++frame) {
    if (jumping && (!frame->path || frame->transparent)) continue;

    auto it = frame->variables.find(name);
    if (it != frame->variables.end()) return it->second;

    if (jumping) break;
    if (!frame->path) continue;
    if (!frame->transparent) break;
    jumping = true;
  }

  auto it = meta.find(name);
  if (it != meta.end()) return it->second;

  return nullptr;
}

Template::Value Template::Context::Model() const {
  for (auto frame = stack.rbegin(); frame != stack.rend(); ++frame) {
    if (!frame->path) continue;
    if (frame->transparent) continue;

    return frame->variables;
  }

  return nullptr;
}

std::filesystem::path Template::Context::Resolve(std::filesystem::path path) {
  if (!path.is_absolute()) {
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
      if (!it->path) continue;

      path = it->path->parent_path() / path;
      break;
    }
  }
  path = std::filesystem::canonical(path);

  auto it = templates.find(path);
  if (it != templates.end()) return path;

  auto source = Document::Read(path);
  auto tmpl = Template::Load(source);
  auto filter = Template::Compile(tmpl);
  templates[path] = filter;
  return path;
}

}  // namespace alioth