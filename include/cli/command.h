#ifndef __CLI_COMMAND_H__
#define __CLI_COMMAND_H__

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "cli/argument.h"
#include "cli/option.h"

namespace cli {

class Command;
using Cmd = std::shared_ptr<Command>;

/**
 * 命令抽象
 *
 * 继承此类后，可以使用将选项和参数定义为成员变量
 * 使用此类提供的接口创建选项和参数确保它们被正确解析
 */
class Command {
  friend class Application;
  friend class Inspect;

 public:
  /**
   * 若当前命令被选中，则执行Run回调函数
   * 回调函数的返回值被用作命令的返回值
   */
  virtual int Run() = 0;

  /**
   * 创建一个与当前命令绑定的选项
   *
   * @param flags 选项标志，参见选项定义获取标志规范
   */
  Opt Option(std::vector<std::string> const& flags);

  /**
   * 创建一个与当前命令绑定的具名参数
   *
   * @param name 参数名称
   */
  Arg Named(std::string const& name);

  /**
   * 创建一个与当前命令绑定的参数，此参数可以被指定一次或多次
   *
   * 此参数必须是最后一个参数
   *
   * @param name 参数名称
   */
  Arg Some(std::string const& name);

  /**
   * 创建一个与当前命令绑定的参数，此参数可以被指定零次或多次
   *
   * 此参数必须是最后一个参数
   */
  Arg More();

  /**
   * 为命令定义简述，简述不能跨行
   *
   * @param brief 简述
   */
  std::string Brief(std::string const& brief);

  /**
   * 为命令定义详细描述，详细描述可以跨行
   *
   * @param doc 详细描述
   */
  std::string Doc(std::string const& doc);

 private:
  std::vector<std::string> keywords_{};
  std::vector<Opt> options_{};
  std::vector<Arg> arguments_{};
  std::optional<std::string> brief_{};
  std::optional<std::string> doc_{};
};

}  // namespace cli

#endif