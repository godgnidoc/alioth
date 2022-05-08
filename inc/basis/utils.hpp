#ifndef __utils__
#define __utils__

#include <functional>
#include <map>
#include <source_location>
#include <string>

namespace alioth {

/** 生成一条内部日志 */
std::string interlog(const std::string& message, const std::source_location& location = std::source_location::current());

using binary = std::vector<unsigned char>;

/** 只允许设置不允许复位的标记 */
class one_shot_flag {
   public:
    operator bool() const;
    one_shot_flag& operator=(bool);

    std::function<void()> cb;

   private:
    bool buf = false;
};

/** 使用分割符将字符串分割 */
std::vector<std::string> split(const std::string&, const char);

/** 寻找字符串中的内容，按照顺序替换
 *  替换内容不会重叠，不会重复替换 */
std::string replace(const std::string&, const std::map<std::string, std::string>& job);

/** 将字符串重复factor次 */
std::string operator*(const std::string& str, int factor);

/** 将字符串重复factor次 */
std::string operator*(int factor, const std::string& str);

template <typename T>
inline std::string strify(T&& val) {
    return std::to_string(std::forward<T>(val));
}

}  // namespace alioth

#endif