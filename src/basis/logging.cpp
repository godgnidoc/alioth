#ifndef __logging_impl__
#define __logging_impl__

#include "logging.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

#include "utils.hpp"

namespace alioth::logging {

templating::node* templating::fmt[4] = {};
std::map<int, templating::node*> templating::msgs;
style_t templating::m_style = COLORED_TEXT;
bool templating::sealed = false;

point::operator bool() const { return line > 0 && column > 0; }

range::operator bool() const { return begin && end; }

templating::node::~node() {
    for (auto n : children) delete n;
}

void templating::seal() { sealed = true; }

void templating::format(severity sev, const std::string& format) {
    watchdog();
    auto n = compile(format);
    if (fmt[sev]) delete fmt[sev];
    fmt[sev] = n;
}

void templating::message(int eno, const std::string& format) {
    watchdog();
    auto n = compile(format);
    if (msgs.count(eno)) delete msgs[eno];
    msgs[eno] = n;
}

void templating::style(logging::style_t style) { m_style = style; }

std::string templating::render(int eno, const record::arguments& args, style_t style) {
    auto in = msgs.find(eno);
    if (in == msgs.end()) throw std::out_of_range(interlog("message template for eno " + strify(eno) + " not found"));
    if (style == UNSET) style = m_style;
    return _render(in->second, args, style);
}

std::string templating::render(const std::string& tmpl, const record::arguments& args, style_t style) {
    auto n = compile(tmpl);
    if (style == UNSET) style = m_style;
    return _render(n, args, style);
}

std::string templating::render(const record& record, const std::string& message) {
    static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char* wdays[] = {"Sun", "Mon", "Wed", "Thurs", "Fri", "Sat"};
    static const char* severities[] = {"ERROR", "WARN", "INFO", "HINT", "DEBUG"};
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto tm = *std::localtime(&now);
    record::arguments args = {{".msg", message},
                              {".begl", record.rng ? strify(record.rng.begin.line) : ""},
                              {".endl", record.rng ? strify(record.rng.end.line) : ""},
                              {".begc", record.rng ? strify(record.rng.begin.column) : ""},
                              {".endc", record.rng ? strify(record.rng.end.column) : ""},
                              {".year", strify(tm.tm_year + 1900)},
                              {".month", months[tm.tm_mon]},
                              {".mday", strify(tm.tm_mday)},
                              {".wday", wdays[tm.tm_wday]},
                              {".hour", strify(tm.tm_hour)},
                              {".minute", strify(tm.tm_min)},
                              {".sec", strify(tm.tm_sec)},
                              {".time", strify(now * 1000)},
                              {".scope", record.scope},
                              {".name", record.name},
                              {".path", record.path}};

    if (m_style == JSON) {
        args[".msg"] = std::visit([&](auto& msg) { return render(msg, record.args, COLORED_TEXT); }, record.msg);
        auto colored = _render(fmt[record.sev], args, COLORED_TEXT);

        args[".msg"] = std::visit([&](auto& msg) { return render(msg, record.args, PLAIN_TEXT); }, record.msg);
        auto plain = _render(fmt[record.sev], args, PLAIN_TEXT);

        args[".msg"] = std::visit([&](auto& msg) { return render(msg, record.args, HTML); }, record.msg);
        auto html = _render(fmt[record.sev], args, HTML);
        json_object_t res = {
            {"severity", severities[record.sev]},
            {"timestamp", now * 1000},
            {"name", record.name},
            {"path", record.path},
            {"scope", record.scope},
            {"range", record.rng ? json((json_object_t){
                                       {"begin", json({{"line", record.rng.begin.line}, {"column", record.rng.begin.column}})},
                                       {"end", json({{"line", record.rng.end.line}, {"column", record.rng.end.column}})}})
                                 : json(json::null)},
            {"message", plain},
            {"colored-message", colored},
            {"html-message", html}};
        return json(res).encode_json();
    } else {
        return _render(fmt[record.sev], args, m_style);
    }
}

templating::node* templating::compile(const std::string& code) {
    auto start = code.c_str();
    return _compile_sequence(start, '\0');
}

templating::node* templating::_compile_sequence(const char*& code, char term) {
    node* n = nullptr;  // node
    std::string cond;
    int state = 1;
    bool stay = false;

    /** 向文ty点或序列中的最后一个ty节点追加文本，尝试创建ty节点以满足条件 */
    auto append = [&](char c, enum node::type ty) {
        if (!n) {
            n = new node{.type = ty, .value = ""s + c};
        } else if (n->type == ty) {
            n->value += c;
        } else if (n->type == node::sequence) {
            if (n->children.empty() or n->children.back()->type != ty) {
                n->children.push_back(new node{.type = ty, .value = ""s + c});
            } else {
                n->children.back()->value += c;
            }
        } else {
            n = new node{.type = node::sequence, .children = {n, new node{.type = ty, .value = ""s + c}}};
        }
    };

    while (state > 0) {
        auto c = *code;

        if ((c == term or c == '\0') && state != 1) {
            throw std::runtime_error(interlog("unterminated expression in template"));
        }

        switch (state) {
            case 1: {
                if (c == '$') {
                    state = 2;
                } else if (c == term) {
                    stay = true;
                    state = 0;
                } else {
                    append(c, node::text);
                }
            } break;
            case 2: {
                switch (c) {
                    case '$': {
                        append(c, node::text);
                    } break;
                    case '(': {
                        state = 3;
                    } break;
                    case '0':
                    case '1':
                    case 'a' ... 'z':
                    case 'A' ... 'Z':
                        if (!n)
                            n = new node{.type = node::color, .value = ""s + c};
                        else if (n->type == node::sequence)
                            n->children.push_back(new node{.type = node::color, .value = ""s + c});
                        else
                            n = new node{.type = node::sequence,
                                         .children = {n, new node{.type = node::color, .value = ""s + c}}};
                        state = 1;
                        break;
                    default:
                        throw std::runtime_error("invalid escape character '"s + c + "'");
                }
            } break;
            case 3: {
                if (c == '?') {
                    state = 200;
                    cond.clear();
                } else {
                    state = 100;
                    stay = true;
                }
            } break;
            case 100: {
                if (c == ')') {
                    state = 1;
                } else {
                    append(c, node::variable);
                }
            } break;
            case 200: {
                if (c == ')') {
                    state = 201;
                } else {
                    cond += c;
                }
            } break;
            case 201: {
                if (c == '{') {
                    state = 202;
                } else {
                    throw std::runtime_error("invalid character '"s + c + "' after condition '" + n->condition + "'");
                }
            } break;
            case 202: {
                auto i = _compile_sequence(code, '}');
                i->condition = cond;
                if (n->type == node::sequence) {
                    n->children.push_back(i);
                } else {
                    n = new node{.type = node::sequence, .children = {n, i}};
                }
                state = 1;
            } break;
        }
        if (stay)
            stay = false;
        else
            code += 1;
    }

    return n;
}

std::string templating::_render(node* n, const record::arguments& args, style_t s) {
    std::string suffix;
    return _render(n, args, s, suffix, 0);
}

std::string templating::_render(node* n, const record::arguments& args, style_t s, std::string& suffix, int depth) {
    if (!n) throw std::invalid_argument(interlog("null pointer received for template structure"));

    if (!n->condition.empty()) {
        bool abandon = false;
        if (n->condition[0] == '!') {
            auto cond = n->condition.substr(1);
            if (args.count(cond) && args.at(cond).length()) abandon = true;
        } else {
            auto& cond = n->condition;
            if (!args.count(cond) || args.at(cond).empty()) abandon = true;
        }
        if (abandon) return "";
    }

    switch (n->type) {
        case node::text: {
            return n->value;
        } break;
        case node::variable: {
            std::string res;
            if (args.count(n->value)) res = args.at(n->value);
            if (s == HTML) return "<code class='log code'>" + res + "</code>";
            return res;
        } break;
        case node::sequence: {
            std::string result;
            for (auto i : n->children) result += _render(i, args, s, suffix, depth + 1);
            if (s == HTML && depth == 0) return "<span class='log'>" + result + "</span>";
            return result;
        } break;
        case node::color: {
            switch (s) {
                case PLAIN_TEXT:
                    return "";
                case COLORED_TEXT: {
                    switch (n->value[0]) {
                        case 'r':
                            return "\033[31m";
                        case 'g':
                            return "\033[32m";
                        case 'y':
                            return "\033[33m";
                        case 'b':
                            return "\033[34m";
                        case 'p':
                            return "\033[35m";
                        case 'c':
                            return "\033[36m";
                        case 'w':
                            return "\033[37m";
                        case '1':
                            return "\033[1m";
                        case '0':
                            return "\033[0m";
                        default:
                            throw std::invalid_argument("invalid color representation '"s + n->value + "'");
                    }
                } break;
                case HTML: {
                    std::string color;
                    switch (n->value[0]) {
                        case 'r':
                            color = "red";
                            break;
                        case 'g':
                            color = "green";
                            break;
                        case 'y':
                            color = "yellow";
                            break;
                        case 'b':
                            color = "blue";
                            break;
                        case 'p':
                            color = "purple";
                            break;
                        case 'c':
                            color = "cyan";
                            break;
                        case 'w':
                            color = "white";
                            break;
                        case '1':
                            color = "bold";
                            break;
                        case '0':
                            return std::move(suffix);
                        default:
                            throw std::invalid_argument("invalid color representation '"s + n->value + "'");
                    }
                    suffix = "</span>" + suffix;
                    return "<span class='log " + color + "'>";
                } break;
            }
        } break;
    }

    return "";
}

void templating::watchdog() {
    if (sealed) throw std::runtime_error(interlog("trying to modify sealed templating instance is not allowed"));
}

logger::logger(const std::string& name, logger* parent) : m_name(name), m_parent(parent), m_level(DEBUG) {}

void logger::debug(const record::message& msg, const record::arguments& args) { emit(DEBUG, range(), msg, args); }
void logger::debug(const range& rng, const record::message& msg, const record::arguments& args) { emit(DEBUG, rng, msg, args); }

void logger::hint(const record::message& msg, const record::arguments& args) { emit(HINT, range(), msg, args); }
void logger::hint(const range& rng, const record::message& msg, const record::arguments& args) { emit(HINT, rng, msg, args); }

void logger::info(const record::message& msg, const record::arguments& args) { emit(INFO, range(), msg, args); }
void logger::info(const range& rng, const record::message& msg, const record::arguments& args) { emit(INFO, rng, msg, args); }

void logger::warn(const record::message& msg, const record::arguments& args) { emit(WARN, range(), msg, args); }
void logger::warn(const range& rng, const record::message& msg, const record::arguments& args) { emit(WARN, rng, msg, args); }

void logger::error(const record::message& msg, const record::arguments& args) { emit(ERROR, range(), msg, args); }
void logger::error(const range& rng, const record::message& msg, const record::arguments& args) { emit(ERROR, rng, msg, args); }

logger& logger::root(const std::string& name) {
    static logger* root_logger = nullptr;
    if (!root_logger) {
        templating::format(ERROR,
                           "$1[ $rERROR $c$(?.scope){$(.scope)$(?.begl){:$(.begl)$(?.begc){:$(.begc)}} }$0$1]$0 $(.msg)");
        templating::format(WARN, "$1[ $pWARN $c$(?.scope){$(.scope)$(?.begl){:$(.begl)$(?.begc){:$(.begc)}} }$0$1]$0 $(.msg)");
        templating::format(INFO, "$1[ $bINFO $c$(?.scope){$(.scope)$(?.begl){:$(.begl)$(?.begc){:$(.begc)}} }$0$1]$0 $(.msg)");
        templating::format(HINT, "$1[ $cHINT $c$(?.scope){$(.scope)$(?.begl){:$(.begl)$(?.begc){:$(.begc)}} }$0$1]$0 $(.msg)");
        templating::format(
            DEBUG,
            "$1[ $(.month). $(.mday), $(.year) $(.hour):$(.minute):$(.sec) DEBUG "
            "$c$(?.scope){$(.scope)$(?.begl){:$(.begl)$(?.begc){:$(.begc)}} }$0$1]$0 $(?.path){$(.path): }$(.msg)");
        root_logger = new logger(name);
    }
    return *root_logger;
}

logger& logger::child(const std::string& name) {
    auto i = m_children.find(name);
    if (i == m_children.end()) {
        auto child = new logger(name, this);
        m_children.insert({name, child});
        return *child;
    } else {
        return *i->second;
    }
}

void logger::set_scope(const std::string& scope) { m_scope = scope; }

void logger::set_handler(handler hdlr) { m_handler = hdlr; }

void logger::emit(severity sev, const range& rng, const record::message& msg, const record::arguments& args) {
    if (sev > m_level) return;
    auto log = record{.tim = time(nullptr),
                      .sev = sev,
                      .scope = m_scope,
                      .name = m_name,
                      .path = get_path(),
                      .rng = rng,
                      .msg = msg,
                      .args = args};
    auto message = format(log);
    handle(log, message);
}

std::string logger::format(const record& record) {
    auto message = std::visit([&](auto&& v) { return templating::render(v, record.args); }, record.msg);
    return templating::render(record, message);
}

void logger::handle(const record& record, const std::string& rendered) {
    if (m_handler)
        m_handler(record, rendered);
    else if (m_parent)
        m_parent->handle(record, rendered);
    else
        std::cerr << rendered << std::endl;
}

std::string logger::get_path() {
    std::string path = m_parent ? m_parent->get_path() : ""s;
    if (path.length())
        if (m_name.length())
            return path + "." + m_name;
        else
            return path;
    else
        return m_name;
}

}  // namespace alioth::logging

#endif