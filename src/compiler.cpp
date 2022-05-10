#ifndef __impl_alioth__
#define __impl_alioth__

#include "compiler.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>

#include "logging.hpp"

#ifndef __ARCH
#define __ARCH undefined
#endif

#ifndef __VERSION
#define __VERSION undefined
#endif

#ifndef __OS
#define __OS undefined
#endif

#define __QUOT2(X) #X
#define __QUOT(X) __QUOT2(X)

namespace alioth {

using namespace std::string_literals;

compiler::compiler() : logger(logging::logger::root("alioth")) {
    name = "alioth";
    version = __QUOT(__VERSION);
    author = "godgnidoc";
    email = "godgnidoc@gmail.com";
    brief = "compiler of the Alioth programming language";
    arch = __QUOT(__ARCH);
    os = __QUOT(__OS);
    cert = "MIT";

    /** begin-code-gen-mark:global-options */
    regist_global_option(OPTION_GLOBAL_REPOSITORY, (cli::option){
        name : "--global-repository",
        brief :
            "usage: --global-repository <path/to/global/repository>\n                                default: "
            "\"/lib/alioth/packages\"\n                                specify the location where to find global packages\n",
        args : 1,
        times : 1,
        required : false
    });

    regist_global_option(OPTION_LOGGING_CONFIG, (cli::option){
        name : "--logging-config",
        brief : "usage: --logging-config <path/to/logging.json>\n                                default: "
                "\"${configure_home}/logging.json\"\n                                specify the loggin config file\n          "
                "                      nothing affected except an warning when failed\n",
        args : 1,
        times : 1,
        required : false
    });

    regist_global_option(OPTION_WORKSPACE_PATH, (cli::option){
        name : "--workspace",
        brief : "usage: --workspace <path/to/workspace>\n                                default: \"${current_path}\"\n        "
                "                        specify the workspace path\n",
        args : 1,
        times : 1,
        required : false
    });
    /** end-code-gen-mark:global-options */

    regist_main_function((cli::function){
        name : "build",
        brief : "compile source code into module",
        accept_more : true,
        entry : std::bind(&compiler::compile, this, std::placeholders::_1)
    });

    interrupter = std::bind(&compiler::common_entry, this, std::placeholders::_1, std::placeholders::_2);
}

int compiler::compile(cli::commandline cmd) { return 0; }

int compiler::common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry) {
    m_configure_home = std::filesystem::absolute(std::filesystem::path("/etc/alioth")).lexically_normal();
    m_workspace_path = std::filesystem::current_path();

    if (cmd[OPTION_LOGGING_CONFIG].size()) {
        m_logging_config = cmd[OPTION_LOGGING_CONFIG][0][0];
    } else {
        m_logging_config = m_configure_home / std::filesystem::path("logging.json"s);
    }
    logger.debug(std::filesystem::absolute(m_logging_config).lexically_normal());
    logger.debug(std::filesystem::absolute(m_configure_home).lexically_normal());
    logger.debug(m_workspace_path);


    return entry(cmd);
}

}  // namespace alioth

int main(int argc, char** argv) {
    alioth::compiler compiler;
    return compiler.execute(argc, argv);
}

#endif