#ifndef __proxy__
#define __proxy__

/**
 * @module proxy
 * @version 2.0.0; Nov. 28, 2020 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  proxy被设计用于跨越数据集代理原子属性的读写服务，基于proxy可以构建数据模型视图
 *  proxy对指针和agent进行了适配，当proxy代理拥有指针行为的对象时，proxy提供间接引用运算符来简化操作 */

#include <functional>

#include "agent.hpp"

namespace alioth {

#define __implement_proxy_class_definition                                          \
   public:                                                                          \
    using getter_t = std::function<client_t()>;                                     \
    using setter_t = std::function<void(client_t const&)>;                          \
                                                                                    \
   private:                                                                         \
    getter_t m_getter;                                                              \
    setter_t m_setter;                                                              \
                                                                                    \
   public:                                                                          \
    proxy() {}                                                                      \
    proxy(getter_t getter, setter_t setter) : m_getter(getter), m_setter(setter) {} \
    proxy(const proxy& p) : m_getter(p.m_getter), m_setter(p.m_setter) {}           \
    proxy(proxy&& p) : m_getter(p.m_getter), m_setter(p.m_setter) {}                \
    operator client_t() const {                                                     \
        if (m_getter)                                                               \
            return m_getter();                                                      \
        else                                                                        \
            throw std::runtime_error("proxy: impossible get");                      \
    }                                                                               \
    client_t operator()() const { return (client_t) * this; }                       \
    proxy& operator=(const client_t& data) {                                        \
        if (m_setter)                                                               \
            m_setter(data);                                                         \
        else                                                                        \
            throw std::runtime_error("proxy: impossible set");                      \
        return *this;                                                               \
    }                                                                               \
    proxy& operator=(const proxy& data) { return (*this = (client_t)data); }        \
    proxy& operator=(proxy&& data) { return (*this = (client_t)data); }             \
                                                                                    \
   public:                                                                          \
    static void setg(proxy& p, getter_t getter) { p.m_getter = getter; }            \
    static void sets(proxy& p, setter_t setter) { p.m_setter = setter; }

template <typename T>
class proxy {
   public:
    using client_t = T;
    __implement_proxy_class_definition
};

template <typename T>
class proxy<T*> {
   public:
    using client_t = T*;
    __implement_proxy_class_definition public : T& operator*() { return *(client_t) * this; }
    T* operator->() { return (client_t) * this; }
};

template <typename T>
class proxy<agent<T>> {
   public:
    using client_t = agent<T>;
    __implement_proxy_class_definition public : T& operator*() { return *(client_t) * this; }
    T* operator->() { return (client_t) * this; }
};

#define __init_prop(name)                                                               \
    name(std::bind(&std::remove_reference<decltype(*this)>::type::getter_##name, this), \
         std::bind(&std::remove_reference<decltype(*this)>::type::setter_##name, this, std::placeholders::_1))

#define __init_prop_r(name) name(std::bind(&std::remove_reference<decltype(*this)>::type::getter_##name, this), nullptr)

#define __init_prop_w(name) \
    name(nullptr, std::bind(&std::remove_reference<decltype(*this)>::type::setter_##name, this, std::placeholders::_1))

#define __def_prop(name, type)       \
   public:                           \
    alioth::proxy<type> name;        \
                                     \
   private:                          \
    void setter_##name(type const&); \
    type getter_##name();

#define __def_prop_c(name, type)     \
   public:                           \
    alioth::proxy<type> name;        \
                                     \
   private:                          \
    void setter_##name(type const&); \
    type getter_##name() const;

#define __def_prop_r(name, type) \
   public:                       \
    alioth::proxy<type> name;    \
                                 \
   private:                      \
    type getter_##name();

#define __def_prop_r_c(name, type) \
   public:                         \
    alioth::proxy<type> name;      \
                                   \
   private:                        \
    type getter_##name() const;

#define __def_prop_w(name, type) \
   public:                       \
    alioth::proxy<type> name;    \
                                 \
   private:                      \
    void setter_##name(type const&);

}  // namespace alioth
#endif