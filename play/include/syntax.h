#ifndef __PLAY_SYNTAX_H__
#define __PLAY_SYNTAX_H__

#include "alioth/ast.h"

namespace play {

struct BranchNode: public alioth::ASTNtrmNode {
    
    ASTAttrs branchs{};
    
    
    
};
struct ControlNode: public alioth::ASTNtrmNode {
    struct BreakControl;
    struct ContinueControl;
    struct ReturnControl;
    

    
    
};
struct ControlNode::BreakControl {
    ASTAttrs action{};
    
    
};
struct ControlNode::ContinueControl {
    ASTAttrs action{};
    
    
};
struct ControlNode::ReturnControl {
    ASTAttrs action{};
    
    ASTAttrs expr{};
    
    
};


struct DeclareNode: public alioth::ASTNtrmNode {
    
    ASTAttrs init{};
    
    ASTAttrs name{};
    
    
    
};
struct ElseNode: public alioth::ASTNtrmNode {
    
    ASTAttrs stmts{};
    
    
    
};
struct ElseifNode: public alioth::ASTNtrmNode {
    
    ASTAttrs condition{};
    
    ASTAttrs stmts{};
    
    
    
};
struct ExpressionNode: public alioth::ASTNtrmNode {
    struct BinaryExpression;
    struct MonoExpression;
    struct SubExpression;
    struct ValueExpression;
    struct VarExpression;
    

    
    
};
struct ExpressionNode::BinaryExpression {
    ASTAttrs lhs{};
    
    ASTAttrs op{};
    
    ASTAttrs rhs{};
    
    
};
struct ExpressionNode::MonoExpression {
    ASTAttrs op{};
    
    ASTAttrs rhs{};
    
    
};
struct ExpressionNode::SubExpression {
    ASTAttrs expr{};
    
    
};
struct ExpressionNode::ValueExpression {
    ASTAttrs false{};
    
    ASTAttrs null{};
    
    ASTAttrs number{};
    
    ASTAttrs string{};
    
    ASTAttrs true{};
    
    
};
struct ExpressionNode::VarExpression {
    ASTAttrs name{};
    
    
};


struct FunctionNode: public alioth::ASTNtrmNode {
    
    ASTAttrs name{};
    
    ASTAttrs params{};
    
    ASTAttrs stmts{};
    
    
    
};
struct IfNode: public alioth::ASTNtrmNode {
    
    ASTAttrs condition{};
    
    ASTAttrs stmts{};
    
    
    
};
struct IterateNode: public alioth::ASTNtrmNode {
    
    ASTAttrs cond{};
    
    ASTAttrs ctrl{};
    
    ASTAttrs init{};
    
    ASTAttrs stmts{};
    
    
    
};
struct PlayNode: public alioth::ASTNtrmNode {
    
    ASTAttrs stmts{};
    
    
    
};
struct StmtNode: public alioth::ASTNtrmNode {
    
    ASTAttrs branch{};
    
    ASTAttrs control{};
    
    ASTAttrs declare{};
    
    ASTAttrs expression{};
    
    ASTAttrs function{};
    
    ASTAttrs iterate{};
    
    
    
};


}

#endif