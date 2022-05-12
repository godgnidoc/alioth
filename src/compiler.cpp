#ifndef __impl_alioth__
#define __impl_alioth__

#include "compiler.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>

#include "logging.hpp"

#ifndef __NAME
#define __NAME undefined
#endif
#ifndef __VERSION
#define __VERSION undefined
#endif
#ifndef __BRIEF
#define __BRIEF undefined
#endif
#ifndef __ARCH
#define __ARCH undefined
#endif
#ifndef __OS
#define __OS undefined
#endif
#ifndef __AUTHOR
#define __AUTHOR undefined
#endif
#ifndef __EMAIL
#define __EMAIL undefined
#endif
#ifndef __LICENSE
#define __LICENSE undefined
#endif

#define __QUOT2(X) #X
#define __QUOT(X) __QUOT2(X)

namespace alioth {

using namespace std::string_literals;

compiler::compiler() : logger(logging::logger::root(__QUOT(__NAME))) {
    name = __QUOT(__NAME);
    version = __QUOT(__VERSION);
    author = __QUOT(__AUTHOR);
    email = __QUOT(__EMAIL);
    brief = __QUOT(__BRIEF);
    arch = __QUOT(__ARCH);
    os = __QUOT(__OS);
    cert = __QUOT(__LICENSE);

    register_global_options();

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