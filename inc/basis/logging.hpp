#ifndef __logging__
#define __logging__

#include <any>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "jsonz.hpp"

namespace alioth::logging {

using namespace std::string_literals;

/** 文档内坐标点 */
struct point {
    int line = 0;
    int column = 0;

    operator bool ()const;
};

/** 文档内范围 */
struct range {
    point begin;
    point end;

    operator bool ()const;
};

/**
 * 严重性枚举 */
enum severity {

    /** 错误信息 */
    ERROR,

    /** 警告信息 */
    WARN,

    /** 提示信息 */
    INFO,

    /** 辅助信息 */
    HINT,

    /** 调试信息 */
    DEBUG,
};

/** 日志格式化风格 */
enum style_t {
    /** 以文本格式输出日志，去掉颜色信息 */
    PLAIN_TEXT,
    /** 以文本格式输出日志，将颜色输出为转义序列 */
    COLORED_TEXT,
    /** 以HTML格式输出日志 */
    HTML,
    /** 以JSON格式输出日志 */
    JSON,
    /** 未设定 */
    UNSET,
};

/**
 * 日志记录 */
struct record {
    /** 日志消息模板号或日志消息模板 */
    using message = std::variant<int, std::string>;

    /** 日志参数表 */
    using arguments = std::map<std::string, std::string>;

    /** 时间戳 */
    time_t tim;

    /** 严重性 */
    severity sev;

    /** 作用域 */
    std::string scope;

    /** 日志器名称 */
    std::string name;

    /** 日志器路径 */
    std::string path;

    /** 范围 */
    range rng;

    /** 消息模板 */
    message msg;

    /** 日志参数
     * 每个键对应一列具名参数，每次消耗一个 */
    arguments args;
};

/**
 * @brief 日志信息处理器
 * @param record 日志记录结构化信息
 * @param rendered 格式化后的日志文本
 */
using handler = std::function<void(const record&, const std::string&)>;

/** 日志模板引擎 */
class templating {
   private:
    /** 模板节点
     *  $$
     *  $(var)
     *  $(?var){content}
     *  $color rgybpcw10
     *  .msg .begl .endl .begc .endc
     *  .year .month .mday .wday .hour .minute .sec .time
     *  .scope .name .path
     */
    struct node {
        /** 节点类型 */
        enum type {
            /** 文本节点 */
            text,
            /** 变量节点 */
            variable,
            /** 节点序列 */
            sequence,
            /** 颜色节点 */
            color,
        };

        /** 节点类型 */
        type type;

        /** 当前节点的渲染条件，$(?cond){content} $(?!not-cond){content} */
        std::string condition;

        /** 当前节点的有效值 */
        std::string value;

        /** 子节点序列 */
        std::vector<node*> children;

        /** 析构 */
        ~node();
    };

   private:
    /** 各严重性格式 */
    node* fmt[4] = {};

    /** 各错误号消息格式 */
    std::map<int, node*> msgs;

    /** 渲染样式，封闭后依然可以修改 */
    style_t m_style = COLORED_TEXT;

    /** 封闭标记 */
    bool sealed = false;

   public:
    /** 获取单例 */
    static templating& instance();

    /** 进入多线程使用场景前封闭模板库，确保数据结构不再被修改 */
    void seal();

    /**
     * 尝试设置一种严重性对应的格式，失败则抛出 invalid_argument 异常
     * @param sev 严重性
     * @param format 对应格式
     */
    void format(severity sev, const std::string& format);

    /**
     * 尝试设置一条消息的格式，失败则抛出 invalid_argument 异常，若对应消息号已存在则覆盖
     * @param eno 错误号
     */
    void message(int eno, const std::string& format);

    /**
     * 设置渲染样式，封闭后不影响此操作
     * @param style_t 欲设置的样式
     */
    void style(logging::style_t style);

