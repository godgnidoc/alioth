#ifndef __ALIOTH_GENERIC_H__
#define __ALIOTH_GENERIC_H__

#include <cstddef>
#include <cstdint>
#include <memory>

namespace alioth {

using SymbolID = size_t;
using ContextID = char;
using StateID = size_t;
using FormulaID = size_t;

template <typename T>
struct is_shared_ptr {
  static constexpr bool value = false;
};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> {
  static constexpr bool value = true;
};

template <typename T>
inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

}  // namespace alioth

#endif