#ifndef __impl_alioth__
#define __impl_alioth__

#include "compiler.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <type_traits>

#include "logging.hpp"

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

    interrupter = std::bind(&compiler::common_entry, this, std::placeholders::_1, std::placeholders::_2);
}

int compiler::compile(cli::commandline cmd) {
    using namespace logging;
    auto& logger = logger::root().child("test");
    auto& tmpl = templating::instance();
    tmpl.style(JSON);
    tmpl.format(ERROR, "$1[ $(.month). $(.mday), $(.year) $(.hour):$(.minute):$(.sec) $rERROR $c$(?.scope){$(.scope)}$(?.begl){:$(.begl)}$(?.begc){:$(.begc)} $0$1]$0 $(.msg)");
    logger.set_scope(__FILE__);
    // logger.error("Hello", {});
    logger.error( {.begin={.line=1,.column=2}, .end={.line=3, .column=4}}, "file $c$(file)$0 not found", {{"file", "test.alioth"}});
    return 0;
}

int compiler::common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry) {
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