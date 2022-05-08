#ifndef __cli_cpp__
#define __cli_cpp__

#include "cli.hpp"

#include <iostream>
#include <stdexcept>

#include "utils.hpp"

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
        if (0 == opt.id) return opt;
    return {};
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
        std::cerr << "[ ERROR ] no major function located, please check (";
        describe_binds(1, std::cerr);
        std::cerr << ") option for help" << std::endl;
        return -1;
    }

    auto& func = ifunc->second;
    std::map<std::string, int> optdefs;
    std::map<int, int> appear;
    std::set<int> requirments;
    auto all_options = func.options;
    all_options.insert(m_gopts.begin(), m_gopts.end());
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
                opt << line[i];
                cmd.opts << opt;
            } else {
                std::cerr << "[ ERROR ]unknown option '" << line[i] << "' for function " << func.name << "', please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }
        } else {
            auto odef = all_options[iopt->second];
            if (appear[iopt->second]++ >= odef.times && odef.times != 0) {
                std::cerr << "[ ERROR ]option '" << line[i] << "' appears too many times, please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }

            auto opt = cli::opt{id : iopt->second};
            while (odef.args > 0 && ++i < line.size()) {
                opt << line[i];
                odef.args--;
            }
            if (odef.args > 0) {
                std::cerr << "[ ERROR ]leak of arguments for option '" << odef.name << "', please check (";
                describe_binds(1, std::cerr);
                std::cerr << ") option for help" << std::endl;
                return -1;
            }

            cmd.opts << opt;
        }
    }

    for (auto req : requirments)
        if (appear[req] <= 0) {
            std::cerr << "[ ERROR ]option '" << all_options[req].name << "' required for function '" << func.name
                      << "', please check (";
            describe_binds(1, std::cerr);
            std::cerr << ") option for help" << std::endl;
            return -1;
        }
    if (interrupter) {
        return interrupter(cmd, func.entry);
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

    m_gopts[id] = opt;
    return true;
}

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
    os << "  " << opt.name;
    int ret = opt.name.size() + 2;

    std::string frag;
    if (ret >= 32)
        frag = "\n" + spaces32 + spaces32;
    else
        frag = (32 - ret) * std::string(" ");
    os << frag;
    ret += frag.size();

    if (opt.required) {
        os << "[REQUIRED]";
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
        os << "[DEFAULT]";
        ret += std::string("[DEFAULT]").size();
    }

    os << func.name;
    ret += func.name.size();

    frag = "\n    " + replace(func.brief, {{"\n", "\n    "}});
    os << frag;
    ret += frag.size();

    return ret;
}

int application::default_help(commandline cmd) {
    if (cmd.opts.size() == 0) {
        std::cout << name << ": " << brief << std::endl << std::endl;
        if (m_funcs.count(0)) {
            auto& major = m_funcs[0];
            if (!major.options.empty()) {
                if (major.accept_more)
                    std::cout << "USAGE"
                              << ": " << name << " [OPTIONS] ..." << std::endl
                              << std::endl;
                else
                    std::cout << "USAGE"
                              << ": " << name << " [OPTIONS]" << std::endl
                              << std::endl;

                std::cout << "OPTIONS"
                          << ": " << std::endl;
                for (auto& opt : major.options) {
                    describe_option(opt.second, std::cout);
                    std::cout << std::endl;
                }

                if (!m_gopts.empty()) {
                    std::cout << "OPTIONS[GLOBAL]"
                              << ": " << std::endl;
                    for (auto& opt : m_gopts) {
                        describe_option(opt.second, std::cout);
                        std::cout << std::endl;
                    }
                }
            } else {
                if (major.accept_more)
                    std::cout << "USAGE"
                              << ": " << name << " ..." << std::endl
                              << std::endl;
                else
                    std::cout << "USAGE"
                              << ": " << name << std::endl
                              << std::endl;
            }
            std::cout << std::endl;
        }
        std::cout << "FUNCTIONS"
                  << ": " << std::endl;
        for (auto& func : m_funcs) {
            describe_function(func.first, std::cout);
            std::cout << std::endl;
        }
    } else {
        auto fname = cmd.opts[0][0];
        auto fid = m_binds.find(fname);
        if (fid == m_binds.end()) {
            std::cerr << "[ ERROR ]function '" << fname << "' not found" << std::endl;
            return -1;
        }
        auto& func = m_funcs[fid->second];
        std::cout << name << " -- " << func.name << std::endl << std::endl;

        std::cout << func.brief << std::endl << std::endl;

        std::cout << "SWITCHS"
                  << ":" << std::endl;
        describe_binds(fid->second, std::cout);
        std::cout << std::endl << std::endl;

        std::cout << "OPTIONS"
                  << ":" << std::endl;
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

}  // namespace cli
}  // namespace alioth

#endif