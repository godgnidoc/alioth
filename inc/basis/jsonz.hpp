#ifndef __json__
#define __json__

/**
 * @module jsonz
 * @version 2.1.0; Jan. 03, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  jsonz提供标准的json结构解析与管理功能 */

#include <functional>
#include <initializer_list>
#include <istream>
#include <iterator>
#include <map>
#include <string>

#include "chainz.hpp"

namespace alioth {

class json;
using json_string_t = std::string;
using json_integer_t = long long;
using json_number_t = long double;
using json_object_t = std::map<json_string_t, json>;
using json_array_t = chainz<json>;

/**
 * @class json : json变量
 * @brief
 *  任何类型的json变量都由json类进行抽象 */
class json {
   public:
    /**
     * @enum type : json变量的变量类型 */
    enum type {
        boolean,  // 布尔类型json变量
        integer,  // 整数类型json变量
        number,   // 实数类型json变量
        string,   // 字符串类型json变量
        object,   // 对象类型json变量
        array,    // 数组类型json变量
        null      // 空类型json变量
    };

   private:
    type mtype;   // json变量当前类型
    void* mdata;  // json变量当前实现的指针

   protected:
    void clean();

   public:
    /**
     * @ctor : 构造方法
     * @param type : 对于不同Json类型，初始化数据指针 */
    json(type = null);
    json(char);                  // 初始化整数类型
    json(unsigned char);         // 初始化整数类型
    json(short);                 // 初始化整数类型
    json(unsigned short);        // 初始化整数类型
    json(int);                   // 初始化整数类型
    json(unsigned int);          // 初始化整数类型
    json(long);                  // 初始化整数类型
    json(unsigned long);         // 初始化整数类型
    json(long long);             // 初始化整数类型
    json(unsigned long long);    // 初始化整数类型
    json(float);                 // 初始化实数类型
    json(double);                // 初始化实数类型
    json(long double);           // 初始化实数类型
    json(const char*);           // 初始化字符串类型
    json(const json_string_t&);  // 初始化字符串类型
    json(json_string_t&&);       // 初始化字符串类型
    json(bool);                  // 初始化布尔类型
    json(const json&);           // 拷贝构造函数
    json(json&&);                // 移动构造函数
    json(const json_object_t&);  // 初始化对象类型
    json(json_object_t&&);       // 初始化对象类型
    json(const json_array_t&);   // 初始化数组对象
    json(json_array_t&&);        // 初始化数组对象

#if __cplusplus >= 201703
    template <typename... Args>
    static json arr(Args... args) {
        auto a = json(json::array);
        (a.push(args), ...);
        return a;
    }
#endif

    /**
     * @dtor : 析构方法
     * @brief
     *  清空类型，释放数据指针 */
    ~json();

    /**
     * @method is : 判断Json数据类型
     * @brief
     *  判断Json数据类型 */
    type is() const;
    bool is(type) const;

    /**
     * @operator char ... unsigned long long : 强制类型转换运算符
     * @brief
     *  进行强制类型转换,获得json保存的数据内容
     *  若类型不匹配,抛出异常 */
    explicit operator char() const;
    explicit operator short() const;
    explicit operator int() const;
    explicit operator long() const;
    explicit operator unsigned char() const;
    explicit operator unsigned short() const;
    explicit operator unsigned int() const;
    explicit operator unsigned long() const;
    explicit operator long long&();
    explicit operator const long long&() const;
    explicit operator unsigned long long() const;

    explicit operator bool&();
    explicit operator const bool&() const;

    explicit operator float() const;
    explicit operator double() const;
    explicit operator long double&() const;

    explicit operator json_string_t&();
    explicit operator const json_string_t&() const;

    char asChar() const;
    short asShort() const;
    int asInt() const;
    long asLong() const;
    unsigned char asUnsignedChar() const;
    unsigned short asUnsignedShort() const;
    unsigned int asUnsignedInt() const;
    unsigned long asUnsignedLong() const;
    long long& asLongLong();
    const long long& asLongLong() const;
    unsigned long long asUnsignedLongLong() const;
    bool& asBool();
    const bool& asBool() const;
    float asFloat() const;
    double asDouble() const;
    long double& asLongDouble() const;
    json_string_t& asString();
    const json_string_t& asString() const;
    json_array_t& asArray();
    const json_array_t& asArray() const;
    json_object_t& asObject();
    const json_object_t& asObject() const;

    /**
     * @operator [json_string_t] : 键访问
     * @brief
     *  以键访问Json对象中的属性,若类型不匹配抛出异常.
     *  若属性不存在,创建属性.
     *  若不允许创建不存在的属性,则抛出异常.
     * 类型不匹配抛出异常 */
    json& operator[](const json_string_t&);
    const json& operator[](const json_string_t&) const;

    /**
     * @operator [int] : 下标访问
     * @brief
     *  以下标访问Json数组的元素,支持以负数反向检索
     *  可修改的情况下,若访问下标超过容量,则用null填充扩展数组
     *  不可修改的情况下,抛出异常
     *  类型不匹配抛出异常 */
    json& operator[](int);
    const json& operator[](int) const;

    /**
     * @method count : 统计对象或数组的容量
     * @brief
     *  若类型不匹配抛出异常 */
    int count(const json_string_t&) const;
    int count(const json_string_t&, type) const;
    int count() const;

    /**
     * @method erase : 删除数组或对象的成员
     * @brief
     *  若成员不存在,则无动作
     *  若类型不匹配,抛出异常 */
    void erase(const json_string_t&);
    void erase(int);

    /**
     * @method push : 在数组末尾追加元素
     * @brief
     *  若类型不匹配，抛出异常 */
    void push(const json& el);

    /**
     * @method pop : 删除并返回末尾元素
     * @brief
     *  若成员不存在，抛出异常
     *  若类型不匹配，抛出异常 */
    json pop();

    /**
     * @operator = : 赋值运算
     * @brief
     *  赋值运算符会强制释放资源,改变数据类型
     *  若类型一致,不释放原数据指针 */
    json& operator=(type);
    json& operator=(char);
    json& operator=(short);
    json& operator=(int);
    json& operator=(long);
    json& operator=(unsigned char);
    json& operator=(unsigned short);
    json& operator=(unsigned int);
    json& operator=(unsigned long);
    json& operator=(long long);
    json& operator=(unsigned long long);
    json& operator=(bool);
    json& operator=(float);
    json& operator=(double);
    json& operator=(long double);
    json& operator=(const char*);
    json& operator=(const json_string_t&);
    json& operator=(json_string_t&&);
    json& operator=(const json&);
    json& operator=(json&&);

    bool operator==(const json&) const;
    bool operator!=(const json&) const;

    /**
     * @method decode_json : 从输入流输入Json
     * @param is : 输入流
     * @param lco : 行列偏移量 */
    static json decode_json(std::istream& is, std::tuple<int, int, int>& lco);
    static json decode_json(std::istream& is);
    static json decode_json(const json_string_t& s, std::tuple<int, int, int>& lco);
    static json decode_json(const json_string_t& s);

    /**
     * @method decode_bson : 从数据块解析bson */
    static json decode_bson(char* bson, size_t* len = nullptr);

    /**
     * @method encode_bson : 编码为二进制记录格式
     * @param len : [输出]编码产物的长度
     * @return char* : 返回编码产物的缓冲区，由接收者负责释放 */
    char* encode_bson(size_t& len) const;

    /**
     * @method encode_json : 编码为json格式文本 */
    json_string_t encode_json() const;

   protected:
    size_t encode_wrt(char* buf) const;
};
}  // namespace alioth
#endif