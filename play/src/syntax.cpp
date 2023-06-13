#include "play/syntax.h"

#include "alioth/inspect.h"

using namespace alioth;

namespace play {

Play Play::Parse(alioth::ASTRoot root) {
  Play play{};
  play.root = root;

  auto node = AttrOf(root, "play");
  for (auto func : AttrsOf(node, "stmts")) {
    play.stmts.push_back(ParseStatement(func));
  }

  return play;
}

Stmt Play::ParseStatement(alioth::AST node) {
  if (auto branch = AttrOf(node, "branch")) {
    return ParseBranch(branch);
  }
  if (auto control = AttrOf(node, "control")) {
    return ParseControl(control);
  }
  if (auto declare = AttrOf(node, "declare")) {
    return ParseDeclare(declare);
  }
  if (auto expression = AttrOf(node, "expression")) {
    return ParseExpression(expression);
  }
  if (auto function = AttrOf(node, "function")) {
    return ParseFunction(function);
  }
  if (auto iterate = AttrOf(node, "iterate")) {
    return ParseIterate(iterate);
  }

  throw std::runtime_error(
      fmt::format("Play::ParseStatement: unknown statement type: {}",
                  alioth::NameOf(node)));
}

Stmt Play::ParseBranch(alioth::AST node) {
  auto branch = std::make_shared<Branch>();
  for (auto br : AttrsOf(node, "branches")) {
    Branch::Br block;
    if (auto cond = AttrOf(br, "condition")) {
      block.condition = ParseExpression(cond);
    }
    for (auto stmt : AttrsOf(br, "stmts")) {
      block.stmts.push_back(ParseStatement(stmt));
    }
    branch->branches.push_back(block);
  }
  return branch;
}

Stmt Play::ParseControl(alioth::AST node) {
  if (AttrOf(node, "continue")) {
    return std::make_shared<Control::Continue>();
  }
  if (AttrOf(node, "break")) {
    return std::make_shared<Control::Break>();
  }
  if (AttrOf(node, "return")) {
    auto control = std::make_shared<Control::Return>();
    if (auto result = AttrOf(node, "result")) {
      control->result = ParseExpression(result);
    }
    return control;
  }
  throw std::runtime_error(fmt::format(
      "Play::ParseControl: unknown control type: {}", alioth::NameOf(node)));
}

Stmt Play::ParseDeclare(alioth::AST node) {
  auto declare = std::make_shared<Declare>();
  if (auto name = AttrOf(node, "name")) {
    declare->name = TextOf(name);
  }
  if (auto init = AttrOf(node, "init")) {
    declare->init = ParseExpression(init);
  }
  return declare;
}

Stmt Play::ParseFunction(alioth::AST node) {
  auto function = std::make_shared<Function>();
  if (auto name = AttrOf(node, "name")) {
    function->name = TextOf(name);
  }
  for (auto param : AttrsOf(node, "params")) {
    function->params.push_back(TextOf(param));
  }
  for (auto stmt : AttrsOf(node, "stmts")) {
    function->stmts.push_back(ParseStatement(stmt));
  }
  return function;
}

Stmt Play::ParseIterate(alioth::AST node) {
  auto iterate = std::make_shared<Iterate>();
  if (auto init = AttrOf(node, "init")) {
    iterate->init = ParseExpression(init);
  }
  if (auto cond = AttrOf(node, "cond")) {
    iterate->cond = ParseExpression(cond);
  }
  if (auto ctrl = AttrOf(node, "ctrl")) {
    iterate->ctrl = ParseExpression(ctrl);
  }
  for (auto stmt : AttrsOf(node, "stmts")) {
    iterate->stmts.push_back(ParseStatement(stmt));
  }
  return iterate;
}

Expr Play::ParseExpression(alioth::AST node) {
  if (AttrOf(node, "add")) {
    return ParseAddExpr(node);
  }
  if (AttrOf(node, "and")) {
    return ParseAndExpr(node);
  }
  if (AttrOf(node, "assign")) {
    return ParseAssignExpr(node);
  }
  if (AttrOf(node, "bitand")) {
    return ParseBitandExpr(node);
  }
  if (AttrOf(node, "bitnot")) {
    return ParseBitnotExpr(node);
  }
  if (AttrOf(node, "bitor")) {
    return ParseBitorExpr(node);
  }
  if (AttrOf(node, "bitxor")) {
    return ParseBitxorExpr(node);
  }
  if (AttrOf(node, "div")) {
    return ParseDivExpr(node);
  }
  if (AttrOf(node, "mol")) {
    return ParseMolExpr(node);
  }
  if (AttrOf(node, "mul")) {
    return ParseMulExpr(node);
  }
  if (AttrOf(node, "neg")) {
    return ParseNegExpr(node);
  }
  if (AttrOf(node, "not")) {
    return ParseNotExpr(node);
  }
  if (AttrOf(node, "or")) {
    return ParseOrExpr(node);
  }
  if (AttrOf(node, "sub")) {
    return ParseSubExpr(node);
  }
  if (AttrOf(node, "variable")) {
    return ParseVariableExpr(node);
  }
  if (AttrOf(node, "null")) {
    return ParseNullExpr(node);
  }
  if (AttrOf(node, "boolean")) {
    return ParseBooleanExpr(node);
  }
  if (AttrOf(node, "number")) {
    return ParseNumberExpr(node);
  }
  if (AttrOf(node, "string")) {
    return ParseStringExpr(node);
  }

  throw std::runtime_error(
      fmt::format("Play::ParseExpression: unknown expression type: {}",
                  alioth::NameOf(node)));
}

Expr Play::ParseAddExpr(alioth::AST node) {
  auto expr = std::make_shared<AddExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseAndExpr(alioth::AST node) {
  auto expr = std::make_shared<AndExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseAssignExpr(alioth::AST node) {
  auto expr = std::make_shared<AssignExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseBitandExpr(alioth::AST node) {
  auto expr = std::make_shared<BitandExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseBitnotExpr(alioth::AST node) {
  auto expr = std::make_shared<BitnotExpr>();
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseBitorExpr(alioth::AST node) {
  auto expr = std::make_shared<BitorExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseBitxorExpr(alioth::AST node) {
  auto expr = std::make_shared<BitxorExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseDivExpr(alioth::AST node) {
  auto expr = std::make_shared<DivExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseMolExpr(alioth::AST node) {
  auto expr = std::make_shared<MolExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseMulExpr(alioth::AST node) {
  auto expr = std::make_shared<MulExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseNegExpr(alioth::AST node) {
  auto expr = std::make_shared<NegExpr>();
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseNotExpr(alioth::AST node) {
  auto expr = std::make_shared<NotExpr>();
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseOrExpr(alioth::AST node) {
  auto expr = std::make_shared<OrExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseSubExpr(alioth::AST node) {
  auto expr = std::make_shared<SubExpr>();
  expr->lhs = ParseExpression(AttrOf(node, "lhs"));
  expr->rhs = ParseExpression(AttrOf(node, "rhs"));
  return expr;
}

Expr Play::ParseVariableExpr(alioth::AST node) {
  auto expr = std::make_shared<VariableExpr>();
  if (auto name = AttrOf(node, "name")) {
    expr->name = TextOf(name);
  }
  return expr;
}

Expr Play::ParseNullExpr(alioth::AST node) {
  return std::make_shared<NullExpr>();
}

Expr Play::ParseBooleanExpr(alioth::AST node) {
  auto expr = std::make_shared<BooleanExpr>();
  if (auto boolean = AttrOf(node, "boolean")) {
    expr->boolean = TextOf(boolean) == "true";
  }
  return expr;
}

Expr Play::ParseNumberExpr(alioth::AST node) {
  auto expr = std::make_shared<NumberExpr>();
  if (auto number = AttrOf(node, "number")) {
    expr->number = std::stod(TextOf(number));
  }
  return expr;
}

Expr Play::ParseStringExpr(alioth::AST node) {
  auto expr = std::make_shared<StringExpr>();
  if (auto string = AttrOf(node, "string")) {
    expr->string = TextOf(string);
  }
  return expr;
}
}  // namespace play