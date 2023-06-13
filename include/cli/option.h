#ifndef __CLI_OPTION_H__
#define __CLI_OPTION_H__

#include <memory>
#include <string>
#include <vector>

namespace cli {

class Option;
using Opt = std::shared_ptr<Option>;

class Argument;
using Arg = std::shared_ptr<Argument>;

/**
 * 选项抽象
 */
struct Option : public std::enable_shared_from_this<Option> {
  friend class Application;
  friend class Inspect;

 public:
  /**
   * 创建一个选项
   *
   * 支持长选项和短选项
   *
   * - 长选项必须由 '--' 开头，包含一或多个字母或数字
   * - 短选项必须由 '-' 开头，包含一个字母或数字
   *
   * @param flags 选项标志
   */
  Option(std::vector<std::string> const& flags);

  /**
   * 将当前选项标记为必选
   */
  Opt Required();

  /**
   * 将当前选项标记为可重复
   */
  Opt Repeatable();

  /**
   * 选项可以携带一个具名参数
   */
  Opt Argument(std::string const& name);

  /**
   * 为命令定义简述，简述不能跨行
   *
   * @param brief 简述
   */
  Opt Brief(std::string const& brief);

  /**
   * 为命令定义详细描述，详细描述可以跨行
   *
   * @param doc 详细描述
   */
  Opt Doc(std::string const& doc);

  /**
   * 检查此选项是否被指定过至少一次
   */
  bool HasValue() const;

  /**
   * 尝试获取此选项的第一个值
   *
   * 若选项没有被指定过，则抛出异常
   */
  std::string Value() const;

  /**
   * 尝试获取此选项的所有值
   *
   * 若选项没有被指定过，则返回空数组
   */
  std::vector<std::string> Values() const;

 private:
  std::vector<std::string> flags_;
  bool required_{false};
  bool repeatable_{false};
  Arg argument_{};
  std::optional<std::string> brief_{};
  std::optional<std::string> doc_{};

  std::vector<std::string> values_{};
};

}  // namespace cli

#endif