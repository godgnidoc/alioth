#ifndef __ABNF_H__
#define __ABNF_H__

#include "alioth/lex/lex.h"
#include "alioth/syntax/syntax.h"

class ABNF {
 public:
  static alioth::SyntaxCRef Compile(alioth::SourceRef source);

 private:
  static void PopulateFormulas(nlohmann::json& formulas);
 
  static alioth::LexCRef GetLex();
  static alioth::SyntaxCRef GetSyntax();
};

#endif