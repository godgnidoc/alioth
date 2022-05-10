#ifndef __alioth__
#define __alioth__

#include <filesystem>
#include <set>

#include "cli.hpp"
#include "logging.hpp"

namespace alioth {

class compiler : public cli::application {
   public:
    compiler();

    /** 全局选项ID */
    enum global_options {
        /** 
         * --logging-template <path/to/logging.json>
         * @default "${configure_home}/logging.json"
         * @brief specify the loggin template file
         * nothing affected except an warning when failed
         */
        LOGGING_TEMPLATE = 1024,
        /**
         * --workspace <path/to/workspace>
         * @default "${current_path}"
         * @brief specify the workspace path
         */
        WORKSPACE_PATH,
        /**
         * --global-repository <path/to/global/repository>
         * @default "/lib/alioth/packages"
         * @brief specify the location where to find global packages
         */
        GLOBAL_REPOSITORY
    };

   protected:
    int compile(cli::commandline cmd);
    int common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry);

   protected:
    /** 日志器 */
    logging::logger& logger;

    /** 配置信息目录 */
    std::filesystem::path m_configure_home;

    /** 当前工作空间 */
    std::filesystem::path m_workspace_path;
};

}  // namespace alioth

#endif