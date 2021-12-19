#ifndef __uriz__
#define __uriz__

/**
 * @module uriz
 * @version 1.0.0; Feb. 25, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  根据RFC3986 --- STD66设计的URI结构
 *  scheme://user:password@host:port/path?query#fragment */

#include <string>

namespace alioth {

/**
 * @struct Uri
 * @brief
 *  可用于表示文件路径或统一路径的结构 */
struct uri {
    /**
     * @field scheme : 协议 */
    std::string scheme;

    /**
     * @field user : 用户 */
    std::string user;

    /**
     * @field password : 密码 */
    std::string password;

    /**
     * @field host : 主机 */
    std::string host;

    /**
     * @field port : 端口 */
    int port = -1;

    /**
     * @field path : 路径 */
    std::string path;

    /**
     * @field query : 请求 */
    std::string query;

    /**
     * @field fragment : 片段 */
    std::string fragment;

    /**
     * @method from_string : 从字符串构建URI */
    static uri from_string(const std::string& str);

    /**
     * @method from_path : 从文件路径构建URI */
    static uri from_path(const std::string& str);

    /**
     * @method is_path : 判断字符串是否为文件路径 */
    static bool is_path(const std::string& str);

    /**
     * @method is_relative_path : 判断字符串是否为相对文件路径 */
    static bool is_relative_path(const std::string& str);

    /**
     * @method is_absolute_path : 判断字符串是否为绝对路径 */
    static bool is_absolute_path(const std::string& str);

    /**
     * @method to_string: 转换为字符串 */
    std::string to_string() const;

    /**
     * @method valid : 判断URI当前是否有效 */
    bool valid() const;

    /**
     * @operator == : 判断URI是否相等 */
    bool operator==(const uri&) const;

    /**
     * @operator < : 用于序列化的判断 */
    bool operator<(const uri&) const;
};

}  // namespace alioth
#endif