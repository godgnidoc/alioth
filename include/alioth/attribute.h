#ifndef __ALIOTH_ATTRIBUTE_H__
#define __ALIOTH_ATTRIBUTE_H__

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <variant>

namespace alioth {

struct Attribute {
  using EvalFn = std::function<Attribute()>;
  using Watcher = std::weak_ptr<Attribute*>;
  using Watchers = std::set<Watcher, std::owner_less<Watcher>>;

  struct Undefined : public std::runtime_error {
    Undefined() : std::runtime_error("Undefined") {}
  };
  using Boolean = bool;
  using Integer = int;
  using Number = double;
  using String = std::string;
  using Map = std::map<std::string, Attribute>;
  using List = std::list<Attribute>;

  Attribute(Boolean value) : value_{value} {}
  Attribute(Integer value) : value_{value} {}
  Attribute(Number value) : value_{value} {}
  Attribute(String const& value) : value_{value} {}
  Attribute(Map const& value) : value_{value} {}
  Attribute(List const& value) : value_{value} {}

  Attribute(Attribute const& attr) : value_(attr.value_), eval_(attr.eval_) {
    if (eval_) {
      Eval(eval_);
    }
  }

  Attribute(Attribute&& attr)
      : value_(std::move(attr.value_)), eval_(std::move(attr.eval_)) {
    if (attr.IsWatched()) {
      throw std::runtime_error("Moving watched attribute");
    }

    if (eval_) Eval(eval_);
  }

  Attribute& operator=(Attribute const& attr) {
    value_ = attr.value_;
    eval_ = attr.eval_;
    if (eval_) {
      Eval(eval_);
    }

    Notify();

    return *this;
  }

  Attribute& operator=(Attribute&& attr) {
    if (attr.IsWatched()) {
      throw std::runtime_error("Moving watched attribute");
    }

    value_ = std::move(attr.value_);
    eval_ = std::move(attr.eval_);
    if (eval_) {
      Eval(eval_);
    }
    return *this;
  }

  ~Attribute() {
    value_ = Undefined{};
    if (IsWatched()) {
      throw std::runtime_error("Dropping watched attribute");
    }
  }

  bool IsUndefined() const {
    Reading();
    return std::holds_alternative<Undefined>(value_);
  }
  bool IsBoolean() const {
    Reading();
    return std::holds_alternative<Boolean>(value_);
  }
  bool IsInteger() const {
    Reading();
    return std::holds_alternative<Integer>(value_);
  }
  bool IsNumber() const {
    Reading();
    return std::holds_alternative<Number>(value_);
  }
  bool IsString() const {
    Reading();
    return std::holds_alternative<String>(value_);
  }
  bool IsMap() const {
    Reading();
    return std::holds_alternative<Map>(value_);
  }
  bool IsList() const {
    Reading();
    return std::holds_alternative<List>(value_);
  }

  template <typename E>
  void Eval(E const& eval) {
    eval_ = eval;
    watching_ = std::make_shared<Attribute*>(this);
    evaluating_ = this;

    try {
      *this = eval();
    } catch (Undefined const&) {
      value_ = Undefined{};
    }
    evaluating_ = nullptr;
  }

  void Reading() const {
    if (!evaluating_) return;

    watchers_.insert(evaluating_->watching_);
  }

  void Notify() {
    Watchers watchers = watchers_;
    Watchers dieds;

    for (auto& weak : watchers) {
      auto watcher = weak.lock();
      if (!watcher) {
        dieds.insert(weak);
        continue;
      }

      auto eval = (*watcher)->eval_;
      if (!eval) {
        dieds.insert(weak);
        continue;
      }

      (*watcher)->Eval(eval);
    }

    for (auto& died : dieds) {
      watchers_.erase(died);
    }
  }

  bool IsWatched() const {
    for (auto weak : watchers_) {
      auto watcher = weak.lock();
      if (!watcher) continue;
      if (!(*watcher)->eval_) continue;

      return true;
    }

    return false;
  }

 protected:
  std::variant<Undefined, Boolean, Integer, Number, String, Map, List> value_{};

  std::shared_ptr<Attribute*> watching_{};
  EvalFn eval_{};
  Watchers mutable watchers_{};

  thread_local static Attribute* evaluating_;
};

inline thread_local Attribute* Attribute::evaluating_{};

}  // namespace alioth

#endif