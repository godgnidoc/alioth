#ifndef __logger__
#define __logger__

#include <istream>

#include "logger.helper.hpp"
#include "logging.hpp"

namespace alioth {

/**
 * 加载日志配置文件，对全局日志器进行设置
 */
bool load_logging_config(std::istream& is);

}  // namespace alioth

#endif