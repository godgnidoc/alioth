#include "play/syntax.h"

#include "alioth/parser.h"
#include "nlohmann/json.hpp"

namespace play {

alioth::Syntax SyntaxOf();
alioth::ASTAttr ParseArbitraryAttribute(alioth::AST node);

std::shared_ptr<BranchNode> ParseBranch(alioth::ASTNtrm node) {
  auto n = std::make_shared<BranchNode>(*node);
  for( auto attr : node->Attrs("branchs")) n->branchs.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<ControlNode> ParseBreakControl(alioth::ASTNtrm node) {
  auto n = std::make_shared<ControlNode::Break>(*node);

  

  return n;
}
std::shared_ptr<ControlNode> ParseContinueControl(alioth::ASTNtrm node) {
  auto n = std::make_shared<ControlNode::Continue>(*node);

  

  return n;
}
std::shared_ptr<ControlNode> ParseReturnControl(alioth::ASTNtrm node) {
  auto n = std::make_shared<ControlNode::Return>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}


std::shared_ptr<ControlNode> ParseControl(alioth::ASTNtrm node) {
  std::shared_ptr<ControlNode> n;
  switch(node->OriginFormula()) {
    case 32: 
      n = ParseBreakControl(node);
      break;
    case 33: 
      n = ParseContinueControl(node);
      break;
    case 34: case 35: 
      n = ParseReturnControl(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<DeclareNode> ParseDeclare(alioth::ASTNtrm node) {
  auto n = std::make_shared<DeclareNode>(*node);
  n->init = ParseArbitraryAttribute(node->Attr("init"));
  n->name = ParseArbitraryAttribute(node->Attr("name"));
  return n;
}
std::shared_ptr<ElseNode> ParseElse(alioth::ASTNtrm node) {
  auto n = std::make_shared<ElseNode>(*node);
  for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<ElseifNode> ParseElseif(alioth::ASTNtrm node) {
  auto n = std::make_shared<ElseifNode>(*node);
  n->condition = ParseArbitraryAttribute(node->Attr("condition"));
  for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<ExpressionNode> ParseBinaryExpression(alioth::ASTNtrm node) {
  auto n = std::make_shared<ExpressionNode::Binary>(*node);

  n->lhs = ParseArbitraryAttribute(node->Attr("lhs"));
  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<ExpressionNode> ParseMonoExpression(alioth::ASTNtrm node) {
  auto n = std::make_shared<ExpressionNode::Mono>(*node);

  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<ExpressionNode> ParseSubExpression(alioth::ASTNtrm node) {
  auto n = std::make_shared<ExpressionNode::Sub>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}
std::shared_ptr<ExpressionNode> ParseValueExpression(alioth::ASTNtrm node) {
  auto n = std::make_shared<ExpressionNode::Value>(*node);

  n->boolean = ParseArbitraryAttribute(node->Attr("boolean"));
  n->null = ParseArbitraryAttribute(node->Attr("null"));
  n->number = ParseArbitraryAttribute(node->Attr("number"));
  n->string = ParseArbitraryAttribute(node->Attr("string"));

  return n;
}
std::shared_ptr<ExpressionNode> ParseVarExpression(alioth::ASTNtrm node) {
  auto n = std::make_shared<ExpressionNode::Var>(*node);

  n->name = ParseArbitraryAttribute(node->Attr("name"));

  return n;
}


std::shared_ptr<ExpressionNode> ParseExpression(alioth::ASTNtrm node) {
  std::shared_ptr<ExpressionNode> n;
  switch(node->OriginFormula()) {
    case 41: case 43: case 44: case 46: case 47: case 49: case 50: case 51: case 53: case 54: case 55: 
      n = ParseBinaryExpression(node);
      break;
    case 56: case 57: case 58: 
      n = ParseMonoExpression(node);
      break;
    case 59: 
      n = ParseSubExpression(node);
      break;
    case 61: case 62: case 63: case 64: case 65: 
      n = ParseValueExpression(node);
      break;
    case 60: 
      n = ParseVarExpression(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<FunctionNode> ParseFunction(alioth::ASTNtrm node) {
  auto n = std::make_shared<FunctionNode>(*node);
  n->name = ParseArbitraryAttribute(node->Attr("name"));
  for( auto attr : node->Attrs("params")) n->params.push_back(ParseArbitraryAttribute(attr));
for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<IfNode> ParseIf(alioth::ASTNtrm node) {
  auto n = std::make_shared<IfNode>(*node);
  n->condition = ParseArbitraryAttribute(node->Attr("condition"));
  for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<IterateNode> ParseIterate(alioth::ASTNtrm node) {
  auto n = std::make_shared<IterateNode>(*node);
  n->cond = ParseArbitraryAttribute(node->Attr("cond"));
  n->ctrl = ParseArbitraryAttribute(node->Attr("ctrl"));
  n->init = ParseArbitraryAttribute(node->Attr("init"));
  for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<PlayNode> ParsePlay(alioth::ASTNtrm node) {
  auto n = std::make_shared<PlayNode>(*node);
  for( auto attr : node->Attrs("stmts")) n->stmts.push_back(ParseArbitraryAttribute(attr));
  return n;
}
std::shared_ptr<Priority1Node> ParseBinaryPriority1(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority1Node::Binary>(*node);

  n->lhs = ParseArbitraryAttribute(node->Attr("lhs"));
  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority1Node> ParseMonoPriority1(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority1Node::Mono>(*node);

  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority1Node> ParseSubPriority1(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority1Node::Sub>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}
std::shared_ptr<Priority1Node> ParseValuePriority1(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority1Node::Value>(*node);

  n->boolean = ParseArbitraryAttribute(node->Attr("boolean"));
  n->null = ParseArbitraryAttribute(node->Attr("null"));
  n->number = ParseArbitraryAttribute(node->Attr("number"));
  n->string = ParseArbitraryAttribute(node->Attr("string"));

  return n;
}
std::shared_ptr<Priority1Node> ParseVarPriority1(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority1Node::Var>(*node);

  n->name = ParseArbitraryAttribute(node->Attr("name"));

  return n;
}


std::shared_ptr<Priority1Node> ParsePriority1(alioth::ASTNtrm node) {
  std::shared_ptr<Priority1Node> n;
  switch(node->OriginFormula()) {
    case 53: case 54: case 55: 
      n = ParseBinaryPriority1(node);
      break;
    case 56: case 57: case 58: 
      n = ParseMonoPriority1(node);
      break;
    case 59: 
      n = ParseSubPriority1(node);
      break;
    case 61: case 62: case 63: case 64: case 65: 
      n = ParseValuePriority1(node);
      break;
    case 60: 
      n = ParseVarPriority1(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<Priority2Node> ParseBinaryPriority2(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority2Node::Binary>(*node);

  n->lhs = ParseArbitraryAttribute(node->Attr("lhs"));
  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority2Node> ParseMonoPriority2(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority2Node::Mono>(*node);

  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority2Node> ParseSubPriority2(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority2Node::Sub>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}
std::shared_ptr<Priority2Node> ParseValuePriority2(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority2Node::Value>(*node);

  n->boolean = ParseArbitraryAttribute(node->Attr("boolean"));
  n->null = ParseArbitraryAttribute(node->Attr("null"));
  n->number = ParseArbitraryAttribute(node->Attr("number"));
  n->string = ParseArbitraryAttribute(node->Attr("string"));

  return n;
}
std::shared_ptr<Priority2Node> ParseVarPriority2(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority2Node::Var>(*node);

  n->name = ParseArbitraryAttribute(node->Attr("name"));

  return n;
}


std::shared_ptr<Priority2Node> ParsePriority2(alioth::ASTNtrm node) {
  std::shared_ptr<Priority2Node> n;
  switch(node->OriginFormula()) {
    case 49: case 50: case 51: case 53: case 54: case 55: 
      n = ParseBinaryPriority2(node);
      break;
    case 56: case 57: case 58: 
      n = ParseMonoPriority2(node);
      break;
    case 59: 
      n = ParseSubPriority2(node);
      break;
    case 61: case 62: case 63: case 64: case 65: 
      n = ParseValuePriority2(node);
      break;
    case 60: 
      n = ParseVarPriority2(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<Priority3Node> ParseBinaryPriority3(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority3Node::Binary>(*node);

  n->lhs = ParseArbitraryAttribute(node->Attr("lhs"));
  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority3Node> ParseMonoPriority3(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority3Node::Mono>(*node);

  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority3Node> ParseSubPriority3(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority3Node::Sub>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}
std::shared_ptr<Priority3Node> ParseValuePriority3(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority3Node::Value>(*node);

  n->boolean = ParseArbitraryAttribute(node->Attr("boolean"));
  n->null = ParseArbitraryAttribute(node->Attr("null"));
  n->number = ParseArbitraryAttribute(node->Attr("number"));
  n->string = ParseArbitraryAttribute(node->Attr("string"));

  return n;
}
std::shared_ptr<Priority3Node> ParseVarPriority3(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority3Node::Var>(*node);

  n->name = ParseArbitraryAttribute(node->Attr("name"));

  return n;
}


std::shared_ptr<Priority3Node> ParsePriority3(alioth::ASTNtrm node) {
  std::shared_ptr<Priority3Node> n;
  switch(node->OriginFormula()) {
    case 46: case 47: case 49: case 50: case 51: case 53: case 54: case 55: 
      n = ParseBinaryPriority3(node);
      break;
    case 56: case 57: case 58: 
      n = ParseMonoPriority3(node);
      break;
    case 59: 
      n = ParseSubPriority3(node);
      break;
    case 61: case 62: case 63: case 64: case 65: 
      n = ParseValuePriority3(node);
      break;
    case 60: 
      n = ParseVarPriority3(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<Priority4Node> ParseBinaryPriority4(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority4Node::Binary>(*node);

  n->lhs = ParseArbitraryAttribute(node->Attr("lhs"));
  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority4Node> ParseMonoPriority4(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority4Node::Mono>(*node);

  n->op = ParseArbitraryAttribute(node->Attr("op"));
  n->rhs = ParseArbitraryAttribute(node->Attr("rhs"));

  return n;
}
std::shared_ptr<Priority4Node> ParseSubPriority4(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority4Node::Sub>(*node);

  n->expr = ParseArbitraryAttribute(node->Attr("expr"));

  return n;
}
std::shared_ptr<Priority4Node> ParseValuePriority4(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority4Node::Value>(*node);

  n->boolean = ParseArbitraryAttribute(node->Attr("boolean"));
  n->null = ParseArbitraryAttribute(node->Attr("null"));
  n->number = ParseArbitraryAttribute(node->Attr("number"));
  n->string = ParseArbitraryAttribute(node->Attr("string"));

  return n;
}
std::shared_ptr<Priority4Node> ParseVarPriority4(alioth::ASTNtrm node) {
  auto n = std::make_shared<Priority4Node::Var>(*node);

  n->name = ParseArbitraryAttribute(node->Attr("name"));

  return n;
}


std::shared_ptr<Priority4Node> ParsePriority4(alioth::ASTNtrm node) {
  std::shared_ptr<Priority4Node> n;
  switch(node->OriginFormula()) {
    case 43: case 44: case 46: case 47: case 49: case 50: case 51: case 53: case 54: case 55: 
      n = ParseBinaryPriority4(node);
      break;
    case 56: case 57: case 58: 
      n = ParseMonoPriority4(node);
      break;
    case 59: 
      n = ParseSubPriority4(node);
      break;
    case 61: case 62: case 63: case 64: case 65: 
      n = ParseValuePriority4(node);
      break;
    case 60: 
      n = ParseVarPriority4(node);
      break;
    
    default:
     throw std::runtime_error("unexpected formula");
  }
  
  return n;
}
std::shared_ptr<StmtNode> ParseStmt(alioth::ASTNtrm node) {
  auto n = std::make_shared<StmtNode>(*node);
  n->branch = ParseArbitraryAttribute(node->Attr("branch"));
  n->control = ParseArbitraryAttribute(node->Attr("control"));
  n->declare = ParseArbitraryAttribute(node->Attr("declare"));
  n->expression = ParseArbitraryAttribute(node->Attr("expression"));
  n->function = ParseArbitraryAttribute(node->Attr("function"));
  n->iterate = ParseArbitraryAttribute(node->Attr("iterate"));
  return n;
}


alioth::ASTAttr ParseArbitraryAttribute(alioth::AST node) {
  if(node) switch(node->id) {
    case 48: return ParseBranch(node->AsNtrm());
    case 50: return ParseControl(node->AsNtrm());
    case 47: return ParseDeclare(node->AsNtrm());
    case 57: return ParseElse(node->AsNtrm());
    case 58: return ParseElseif(node->AsNtrm());
    case 52: return ParseExpression(node->AsNtrm());
    case 51: return ParseFunction(node->AsNtrm());
    case 55: return ParseIf(node->AsNtrm());
    case 49: return ParseIterate(node->AsNtrm());
    case 45: return ParsePlay(node->AsNtrm());
    case 63: return ParsePriority1(node->AsNtrm());
    case 62: return ParsePriority2(node->AsNtrm());
    case 61: return ParsePriority3(node->AsNtrm());
    case 60: return ParsePriority4(node->AsNtrm());
    case 46: return ParseStmt(node->AsNtrm());
    
    default: break;
  }

  return node;
}

std::shared_ptr<PlayNode> ParsePlay(alioth::Doc source) {
  auto syntax = SyntaxOf();
  auto parser = alioth::Parser(syntax, source);
  auto root = parser.Parse();
  auto play = ParsePlay(AsNtrm(root->Attr("play")));
  play->root = root;
  return play;
}

alioth::Syntax SyntaxOf() {
  static auto syntax = []{
    using namespace alioth;
    using namespace nlohmann;
    auto lex_builder = Lexicon::Builder("play");
    lex_builder.Define("T_LP", R"(\()"_regex);
    lex_builder.Annotate("T_LP", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_RP", R"(\))"_regex);
    lex_builder.Annotate("T_RP", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_LI", R"(\[)"_regex);
    lex_builder.Annotate("T_LI", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_RI", R"(\])"_regex);
    lex_builder.Annotate("T_RI", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_LS", R"({)"_regex);
    lex_builder.Annotate("T_LS", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_RS", R"(})"_regex);
    lex_builder.Annotate("T_RS", "tokenize", R"({"type":"operator"})"_json);
    
    lex_builder.Define("T_COMMA", R"(,)"_regex);
    
    lex_builder.Define("T_SEMI", R"(;)"_regex);
    
    lex_builder.Define("T_DOT", R"(\.)"_regex);
    
    lex_builder.Define("T_ADD", R"(\+)"_regex);
    
    lex_builder.Define("T_SUB", R"(-)"_regex);
    
    lex_builder.Define("T_MUL", R"(\*)"_regex);
    
    lex_builder.Define("T_DIV", R"(\/)"_regex);
    
    lex_builder.Define("T_MOL", R"(%)"_regex);
    
    lex_builder.Define("T_LT", R"(<)"_regex);
    
    lex_builder.Define("T_GT", R"(>)"_regex);
    
    lex_builder.Define("T_LE", R"(<=)"_regex);
    
    lex_builder.Define("T_GE", R"(>=)"_regex);
    
    lex_builder.Define("T_EQ", R"(==)"_regex);
    
    lex_builder.Define("T_NE", R"(!=)"_regex);
    
    lex_builder.Define("T_AND", R"(&&)"_regex);
    
    lex_builder.Define("T_OR", R"(\|\|)"_regex);
    
    lex_builder.Define("T_NOT", R"(!)"_regex);
    
    lex_builder.Define("T_BITAND", R"(&)"_regex);
    
    lex_builder.Define("T_BITOR", R"(\|)"_regex);
    
    lex_builder.Define("T_BITNOT", R"(~)"_regex);
    
    lex_builder.Define("T_BITXOR", R"(^)"_regex);
    
    lex_builder.Define("T_ASSIGN", R"(=)"_regex);
    
    lex_builder.Define("T_LET", R"(let)"_regex);
    
    lex_builder.Define("T_FN", R"(fn)"_regex);
    
    lex_builder.Define("T_IF", R"(if)"_regex);
    
    lex_builder.Define("T_ELSE", R"(else)"_regex);
    
    lex_builder.Define("T_FOR", R"(for)"_regex);
    
    lex_builder.Define("T_CONTINUE", R"(continue)"_regex);
    
    lex_builder.Define("T_BREAK", R"(break)"_regex);
    
    lex_builder.Define("T_RETURN", R"(return)"_regex);
    
    lex_builder.Define("T_NULL", R"(null)"_regex, { "json",  });
    
    lex_builder.Define("T_TRUE", R"(true)"_regex, { "json",  });
    
    lex_builder.Define("T_FALSE", R"(false)"_regex, { "json",  });
    
    lex_builder.Define("T_STRING", R"(\"[^\"\n\\]|(\\[^\n])*\")"_regex, { "json",  });
    
    lex_builder.Define("T_NUMBER", R"(0|([1-9]\d*)\.\d+?[eE][+-]?\d+?)"_regex);
    
    lex_builder.Define("T_ID", R"([a-zA-Z_]\w*)"_regex);
    
    lex_builder.Define("T_SPACE", R"(\s+)"_regex);
    
    
    auto lex = lex_builder.Build();

    auto syntax_builder = Syntactic::Builder(lex);
    syntax_builder.Ignore("T_SPACE");
    
    syntax_builder.Forluma("play").Symbol("stmt", "stmts").Commit();
    syntax_builder.Forluma("play").Symbol("play", "...").Symbol("T_SEMI").Symbol("stmt", "stmts").Commit();
    syntax_builder.Forluma("stmt").Symbol("declare", "declare").Commit();
    syntax_builder.Forluma("stmt").Symbol("branch", "branch").Commit();
    syntax_builder.Forluma("stmt").Symbol("iterate", "iterate").Commit();
    syntax_builder.Forluma("stmt").Symbol("control", "control").Commit();
    syntax_builder.Forluma("stmt").Symbol("function", "function").Commit();
    syntax_builder.Forluma("stmt").Symbol("expression", "expression").Commit();
    syntax_builder.Forluma("block").Symbol("T_LS").Symbol("stmts", "...").Symbol("T_RS").Commit();
    syntax_builder.Forluma("stmts").Symbol("stmt", "stmts").Commit();
    syntax_builder.Forluma("stmts").Symbol("stmts", "...").Symbol("T_SEMI").Commit();
    syntax_builder.Forluma("stmts").Symbol("stmts", "...").Symbol("T_SEMI").Symbol("stmt", "stmts").Commit();
    syntax_builder.Forluma("declare").Symbol("T_LET").Symbol("T_ID", "name").Commit();
    syntax_builder.Forluma("declare").Symbol("T_LET").Symbol("T_ID", "name").Symbol("T_ASSIGN").Symbol("expression", "init").Commit();
    syntax_builder.Forluma("branch").Symbol("if", "branchs").Commit();
    syntax_builder.Forluma("branch").Symbol("if", "branchs").Symbol("elseifs", "...").Commit();
    syntax_builder.Forluma("branch").Symbol("if", "branchs").Symbol("else", "branchs").Commit();
    syntax_builder.Forluma("branch").Symbol("if", "branchs").Symbol("elseifs", "...").Symbol("else", "branchs").Commit();
    syntax_builder.Forluma("if").Symbol("T_IF").Symbol("T_LP").Symbol("expression", "condition").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("elseifs").Symbol("elseif", "branchs").Commit();
    syntax_builder.Forluma("elseifs").Symbol("elseifs", "...").Symbol("elseif", "branchs").Commit();
    syntax_builder.Forluma("elseif").Symbol("T_ELSE").Symbol("T_IF").Symbol("T_LP").Symbol("expression", "condition").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("else").Symbol("T_ELSE").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("T_SEMI").Symbol("T_SEMI").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("declare", "init").Symbol("T_SEMI").Symbol("T_SEMI").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("T_SEMI").Symbol("expression", "cond").Symbol("T_SEMI").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("declare", "init").Symbol("T_SEMI").Symbol("expression", "cond").Symbol("T_SEMI").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("T_SEMI").Symbol("T_SEMI").Symbol("expression", "ctrl").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("declare", "init").Symbol("T_SEMI").Symbol("T_SEMI").Symbol("expression", "ctrl").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("T_SEMI").Symbol("expression", "cond").Symbol("T_SEMI").Symbol("expression", "ctrl").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("iterate").Symbol("T_FOR").Symbol("T_LP").Symbol("declare", "init").Symbol("T_SEMI").Symbol("expression", "cond").Symbol("T_SEMI").Symbol("expression", "ctrl").Symbol("T_RP").Symbol("block", "...").Commit();
    syntax_builder.Forluma("control", "break").Symbol("T_BREAK").Commit();
    syntax_builder.Forluma("control", "continue").Symbol("T_CONTINUE").Commit();
    syntax_builder.Forluma("control", "return").Symbol("T_RETURN").Commit();
    syntax_builder.Forluma("control", "return").Symbol("T_RETURN").Symbol("expression", "expr").Commit();
    syntax_builder.Forluma("function").Symbol("T_FN").Symbol("T_ID", "name").Symbol("T_LP").Symbol("T_RP").Symbol("block", "...")
      .Annotate("name", "type", R"("function")"_json);
      .Commit();
    syntax_builder.Forluma("function").Symbol("T_FN").Symbol("T_ID", "name").Symbol("T_LP").Symbol("params", "...").Symbol("T_RP").Symbol("block", "...")
      .Annotate("name", "type", R"("function")"_json);
      .Commit();
    syntax_builder.Forluma("params").Symbol("T_ID", "params").Commit();
    syntax_builder.Forluma("params").Symbol("params", "...").Symbol("T_COMMA").Symbol("T_ID", "params").Commit();
    syntax_builder.Forluma("expression").Symbol("priority4", "...").Commit();
    syntax_builder.Forluma("expression", "binary").Symbol("expression", "lhs").Symbol("T_ASSIGN", "op").Symbol("priority4", "rhs").Commit();
    syntax_builder.Forluma("priority4").Symbol("priority3", "...").Commit();
    syntax_builder.Forluma("priority4", "binary").Symbol("priority4", "lhs").Symbol("T_AND", "op").Symbol("priority3", "rhs").Commit();
    syntax_builder.Forluma("priority4", "binary").Symbol("priority4", "lhs").Symbol("T_OR", "op").Symbol("priority3", "rhs").Commit();
    syntax_builder.Forluma("priority3").Symbol("priority2", "...").Commit();
    syntax_builder.Forluma("priority3", "binary").Symbol("priority3", "lhs").Symbol("T_ADD", "op").Symbol("priority2", "rhs").Commit();
    syntax_builder.Forluma("priority3", "binary").Symbol("priority3", "lhs").Symbol("T_SUB", "op").Symbol("priority2", "rhs").Commit();
    syntax_builder.Forluma("priority2").Symbol("priority1", "...").Commit();
    syntax_builder.Forluma("priority2", "binary").Symbol("priority2", "lhs").Symbol("T_MUL", "op").Symbol("priority1", "rhs").Commit();
    syntax_builder.Forluma("priority2", "binary").Symbol("priority2", "lhs").Symbol("T_DIV", "op").Symbol("priority1", "rhs").Commit();
    syntax_builder.Forluma("priority2", "binary").Symbol("priority2", "lhs").Symbol("T_MOL", "op").Symbol("priority1", "rhs").Commit();
    syntax_builder.Forluma("priority1").Symbol("priority0", "...").Commit();
    syntax_builder.Forluma("priority1", "binary").Symbol("priority1", "lhs").Symbol("T_BITAND", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority1", "binary").Symbol("priority1", "lhs").Symbol("T_BITOR", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority1", "binary").Symbol("priority1", "lhs").Symbol("T_BITXOR", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority0", "mono").Symbol("T_SUB", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority0", "mono").Symbol("T_NOT", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority0", "mono").Symbol("T_BITNOT", "op").Symbol("priority0", "rhs").Commit();
    syntax_builder.Forluma("priority0", "sub").Symbol("T_LP").Symbol("expression", "expr").Symbol("T_RP").Commit();
    syntax_builder.Forluma("priority0", "var").Symbol("T_ID", "name").Commit();
    syntax_builder.Forluma("priority0", "value").Symbol("T_STRING", "string").Commit();
    syntax_builder.Forluma("priority0", "value").Symbol("T_NUMBER", "number").Commit();
    syntax_builder.Forluma("priority0", "value").Symbol("T_TRUE", "boolean").Commit();
    syntax_builder.Forluma("priority0", "value").Symbol("T_FALSE", "boolean").Commit();
    syntax_builder.Forluma("priority0", "value").Symbol("T_NULL", "null").Commit();
    
    return syntax_builder.Build();
  }();

  return syntax;
}

}