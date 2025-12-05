#ifndef __ALIOX_GRAMMAR_H__
#define __ALIOX_GRAMMAR_H__

#include <functional>
#include <string>
#include <vector>

#include "alioth/ast.h"
#include "alioth/document.h"

namespace alioth {

/**
 * 外部语法加载器类型
 * 
 * 用于加载外部符号引用的语法规则
 * 参数为外部语法的路径或标识符
 * 返回对应的语法规则，如果无法加载则返回nullptr
 */
using ExternalLoader = std::function<Syntax(std::string const&)>;

struct Grammar {
  /**
   * 将文法定义编译为语法规则
   *
   * @param grammar 文法源码
   */
  static Syntax Compile(Doc grammar);

  /**
   * 将文法定义编译为语法规则
   *
   * @param grammar 文法源码
   * @param loader 外部语法加载器，用于加载外部符号引用的语法
   */
  static Syntax Compile(Doc grammar, ExternalLoader loader);
};

}  // namespace alioth

#endif