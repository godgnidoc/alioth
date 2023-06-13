#ifndef __ALIOTH_LEX_TOKEN_STREAM_H__
#define __ALIOTH_LEX_TOKEN_STREAM_H__

#include "alioth/lex/lex-fwd.h"
#include "alioth/lex/scanner-fwd.h"
#include "alioth/source.h"
#include "alioth/token.h"

namespace alioth::lex {

/**
 * 词法分析器，加词法规则和源码后，可以提取Token
 */
class Scanner {
 private:
  Scanner(LexCRef lex, SourceRef source);

 public:
  /**
   * 从文本创建词法分析器
   *
   * @param lex 词法规则
   * @param source 源码
   */
  static ScannerRef Create(LexCRef lex, SourceRef source);

  /**
   * 克隆词法分析器
   */
  ScannerRef Clone() const;

  Scanner(const Scanner&) = delete;
  Scanner(Scanner&&) = delete;

  Scanner& operator=(const Scanner&) = delete;
  Scanner& operator=(Scanner&&) = delete;

  /**
   * 提取下一个Token
   */
  Token NextToken();

  /**
   * 修改上下文
   *
   * @param context 上下文ID
   */
  void SetContext(int context);

  /**
   * 修改上下文
   *
   * @param context 上下文名称
   */
  void SetContext(std::string const& context);

 private:
  int context_;  // 当前上下文

  size_t line_;             // 当前行号
  size_t column_;           // 当前列号
  size_t offset_;           // 当前偏移量
  SourceRef const source_;  // 源码

 public:
  LexCRef const lex_;  // 词法规则
};

}  // namespace alioth::lex

#endif