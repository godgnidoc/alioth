#ifndef __ALIOTH_LOGGING_H__
#define __ALIOTH_LOGGING_H__

#include <optional>

#include "spdlog/spdlog.h"

namespace alioth {

/**
 * 日志器封装
 *
 * !!! 系统中所有日志器都应当使用此封装版
 * !!! 否则日志后端初始化动作将多线程不安全
 *
 * 在后端初始化前被打印的日志会被缓冲，直到后端初始化完成后才会被打印
 * 构造过程有加锁，应当避免在多线程环境下频繁创建日志器
 */
class Logger final {
 public:
  Logger(std::string const& name = "");
  Logger(Logger const&) = default;
  Logger(Logger&&) = default;
  ~Logger() = default;

  spdlog::logger* operator*() const;
  spdlog::logger* operator->() const;
  spdlog::logger* get() const;

 private:
  std::shared_ptr<spdlog::logger> const logger_;
};

/**
 * 初始化日志系统后端
 *
 * 使用指定的后端初始化日志系统
 * - 日志系统初始化后创建的日志器拥有最新的后端
 * - 日志系统初始化前创建的日志器后端会被替换，此过程多线程安全
 *
 * @param sinks 日志后端
 */
void InitLogging(std::vector<spdlog::sink_ptr> const& sinks);

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(std::exception const& errors);

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(std::vector<std::string> const& errors);

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(Logger const& logger, std::exception const& errors);

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(Logger const& logger, std::vector<std::string> const& errors);

}  // namespace alioth

#endif