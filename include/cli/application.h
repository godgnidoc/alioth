#ifndef __CLI_APPLICATION_H__
#define __CLI_APPLICATION_H__

#include <string>
#include <vector>

#include "cli/argument.h"
#include "cli/command.h"
#include "cli/option.h"

namespace cli {

struct Scope {
  Cmd command;
  std::map<std::string, Scope> subcommands;
};

/**
 * 应用程序抽象
 */
class Application : public Scope {
  friend class Inspect;

 public:
  /**
   * 命令行选项终止符，表示后续参数应当被视为命令参数而非选项
   */
  static constexpr char const* kEOO = "--";

  /**
   * 解析命令行参数，执行命令
   *
   * @param argc 命令行参数个数
   * @param argv 命令行参数列表
   */
  static int Run(int argc, char** argv);

  /**
   * 解析命令行参数，执行命令
   *
   * 参数列表中的第一个参数被视为命令行应用名称
   */
  static int Run(std::vector<std::string> args);

  /**
   * 验证命令行应用定义是否存在冲突
   */
  void Validate();

  /**
   * 设置应用程序名称
   *
   * @param name 应用程序名称
   */
  static void Name(std::string const& name);

  /**
   * 设置应用程序简要描述
   *
   * @param brief 应用程序简要描述
   */
  static void Brief(std::string const& brief);

  /**
   * 设置应用程序详细描述
   *
   * @param doc 应用程序详细描述
   */
  static void Doc(std::string const& doc);

  /**
   * 设置应用程序版本号
   *
   * @param version 应用程序版本号
   */
  static void Version(std::string const& version);

  /**
   * 设置应用程序作者信息
   *
   * @param author 应用程序作者信息
   */
  static void Author(std::string const& author);

  /**
   * 注册一个命令
   *
   * @param command 命令实例
   * @param keywords 命令关键字，省略表示注册为默认命令
   */
  static void Command(Cmd command,
                      std::vector<std::string> const& keywords = {});

  /**
   * 为应用定义全局选项
   *
   * @param flags 选项标志，参见选项定义获取标志规范
   */
  static Opt Option(std::vector<std::string> const& flags);

 private:
  static Application& Inst();

 private:
  std::string name_{};
  std::string brief_{};
  std::string doc_{};
  std::string version_{};
  std::string author_{};
  std::vector<Opt> options_{};
};

}  // namespace cli

#endif