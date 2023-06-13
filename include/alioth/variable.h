#ifndef __ALIOTH_VARIABLE_H__
#define __ALIOTH_VARIABLE_H__

#include <functional>
#include <memory>
#include <set>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace alioth {

struct Variable {
  using Fn = std::function<void()>;
  using Watcher = std::weak_ptr<Fn>;
  using Watchers = std::set<Watcher, std::owner_less<Watcher>>;
  struct Undefined : public std::runtime_error {
    Undefined() : std::runtime_error("Undefined") {}
  };

  template <typename T>
  void Set(T const& value) {
    if (value_ != nullptr) {
      delete_();
    }

    value_ = new T(value);
    type_ = typeid(T);
    delete_ = [this]() { delete static_cast<T*>(value_); };

    Notify();
  }

  void Clear() {
    if (value_ != nullptr) {
      delete_();
      delete_ = nullptr;
      value_ = nullptr;
      type_ = typeid(void);
    }
  }

  template <typename E>
  void Eval(E const& eval) {
    eval_ = std::make_shared<Fn>([this, eval] { Eval(eval); });
    evaluating_ = eval_;

    try {
      Set(eval());
    } catch (Undefined const&) {
      Clear();
    }
    evaluating_ = nullptr;
  }

  template <typename T>
  T Get() const {
    if (evaluating_) {
      watchers_.insert(evaluating_);
    }

    if (!value_) throw Undefined{};
    if (typeid(T) != type_) {
      throw std::bad_cast();
    }
    return *static_cast<T*>(value_);
  }

  bool HasValue() const { return value_ != nullptr; }

  Variable() = default;
  Variable(Variable const&) = delete;
  Variable(Variable&&) = delete;
  Variable& operator=(Variable const&) = delete;
  Variable& operator=(Variable&&) = delete;
  ~Variable() { Clear(); }

 protected:
  void Notify() {
    Watchers watchers = watchers_;
    Watchers dieds;

    for (auto& watcher : watchers) {
      auto fn = watcher.lock();
      if (!fn) {
        dieds.insert(watcher);
        continue;
      }

      (*fn)();
    }

    for (auto& died : dieds) {
      watchers_.erase(died);
    }
  }

 protected:
  void* value_{};
  Fn delete_{};
  std::type_index type_{typeid(void)};

  std::shared_ptr<Fn> eval_{};
  Watchers mutable watchers_{};

  thread_local static std::shared_ptr<Fn> evaluating_;
};

inline thread_local std::shared_ptr<Variable::Fn> Variable::evaluating_{};

}  // namespace alioth

#endif