    /**
     * 根据错误号寻找，模板并渲染消息，若错误号不对应模板则抛出异常
     * @param eno 错误号
     * @param args 消息参数
     * @param style 预期渲染样式，省略则取当前样式
     * @return 渲染完毕的消息
     */
    std::string render(int eno, const record::arguments& args, style_t style = UNSET);

    /**
     * 编译模板并渲染消息，编译失败则抛出异常
     * @param tmpl 模板
     * @param args 消息参数
     * @param style 预期渲染样式，省略则取当前样式
     * @return 渲染完毕的消息
     */
    std::string render(const std::string& tmpl, const record::arguments& args, style_t style = UNSET);

    /**
     * 渲染日志
     * @param record 日志记录
     * @param message 提前渲染完毕的日志消息文本
     * @return 渲染完毕的完整日志文本
     */
    std::string render(const record& record, const std::string& message);

   private:
    /** 将一个模板编译成为模板节点失败则返回空指针，失败则抛出异常 invalid_argument */
    static node* compile(const std::string& code);

    static node* _compile_sequence(const char*& code, char term);

    /** 
     * 根据传入的参数实现最终渲染
     * @param n 欲渲染的语法结构
     * @param args 渲染参数
     * @param s 渲染样式，其中，传入JSON格式是无意义的
     */
    static std::string _render(node* n, const record::arguments& args, style_t s);
    static std::string _render(node* n, const record::arguments& args, style_t s, std::string& suffix, int depth);

    /** 检查封装标记，若已封装则抛出异常 */
    void watchdog();
};

/** 日志器 */
class logger {
   private:
    /** 父级日志器 */
    logger* m_parent;

    /** 当前日志器的名称 */
    std::string m_name;

    /** 日志相关作用域 */
    std::string m_scope;

    /** 日志处理器 */
    handler m_handler;

    /** 日志器当前能够触发的日志的最低严重性 */
    severity m_level;

    /** 子级日志器表 */
    std::map<std::string, logger*> m_children;

   private:
    /**
     * 构造日志器
     * @param scope 日志器名
     * @param the_handler 日志处理器 */
    logger(const std::string& name, logger* parent = nullptr);

   public:
    /** 产生调试信息，并格式化后送往处理器 */
    void debug(const record::message& msg, const record::arguments& args);
    void debug(const range& rng, const record::message& msg, const record::arguments& args);

    /** 产生提示信息，并格式化后送往处理器 */
    void hint(const record::message& msg, const record::arguments& args);
    void hint(const range& rng, const record::message& msg, const record::arguments& args);

    /** 产生提示信息，并格式化后送往处理器 */
    void info(const record::message& msg, const record::arguments& args);
    void info(const range& rng, const record::message& msg, const record::arguments& args);

    /** 产生警告信息，并格式化后送往处理器 */
    void warn(const record::message& msg, const record::arguments& args);
    void warn(const range& rng, const record::message& msg, const record::arguments& args);

    /** 产生错误信息，并格式化后送往处理器 */
    void error(const record::message& msg, const record::arguments& args);
    void error(const range& rng, const record::message& msg, const record::arguments& args);

    /** 获取根日志器 */
    static logger& root();

    /** 获取子日志器，若不存在则创建 */
    logger& child(const std::string& name);

    /** 设置作用域 */
    void set_scope(const std::string& scope);

    /** 设置日志处理器 */
    void set_handler(handler hdlr);

    /** 设置日志过滤级别，被设置的级别及以上允许触发 */
    void set_level(severity sev);

   private:
    /** 产生渲染并发送一条日志 */
    void emit(severity sev, const range& rng, const record::message& msg, const record::arguments& args);

    /** 渲染一条日志 */
    std::string format(const record& record);

    /** 处理一条渲染完毕的日志 */
    void handle(const record& record, const std::string& rendered);

    /** 获取日志器路径 */
    std::string get_path();

   public:
    /** @mark:logging-functions:start **/
    /** @mark:logging-functions:end **/
};

}  // namespace alioth::logging

#endif