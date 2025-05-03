#ifndef __ALIOTH_GENERIC_H__
#define __ALIOTH_GENERIC_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <type_traits>
#include <vector>

namespace alioth {

using SymbolID = size_t;
using ContextID = char;
using StateID = size_t;
using FormulaID = size_t;

namespace generic {

/**
 * 用于停止迭代的异常类
 */
struct nomore {};

/**
 * 用于跳过元素的异常类
 */
struct skip {};

/**
 * 用于标记多重值的类
 */
struct multiple {
  template <typename... Args>
  static constexpr bool isset() {
    return (std::is_same_v<Args, multiple> || ...);
  }
};

/**
 * 从容器收集元素
 *
 * @tparam Args 选项
 * - multiple: 将值收集到 vector, 省略则收集到 set 中
 * @param container 容器
 * @param generator 生成器
 * 生成器类型，生成器可以直接返回预期元素，也可以返回一个迭代器，迭代器抛出
 * nomore 异常来停止迭代
 */
template <typename... Args>
inline auto collect(auto const& container, auto const& generator) {
  using C = std::decay_t<decltype(container)>;
  using E = std::decay_t<decltype(generator)>;
  using invoke_result = std::invoke_result_t<E, typename C::value_type>;
  if constexpr (std::is_invocable_v<invoke_result>) {
    using element_type = std::invoke_result_t<invoke_result>;
    if constexpr (multiple::isset<Args...>()) {
      std::vector<element_type> result;
      for (auto const& it : container) {
        auto iter = generator(it);
        try {
          while (true) result.push_back(iter());
        } catch (nomore const&) {
          continue;
        } catch (skip const&) {
          // nothing
        }
      }
      return result;
    } else {
      std::set<element_type> result;
      for (auto const& it : container) {
        auto iter = generator(it);
        try {
          while (true) result.insert(iter());
        } catch (nomore const&) {
          continue;
        } catch (skip const&) {
          // nothing
        }
      }
      return result;
    }
  } else {
    if constexpr (multiple::isset<Args...>()) {
      std::vector<invoke_result> result;
      for (auto const& it : container) {
        try {
          result.push_back(generator(it));
        } catch (nomore const&) {
          break;
        } catch (skip const&) {
          continue;
        }
      }
      return result;
    } else {
      std::set<invoke_result> result;
      for (auto const& it : container) {
        try {
          result.insert(generator(it));
        } catch (nomore const&) {
          break;
        } catch (skip const&) {
          continue;
        }
      }
      return result;
    }
  }
}

/**
 * 将容器中的元素关联到一个 map 中
 * @tparam Args 指定关联选项
 * - multiple: 将值关联到一个 vector 中
 */
template <typename... Args>
inline auto associate(auto const& container) {
  using C = std::decay_t<decltype(container)>;
  using key_type = typename C::value_type::first_type;
  using value_type = typename C::value_type::second_type;

  if constexpr (multiple::isset<Args...>()) {
    std::map<key_type, std::vector<value_type>> result;
    for (auto const& [key, value] : container) {
      result[key].push_back(value);
    }
    return result;
  } else {
    std::map<key_type, value_type> result;
    for (auto const& [key, value] : container) {
      result[key] = value;
    }
    return result;
  }
}

struct text {
  template <typename T>
  auto operator()(T const& it) const {
    return it->Text();
  }

  template <typename T>
  static std::optional<std::string> maybe(T const& it) {
    if (it) return it->Text();
    return std::nullopt;
  }
};

struct id {
  template <typename T>
  auto operator()(T const& it) const {
    return it->id;
  }

  template <typename T>
  static std::optional<SymbolID> maybe(T const& it) {
    if (it) return it->id;
    return std::nullopt;
  }
};

template <typename Attr>
struct less {
  template <typename LHS, typename RHS>
  bool operator()(LHS const& lhs, RHS const& rhs) {
    return Attr{}(lhs) < Attr{}(rhs);
  }
};

}  // namespace generic

}  // namespace alioth

#endif