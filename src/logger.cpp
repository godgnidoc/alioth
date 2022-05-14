#ifndef __logger_impl__
#define __logger_impl__

#include "logger.hpp"

namespace alioth {

int logging_error_code(const std::string& name) {
    static std::map<std::string, int> error_code_table = {
        /** begin-code-gen-mark:logging-error-code */
        {"file_not_found", 0},
        {"unrecognized_token", 1},
        /** end-code-gen-mark:logging-error-code */
    };
    auto it = error_code_table.find(name);
    if( it == error_code_table.end() )
        return -1;
    return it->second;
};

bool load_logging_config(std::istream& is) {
    using namespace logging;
    auto config = json::decode_json(is);
    templating::format(ERROR, (std::string)config["logging"]["formats"]["ERROR"]);
    templating::format(WARN, (std::string)config["logging"]["formats"]["WARN"]);
    templating::format(INFO, (std::string)config["logging"]["formats"]["INFO"]);
    templating::format(HINT, (std::string)config["logging"]["formats"]["HINT"]);
    templating::format(DEBUG, (std::string)config["logging"]["formats"]["DEBUG"]);
    for (auto& [ename, tmpl] : config["logging"]["templates"].asObject()) {
        auto eno = logging_error_code(ename);
        if( eno >= 0 ) {
            templating::message(eno, (std::string)tmpl);
        } else {
            logging::logger::root().warn("undefined error tag '$1$c$(e)$0' ignored", {
                {"e", ename}
            });
        }
    }
    return true;
}

}  // namespace alioth

#endif