#ifndef __tlog__
#define __tlog__

/**
 * @module tlog
 * @version 1.0.1; Mar. 09, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  tlog模块提供基于模板的结构化日志管理功能 */

#include <chrono>
#include <map>
#include <set>

#include "chainz.hpp"
#include "jsonz.hpp"
#include "rutex.hpp"

namespace alioth {
namespace tlog {

/**
 * @constants : 用于logrepo的选项 */
static const int OPT_RECENT_FIRST = 0x0001;  // 按时间顺序倒序排列
static const int OPT_NO_HINT = 0x0002;       // 忽略HINT严重程度的日志
static const int OPT_NO_INFO = 0x0004;       // 忽略INFO严重程度的日志
static const int OPT_NO_WARNING = 0x0008;    // 忽略WARNING严重程度的日志
static const int OPT_NO_ERROR = 0x0010;      // 忽略ERROR严重程度的日志

/**
 * @enum Servity : 严重性 */
enum Severity { SEV_ERROR, SEV_WARNING, SEV_INFO, SEV_HINT };

/**
 * @namespace VAR : 日志模板中可用的变量 */
namespace VAR {
const static uint32_t STRING = 0x00;  // 常量字符串

const static uint32_t COLUMN = 0x01 << 0;  // %:c 相关代码的起始列，行列从1开始，小于等于0的值被视为无效
const static uint32_t COLUMN_END = 0x01 << 1;  // %:C 相关代码的终止列，行列从1开始，小于等于0的值被视为无效
const static uint32_t DATE_YYYYMMDD = 0x01 << 2;   // %:d YYYY/MM/DD 格式的日期
const static uint32_t DATE_MMMDDYYYY = 0x01 << 3;  // %:D MMM. DD, YYYY 格式的日期
const static uint32_t CODE = 0x01 << 4;            // %:E 日志模板号码
const static uint32_t LINE = 0x01 << 5;  // %:l 相关代码的起始行，行列从1开始，小于等于0的值被视为无效
const static uint32_t LINE_END = 0x01 << 6;  // %:L 相关代码的终止行，行列从1开始，小于等于0的值被视为无效
const static uint32_t MESSAGE = 0x01 << 7;         // %:m 日志文本内容
const static uint32_t PREFIX = 0x01 << 8;          // %:p 前缀信息，通常是文件路径
const static uint32_t SEVERITY = 0x01 << 9;        // %:s 严重性文本
const static uint32_t SEVERITY_CODE = 0x01 << 10;  // %:S 严重性数字
const static uint32_t TIME = 0x01 << 11;           // %:t hh:mm:ss.xxx 格式的时间
const static uint32_t TIMESTAMP = 0x01 << 12;      // %:T 13位时间戳
const static uint32_t ARG0 = 0x01 << 13;           // %0 参数0
const static uint32_t ARG1 = 0x01 << 14;           // %1 参数1
const static uint32_t ARG2 = 0x01 << 15;           // %2 参数2
const static uint32_t ARG3 = 0x01 << 16;           // %3 参数3
const static uint32_t ARG4 = 0x01 << 17;           // %4 参数4
const static uint32_t ARG5 = 0x01 << 18;           // %5 参数5
const static uint32_t ARG6 = 0x01 << 19;           // %6 参数6
const static uint32_t ARG7 = 0x01 << 20;           // %7 参数7
const static uint32_t ARG8 = 0x01 << 21;           // %8 参数8
const static uint32_t ARG9 = 0x01 << 22;           // %9 参数9

/*
 * @cond: 将变量用作条件判断
 *  格式 ： %?*{...} 可用于变量和参数当且仅当用作条件的变量有效时花括号中的表达式才被解析
 *  %?c 相关代码起始列不为0
 *  %?C 相关代码终止列不为0
 *  %?l 相关代码起始行不为0
 *  %?L 相关代码终止行不为0
 *  %?p 前缀信息不为空
 *  %?0 ~ %?9 第n个参数不为空  */
const static uint32_t COND_MASK = COLUMN | COLUMN_END | LINE | LINE_END | PREFIX | ARG0 | ARG1 | ARG2 | ARG3 | ARG4 | ARG5 |
                                  ARG6 | ARG7 | ARG8 | ARG9;  // 条件掩码

/**
 * @color: 控制接下来的文本的颜色 */
const static uint32_t RED = 0x01 << 24;      // %@r 红色
const static uint32_t GREEN = 0x01 << 25;    // %@g 绿色
const static uint32_t BLUE = 0x01 << 26;     // %@b 蓝色
const static uint32_t YELLOW = 0x01 << 27;   // %@y 黄色
const static uint32_t PURPLE = 0x01 << 28;   // %@p 紫色
const static uint32_t CYAN = 0x01 << 29;     // %@c 青色
const static uint32_t BLOB = 0x01 << 30;     // %@1 加粗
const static uint32_t RECOVER = 0x01 << 31;  // %@0 清空字体效果
}  // namespace VAR

/**
 * @struct logtf : 日志片段
 * @brief
 *  用于分割日志内容中具有显示条件的片段的结构，在日志模板中，条件表达式可以嵌套
 *  样例：%?p{%:p%?l{:%:l%?c{:%:c}}:} [%:s] %:m */
struct logtf {
    /**
     * @member condition : 片段的展示条件，多种条件可以合并 */
    uint32_t condition;

