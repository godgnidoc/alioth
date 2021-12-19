#ifndef __agent__
#define __agent__

/**
 * @module agent
 * @version 1.2.1; Feb. 23, 2021 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  agent模块实现基于引用计数的智能指针，方便对象生命周期管理。
 *  agent模块将引用计数存放于即将被管理的对象中，以此确保引用计数同步正确。
 *  要使用agent管理对象生命周期，类必须继承自basic_thing。 */

#include <atomic>
#include <type_traits>

namespace alioth {

template <typename T>
class agent;

class basic_thing {
   protected:
    std::atomic_int ref_count;
    virtual ~basic_thing() {}

   public:
    basic_thing() { ref_count = 0; }
    basic_thing(const basic_thing&) { ref_count = 0; }
    basic_thing(basic_thing&&) { ref_count = 0; }
    basic_thing& operator=(const basic_thing&) { return *this; }
    basic_thing& operator=(basic_thing&&) { return *this; }

    template <typename T>
    friend class agent;
};

class basic_agent {
   protected:
    virtual ~basic_agent(){};
};

template <typename T>
class agent : public basic_agent {
   public:
    using client_t = T;

   private:
    /** static_assert(std::is_base_of<basic_thing, T>::value, "\033[1;31mT should be derived type of basic_thing\033[0m"); */
    basic_thing* p;

    basic_thing* get(basic_thing* stp) const {
        if (stp && ((stp->ref_count += 1) > 0)) return stp;
        return nullptr;
    }
    void fre(basic_thing* tp) const {
        if (tp && ((tp->ref_count -= 1) == 0)) delete tp;
    }

   public:
    agent(basic_thing* t = nullptr) : p(get(t)) {}
    agent(const agent& an) : p(get(an.p)) {}
    agent(agent&& an) : p(an.p) { an.p = nullptr; }
    ~agent() {
        fre(p);
        p = nullptr;
    }

    agent& operator=(basic_thing* t) {
        fre(p);
        p = get(t);
        return *this;
    }
    agent& operator=(const agent& an) {
        an.get(an.p);
        fre(p);
        p = an.p;
        return *this;
    }
    agent& operator=(agent&& an) {
        if (p != an.p) fre(p);
        p = an.p;
        an.p = nullptr;
        return *this;
    }

    bool operator==(T* t) const { return t == *this; }
    bool operator==(const agent& an) const { return an.p == p; }
    bool operator!=(T* t) const { return !(p == t); }
    bool operator!=(const agent& an) const { return an.p != p; }

    T* operator->() const { return dynamic_cast<T*>(p); }
    operator T*() const { return dynamic_cast<T*>(p); }
};

}  // namespace alioth
#endif