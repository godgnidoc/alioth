#ifndef __ALIOTH_UTILS_HEX_H__
#define __ALIOTH_UTILS_HEX_H__

#include <memory>
#include <string>
#include <type_traits>

namespace alioth {

class Hex {
 private:
  template <typename T>
  struct UnsignedType {
    using type = std::make_unsigned_t<T>;
    static type Get(T n) { return static_cast<type>(n); }
  };

  template <typename T>
  struct UnsignedType<T*> {
    using type = uintptr_t;
    static type Get(T* n) { return reinterpret_cast<type>(n); }
  };

  template <typename T>
  struct UnsignedType<std::shared_ptr<T>> {
    using type = uintptr_t;
    static type Get(std::shared_ptr<T> n) {
      return reinterpret_cast<uintptr_t>(n.get());
    }
  };

 public:
  /**
   * 解析十六进制字符
   *
   * @param c : 十六进制字符
   */
  static unsigned char Parse(char c);

  /**
   * 判断字符是否为十六进制数字
   *
   * @param c : 字符
   */
  static bool IsHex(char c);

  /**
   * 将数字转换为十六进制字符串
   *
   * @tparam T : 数字类型
   * @param n : 数字
   */
  template <typename T>
  static std::string Format(T n) {
    std::string s;
    auto u = UnsignedType<T>::Get(n);
    auto size = sizeof(u);
    bool first = true;
    for (int off = size * 8 - 4; off >= 0; off -= 4) {
      auto bit = (u >> off) & 0xF;
      if (first && bit == 0 && off != 0) continue;

      s.push_back("0123456789ABCDEF"[bit]);
      first = false;
    }
    return s;
  }
};

}  // namespace alioth

#endif