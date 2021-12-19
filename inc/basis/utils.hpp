#ifndef __utils__
#define __utils__

#include <functional>
#include <map>
#include <string>

namespace alioth {

using std::map;
using std::string;
using std::vector;
using binary = vector<unsigned char>;

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
vector<string> split(const string&, const char);

/** 寻找字符串中的内容，按照顺序替换
 *  替换内容不会重叠，不会重复替换 */
string replace(const string&, const map<string, string>& job);

/** 将字符串重复factor次 */
string operator*(const string& str, int factor);

/** 将字符串重复factor次 */
string operator*(int factor, const string& str);

}  // namespace alioth

#endif