#ifndef __ALIOX_GRAMMAR_H__
#define __ALIOX_GRAMMAR_H__

#include <vector>

#include "alioth/ast.h"
#include "alioth/document.h"

namespace alioth {

struct Grammar {
  /**
   * 将文法定义编译为语法规则
   *
   * @param grammar 文法源码
   * @param known 已知的语法规则
   */
  static Syntax Compile(Doc grammar, std::map<std::string, Syntax> known = {});
};

}  // namespace alioth

#endif