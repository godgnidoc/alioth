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

compiler::compiler() : logger(logging::logger::root("alioth")) {
    name = "alioth";
    version = __QUOT(__VERSION);
    author = "godgnidoc";
    email = "godgnidoc@gmail.com";
    brief = "compiler of the Alioth programming language";
    arch = __QUOT(__ARCH);
    os = __QUOT(__OS);
    cert = "MIT";

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
    m_configure_home = std::filesystem::path("/etc/alioth");
    m_workspace_path = std::filesystem::current_path();

    /** begin-code-gen-mark:global-options */
    regist_global_option(GLOBAL_REPOSITORY, (cli::option){
        name : "--global-repository",
        brief : "usage: --global-repository <path/to/global/repository>\n                                default: \"/lib/alioth/packages\"\n                                specify the location where to find global packages\n",
        args : 1,
        times : 1,
        required : false
    });
    
    regist_global_option(LOGGING_TEMPLATE, (cli::option){
        name : "--logging-template",
        brief : "usage: --logging-template <path/to/logging.json>\n                                default: \"${configure_home}/logging.json\"\n                                specify the loggin template file\n                                nothing affected except an warning when failed\n",
        args : 1,
        times : 1,
        required : false
    });
    
    regist_global_option(WORKSPACE_PATH, (cli::option){
        name : "--workspace",
        brief : "usage: --workspace <path/to/workspace>\n                                default: \"${current_path}\"\n                                specify the workspace path\n",
        args : 1,
        times : 1,
        required : false
    });
    /** end-code-gen-mark:global-options */

    return entry(cmd);
    if (cmd.more().size() == 0) {
        return 1;
    }
}

}  // namespace alioth

int main(int argc, char** argv) {
    alioth::compiler compiler;
    return compiler.execute(argc, argv);
}

#endif