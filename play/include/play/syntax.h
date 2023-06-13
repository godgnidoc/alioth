#ifndef __PLAY_SYNTAX_H__
#define __PLAY_SYNTAX_H__

#include "alioth/ast.h"

namespace play {

struct Play;
struct Statement;
struct Branch;
struct Control;
struct Declare;
struct Function;
struct Iterate;
struct Expression;
struct AddExpr;
struct AndExpr;
struct AssignExpr;
struct BitandExpr;
struct BitnotExpr;
struct BitorExpr;
struct BitxorExpr;
struct DivExpr;
struct MolExpr;
struct MulExpr;
struct NegExpr;
struct NotExpr;
struct OrExpr;
struct SubExpr;
struct VariableExpr;
struct NullExpr;
struct BooleanExpr;
struct NumberExpr;
struct StringExpr;

using Stmt = std::shared_ptr<Statement>;
using Expr = std::shared_ptr<Expression>;

struct Play {
  alioth::ASTRoot root;
  std::vector<Stmt> stmts;

  static Play Parse(alioth::ASTRoot root);
  static Stmt ParseStatement(alioth::AST node);
  static Stmt ParseBranch(alioth::AST node);
  static Stmt ParseControl(alioth::AST node);
  static Stmt ParseDeclare(alioth::AST node);
  static Stmt ParseFunction(alioth::AST node);
  static Stmt ParseIterate(alioth::AST node);
  static Expr ParseExpression(alioth::AST node);
  static Expr ParseAddExpr(alioth::AST node);
  static Expr ParseAndExpr(alioth::AST node);
  static Expr ParseAssignExpr(alioth::AST node);
  static Expr ParseBitandExpr(alioth::AST node);
  static Expr ParseBitnotExpr(alioth::AST node);
  static Expr ParseBitorExpr(alioth::AST node);
  static Expr ParseBitxorExpr(alioth::AST node);
  static Expr ParseDivExpr(alioth::AST node);
  static Expr ParseMolExpr(alioth::AST node);
  static Expr ParseMulExpr(alioth::AST node);
  static Expr ParseNegExpr(alioth::AST node);
  static Expr ParseNotExpr(alioth::AST node);
  static Expr ParseOrExpr(alioth::AST node);
  static Expr ParseSubExpr(alioth::AST node);
  static Expr ParseVariableExpr(alioth::AST node);
  static Expr ParseNullExpr(alioth::AST node);
  static Expr ParseBooleanExpr(alioth::AST node);
  static Expr ParseNumberExpr(alioth::AST node);
  static Expr ParseStringExpr(alioth::AST node);
};

struct Statement {
  virtual ~Statement() = default;
};
struct Branch : public Statement {
  struct Br;

  std::vector<Br> branches;
};
struct Branch::Br {
  Expr condition;
  std::vector<Stmt> stmts;
};
struct Control : public Statement {
  struct Continue;
  struct Break;
  struct Return;
};
struct Control::Continue : public Control {};
struct Control::Break : public Control {};
struct Control::Return : public Control {
  Expr result;
};
struct Declare : public Statement {
  std::string name;
  Expr init;
};
struct Function : public Statement {
  std::string name;
  std::vector<std::string> params;
  std::vector<Stmt> stmts;
};
struct Iterate : public Statement {
  Expr init;
  Expr cond;
  Expr ctrl;
  std::vector<Stmt> stmts;
};
struct Expression : public Statement {};
struct AddExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct AndExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct AssignExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct BitandExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct BitnotExpr : public Expression {
  Expr rhs;
};
struct BitorExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct BitxorExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct DivExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct MolExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct MulExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct NegExpr : public Expression {
  Expr rhs;
};
struct NotExpr : public Expression {
  Expr rhs;
};
struct OrExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct SubExpr : public Expression {
  Expr lhs;
  Expr rhs;
};
struct VariableExpr : public Expression {
  std::string name;
};
struct NullExpr : public Expression {};
struct BooleanExpr : public Expression {
  bool boolean;
};
struct NumberExpr : public Expression {
  double number;
};
struct StringExpr : public Expression {
  std::string string;
};

}  // namespace play

#endif