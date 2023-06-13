#ifndef __ALIOTH_UTILS_CLAN_H__
#define __ALIOTH_UTILS_CLAN_H__

#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace alioth {

/**
 * 字符族，使用最小序列表示一族字符
 *
 * 使用一个最简序列表示数字集合以节省空间
 * 可以表示的数字范围为[0, 255]
 *
 * 序列的每个元素为一个 unsigned int 可以表示一个数字或一个数列
 *
 * 一个数字的表示法为 0xFF'NN 其中 NN 为数字，FF 表示标记数字 255
 * 连续数字的表示法为 0xBB'EE 其中 BB EE 分别为起始数字和结束数字
 */
class Clan {
 public:
  using number = unsigned char;
  using element = unsigned short;
  friend struct iterator;

  /**
   * @struct iterator : 定义迭代器类以支持range-based for特性 */
  struct iterator {
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = Clan::number;
    using difference_type = value_type;
    using pointer = value_type*;
    using reference = value_type&;

    Clan& ref_;
    size_t index_;
    Clan::number value_;

    bool eoc() const;
    iterator(Clan& ref, int index, Clan::number value);
    iterator(const iterator& i);
    ~iterator();
    iterator& operator++();
    iterator operator++(int);
    iterator& operator--();
    iterator operator--(int);
    reference operator*();
    bool operator==(const iterator& an);
    bool operator!=(const iterator& an);
  };

 private:
  static constexpr element kMask[] = {0x00FF, 0xFF00};
  static constexpr element kOffset[] = {0, 8};

 public:
  Clan() = default;

  /**
   * 从单个数字构造字符集合
   *
   * @param n : 数字
   */
  Clan(number n);

  /**
   * 从数字范围构造字符集合
   *
   * @param begin : 起始数字，包含
   * @param end : 结束数字，包含
   */
  Clan(number begin, number end);

  /**
   * 从数字集合构造字符集合
   *
   * @param s : 数字集合
   */
  Clan(std::set<number> const& s);

  Clan(Clan const&) = default;
  Clan(Clan&&) = default;
  Clan& operator=(Clan const&) = default;
  Clan& operator=(Clan&&) = default;

  bool operator==(Clan const& rhs) const;
  bool operator!=(Clan const& rhs) const;

  /**
   * 插入单个数字
   *
   * @param n : 数字
   */
  void Insert(number n);

  /**
   * 插入数字范围
   *
   * @param begin : 起始数字，包含
   * @param end : 结束数字，包含
   */
  void Insert(number begin, number end);

  /**
   * 插入数字集合
   *
   * @param s : 数字集合
   */
  void Insert(std::set<number> const& s);

  /**
   * 合并字符集合
   *
   * @param rhs : 另一个字符集合
   */
  Clan& Merge(Clan const& rhs);

  /**
   * 移除单个数字
   *
   * @param n : 数字
   */
  void Remove(number n);

  /**
   * 移除数字集合
   *
   * @param s : 数字集合
   */
  void Remove(std::set<number> const& s);

  /**
   * 判断是否包含某个数字
   *
   * @param n : 数字
   */
  bool Contains(number n) const;

  /**
   * 获取字符集合大小
   */
  size_t Size() const;

  /**
   * 判断字符集合是否为空
   */
  bool IsEmpty() const;

  /**
   * 获取字符集合
   */
  std::set<number> GetChars() const;

  /**
   * 获取字符集合的序列表示
   */
  std::vector<element> GetElements() const;

  /**
   * 获取字符集合的字符串表示
   */
  std::string Format() const;

  /**
   * 从字符串解析字符集合
   *
   * @param s : 字符串
   */
  static Clan Parse(std::string const& s);

  /**
   * 获取字符集合的迭代器
   */
  iterator begin();

  /**
   * 获取字符集合的迭代器
   */
  iterator end();

 private:
  /**
   * 查找数字在序列表示中的位置
   *
   * @param n : 数字
   */
  int Find(number n) const;

  /**
   * 获取序列表示的起始数字
   *
   * @param e : 序列表示
   */
  static number GetBegin(element e);

  /**
   * 获取序列表示的结束数字
   *
   * @param e : 序列表示
   */
  static number GetEnd(element e);

  /**
   * 判断数字是否在序列表示中
   *
   * @param e : 序列表示
   * @param n : 数字
   */
  static bool Containes(element e, number n);

  /**
   * 扩展序列表示的上界
   *
   * @param e : 序列表示
   * @param end : 上界
   */
  static element UpperTo(element e, number end);

  /**
   * 缩小序列表示的下界
   *
   * @param e : 序列表示
   * @param begin : 下界
   */
  static element LowerTo(element e, number begin);

  /**
   * 构造序列表示的单个数字
   *
   * @param n : 数字
   */
  static element PointTo(number n);

 private:
  /**
   * 序列表示的数字集合
   */
  std::vector<element> elements_;
};

}  // namespace alioth

#endif