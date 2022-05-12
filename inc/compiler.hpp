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
         * --logging-config <path/to/logging.json>
         * @default "${configure_home}/logging.json"
         * @brief specify the loggin config file
         * nothing affected except an warning when failed
         */
        OPTION_LOGGING_CONFIG = 1024,
        /**
         * --workspace <path/to/workspace>
         * @default "${current_path}"
         * @brief specify the workspace path
         */
        OPTION_WORKSPACE_PATH,
        /**
         * --global-repository <path/to/global/repository>
         * @default "/lib/alioth/packages"
         * @brief specify the location where to find global packages
         */
        OPTION_GLOBAL_REPOSITORY,
        /**
         * --logging-level <LEVEL>
         * @default "HINT"
         * @brief specify the logging filtering level
         */
        OPTION_LOGGING_LEVEL,
    };

   protected:
    int compile(cli::commandline cmd);
    int common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry);
    void register_global_options();
    bool load_logging_config(const std::filesystem::path& fpath );

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