    /**
     * @member content : 片段内容 */
    uint32_t content;

    /**
     * @member text : 片段内容 */
    std::string text;
};

/**
 * @struct logti : 日志项模板 */
struct logti {
    /**
     * @member code : 日志模板编号 */
    int code;

    /**
     * @member severity : 严重性 */
    int severity;

    /**
     * @member frags : 模板内的片段 */
    chainz<logtf> frags;
};

/**
 * @struct logt : 日志模板 */
struct logt {
    /**
     * @member severities : 严重性文本 */
    chainz<logtf> severities[4];

    /**
     * @member templates : 日志项模板库 */
    std::map<int, logti> message;

    /**
     * @member format : 日志格式 */
    chainz<logtf> format;

    /**
     * @method compile : 加载日志模板
     * @brief
     *  从json日志模板描述中加载日志模板
     * @param tmpl : 模板语法样例如下：
     *  {
     *      "format" : "%@1%?p{%:p%?l{:%:l%?c{:%:c}}:}%@0 %@1[%@0%:s - %:E%@1]%@0 %:m",
     *      "severity" : [
     *          "%@1%@rERROR%@0",
     *          "%@1%@pWARNING%@0",
     *          "%@1%@cINFO%@0",
     *          "%@1%@bHINT%@0"
     *      ],
     *      "message" : {
     *          "1000" : {
     *              "msg": "option %@c%0%@0 required for function %@b%1%@0",
     *              "sev": 0
     *          }
     *          ......
     *      }
     *  }
     * @return logt : 日志模板 */
    static logt compile(const json& tmpl);

   private:
    /**
     * @method compile_fragment : 编译模板片段
     * @brief
     *  从模板表达式提取模板片段
     * @param expr : 模板表达式迭代器
     * @param cond : 当前语境下的条件
     * @return chainz<logtf> : 模板片段 */
    static chainz<logtf> compile_fragment(const char*& ptr, uint32_t cond = 0);
    static chainz<logtf> compile_fragment(const std::string& expr);

    /**
     * @method extract_placeholder : 提取占位符
     * @brief
     *  此函数用于提取变量占位符，不考虑字体效果占位符
     * @param c : 占位符字符
     * @return uint32_t : 若提取成功，返回变量标识，否则返回0 */
    static uint32_t extract_placeholder(char c);
};

/**
 * @struct logi : 日志项
 * @brief
 *  用于结构化管理日志内容 */
struct logi {
    /**
     * @member code : 模板号 */
    int code;

    /**
     * @member timestamp : 13位毫秒级时间戳 */
    uint64_t timestamp;

    /**
     * @member bl,bc,el,ec : 起止行列 */
    int bl, bc, el, ec;

    /**
     * @member prefix : 日志项前缀信息 */
    std::string prefix;

    /**
     * @member args : 模板参数 */
    chainz<std::string> args;
};

/**
 * @class logrepo : 日志仓库
 * @brief
 *  用于结构化管理日志的模块，可多线程重入 */
class logrepo : private rutex {
   private:
    /**
     * @member m_items : 日志信息按前缀分类存储 */
    std::map<std::string, chainz<logi>> m_items;

    /**
     * @member m_template : 日志模板
     * @brief
     *  可选的成员，按严重程度分类等筛选条件需要日志模板作为辅助 */
    logt* m_template;

   public:
    logrepo();

    /**
     * @method log : 记录一条日志
     * @param i : 日志项 */
    void log(const logi& i);

    /**
     * @method tmpl : 设置模板 */
    void set_template(logt& tmpl);

    /**
     * @method prefixs : 列举前缀
     * @brief
     *  列举所有日志的前缀 */
    std::set<std::string> prefixs();

    /**
     * @method duplicate : 拷贝日志
     * @brief
     *  查看某前缀下的所有日志
     * @param prefix : 前缀
     * @param opt : 选项可以用于控制函数行为
     *  OPT_RECENT_FIRST
     *  OPT_NO_HINT
     *  OPT_NO_INFO
     *  OPT_NO_WARNING
     *  OPT_NO_ERROR */
    chainz<logi> duplicate(const std::string& prefix, int opt = 0);
};

/**
 * @class shader : 日志着色器 */
class shader {
   public:
    /**
     * @enum color_mode_t : 着色模式 */
    enum color_mode_t {
        /**
         * @enum plaintext : 纯文本 */
        PLAINTEXT,

        /**
         * @enum consol : 控制台着色字符 */
        CONSOLE,

        /**
         * @enum html : html着色样式 */
        HTML,
    };

   private:
    /**
     * @member m_template : 模板 */
    logt m_template;

    /**
     * @member m_color_mode : 着色模式 */
    color_mode_t m_color_mode;

