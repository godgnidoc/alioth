#ifndef __PLAY_SYNTAX_H__
#define __PLAY_SYNTAX_H__

#include "alioth/ast.h"

namespace play {

struct BranchNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct ControlNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct DeclareNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct ElseNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct ElseifNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct ExpressionNode: public alioth::ASTNtrmNode {
    struct AddExpression;
    struct AndExpression;
    struct AssignExpression;
    struct BitandExpression;
    struct BitnotExpression;
    struct BitorExpression;
    struct BitxorExpression;
    struct BooleanExpression;
    struct DivExpression;
    struct MolExpression;
    struct MulExpression;
    struct NegExpression;
    struct NotExpression;
    struct NullExpression;
    struct NumberExpression;
    struct OrExpression;
    struct StringExpression;
    struct SubExpression;
    struct SubexprExpression;
    struct VarExpression;
    
    
};
struct FunctionNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct IfNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct IterateNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct PlayNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};
struct Priority0Node: public alioth::ASTNtrmNode {
    struct BitnotPriority0;
    struct BooleanPriority0;
    struct NegPriority0;
    struct NotPriority0;
    struct NullPriority0;
    struct NumberPriority0;
    struct StringPriority0;
    struct SubexprPriority0;
    struct VarPriority0;
    
    
};
struct Priority1Node: public alioth::ASTNtrmNode {
    struct BitandPriority1;
    struct BitnotPriority1;
    struct BitorPriority1;
    struct BitxorPriority1;
    struct BooleanPriority1;
    struct NegPriority1;
    struct NotPriority1;
    struct NullPriority1;
    struct NumberPriority1;
    struct StringPriority1;
    struct SubexprPriority1;
    struct VarPriority1;
    
    
};
struct Priority2Node: public alioth::ASTNtrmNode {
    struct BitandPriority2;
    struct BitnotPriority2;
    struct BitorPriority2;
    struct BitxorPriority2;
    struct BooleanPriority2;
    struct DivPriority2;
    struct MolPriority2;
    struct MulPriority2;
    struct NegPriority2;
    struct NotPriority2;
    struct NullPriority2;
    struct NumberPriority2;
    struct StringPriority2;
    struct SubexprPriority2;
    struct VarPriority2;
    
    
};
struct Priority3Node: public alioth::ASTNtrmNode {
    struct AddPriority3;
    struct BitandPriority3;
    struct BitnotPriority3;
    struct BitorPriority3;
    struct BitxorPriority3;
    struct BooleanPriority3;
    struct DivPriority3;
    struct MolPriority3;
    struct MulPriority3;
    struct NegPriority3;
    struct NotPriority3;
    struct NullPriority3;
    struct NumberPriority3;
    struct StringPriority3;
    struct SubPriority3;
    struct SubexprPriority3;
    struct VarPriority3;
    
    
};
struct Priority4Node: public alioth::ASTNtrmNode {
    struct AddPriority4;
    struct AndPriority4;
    struct BitandPriority4;
    struct BitnotPriority4;
    struct BitorPriority4;
    struct BitxorPriority4;
    struct BooleanPriority4;
    struct DivPriority4;
    struct MolPriority4;
    struct MulPriority4;
    struct NegPriority4;
    struct NotPriority4;
    struct NullPriority4;
    struct NumberPriority4;
    struct OrPriority4;
    struct StringPriority4;
    struct SubPriority4;
    struct SubexprPriority4;
    struct VarPriority4;
    
    
};
struct StmtNode: public alioth::ASTNtrmNode {
    
    // not formed
    
};


}

#endif