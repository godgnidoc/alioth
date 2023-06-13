#ifndef __ALIOTH_CLI_SYNTAX_H__
#define __ALIOTH_CLI_SYNTAX_H__

#include "alioth/syntax.h"
#include "cli/cli.h"

struct Syntax : public cli::Command {
  int Run() override;

  cli::Arg gpath = Named("grammar");

  /**
   * 从指定路径加载语法
   *
   * 若 path 为 - 则从标准输入加载
   * 否则从指定路径加载
   *
   * 同时支持 json 格式和 grammar 格式
   *
   * @param path 语法文件路径
   */
  static alioth::Syntax Load(std::string const& path);
};

#endif