   public:
    /**
     * @method set_template : 设置模版
     * @param tmpl : 模板信息 */
    void set_template(const logt& tmpl);

    /**
     * @method set_color_mode : 设置着色模式
     * @brief
     *  设置着色模式
     * @param mode : 着色模式 */
    void set_color_mode(color_mode_t mode);

    /**
     * @method emit_string : 触发字符串
     * @brief
     *  将日志打印到字符串格式
     * @param item: 日志项
     * @param color_mode : 着色选项 */
    std::string emit_string(const logi& item);

    /**
     * @method emit_json : 触发json
     * @brief
     *  将日志信息打印到json结构中存储
     * @param item : 日志项
     * @param color_mode : 着色选项
     * @return
     *  {
     *      "timestamp" -- 十三位毫秒级时间戳
     *      "code" -- 错误号
     *      "begin-line" -- 起始行
     *      "begin-column" -- 起始列
     *      "end-line" -- 终止行
     *      "end-column" -- 终止列
     *      "severity" -- 严重性
     *      "prefix" -- 日志前缀
     *      "message" -- 渲染后的消息内容
     * } */
    json emit_json(const logi& item);

   private:
    /**
     * @method emit_fragments : 触发片段
     * @brief
     *  根据日志信息将一个片段编译成为文本
     * @param frags : 片段序列
     * @param item : 日志项 */
    std::string emit_fragments(const chainz<logtf>& frags, const logi& item);

    /**
     * @method mkvar : 构造变量
     * @brief
     *  此函数从日志信息中构造变量，若没有此变量则返回空字符串
     * @param item : 日志项
     * @param key : 变量名
     * @return string : 返回变量文本 */
    std::string mkvar(const logi& item, uint32_t key);

    /**
     * @method render :文本着色
     * @brief
     *  对文本进行着色
     * @param str : 文本内容
     * @param color_mode : 着色风格
     * @param color : 目标颜色
     * @return string : 着色后的文本 */
    std::string render(std::string str, uint32_t color);

    /**
     * @method test_cond : 测试条件
     * @brief
     *  对条件进行测试，不属于条件变量的均视为true
     * @param item : 日志项
     * @param key : 变量名
     * @return bool : 条件是否成立 */
    bool test_cond(const logi& item, uint32_t key);
};

/**
 * @class logger : 日志器
 * @brief
 *  用于产生日志并自动管理日志信息的流向，单线程使用 */
class logger {
   public:
    /**
     * @type cb_t : 日志回调函数类型 */
    using cb_t = std::function<bool(const logi&)>;

   public:
    /**
     * @member repo : 日志仓库 */
    logrepo& repo;

    /**
     * @member m_cb : 日志回调 */
    cb_t cb;

    /**
     * @member m_prefix : 日志前缀 */
    std::string prefix;

   public:
    /**
     * @ctor
     * @brief
     *  构造日志器用于向日志仓库存入日志信息
     * @param repo : 日志仓库
     * @param cb : 日志回调，一条日志被产生时会调用此函数，回调返回为true的日志才会被从往日志仓库 */
    logger(logrepo& repo, cb_t cb = nullptr);

    /**
     * @operator [] : 设置前缀
     * @brief
     *  设置前缀并返回自身 */
    logger& operator[](const std::string& prefix);

    /**
     * @operator () : 向日志器添加一条日志 */
    template <typename... Args>
    int operator()(int code, int bl, int bc, int el, int ec, Args&&... args);

    /**
     * @operator () : 向日志器添加一条日志 */
    template <typename... Args>
    int operator()(int code, Args&&... args);

   private:
    template <typename... Args>
    static void gather_args(chainz<std::string>& repo, const std::string& arg, Args&&... args);
    template <typename... Args>
    static void gather_args(chainz<std::string>& repo, const char* arg, Args&&... args);
    static void gather_args(chainz<std::string>& repo);
};

template <typename... Args>
int logger::operator()(int code, int bl, int bc, int el, int ec, Args&&... args) {
    using namespace std::chrono;
    logi item;
    item.timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    item.code = code;
    item.bl = bl;
    item.bc = bc;
    item.el = el;
    item.ec = ec;
    item.prefix = prefix;
    gather_args(item.args, args...);
    if (!cb || cb(item)) repo.log(item);
    return code;
}

template <typename... Args>
int logger::operator()(int code, Args&&... args) {
    using namespace std::chrono;
    logi item;
    item.timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    item.code = code;
    item.bl = item.bc = item.el = item.ec = 0;
    item.prefix = prefix;
    gather_args(item.args, args...);
    if (!cb || cb(item)) repo.log(item);
    return code;
}

template <typename... Args>
void logger::gather_args(chainz<std::string>& repo, const std::string& arg, Args&&... args) {
    repo << arg;
    gather_args(repo, args...);
}
template <typename... Args>
void logger::gather_args(chainz<std::string>& repo, const char* arg, Args&&... args) {
    repo << arg;
    gather_args(repo, args...);
}

}  // namespace tlog
}  // namespace alioth
#endif