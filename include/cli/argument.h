#ifndef __CLI_ARGUMENT_H__
#define __CLI_ARGUMENT_H__

#include <memory>
#include <string>
#include <vector>

namespace cli {

class Argument;
using Arg = std::shared_ptr<Argument>;

/**
 * 参数抽象
 */
class Argument {
  friend class Application;
  friend class Inspect;

 public:
  /**
   * 常规参数名由字母和数字构成
   *
   * 参数名后追加省略号表示参数可以出现一次或多次
   *
   * 仅由省略号构成的参数表示剩余参数，可以出现零次或多次
   *
   * @param name 参数名
   */
  Argument(std::string const& name);

  /**
   * 检查此参数是否被指定过至少一次
   */
  bool HasValue() const;

  /**
   * 尝试获取此参数的第一个值
   *
   * 若参数没有被指定过，则抛出异常
   */
  std::string Value() const;

  /**
   * 尝试获取此参数的所有值
   *
   * 若参数没有被指定过，则返回空数组
   */
  std::vector<std::string> Values() const;

 private:
  std::string name_{};
  std::vector<std::string> values_{};
};

}  // namespace cli

#endif