#ifndef __impl_utils__
#define __impl_utils__

#include "utils.hpp"

#include <stdexcept>
namespace alioth {

std::string interlog(const std::string& message, const std::source_location& location) {
    using namespace std::string_literals;

    return "[ "s + location.file_name() + ":" + std::to_string(location.line()) + " ] " + message;
}

one_shot_flag::operator bool() const { return buf; }

one_shot_flag& one_shot_flag::operator=(bool v) {
    if (buf && !v) throw std::logic_error("trying to reset one shot flag");
    if (!buf && v && cb) {
        buf = v;  // 确保状态正确
        cb();
    }
    buf = v;
    return *this;
}

std::vector<std::string> split(const std::string& str, const char spliter) {
    std::vector<std::string> ret;
    std::string it;
    for (auto c : str) {
        if (c == spliter) {
            ret.push_back(it);
            it.clear();
        } else {
            it += c;
        }
    }
    ret.push_back(it);
    return ret;
}

std::string replace(const std::string& str, const std::map<std::string, std::string>& job) {
    std::string ret;
    size_t off = 0;
    while (true) {
        std::string key;
        size_t cut = std::string::npos;
        for (auto& it : job) {
            auto k = it.first;
            auto c = str.find(k, off);
            if (c != std::string::npos && (cut == std::string::npos || c < cut)) {
                cut = c;
                key = k;
            }
        }
        if (cut != std::string::npos) {
            ret += str.substr(off, cut - off) + job.at(key);
            off = cut + key.size();
        } else {
            ret += str.substr(off);
            break;
        }
    }
    return ret;
}

std::string operator*(const std::string& str, int factor) {
    std::string s;
    while (factor-- > 0) s += str;
    return s;
}

std::string operator*(int factor, const std::string& str) {
    std::string s;
    while (factor-- > 0) s += str;
    return s;
}

}  // namespace alioth

#endif