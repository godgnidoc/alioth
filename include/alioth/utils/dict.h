#ifndef __ALIOTH_UTILS_DICT_H__
#define __ALIOTH_UTILS_DICT_H__

#include <assert.h>
#include <stdint.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <set>

namespace alioth {

/**
 * 字典
 *
 * @tparam K 键类型
 * @tparam V 值类型
 */
template <typename K, typename V>
class Dict {
 public:
  Dict() = default;
  Dict(Dict const&) = delete;
  Dict(Dict&&) = delete;
  Dict& operator=(Dict const&) = delete;
  Dict& operator=(Dict&&) = delete;
  ~Dict() = default;

  /**
   * 设置值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   * @param value 值
   */
  template <typename T>
  void Set(T const& phrase, V const& value) {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto q = p;
      p = &p->dict_[c];
      p->parent_ = q;
      p->key_ = c;
    }

    p->value_ = value;
    values_.insert(&p->value_);
  }

  /**
   * 获取值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   * @return 值
   */
  template <typename T>
  V const& Get(T const& phrase) const {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto it = p->dict_.find(c);
      if (it == p->dict_.end()) throw std::runtime_error("Key not found");
      p = &it->second;
    }

    if (!p->value_.has_value()) throw std::runtime_error("Value not found");
    return *p->value_;
  }

  /**
   * 获取值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   * @return 值
   */
  template <typename T>
  V& Get(T const& phrase) {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto it = p->dict_.find(c);
      if (it == p->dict_.end()) throw std::runtime_error("Key not found");
      p = &it->second;
    }

    if (!p->value_.has_value()) throw std::runtime_error("Value not found");
    return *p->value_;
  }

  /**
   * 是否包含值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   * @return 是否包含
   */
  template <typename T>
  bool Has(T const& phrase) const {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto it = p->dict_.find(c);
      if (it == p->dict_.end()) return false;
      p = &it->second;
    }

    return p->value_.has_value();
  }

  /**
   * 获取值，若不存在则设置并返回指定的初始值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   * @param init 初始值
   * @return 值
   */
  template <typename T>
  V& Touch(T const& phrase, V const& init) {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto q = p;
      p = &p->dict_[c];
      p->parent_ = q;
      p->key_ = c;
    }

    if (!p->value_.has_value()) {
      p->value_ = init;
      values_.insert(&p->value_);
    }
    return *p->value_;
  }

  /**
   * 移除值
   *
   * @tparam T 词组类型
   * @param phrase 词组
   */
  template <typename T>
  void Remove(T const& phrase) {
    AssertRoot();
    auto p = this;
    for (auto const& c : phrase) {
      auto it = p->dict_.find(c);
      if (it == p->dict_.end()) return;
      p = &it->second;
    }
    if (!p->value_) return;
    p->value_.reset();
    values_.erase(&p->value_);

    while (!p->value_ && p->dict_.empty()) {
      auto k = p->key_;
      p = p->parent_;
      if (p == nullptr) break;
      p->dict_.erase(k);
    }
  }

  /**
   * 遍历值对应的键
   *
   * @tparam F 回调函数类型 bool(K)
   * @param v 值
   * @param f 回调函数
   */
  template <typename F>
  void ForKeys(V const& v, F const& f) {
    AssertRoot();
    auto p = Find(v);
    if (p == nullptr) throw std::runtime_error("Value not found");

    p->DoForKeys(f);
  }

 private:
  void AssertRoot() {
    if (parent_)
      throw std::runtime_error("Cannot perform this operation on sub dict");
  }

  Dict* Find(V const& v) {
    AssertRoot();

    auto it = std::find_if(values_.begin(), values_.end(),
                           [&v](auto* p) { return *p == v; });
    if (it == values_.end()) return nullptr;

    auto ovp = reinterpret_cast<uint8_t*>(*it);
    auto p = ovp - (reinterpret_cast<uint8_t*>(&value_) -
                    reinterpret_cast<uint8_t*>(this));
    return reinterpret_cast<Dict*>(p);
  }

  template <typename F>
  bool DoForKeys(F const& f) {
    if (nullptr == parent_) return true;
    if (!parent_->DoForKeys(f)) return false;
    return f(key_);
  }

 private:
  Dict* parent_{nullptr};
  K key_{};
  std::optional<V> value_{};
  std::map<K, Dict> dict_{};
  std::set<std::optional<V>*> values_{};
};
}  // namespace alioth

#endif