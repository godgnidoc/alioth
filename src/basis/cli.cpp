#ifndef __cli_cpp__
#define __cli_cpp__

#include "cli.hpp"

#include <iostream>
#include <stdexcept>

#include "utils.hpp"

#ifdef _WIN32
#include <windows.h>
static bool s_IS_BLOB = false;
alioth::cli::color_mode s_COLOR_MODE = alioth::cli::color_mode::SYSTEMCALL;
#else
alioth::cli::color_mode s_COLOR_MODE = alioth::cli::color_mode::SEQUENCE;
#endif

namespace alioth {
namespace cli {

chainz<opt> commandline::operator[](int id) {
    chainz<opt> os;
    for (auto& opt : opts)
        if (id == opt.id) os << opt;
    return os;
}

chainz<std::string> commandline::more() const {
    for (auto& opt : opts)
        if (0 == opt.id) return opt.args;
    return {};
}

rich_string bolb(const std::string& str) {
    rich_string result;
    result.s = str;
    result.b = 1;
    result.c = 0;
    return result;
}
rich_string bolb(const rich_string& str) {
    rich_string result = str;
    result.b = 1;
    return result;
}
rich_string black(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 30;
    return result;
}
rich_string red(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 31;
    return result;
}
rich_string green(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 32;
    return result;
}
rich_string yellow(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 33;
    return result;
}
rich_string blue(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 34;
    return result;
}
rich_string purple(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 35;
    return result;
}
rich_string cyan(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 36;
    return result;
}
rich_string white(const std::string& str, bool b) {
    rich_string result;
    result.s = str;
    result.b = b ? 1 : 0;
    result.c = 37;
    return result;
}
rich_string black(const rich_string& str) {
    rich_string result = str;
    result.c = 30;
    return result;
}
rich_string red(const rich_string& str) {
    rich_string result = str;
    result.c = 31;
    return result;
}
rich_string green(const rich_string& str) {
    rich_string result = str;
    result.c = 32;
    return result;
}
rich_string yellow(const rich_string& str) {
    rich_string result = str;
    result.c = 33;
    return result;
}
rich_string blue(const rich_string& str) {
    rich_string result = str;
    result.c = 34;
    return result;
}
rich_string purple(const rich_string& str) {
    rich_string result = str;
    result.c = 35;
    return result;
}
rich_string cyan(const rich_string& str) {
    rich_string result = str;
    result.c = 36;
    return result;
}
rich_string white(const rich_string& str) {
    rich_string result = str;
    result.c = 37;
    return result;
}

application::application() {
    function func;
    func.name = "help";
    func.brief = "display this help page";
    // func.options = {};
    func.accept_more = true;
    func.entry = std::bind(&application::default_help, this, std::placeholders::_1);
    regist_help_function(func, {"-h", "--help"});

    func.name = "version";
    func.brief = "display version information";
    // func.options = {};
    func.accept_more = false;
    func.entry = std::bind(&application::default_version, this, std::placeholders::_1);
    regist_common_function(func, {"-v", "--version"});
}

int application::execute(int argc, char** argv) {
    commandline cmd;

    for (auto i = 0; i < argc; i++) cmd.raw << argv[i];
    auto line = cmd.raw;

    auto ifunc = m_funcs.find(0);
    for (auto i = 1; i < line.size(); i++) {
        auto fid = m_binds.find(line[i]);
        if (fid != m_binds.end()) {
            ifunc = m_funcs.find(fid->second);
            if (ifunc != m_funcs.end()) {
                line.remove(i);
                break;
            }
        }
    }

    if (ifunc == m_funcs.end()) {
        std::cerr << "[" << red("error", true) << "]no major function located, please check (";
        describe_binds(1, std::cerr);
        std::cerr << ") option for help" << std::endl;
        return -1;
    }

    auto& func = ifunc->second;
    std::map<std::string, int> optdefs;
    std::map<int, int> appear;
    std::set<int> requirments;
    auto all_options = func.options;
    all_options.insert(m_opts.begin(), m_opts.end());
    for (auto opt = all_options.begin(); opt != all_options.end(); opt++) {
        optdefs[opt->second.name] = opt->first;
        if (opt->second.required) requirments.insert(opt->first);
        appear[opt->first] = 0;
    }

    for (auto i = 1; i < line.size(); i++) {
        auto iopt = optdefs.find(line[i]);
        if (iopt == optdefs.end()) {
            if (func.accept_more) {
                cli::opt opt;
                opt.id = 0;
                opt.args = {line[i]};
                cmd.opts << opt;
            } else {
                std::cerr << "[" << red("error", true) << "]unknown option '" << cyan(line[i], true) << "' for function "
                          << green(func.name, true) << "', please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }
        } else {
            auto odef = all_options[iopt->second];
            if (appear[iopt->second]++ >= odef.times && odef.times != 0) {
                std::cerr << "[" << red("error", true) << "]option '" << cyan(line[i])
                          << "' appears too many times, please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }

            auto opt = cli::opt{iopt->second};
            while (odef.args > 0 && ++i < line.size()) {
                opt.args << line[i];
                odef.args--;
            }
            if (odef.args > 0) {
                std::cerr << "[" << red("error", true) << "]leak of arguments for option '" << cyan(odef.name)
                          << "', please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }

            cmd.opts << opt;
        }
    }

    for (auto req : requirments)
        if (appear[req] <= 0) {
            std::cerr << "[" << red("error", true) << "]option '" << cyan(all_options[req].name) << "' required for function '"
                      << green(func.name) << "', please check (";
            describe_binds(1, std::cerr);
            std::cerr << ") option for help" << std::endl;
            return -1;
        }
    if (preprocess) {
        return preprocess(cmd, func.entry);
    } else {
        return func.entry(cmd);
    }
}

int application::regist_main_function(const function& func, const chainz<std::string>& binds) {
    return regist_function(0, func, binds);
}
int application::regist_help_function(const function& func, const chainz<std::string>& binds) {
    return regist_function(1, func, binds);
}
int application::regist_common_function(const function& func, const chainz<std::string>& binds) {
    int id;
    for (id = 2; m_funcs.count(id); id++)
        ;
    return regist_function(id, func, binds);
}

bool application::regist_global_option(int id, const option& opt) {
    if (opt.name.empty()) return false;

    m_opts[id] = opt;
    return true;
}

void application::set_color_mode(color_mode mode) { s_COLOR_MODE = mode; }

int application::regist_function(int id, const function& func, const chainz<std::string>& binds) {
    if (!func.entry || func.name.empty()) return -1;
    for (auto opt : func.options)
        if (opt.second.name.empty()) return -2;
    m_funcs[id] = func;
    for (auto bind : binds) m_binds[bind] = id;
    return id;
}

int application::describe_binds(int id, std::ostream& os) {
    std::map<int, chainz<std::string>> raw_binds;
    for (auto& bind : m_binds) {
        if (bind.second == id) {
            raw_binds[bind.first.size()] << bind.first;
        }
    }
    std::string binds;
    for (auto& bs : raw_binds)
        for (auto& bind : bs.second)
            if (binds.empty())
                binds = bind;
            else
                binds += ", " + bind;
    os << binds;
    return binds.size();
}

int application::describe_option(option& opt, std::ostream& os) {
    const static std::string spaces32 = std::string(" ") * 32;
    os << "  " << bolb(opt.name);
    int ret = opt.name.size() + 2;

    std::string frag;
    if (ret >= 32)
        frag = "\n" + spaces32 + spaces32;
    else
        frag = (32 - ret) * std::string(" ");
    os << frag;
    ret += frag.size();

    if (opt.required) {
        os << "[" << bolb("REQUIRED") << "]";
        ret += std::string("[REQUIRED]").size();
    }

    os << opt.brief;
    ret += opt.brief.size();

    return ret;
}

int application::describe_function(int id, std::ostream& os) {
    auto& func = m_funcs[id];

    os << "  ";
    auto ret = describe_binds(id, os) + 2;

    auto frag = ((32 - ret) * std::string(" "));
    os << frag;
    ret += frag.size();

    if (id == 0) {
        os << "[" << bolb("DEFAULT") << "]";
        ret += std::string("[DEFAULT]").size();
    }

    os << bolb(func.name);
    ret += func.name.size();

    frag = "\n    " + replace(func.brief, {{"\n", "\n    "}});
    os << frag;
    ret += frag.size();

    return ret;
}

int application::default_help(commandline cmd) {
    if (cmd.opts.size() == 0) {
        std::cout << bolb(name) << ": " << brief << std::endl << std::endl;
        if (m_funcs.count(0)) {
            auto& major = m_funcs[0];
            if (!major.options.empty()) {
                if (major.accept_more)
                    std::cout << bolb("USAGE") << ": " << name << " [OPTIONS] ..." << std::endl << std::endl;
                else
                    std::cout << bolb("USAGE") << ": " << name << " [OPTIONS]" << std::endl << std::endl;

                std::cout << bolb("OPTIONS") << ": " << std::endl;
                for (auto& opt : major.options) {
                    describe_option(opt.second, std::cout);
                    std::cout << std::endl;
                }

                if (!m_opts.empty()) {
                    std::cout << bolb("OPTIONS[GLOBAL]") << ": " << std::endl;
                    for (auto& opt : m_opts) {
                        describe_option(opt.second, std::cout);
                        std::cout << std::endl;
                    }
                }
            } else {
                if (major.accept_more)
                    std::cout << bolb("USAGE") << ": " << name << " ..." << std::endl << std::endl;
                else
                    std::cout << bolb("USAGE") << ": " << name << std::endl << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << bolb("FUNCTIONS") << ": " << std::endl;
        for (auto& func : m_funcs) {
            describe_function(func.first, std::cout);
            std::cout << std::endl;
        }
    } else {
        auto fname = cmd.opts[0].args[0];
        auto fid = m_binds.find(fname);
        if (fid == m_binds.end()) {
            std::cerr << "[" << red("error", true) << "]function '" << blue(fname, true) << "' not found" << std::endl;
            return -1;
        }
        auto& func = m_funcs[fid->second];
        std::cout << bolb(name) << " -- " << bolb(func.name) << std::endl << std::endl;

        std::cout << func.brief << std::endl << std::endl;

        std::cout << bolb("SWITCHS") << ":" << std::endl;
        describe_binds(fid->second, std::cout);
        std::cout << std::endl << std::endl;

        std::cout << bolb("OPTIONS") << ":" << std::endl;
        for (auto opt : func.options) {
            describe_option(opt.second, std::cout);
            std::cout << std::endl;
        }
    }
    return 0;
}

int application::default_version(commandline) {
    std::cout << name << " (" << os << "-" << arch << ") " << version << std::endl;
    std::cout << "  " << cert << " by " << author << " <" << email << ">" << std::endl;
    std::cout << "    " << brief << std::endl;
    return 0;
}

std::ostream& operator<<(std::ostream& os, const rich_string& c) {
    if (s_COLOR_MODE == color_mode::SEQUENCE) {
        if (c.c) {
            os << "\033[" << c.b << ";" << c.c << "m" << c.s << "\033[0m";
        } else {
            os << "\033[" << c.b << "m" << c.s << "\033[0m";
        }
    } else {
#ifdef _WIN32
        WORD opt = s_IS_BLOB ? FOREGROUND_INTENSITY : 0;
        switch (c) {
            case fgcolor::normal: {
                s_IS_BLOB = false;
                opt = 0x07;
            } break;
            case fgcolor::blob: {
                s_IS_BLOB = true;
                opt = 0x0f;
            } break;
            case fgcolor::black: {
                /* opt |= 0 */
            } break;
            case fgcolor::red: {
                opt |= FOREGROUND_RED;
            } break;
            case fgcolor::green: {
                opt |= FOREGROUND_GREEN;
            } break;
            case fgcolor::yellow: {
                opt |= FOREGROUND_RED | FOREGROUND_GREEN;
            } break;
            case fgcolor::blue: {
                opt |= FOREGROUND_BLUE;
            } break;
            case fgcolor::purple: {
                opt |= FOREGROUND_RED | FOREGROUND_BLUE;
            } break;
            case fgcolor::cyan: {
                opt |= FOREGROUND_BLUE | FOREGROUND_GREEN;
            } break;
            case fgcolor::white: {
                opt |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            } break;
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), opt);
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), opt);
#endif
    }

    return os;
}

}  // namespace cli
}  // namespace alioth

#endif