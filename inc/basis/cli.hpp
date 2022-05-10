#ifndef __cli__
#define __cli__

/**
 * @module cli
 * @version 2.0.0; Mar. 31, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  命令行应用程序框架 */

#include <functional>
#include <map>
#include <set>
#include <string>

#include "chainz.hpp"

namespace alioth {
namespace cli {

/**
 * @struct opt : 选项
 * @brief
 *  描述命令行参数传入的选项，其内容是选先所携带的参数
 *   */
struct opt : public chainz<std::string> {
    /**
     * @field id : 选项id
     * @brief
     *  0被用作其他未定义选项，其它数值可以自由使用 */
    int id;
};

/**
 * @struct commandline : 命令行
 * @brief
 *  描述一个被整理过的命令行 */
struct commandline {
    /**
     * @field raw : 原始命令行 */
    chainz<std::string> raw;

    /**
     * @field opts : 整理后的选项序列 */
    chainz<opt> opts;

    /**
     * @operator [] : 查找选项
     * @brief
     *  根据id查找所有出现的选项
     * @param id */
    chainz<opt> operator[](int id);

    /**
     * 返回未被识别为已知选项的命令行参数 */
    chainz<std::string> more() const;
};

/**
 * @struct option : 选项
 * @brief
 *  用于描述某功能的一个选项 */
struct option {
    /**
     * @field name : 选项名
     * @brief
     *  选项名称被用作命令行选项的精确匹配 */
    std::string name;

    /**
     * @field brief : 简介
     * @brief
     *  简介用于在功能详细介绍中展示 */
    std::string brief;

    /**
     * @field args : 参数个数
     * @brief
     *  约束选项能接受的参数个数 */
    unsigned int args;

    /**
     * @field times : 次数限制
     * @brief
     *  限制选项可以出现的次数，0表示可以出现任意次 */
    unsigned int times;

    /**
     * @field required : 选项是否必须
     * @brief
     *  选项是否为必须项 */
    bool required;
};

/**
 * @struct function : 功能
 * @brief
 *  用于描述cli的一个功能 */
struct function {
    /**
     * @field name : 功能名称
     * @brief
     *  用于在帮助页面展示 */
    std::string name;

    /**
     * @field brief : 简介
     * @brief
     *  用于在帮助页面展示 */
    std::string brief;

    /**
     * @field options : 选项族
     * @brief
     *  定义此功能可用的选项族<id,def>，0和负数被系统占用 */
    std::map<int, option> options;

    /**
     * @field accept_more : 是否接受选项以外的内容作为参数 */
    bool accept_more;

    /**
     * @field entry : 功能的入口
     * @param commandline : 整理后的命令行
     * @return int : 用作应用程序的结束码 */
    std::function<int(commandline)> entry;
};

/**
 * @class application : 应用
 * @brief
 *  此类抽象一个应用程序，继承此类或直接使用均能完全使用所有功能 */
class application {
   private:
    /**
     * @member m_funcs : 功能集
     * @brief
     *  功能注册的格式为<id,func>
     *  其中ID为0的功能用作主功能，ID为1的功能被视为帮助页
     *  主功能不需要绑定开关，若未找到任何功能开关，则默认执行主功能
     *  若应用没有主功能，且未找到任何开关时，将执行失败 */
    std::map<int, function> m_funcs;

    /**
     * @member m_opts : 全局选项集
     * @brief
     *  全局选项格式为<id,opt>
     *  全局选项会被注入每一个功能的命令行 */
    std::map<int, option> m_gopts;

    /**
     * @member m_binds : 功能开关绑定<switch,id> */
    std::map<std::string, int> m_binds;

   public:
    /**
     * @member name : 应用名称 */
    std::string name;

    /**
     * @member arch : 应用执行架构 */
    std::string arch;

    /**
     * @member os : 应用执行操作系统 */
    std::string os;

    /**
     * @member version : 应用版本号 */
    std::string version;

    /**
     * @member cert : 应用授权信息 */
    std::string cert;

