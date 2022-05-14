#ifndef __impl_alioth__
#define __impl_alioth__

#include "compiler.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>

#include "logger.hpp"
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
    using logging::severity;
    m_configure_home = std::filesystem::absolute(std::filesystem::path("/etc/alioth")).lexically_normal();
    m_workspace_path = std::filesystem::current_path();
    logger->set_level(logging::HINT);

    if (cmd[OPTION_LOGGING_LEVEL].size()) {
        auto level = cmd[OPTION_LOGGING_LEVEL][0][0];
        if (level == "ERROR")
            logger->set_level(logging::ERROR);
        else if (level == "WARN")
            logger->set_level(logging::WARN);
        else if (level == "INFO")
            logger->set_level(logging::INFO);
        else if (level == "HINT")
            logger->set_level(logging::HINT);
        else if (level == "DEBUG")
            logger->set_level(logging::DEBUG);
        else
            logger->warn("unsupported logging level '$1$c$(l)$0', ignored", {{"l", level}});
    }

    auto logging_config_path = m_configure_home / std::filesystem::path("logging.json"s);
    if (cmd[OPTION_LOGGING_CONFIG].size()) logging_config_path = cmd[OPTION_LOGGING_CONFIG][0][0];
    logging_config_path = std::filesystem::absolute(logging_config_path).lexically_normal();

    auto is = std::ifstream(logging_config_path);
    if (!is) {
        logger->error("error loading logging config: cannot open file '$1$c$(p)$0'", {{"p", logging_config_path}});
        return false;
    }
    try {
        load_logging_config(is);
    } catch (std::exception& e) {
        logger->error("error loading logging config: $(what)", {{"what", e.what()}});
    }

    return entry(cmd);
}
}  // namespace alioth

int main(int argc, char** argv) {
    alioth::compiler compiler;
    return compiler.execute(argc, argv);
}

#endif