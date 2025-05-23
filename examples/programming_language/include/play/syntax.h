#ifndef __PLAY_SYNTAX_H__
#define __PLAY_SYNTAX_H__

#include "alioth/ast.h"
#include "alioth/document.h"

namespace play {

constexpr alioth::SymbolID T_LP = 1;
constexpr alioth::SymbolID T_RP = 2;
constexpr alioth::SymbolID T_LI = 3;
constexpr alioth::SymbolID T_RI = 4;
constexpr alioth::SymbolID T_LS = 5;
constexpr alioth::SymbolID T_RS = 6;
constexpr alioth::SymbolID T_COMMA = 7;
constexpr alioth::SymbolID T_SEMI = 8;
constexpr alioth::SymbolID T_DOT = 9;
constexpr alioth::SymbolID T_ADD = 10;
constexpr alioth::SymbolID T_SUB = 11;
constexpr alioth::SymbolID T_MUL = 12;
constexpr alioth::SymbolID T_DIV = 13;
constexpr alioth::SymbolID T_MOL = 14;
constexpr alioth::SymbolID T_LT = 15;
constexpr alioth::SymbolID T_GT = 16;
constexpr alioth::SymbolID T_LE = 17;
constexpr alioth::SymbolID T_GE = 18;
constexpr alioth::SymbolID T_EQ = 19;
constexpr alioth::SymbolID T_NE = 20;
constexpr alioth::SymbolID T_AND = 21;
constexpr alioth::SymbolID T_OR = 22;
constexpr alioth::SymbolID T_NOT = 23;
constexpr alioth::SymbolID T_BITAND = 24;
constexpr alioth::SymbolID T_BITOR = 25;
constexpr alioth::SymbolID T_BITNOT = 26;
constexpr alioth::SymbolID T_BITXOR = 27;
constexpr alioth::SymbolID T_ASSIGN = 28;
constexpr alioth::SymbolID T_LET = 29;
constexpr alioth::SymbolID T_FN = 30;
constexpr alioth::SymbolID T_IF = 31;
constexpr alioth::SymbolID T_ELSE = 32;
constexpr alioth::SymbolID T_FOR = 33;
constexpr alioth::SymbolID T_CONTINUE = 34;
constexpr alioth::SymbolID T_BREAK = 35;
constexpr alioth::SymbolID T_RETURN = 36;
constexpr alioth::SymbolID T_NULL = 37;
constexpr alioth::SymbolID T_TRUE = 38;
constexpr alioth::SymbolID T_FALSE = 39;
constexpr alioth::SymbolID T_STRING = 40;
constexpr alioth::SymbolID T_NUMBER = 41;
constexpr alioth::SymbolID T_ID = 42;
constexpr alioth::SymbolID T_SPACE = 43;


struct BranchNode: public alioth::ASTNtrmNode {
  alioth::ASTAttrs branchs{}; // if else elseif
  
  
  BranchNode() = default;
  BranchNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  BranchNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~BranchNode() = default;
};

struct ControlNode: public alioth::ASTNtrmNode {
  struct Break;
  struct Continue;
  struct Return;
  
  
  
  
  ControlNode() = default;
  ControlNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  ControlNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~ControlNode() = default;
};
struct ControlNode::Break: public ControlNode {
  using ControlNode::ControlNode;

  
};
struct ControlNode::Continue: public ControlNode {
  using ControlNode::ControlNode;

  
};
struct ControlNode::Return: public ControlNode {
  using ControlNode::ControlNode;

  alioth::ASTAttr expr{}; // expression
};


struct DeclareNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr init{}; // expression 
  alioth::ASTAttr name{}; // T_ID
  
  
  DeclareNode() = default;
  DeclareNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  DeclareNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~DeclareNode() = default;
};

struct ElseNode: public alioth::ASTNtrmNode {
  alioth::ASTAttrs stmts{}; // stmt
  
  
  ElseNode() = default;
  ElseNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  ElseNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~ElseNode() = default;
};

struct ElseifNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr condition{}; // expression 
  alioth::ASTAttrs stmts{}; // stmt
  
  
  ElseifNode() = default;
  ElseifNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  ElseifNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~ElseifNode() = default;
};

struct ExpressionNode: public alioth::ASTNtrmNode {
  struct Binary;
  struct Mono;
  struct Sub;
  struct Value;
  struct Var;
  
  
  
  
  ExpressionNode() = default;
  ExpressionNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  ExpressionNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~ExpressionNode() = default;
};
struct ExpressionNode::Binary: public ExpressionNode {
  using ExpressionNode::ExpressionNode;

  alioth::ASTAttr lhs{}; // expression 
  alioth::ASTAttr op{}; // T_ADD T_SUB T_MUL T_DIV T_MOL T_AND T_OR T_BITAND T_BITOR T_BITXOR T_ASSIGN 
  alioth::ASTAttr rhs{}; // expression
};
struct ExpressionNode::Mono: public ExpressionNode {
  using ExpressionNode::ExpressionNode;

  alioth::ASTAttr op{}; // T_SUB T_NOT T_BITNOT 
  alioth::ASTAttr rhs{}; // expression
};
struct ExpressionNode::Sub: public ExpressionNode {
  using ExpressionNode::ExpressionNode;

  alioth::ASTAttr expr{}; // expression
};
struct ExpressionNode::Value: public ExpressionNode {
  using ExpressionNode::ExpressionNode;

  alioth::ASTAttr boolean{}; // T_TRUE T_FALSE 
  alioth::ASTAttr null{}; // T_NULL 
  alioth::ASTAttr number{}; // T_NUMBER 
  alioth::ASTAttr string{}; // T_STRING
};
struct ExpressionNode::Var: public ExpressionNode {
  using ExpressionNode::ExpressionNode;

  alioth::ASTAttr name{}; // T_ID
};


struct FunctionNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr name{}; // T_ID 
  alioth::ASTAttrs params{}; // T_ID 
  alioth::ASTAttrs stmts{}; // stmt
  
  
  FunctionNode() = default;
  FunctionNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  FunctionNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~FunctionNode() = default;
};

struct IfNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr condition{}; // expression 
  alioth::ASTAttrs stmts{}; // stmt
  
  
  IfNode() = default;
  IfNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  IfNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~IfNode() = default;
};

struct IterateNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr cond{}; // expression 
  alioth::ASTAttr ctrl{}; // expression 
  alioth::ASTAttr init{}; // declare 
  alioth::ASTAttrs stmts{}; // stmt
  
  
  IterateNode() = default;
  IterateNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  IterateNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~IterateNode() = default;
};

struct PlayNode: public alioth::ASTNtrmNode {
  alioth::ASTAttrs stmts{}; // stmt
  alioth::ASTRoot root;
  
  PlayNode() = default;
  PlayNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  PlayNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~PlayNode() = default;
};

struct StmtNode: public alioth::ASTNtrmNode {
  alioth::ASTAttr branch{}; // branch 
  alioth::ASTAttr control{}; // control 
  alioth::ASTAttr declare{}; // declare 
  alioth::ASTAttr expression{}; // expression 
  alioth::ASTAttr function{}; // function 
  alioth::ASTAttr iterate{}; // iterate
  
  
  StmtNode() = default;
  StmtNode(ASTNtrmNode const& node): ASTNtrmNode{node} {}
  StmtNode(ASTNtrmNode && node): ASTNtrmNode{std::move(node)} {}
  ~StmtNode() = default;
};



std::shared_ptr<PlayNode> ParsePlay(alioth::Doc source);

}

#endif