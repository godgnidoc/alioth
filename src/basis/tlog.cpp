#ifndef __tlog_cpp__
#define __tlog_cpp__

#include "tlog.hpp"

#include <ctime>
#include <iostream>

#ifdef _WIN32
#define sprintf sprintf_s
#endif

namespace alioth {
namespace tlog {

logt logt::compile(const json& tmpl) {
    logt ret;
    ret.format = compile_fragment(tmpl["format"].asString());
    for (auto i = 0; i < 4; i++) ret.severities[i] = compile_fragment(tmpl["severity"][i].asString());
    for (auto it : tmpl["message"].asObject()) {
        auto code = std::stoi(it.first);
        logti ti;
        ti.code = code;
        ti.severity = it.second["sev"].asInt();  //[TODO] 检查严重性代码是否合法
        ti.frags = compile_fragment(it.second["msg"].asString());
        ret.message[code] = ti;
    }
    return ret;
}

chainz<logtf> logt::compile_fragment(const char*& ptr, uint32_t cond) {
    chainz<logtf> frags;

    int state = 1;
    bool stay = false;
    auto base = ptr;
    const char term = (cond ? '}' : '\0');
    uint32_t inner_cond;

    while (state > 0) {
        switch (state) {
            case 1: {
                if (*ptr == term) {
                    if (ptr != base) {
                        logtf tf;
                        tf.condition = cond;
                        tf.content = VAR::STRING;
                        tf.text = std::string(base, ptr);
                        frags << tf;  // logtf { condition: cond, content: VAR::STRING, text: std::string(base, ptr) };
                    }
                    state = 0;
                } else if (*ptr == '\0') {
                    throw std::runtime_error("tlog::logger::compile_fragment: unexcepted terminate !");
                } else if (*ptr == '%') {
                    if (ptr != base) {
                        logtf tf;
                        tf.condition = cond;
                        tf.content = VAR::STRING;
                        tf.text = std::string(base, ptr);
                        frags << tf;  // logtf { condition: cond, content: VAR::STRING, text: std::string(base, ptr) };
                        base = ptr;
                    }
                    state = 2;
                } else {
                    /* [DO NOTHING] */
                }
            } break;
            case 2: {
                if (*ptr == '%') {
                    base = ptr;
                    state = 1;
                } else if (*ptr == '?') {
                    state = 3;
                } else if (*ptr == ':') {
                    state = 5;
                } else if (*ptr == '@') {
                    state = 6;
                } else if (*ptr >= '0' && *ptr <= '9') {
                    stay = true;
                    state = 5;
                } else {
                    throw std::runtime_error("tlog::logger::compile_fragment: bad placeholder !");
                }
            } break;
            case 3: {
                inner_cond = extract_placeholder(*ptr);
                if (inner_cond == 0) {
                    throw std::runtime_error("tlog::logger::compile_fragment: unknown variable placeholder !");
                } else if ((inner_cond & VAR::COND_MASK) == 0) {
                    throw std::runtime_error("tlog::logger::compile_fragment: unavaliable variable for condition !");
                }
                state = 4;
            } break;
            case 4: {
                if (*ptr == '{') {
                    frags += compile_fragment(++ptr, cond | inner_cond);
                    state = 1;
                    stay = true;
                    base = ptr;
                } else {
                    throw std::runtime_error("tlog::logger::compile_fragment: '{' missing after conditon !");
                }
            } break;
            case 5: {
                auto var = extract_placeholder(*ptr);
                if (var == 0) {
                    throw std::runtime_error("tlog::logger::compile_fragment: '{' missing after conditon !");
                }
                logtf tf;
                tf.condition = cond;
                tf.content = var;
                frags << tf;  // logtf { condition:cond, content: var };
                state = 1;
                base = ptr + 1;
            } break;
            case 6: {
                uint32_t var;
                switch (*ptr) {
                    case 'r':
                        var = VAR::RED;
                        break;
                    case 'g':
                        var = VAR::GREEN;
                        break;
                    case 'b':
                        var = VAR::BLUE;
                        break;
                    case 'y':
                        var = VAR::YELLOW;
                        break;
                    case 'p':
                        var = VAR::PURPLE;
                        break;
                    case 'c':
                        var = VAR::CYAN;
                        break;
                    case '1':
                        var = VAR::BLOB;
                        break;
                    case '0':
                        var = VAR::RECOVER;
                        break;
                    default:
                        throw std::runtime_error("tlog::logger::compile_fragment: unsupported color placeholder !");
                }
                logtf tf;
                tf.condition = cond;
                tf.content = var;
                frags << tf;  // logtf{ condition: cond, content: var };
                state = 1;
                base = ptr + 1;
            } break;
        }

        if (stay)
            stay = false;
        else
            ptr += 1;
    }

    return frags;
}

chainz<logtf> logt::compile_fragment(const std::string& expr) {
    // auto it = expr.cbegin();
    auto ptr = expr.data();
    return compile_fragment(ptr);
}

uint32_t logt::extract_placeholder(char c) {
    switch (c) {
        case 'c':
            return VAR::COLUMN;
        case 'C':
            return VAR::COLUMN_END;
        case 'd':
            return VAR::DATE_YYYYMMDD;
        case 'D':
            return VAR::DATE_MMMDDYYYY;
        case 'E':
            return VAR::CODE;
        case 'l':
            return VAR::LINE;
        case 'L':
            return VAR::LINE_END;
        case 'm':
            return VAR::MESSAGE;
        case 'p':
            return VAR::PREFIX;
        case 's':
            return VAR::SEVERITY;
        case 'S':
            return VAR::SEVERITY_CODE;
        case 't':
            return VAR::TIME;
        case 'T':
            return VAR::TIMESTAMP;
        case '0':
            return VAR::ARG0;
        case '1':
            return VAR::ARG1;
        case '2':
            return VAR::ARG2;
        case '3':
            return VAR::ARG3;
        case '4':
            return VAR::ARG4;
        case '5':
            return VAR::ARG5;
        case '6':
            return VAR::ARG6;
        case '7':
            return VAR::ARG7;
        case '8':
            return VAR::ARG8;
        case '9':
            return VAR::ARG9;
        default:
            return 0;
    }
}

logrepo::logrepo() : m_template(nullptr) {}

void logrepo::log(const logi& i) {
    rlock lock(*this);
    auto& ref = m_items[i.prefix];
    auto offset = ref.size();
    while (offset > 0 && ref[offset - 1].timestamp > i.timestamp) offset -= 1;
    ref.insert(i, offset);
}

void logrepo::set_template(logt& tmpl) {
    rlock lock(*this);
    m_template = &tmpl;
}

std::set<std::string> logrepo::prefixs() {
    rlock lock(*this);
    std::set<std::string> res;
    for (auto& it : m_items) res.insert(it.first);
    return res;
}

chainz<logi> logrepo::duplicate(const std::string& prefix, int opt) {
    chainz<logi> org;
    chainz<logi> res;
    logt* tmpl;
    {
        rlock lock(*this);
        org = m_items[prefix];
        tmpl = m_template;
    }
    for (auto& it : org) {
        auto sev = (tmpl ? tmpl->message[it.code].severity : 0);
        if (tmpl != nullptr && ((OPT_NO_ERROR >> sev) & opt) != 0) continue;
        if ((opt & OPT_RECENT_FIRST) != 0) {
            res.insert(it, 0);
        } else {
            res.push(it);
        }
    }
    return res;
}

void shader::set_template(const logt& tmpl) { m_template = tmpl; }
void shader::set_color_mode(color_mode_t mode) { m_color_mode = mode; }

std::string shader::emit_string(const logi& item) { return emit_fragments(m_template.format, item); }

json shader::emit_json(const logi& item) {
    json result = json::object;
    auto& tmpl = m_template.message[item.code];
    result["timestamp"] = item.timestamp;
    result["code"] = item.code;
    result["begin-line"] = item.bl;
    result["begin-column"] = item.bc;
    result["end-line"] = item.el;
    result["end-column"] = item.ec;
    result["severity"] = tmpl.severity;
    result["prefix"] = item.prefix;
    result["message"] = emit_fragments(tmpl.frags, item);
    return result;
}

std::string shader::emit_fragments(const chainz<logtf>& frags, const logi& item) {
    std::string result;
    uint32_t color = VAR::RECOVER;
    for (auto& frag : frags) {
        if (!test_cond(item, frag.condition)) continue;
        switch (frag.content) {
            case VAR::STRING:
                result += render(frag.text, color);
                break;
            case VAR::COLUMN:
            case VAR::COLUMN_END:
            case VAR::DATE_YYYYMMDD:
            case VAR::DATE_MMMDDYYYY:
            case VAR::CODE:
            case VAR::PREFIX:
            case VAR::MESSAGE:
            case VAR::LINE:
            case VAR::LINE_END:
            case VAR::SEVERITY:
            case VAR::SEVERITY_CODE:
            case VAR::TIME:
            case VAR::TIMESTAMP:
            case VAR::ARG0:
            case VAR::ARG1:
            case VAR::ARG2:
            case VAR::ARG3:
            case VAR::ARG4:
            case VAR::ARG5:
            case VAR::ARG6:
            case VAR::ARG7:
            case VAR::ARG8:
            case VAR::ARG9:
                result += render(mkvar(item, frag.content), color);
                break;

            case VAR::RED:
            case VAR::GREEN:
            case VAR::BLUE:
            case VAR::YELLOW:
            case VAR::PURPLE:
            case VAR::CYAN:
            case VAR::BLOB:
                color |= frag.content;
                break;
            case VAR::RECOVER:
                color = VAR::RECOVER;
        }
    }
    return result;
}

std::string shader::mkvar(const logi& item, uint32_t key) {
    switch (key) {
        case VAR::COLUMN:
            return std::to_string(item.bc);
        case VAR::COLUMN_END:
            return std::to_string(item.ec);
        case VAR::DATE_YYYYMMDD: {
            time_t seconds = item.timestamp / 1000;
#ifdef _WIN32
            ::tm tm;
            localtime_s(&tm, &seconds);
#else
            auto tm = *localtime(&seconds);
#endif
            char buf[16];
            sprintf(buf, "%04d/%02d/%02d", tm.tm_year, tm.tm_mon + 1, tm.tm_mday);
            return buf;
        }
        case VAR::DATE_MMMDDYYYY: {
            static const char month[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            time_t seconds = item.timestamp / 1000;
#ifdef _WIN32
            ::tm tm;
            localtime_s(&tm, &seconds);
#else
            auto tm = *localtime(&seconds);
#endif
            char buf[16];
            sprintf(buf, "%s. %02d, %04d", month[tm.tm_mon], tm.tm_mday, tm.tm_year);
            return buf;
        }
        case VAR::CODE:
            return std::to_string(item.code);
        case VAR::LINE:
            return std::to_string(item.bl);
        case VAR::LINE_END:
            return std::to_string(item.el);
        case VAR::MESSAGE: {
            auto& frags = m_template.message[item.code].frags;
            return emit_fragments(frags, item);
        }
        case VAR::PREFIX:
            return item.prefix;
        case VAR::SEVERITY: {
            auto& frags = m_template.severities[m_template.message[item.code].severity];
            return emit_fragments(frags, item);
        }
        case VAR::SEVERITY_CODE:
            return std::to_string(m_template.message[item.code].severity);
        case VAR::TIME: {
            time_t seconds = item.timestamp / 1000;
#ifdef _WIN32
            ::tm tm;
            localtime_s(&tm, &seconds);
#else
            auto tm = *localtime(&seconds);
#endif
            char buf[16];
            sprintf(buf, "%02d:%02d:%02d.%03ld", tm.tm_hour, tm.tm_min, tm.tm_sec, item.timestamp % 1000);
            return buf;
        }
        case VAR::TIMESTAMP:
            return std::to_string(item.timestamp);
        case VAR::ARG0:
            return (item.args.size() > 0) ? item.args[0] : "";
        case VAR::ARG1:
            return (item.args.size() > 1) ? item.args[1] : "";
        case VAR::ARG2:
            return (item.args.size() > 2) ? item.args[2] : "";
        case VAR::ARG3:
            return (item.args.size() > 3) ? item.args[3] : "";
        case VAR::ARG4:
            return (item.args.size() > 4) ? item.args[4] : "";
        case VAR::ARG5:
            return (item.args.size() > 5) ? item.args[5] : "";
        case VAR::ARG6:
            return (item.args.size() > 6) ? item.args[6] : "";
        case VAR::ARG7:
            return (item.args.size() > 7) ? item.args[7] : "";
        case VAR::ARG8:
            return (item.args.size() > 8) ? item.args[8] : "";
        case VAR::ARG9:
            return (item.args.size() > 9) ? item.args[9] : "";
        default:
            return "";
    }
}

std::string shader::render(std::string str, uint32_t color) {
    if (m_color_mode == PLAINTEXT || color == VAR::RECOVER) {
        return str;
    } else if (m_color_mode == CONSOLE) {
        if ((VAR::RED & color) != 0) str = "\033[31m" + str;
        if ((VAR::GREEN & color) != 0) str = "\033[32m" + str;
        if ((VAR::YELLOW & color) != 0) str = "\033[33m" + str;
        if ((VAR::BLUE & color) != 0) str = "\033[34m" + str;
        if ((VAR::PURPLE & color) != 0) str = "\033[35m" + str;
        if ((VAR::CYAN & color) != 0) str = "\033[36m" + str;
        if ((VAR::BLOB & color) != 0) str = "\033[1m" + str;
        return str + "\033[0m";
    } else if (m_color_mode == HTML) {
        std::string style = "<span style='";
        if ((VAR::RED & color) != 0) style += "color:darkred; ";
        if ((VAR::GREEN & color) != 0) style += "color:darkgreen; ";
        if ((VAR::YELLOW & color) != 0) style += "color:yellow; ";
        if ((VAR::BLUE & color) != 0) style += "color:darkblue; ";
        if ((VAR::PURPLE & color) != 0) style += "color:purple; ";
        if ((VAR::CYAN & color) != 0) style += "color:darkcyan; ";
        if ((VAR::BLOB & color) != 0) style += "font-weight:900; ";
        style += "'>";
        return style + str + "</span>";
    } else {
        return str;
    }
}

bool shader::test_cond(const logi& item, uint32_t key) {
    bool result = true;

    if ((key & VAR::COLUMN) != 0) result = result && item.bc > 0;
    if ((key & VAR::COLUMN_END) != 0) result = result && item.ec > 0;
    if ((key & VAR::LINE) != 0) result = result && item.bl > 0;
    if ((key & VAR::LINE_END) != 0) result = result && item.el > 0;
    if ((key & VAR::PREFIX) != 0) result = result && item.prefix.size();
    if ((key & VAR::ARG0) != 0) result = result && item.args.size() >= 1 && item.args[0].size();
    if ((key & VAR::ARG1) != 0) result = result && item.args.size() >= 2 && item.args[1].size();
    if ((key & VAR::ARG2) != 0) result = result && item.args.size() >= 3 && item.args[2].size();
    if ((key & VAR::ARG3) != 0) result = result && item.args.size() >= 4 && item.args[3].size();
    if ((key & VAR::ARG4) != 0) result = result && item.args.size() >= 5 && item.args[4].size();
    if ((key & VAR::ARG5) != 0) result = result && item.args.size() >= 6 && item.args[5].size();
    if ((key & VAR::ARG6) != 0) result = result && item.args.size() >= 7 && item.args[6].size();
    if ((key & VAR::ARG7) != 0) result = result && item.args.size() >= 8 && item.args[7].size();
    if ((key & VAR::ARG8) != 0) result = result && item.args.size() >= 9 && item.args[8].size();
    if ((key & VAR::ARG9) != 0) result = result && item.args.size() >= 10 && item.args[9].size();

    return result;
}

logger::logger(logrepo& _repo, cb_t _cb) : repo(_repo), cb(_cb) {}
logger& logger::operator[](const std::string& prefix) {
    this->prefix = prefix;
    return *this;
}
void logger::gather_args(chainz<std::string>& repo) {}

}  // namespace tlog
}  // namespace alioth

#endif