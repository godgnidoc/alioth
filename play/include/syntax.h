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
    

    ASTAttr action{};
    
    
    
};
struct ControlNode::BreakControl {
    ASTAttr action{};
    
    
};
struct ControlNode::ContinueControl {
    ASTAttr action{};
    
    
};
struct ControlNode::ReturnControl {
    ASTAttr action{};
    
    ASTAttr expr{};
    
    
};


struct DeclareNode: public alioth::ASTNtrmNode {
    
    ASTAttr init{};
    
    ASTAttr name{};
    
    
    
};
struct ElseNode: public alioth::ASTNtrmNode {
    
    ASTAttrs stmts{};
    
    
    
};
struct ElseifNode: public alioth::ASTNtrmNode {
    
    ASTAttr condition{};
    
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
    ASTAttr lhs{};
    
    ASTAttr op{};
    
    ASTAttr rhs{};
    
    
};
struct ExpressionNode::MonoExpression {
    ASTAttr op{};
    
    ASTAttr rhs{};
    
    
};
struct ExpressionNode::SubExpression {
    ASTAttr expr{};
    
    
};
struct ExpressionNode::ValueExpression {
    ASTAttr false{};
    
    ASTAttr null{};
    
    ASTAttr number{};
    
    ASTAttr string{};
    
    ASTAttr true{};
    
    
};
struct ExpressionNode::VarExpression {
    ASTAttr name{};
    
    
};


struct FunctionNode: public alioth::ASTNtrmNode {
    
    ASTAttr name{};
    
    ASTAttrs params{};
    
    ASTAttrs stmts{};
    
    
    
};
struct IfNode: public alioth::ASTNtrmNode {
    
    ASTAttr condition{};
    
    ASTAttrs stmts{};
    
    
    
};
struct IterateNode: public alioth::ASTNtrmNode {
    
    ASTAttr cond{};
    
    ASTAttr ctrl{};
    
    ASTAttr init{};
    
    ASTAttrs stmts{};
    
    
    
};
struct PlayNode: public alioth::ASTNtrmNode {
    
    ASTAttrs stmts{};
    
    
    
};
struct StmtNode: public alioth::ASTNtrmNode {
    
    ASTAttr branch{};
    
    ASTAttr control{};
    
    ASTAttr declare{};
    
    ASTAttr expression{};
    
    ASTAttr function{};
    
    ASTAttr iterate{};
    
    
    
};


}

#endif