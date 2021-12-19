#ifndef __uriz_cpp__
#define __uriz_cpp__

#include "uriz.hpp"

#include <stdlib.h>

#include <stdexcept>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#define _getcwd getcwd
#endif

namespace alioth {

#ifdef _WIN32
static const std::string dirdvs = "\\";
static const char dirdvc = '\\';
#else
static const std::string dirdvs = "/";
static const char dirdvc = '/';
#endif

uri uri::from_string(const std::string& str) {
    if (is_path(str)) return from_path(str);

    uri uri;
    auto it = str.begin();

    /** 扫描scheme */
    while (isalpha(*it)) it++;
    if (it == str.begin()) throw std::invalid_argument("uri::from_string( const std::string& str): no scheme found");
    if (*it++ != ':' || *it++ != '/' || *it++ != '/')
        throw std::invalid_argument("uri::from_string( const std::string& str ): format error 01");
    uri.scheme = std::string(str.begin(), it - 3);

    /** 扫描后续格式定界 */
    decltype(it) boundary[4] = {str.begin()};
    int c = 0;
    int base = 0;
    for (auto i = it; i != str.end(); i++) {
        if (*i == ':' || *i == '@' || *i == '/' || *i == '?' || *i == '#') boundary[c++] = i;
        if (*i == '/' || *i == '?' || *i == '#') break;
    }

    /** 利用定界分析用户信息 */
    if (*boundary[0] == ':' && *boundary[1] == '@') {
        uri.user = std::string(it, boundary[0]);
        uri.password = std::string(boundary[0] + 1, boundary[1]);
        it = boundary[1] + 1;
        base = 2;
    } else if (*boundary[0] == '@') {
        uri.user = std::string(it, boundary[0]);
        it = boundary[0] + 1;
        base = 1;
    }

    /** 利用定界分析主机 */
    if (c > base) {
        uri.host = std::string(it, boundary[base]);
        it = boundary[base++];
    } else {
        uri.host = std::string(it, str.end());
        it = str.end();
    }

    /** 扫描端口 */
    if (*it == ':') {
        for (it += 1; isdigit(*it); it++) {
            uri.port = uri.port * 10 + *it - '0';
        }
    }

    /** 扫描路径 */
    if (*it == '/') {
        decltype(it) i = it++;
        for (; *i != '?' && *i != '#' && i != str.end(); i++)
            ;
        uri.path = std::string(it, i);
        it = i;
    }

    /** 扫描请求 */
    if (*it == '?') {
        decltype(it) i = it++;
        for (; *i != '#' && i != str.end(); i++)
            ;
        uri.query = std::string(it, i);
        it = i;
    }

    /** 扫描请求 */
    if (*it == '#') {
        uri.fragment = std::string(it + 1, str.end());
    }

    return uri;
}

uri uri::from_path(const std::string& str) {
    uri uri;
    uri.scheme = "file";
    if (is_relative_path(str)) {
        auto path = _getcwd(nullptr, 0);
        uri.path = std::string(path + 1);
        free(path);
        if (uri.path.back() != dirdvc) uri.path += dirdvs;
        uri.path += std::string(str.begin() + 2, str.end());
    } else if (is_absolute_path(str)) {
        uri.path = std::string(str.begin() + 1, str.end());
    } else {
        throw std::invalid_argument("uri::from_path( const std::string& str ): inacceptable path");
    }
    return uri;
}

bool uri::is_path(const std::string& str) { return is_relative_path(str) || is_absolute_path(str); }

bool uri::is_relative_path(const std::string& str) { return str[0] == '.' && str[1] == dirdvc; }

bool uri::is_absolute_path(const std::string& str) { return str[0] == dirdvc; }

std::string uri::to_string() const {
    return scheme + "://" + user + (password.size() ? ":" : "") + (user.size() ? "@" : "") + host +
           ((port >= 0) ? (":" + std::to_string(port)) : "") + (path.size() ? "/" : "") + path + (query.size() ? "?" : "") +
           query + (fragment.size() ? "#" : "") + fragment;
}

bool uri::valid() const { return port >= 0; }

bool uri::operator==(const uri& an) const {
    if (port < 0 && an.port < 0) return false;
    return scheme == an.scheme && user == an.user && password == an.password && host == an.host && port == an.port &&
           path == an.path && query == an.query && fragment == an.fragment;
}

bool uri::operator<(const uri& an) const { return to_string() < an.to_string(); }

}  // namespace alioth
#endif