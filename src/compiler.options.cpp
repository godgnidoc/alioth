#ifndef __options_impl__
#define __options_impl__

#include "compiler.hpp"

void alioth::compiler::register_global_options() {
    /** begin-code-gen-mark:global-options */
    regist_global_option(OPTION_GLOBAL_REPOSITORY, (cli::option){
        name : "--global-repository",
        brief : "usage: --global-repository <path/to/global/repository>\n                                default: \"/lib/alioth/packages\"\n                                specify the location where to find global packages\n",
        args : 1,
        times : 1,
        required : false
    });
    
    regist_global_option(OPTION_LOGGING_CONFIG, (cli::option){
        name : "--logging-config",
        brief : "usage: --logging-config <path/to/logging.json>\n                                default: \"${configure_home}/logging.json\"\n                                specify the loggin config file\n                                nothing affected except an warning when failed\n",
        args : 1,
        times : 1,
        required : false
    });
    
    regist_global_option(OPTION_LOGGING_LEVEL, (cli::option){
        name : "--logging-level",
        brief : "usage: --logging-level <LEVEL>\n                                default: \"HINT\"\n                                specify the logging filtering level\n",
        args : 1,
        times : 1,
        required : false
    });
    
    regist_global_option(OPTION_WORKSPACE_PATH, (cli::option){
        name : "--workspace",
        brief : "usage: --workspace <path/to/workspace>\n                                default: \"${current_path}\"\n                                specify the workspace path\n",
        args : 1,
        times : 1,
        required : false
    });
    /** end-code-gen-mark:global-options */
}

#endif