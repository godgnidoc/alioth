#ifndef __ALIOTH_PARSER_H__
#define __ALIOTH_PARSER_H__

#include "alioth/ast.h"
#include "alioth/error.h"
#include "alioth/generic.h"
#include "alioth/lexicon.h"
#include "alioth/syntax.h"

namespace alioth {

/**
 * 语法分析器
 */
class Parser {
 public:
  struct Thread;

 public:
  Parser(Syntax syntax, Doc doc);

  /**
   * 解析源码
   */
  ASTNtrm Parse();

  struct ParseError : public Error {
    ParseError() : Error("Parse error occured") {}
  };

 protected:
  /**
   * 为所有分析线路准备下一个输入单元
   *
   * 若某个分析线路所处的状态具有多个可能的上下文
   * 且从这些上下文中扫描出的语法单元不相同
   * 则将这些上下文分离成新的分析线路
   */
  void ScanAndFork();

  /**
   * 按需忽略输入符号，若忽略则返回true
   *
   * @param thread 当前分析线路
   */
  bool IgnoreOrFalse(Thread& thread);

  /**
   * 尝试归约，若归约成功则返回true
   *
   * @param thread 当前分析线路
   */
  bool ReduceOrFalse(Thread& thread);

  /**
   * 尝试移进，若移进成功则返回true
   *
   * @param thread 当前分析线路
   */
  bool ShiftOrFalse(Thread& thread);

  /**
   * 清除所有失败的分析线路
   */
  void ClearOrCrash(std::vector<size_t> const& failed,
                    std::vector<size_t> const& accepted);

  /**
   * 若分析线路数量不为1，分析失败
   */
  ASTNtrm AcceptOrAmbiguous();

  /**
   * 依据下标清除分析线路
   *
   * @param according 需要清除的分析线路下标
   */
  void Clean(std::vector<size_t> const& according);

  /**
   * 若全部分析线路都失败，分析失败
   */
  void Crash();

  /**
   * 从文档中扫描一个词法单元
   *
   * @param thread 当前分析线路
   * @param context 上下文
   */
  ASTTerm Scan(Thread& thread, ContextID context = 0);

 protected:
  Doc doc_;                          // 正在分析的源码
  Syntax syntax_;                    // 语法规则
  std::vector<Thread> threads_;      // 分析线路
  std::vector<ASTNtrm> candidates_;  // 已分析完毕的候选语法树
};

struct Parser::Thread {
  size_t offset{};                 // 当前扫描位置
  std::vector<StateID> stack{};    // 状态栈，从尾部出入栈
  std::vector<AST> seens{};        // 已经识别的语法单元，按识别顺序排列
  std::vector<ASTTerm> ignores{};  // 被忽略的语法单元，按忽略顺序排列
  std::vector<AST> inputs{};       // 输入的语法单元，按读取顺序排列
};

}  // namespace alioth

#endif