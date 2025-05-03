#ifndef __ALIOX_GRAMMAR_H__
#define __ALIOX_GRAMMAR_H__

#include "alioth/ast.h"
#include "alioth/document.h"

namespace alioth {

struct Grammar {
  ASTRoot root;

  /**
   * 将文法定义编译为语法规则
   */
  Syntax Compile() const;

  /**
   * 从文法源码加载文法定义
   */
  static Grammar Parse(Doc source);

  /**
   * 从 AST 语法树加载文法定义
   */
  static Grammar Parse(ASTRoot root);

  /**
   * 获取语法规则
   */
  static Syntax SyntaxOf();
};

}  // namespace alioth

#endif