    /**
     * @member author : 作者名 */
    std::string author;

    /**
     * @member email : 作者邮箱 */
    std::string email;

    /**
     * @member brief : 应用简介 */
    std::string brief;

    /**
     * @member interrupter : 预处理器，若设置则此函数拦截执行流 */
    std::function<int(commandline, std::function<int(commandline)>)> interrupter;

   public:
    /**
     * @ctor
     * @brief
     *  装载默认帮助功能和默认版本功能 */
    application();

    /**
     * @method execute : 执行
     * @brief
     *  若在执行具体功能之前出现错误则报错后退出，返回值为-1。
     *  若顺利执行功能则返回功能的返回值 */
    int execute(int argc, char** argv);

    /**
     * @method regist_main_function : 注册主功能
     * @brief
     *  向应用注册主功能，若功能描述不完整则失败。
     *  若功能已存在则替换。若开关绑定冲突则替换
     * @param func : 功能描述符
     * @param binds : 开关绑定，可以为空
     * @return int : 非负数表示功能id，负数表示错误号 */
    int regist_main_function(const function& func, const chainz<std::string>& binds = {});

    /**
     * @method regist_help_function : 注册帮助功能
     * @brief
     *  向应用注册帮助功能，若功能描述不完整则失败。
     *  若功能已存在则替换。若开关绑定冲突则替换
     * @param func : 功能描述符
     * @param binds : 开关绑定，可以为空
     * @return int : 非负数表示功能id，负数表示错误号 */
    int regist_help_function(const function& func, const chainz<std::string>& binds = {});

    /**
     * @method regist_common_function : 注册通用功能
     * @brief
     *  向应用注册通用功能，若功能描述不完整则失败。
     *  若功能已存在则替换。若开关绑定冲突则替换
     * @param func : 功能描述符
     * @param binds : 开关绑定，可以为空
     * @return int : 非负数表示功能id，负数表示错误号 */
    int regist_common_function(const function& func, const chainz<std::string>& binds = {});

    /**
     * @method regist_global_option : 注册全局选项
     * @brief
     *  向应用注册全局选项
     *  全局选项会被注入每个功能的命令行，ID冲突的情况是未定义的
     * @param id : 全局选项的ID，请不要与任何功能内部的选项ID冲突
     * @param opt : 选项描述符
     * @param bool : 添加成功与否 */
    bool regist_global_option(int id, const option& opt);

   protected:
    /**
     * @method regist_function : 注册功能
     * @brief
     *  向应用注册功能，若功能描述不完整则失败。
     *  若功能已存在则替换。若开关绑定冲突则替换
     * @param id : 绑定功能的id，0表示主功能，1表示帮助页
     * @param func : 功能描述符
     * @param binds : 开关绑定，可以为空
     * @return int : 绑定成功则返回id，负数表示错误号 */
    int regist_function(int id, const function& func, const chainz<std::string>& binds);

    /**
     * @method describe_binds : 描述功能的开关绑定
     * @brief
     *  根据开关id，查询所有的开关绑定，并用逗号拼接
     * @param id : 功能id
     * @param os : 输出流
     * @return int : 输出的有效字节数 */
    int describe_binds(int id, std::ostream& os);

    /**
     * @method describe_option : 描述选项
     * @param opt : 选项信息
     * @param os : 输出流
     * @return int : 输出的有效字节数 */
    int describe_option(option& opt, std::ostream& os);

    /**
     * @method describe_function : 描述功能
     * @param id : 功能id
     * @param os : 输出流
     * @return int : 输出的有效字节数 */
    int describe_function(int id, std::ostream& os);

   private:
    /**
     * @method look_up_binds : 查找功能开关
     * @param id : 功能id
     * @return chainz<std::string> 功能开关列表 */
    chainz<std::string> look_up_binds(int id);
    /**
     * @method default_help : 默认帮助页 */
    int default_help(commandline);

    /**
     * @method default_version : 默认版本页 */
    int default_version(commandline);
};

}  // namespace cli
}  // namespace alioth
#endif
