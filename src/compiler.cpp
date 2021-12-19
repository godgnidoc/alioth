#ifndef __impl_alioth__
#define __impl_alioth__

#include "compiler.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>

#ifndef __ARCH
#define __ARCH "undefined"
#endif

#ifndef __VERSION
#define __VERSION "undefined"
#endif

#ifndef __OS
#define __OS "undefined"
#endif

namespace alioth {

compiler::compiler() {
    name = "alioth";
    version = __VERSION;
    author = "godgnidoc";
    email = "godgnidoc@gmail.com";
    brief = "compiler of the Alioth programming language";
    arch = __ARCH;
    os = __OS;
    cert = "MIT";

    regist_main_function((cli::function){
        name : "compile",
        brief : "compile source code into module",
        accept_more : true,
        entry : std::bind(&compiler::compile, this, std::placeholders::_1)
    });

    preprocess = std::bind(&compiler::common_entry, this, std::placeholders::_1, std::placeholders::_2);
}

int compiler::compile(cli::commandline cmd) { return 0; }

int compiler::common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry) {
    if( cmd.more().size() == 0 ) {
        return 1;
    }
    return entry(cmd);
}

}  // namespace alioth

int main(int argc, char** argv) {
    alioth::compiler compiler;
    return compiler.execute(argc, argv);
}

#endif