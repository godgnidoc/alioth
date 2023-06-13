#ifndef __ALIOTH_SYNTAX_PARSER_H__
#define __ALIOTH_SYNTAX_PARSER_H__

#include <memory>

#include "alioth/lex/scanner-fwd.h"
#include "alioth/ast.h"
#include "alioth/syntax/syntax-fwd.h"
#include "alioth/logging.h"
#include "alioth/source.h"

namespace alioth::syntax {

class Parser {
 private:
  Parser() = default;
  AST _Parse();

 public:
  static AST Parse(SyntaxCRef syntax, SourceRef source);

 private:
  std::shared_ptr<ast::Root> root_;
  std::vector<AST> seens_;
  std::vector<int> stack_;

  Logger logger_{"parser"};
};

}  // namespace alioth::syntax

#endif