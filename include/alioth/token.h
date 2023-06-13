#ifndef __ALIOTH_TOKEN_H__
#define __ALIOTH_TOKEN_H__

#include <memory>
#include <string>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/source.h"

namespace alioth {

/**
 * Token
 *
 * 提供基于TokenStream的Token对象
 */
struct Token {
  static constexpr int kEOF = 0;
  static constexpr int kError = -1;

  int id;               // 词法单元的ID
  size_t start_line;    // 词法单元的起始行
  size_t start_column;  // 词法单元的起始列
  size_t end_line;      // 词法单元的结束行
  size_t end_column;    // 词法单元的结束列
  size_t offset;        // 词法单元的偏移量
  size_t length;        // 词法单元的长度
  SourceRef source;     // 词法单元的所属源码
  LexCRef lex;         // 词法规则

  /**
   * 获取词法单元的文本
   */
  std::string GetText() const;

  /**
   * 获取词法单元的名称
   */
  std::string GetName() const;

  /**
   * 获取词法单元的位置
   */
  std::string GetLocation() const;
};

}  // namespace alioth

#endif