#ifndef __chainz__
#define __chainz__

/**
 * @module chainz
 * @version 1.0.0; Dec. 02, 2020 by GodGnidoc
 * @encoding UTF8
 * @brief
 *  chainz是基于指针表的线性容器 */

#include <cstring>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace alioth {

/**
 * @struct basic_chainz_iterator : 定义迭代器类以支持range-based for特性 */
template <typename Tc>
struct basic_chainz_iterator {
    using iterator_category = std::random_access_iterator_tag;
    using Tcv = typename Tc::value_t;
    using value_type = typename std::conditional<std::is_const<Tc>::value, const Tcv, Tcv>::type;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type&;

    Tc& ref;  //迭代器所对应的容器引用
    int pos;  //迭代器当前所指示的位置
    bool eoc() const { return pos >= ref.size(); }
    basic_chainz_iterator(Tc& _ref, int _pos) : ref(_ref), pos(_pos) {}
    basic_chainz_iterator(const basic_chainz_iterator& i) : ref(i.ref), pos(i.pos) {}
    ~basic_chainz_iterator() {}
    basic_chainz_iterator& operator++() {
        pos += 1;
        return *this;
    }
    basic_chainz_iterator operator++(int) {
        basic_chainz_iterator an = *this;
        pos += 1;
        return an;
    }
    basic_chainz_iterator& operator--() {
        pos -= 1;
        return *this;
    }
    basic_chainz_iterator operator--(int) {
        basic_chainz_iterator an = *this;
        pos -= 1;
        return an;
    }
    basic_chainz_iterator operator+(int a) { return basic_chainz_iterator(ref, pos + a); }
    basic_chainz_iterator operator-(int s) { return basic_chainz_iterator(ref, pos - s); }
    basic_chainz_iterator& operator+=(int a) {
        pos += a;
        return *this;
    }
    basic_chainz_iterator& operator-=(int s) {
        pos -= s;
        return *this;
    }
    reference operator*() { return ref[pos]; }
    pointer operator->() { return ref.get(pos); }
    bool operator==(const basic_chainz_iterator& an) { return &ref == &an.ref && ((eoc() && an.eoc()) || pos == an.pos); }
    bool operator!=(const basic_chainz_iterator& an) { return !(*this == an); }
};

template <typename T>
class chainz {
   public:
    using value_t = T;
    using iterator_t = basic_chainz_iterator<chainz>;
    using const_iterator_t = basic_chainz_iterator<const chainz>;

   protected:
    /**
     * chainz的内容是元素的指针的列表
     * 所以将元素指针视为一种独立的类型 */
    using R = value_t*;

    R* m_pool;    //元素指针列表
    int m_count;  //元素总量统计

   public:
    /**
     * @brief 构造函数 */
    chainz() : m_pool(nullptr), m_count(0) {}

    chainz(std::initializer_list<T> list) : chainz() {
        for (auto e : list) construct(-1, e);
    }

    /**
     * @param another: 另一个chainz实例
     * @brief 拷贝构造函数,从另一个实例按照原顺序拷贝每一个元素 */
    chainz(const chainz& another) : m_pool(nullptr), m_count(0) {
        for (int i = 0; i < another.m_count; i++) push(another[i]);
    }

    /**
     * @param temp: 一个右值实例
     * @brief 移动构造函数,从右值实例中抓取系统资源 */
    chainz(chainz&& temp) : m_pool(temp.m_pool), m_count(temp.m_count) {
        temp.m_pool = nullptr;
        temp.m_count = 0;
    }

#if _cplusplus >= 201703
    /**
     * @param data : 首个元素
     * @param args : 参数包 */
    template <typename... Args>
    chainz(T&& data, Args&&... args) : chainz() {
        construct(-1, std::forward<T>(data)), (construct(-1, std::forward<Args>(args)), ...);
    }
#endif

    /**
     * @brief 析构函数 */
    ~chainz() { clear(); }

    /**
     * @param another: 另一个chainz实例
     * @return: 自身引用
     * @brief 赋值运算,将另一个chainz实例的快照装入
     * ---- 容器原有数据会被清空 */
    chainz& operator=(const chainz& another) {
        chainz temp = another;
        *this = std::move(temp);
        return *this;
    }

    /**
     * @param temp: 一个右值实例
     * @return: 自身引用
     * @brief 赋值运算,从右值实例中移动数据
     * ---- 容器原有数据会被清空 */
    chainz& operator=(chainz&& temp) {
        chainz tempz = std::move(temp);
        clear();
        m_pool = tempz.m_pool;
        m_count = tempz.m_count;
        tempz.m_count = 0;
        tempz.m_pool = nullptr;
        return *this;
    }

    /**
     * @return: 容器内容单位总量
     * @brief 统计容器中内容单位的总量 */
    int size() const { return m_count; }

    /**
     * @param data: 插入数据的实例
     * @param index: 数据插入后,所在的位置
     * @return: 插入是否成功
     * @brief 插入方法,将data插入到容器中,使得插入后data的位置为index
     * ---- 若index >= 0
     * ---- ---- 则,index的范围是 [0,size()],代表了一个从0开始的正向地址
     * ---- 若index < 0
     * ---- ---- 则,index的范围是 [-1,-1-size()],代表了一个从size()开始的负向地址 */
    bool insert(/**input*/ const T& data, /**input*/ int index) {
        T dat(data);
        return insert(std::move(dat), index);
    }
    bool insert(/**input*/ T&& data, /**input*/ int index) {
        //若表为空,则情况单独处理
        if (m_count == 0) {
            if (index == 0 || index == -1) {
                m_pool = new R[1];
                m_pool[0] = new T(std::move(data));
            } else {
                return false;
            }

            //若表不为空
        } else {
            //若索引小于0,则先修正索引到正向
            if (index < 0) index = m_count + 1 + index;
            //若正向索引超出范围,则返回失败
            if (index > m_count || index < 0) return false;

            //创建新表,将原表中的元素指针映射到新表中,同时插入新的元素的指针
            R* nr = new R[m_count + 1];
            memcpy(nr, m_pool, index * sizeof(R));
            nr[index] = new T(std::move(data));
            memcpy(nr + index + 1, m_pool + index, (m_count - index) * sizeof(R));

            //删除原表,替换表指针
            delete[] m_pool;
            m_pool = nr;
        }

        //收尾工作
        m_count += 1;
        return true;
    }
    bool insert(/**input*/ T* pdata, /**input*/ int index) {
        //若表为空,则情况单独处理
        if (m_count == 0) {
            if (index == 0 || index == -1) {
                m_pool = new R[1];
                m_pool[0] = pdata;
            } else {
                return false;
            }

            //若表不为空
        } else {
            //若索引小于0,则先修正索引到正向
            if (index < 0) index = m_count + 1 + index;
            //若正向索引超出范围,则抛出异常
            if (index > m_count || index < 0) return false;

            //创建新表,将原表中的元素指针映射到新表中,同时插入新的元素的指针
            R* nr = new R[m_count + 1];
            memcpy(nr, m_pool, index * sizeof(R));
            nr[index] = pdata;
            memcpy(nr + index + 1, m_pool + index, (m_count - index) * sizeof(R));

            //删除原表,替换表指针
            delete[] m_pool;
            m_pool = nr;
        }

        //收尾工作
        m_count += 1;
        return true;
    }

    /**
     * @param tag : 要取出的对象指针
     * @param index : 要取出的对象的索引
     * @state : 若对象在容器内,取出对象,交给调用者,否则无动作,返回空指针 */
    T* pickout(/**input*/ T* tag) {
        if (!tag) return nullptr;
        for (int i = 0; i < m_count; i++)
            if (m_pool[i] == tag) return pickout(i);
        return nullptr;
    }
    T* pickout(int index) {
        T* t = get(index);
        if (!t) return nullptr;
        //若容器只有此元素一个,则执行清空动作
        if (m_count == 1) {
            delete[] m_pool;
            m_pool = nullptr;
            m_count = 0;

            //否则,应当重新映射指针表到新的表格中
        } else {
            R* nr = new R[m_count - 1];
            memcpy(nr, m_pool, index * sizeof(R));
            memcpy(nr + index, m_pool + index + 1, (m_count - index - 1) * sizeof(R));
            delete[] m_pool;
            m_pool = nr;
            m_count -= 1;
        }
        return t;
    }

    /**
     * @param index: 创建实例的插入点
     * @param args: 对象实例的构造函数的参数 */
    T& construct(/**input*/ int index) {
        T* t = new T();
        if (insert(std::move(t), index)) return *t;
        throw std::runtime_error("chainz::construct( int index ): fail to insert object");
    }
    template <typename... Args>
    T& construct(/**input*/ int index, Args&&... args) {
        T* t = new T(std::forward<Args>(args)...);
        if (insert(std::move(t), index)) return *t;
        throw std::runtime_error("chainz::construct( int index ): fail to insert object");
    }

    /**
     * @param index: 欲删除元素的位置
     * @return: 删除是否成功
     * @brief 删除index位置上的元素
     * ---- 其中,index可以小于零,相应规则同insert方法 */
    bool remove(/**input*/ int index) {
        if (index < 0) index = m_count + index;
        if (index >= m_count || index < 0) return false;

        //删除index对应位置上的元素
        delete m_pool[index];

        //若容器只有此元素一个,则执行清空动作
        if (m_count == 1) {
            delete[] m_pool;
            m_pool = nullptr;
            m_count = 0;

            //否则,应当重新映射指针表到新的表格中
        } else {
            R* nr = new R[m_count - 1];
            memcpy(nr, m_pool, index * sizeof(R));
            memcpy(nr + index, m_pool + index + 1, (m_count - index - 1) * sizeof(R));
            delete[] m_pool;
            m_pool = nr;
            m_count -= 1;
        }

        //收尾
        return true;
    }

    /**
     * @param ref: 数据实例
     * @return: 删除是否成功
     * @brief 若ref实例在容器内,则删除ref实例 */
    bool remover(/**input*/ const T& ref) {
        //遍历地址表,判断实例是否属于容器
        for (int i = 0; i < m_count; i++)
            if (m_pool[i] == &ref) return remove(i);

        //若执行到此,说明实例不在容器内
        return false;
    }

    /**
     * @return: 无
     * @brief 清空容器,删除所有内容 */
    void clear() {
        if (m_count != 0) {
            for (int i = 0; i < m_count; i++)
                if (m_pool[i]) delete m_pool[i];
            delete[] m_pool;
            m_pool = nullptr;
            m_count = 0;
        }
        return;
    }

    /**
     * @param data: 将要入栈的数据实例
     * @return: 自身实例
     * @brief 将容器视为栈,在栈顶插入新的数据 */
    bool push(/**input*/ const T& data) { return insert(data, -1); }
    bool push(/**input*/ T&& data) { return insert(std::move(data), -1); }

    /**
     * @return: 是否成功
     * @brief 将容器视为栈,从栈顶删除一个单位 */
    bool pop() { return remove(-1); }

    /**
     * @param data: 用于接收数据的实例
     * @return: 是否成功
     * @brief 将容器视为栈,从栈顶移出一个单位 */
    bool pop(/**output*/ T& data) {
        T* p = get(-1);
        if (p == nullptr) return false;
        data = *p;
        return pop();
    }

    /**
     * @param data: 即将入队的数据实例
     * @return: 自身实例或操作是否成功
     * @brief 将容器视为队列,在队尾插入数据 */
    bool inqueue(/**input*/ const T& data) { return insert(data, -1); }
    bool inqueue(/**input*/ T&& data) { return insert(std::move(data), -1); }
    chainz& operator<<(/**input*/ const T& data) {
        inqueue(data);
        return *this;
    }
    chainz& operator<<(/**input*/ T&& data) {
        inqueue(data);
        return *this;
    }

    /**
     * @return: 操作是否成功
     * @brief 将容器视为队列,从队头删除一个实例 */
    bool outqueue() { return remove(0); }

    /**
     * @peram data: 用于接受数据的实例
     * @return: 操作是否成功
     * @brief 将容器视为队列,从队头移出一个单位 */
    bool outqueue(/**output*/ T& data) {
        T* p = get(0);
        if (p == nullptr) return false;
        data = *p;
        return outqueue();
    }

    /**
     * @param index: 元素的索引
     * @return: 索引位置上,元素的指针
     * @brief 获取index位置上的元素的指针
     * ---- 若index位置上没有数据,则返回空指针 */
    T* get(/**input*/ int index) {
        if (index < 0) index = m_count + index;  //与插入时不同,引用时容器内容总量不改变,所以不加一
        if (index >= m_count || index < 0)
            return nullptr;
        else
            return m_pool[index];
    }
    const T* get(/**input*/ int index) const {
        if (index < 0) index = m_count + index;
        if (index >= m_count || index < 0)
            return nullptr;
        else
            return m_pool[index];
    }

    /**
     * @param index: 元素索引
     * @return: 索引位置上,元素的引用
     * @brief 获取index位置上的元素的引用
     * ---- 若index位置上没有数据,则抛出异常 */
    T& operator[](/**input*/ int index) {
        T* p = get(index);
        if (!p) throw std::out_of_range("chainz::operator[](int index): out of range");
        return *p;
    }
    const T& operator[](/**input*/ int index) const {
        const T* p = get(index);
        if (!p) throw std::out_of_range("chainz::operator[](int index)const: out of range");
        return *p;
    }

    /**
     * @param ref: 数据实例的引用
     * @return: ref实例在容器中的位置
     * @brief 若ref实例在容器中,则获取ref实例的索引值 */
    int index(/**input*/ const T& ref) const {
        for (int i = 0; i < m_count; i++)
            if (&ref == m_pool[i]) return i;
        return -1;
    }

    /**
     * @return: 迭代器
     * @brief 从不同环境创造迭代器
     * ---- 用以支持range-based-for */
    iterator_t begin() { return iterator_t(*this, 0); }
    const_iterator_t begin() const { return const_iterator_t(*this, 0); }
    iterator_t end() { return iterator_t(*this, m_count); }
    const_iterator_t end() const { return const_iterator_t(*this, m_count); }

    /**
     * @param func: 筛选函数
     * @return: 筛选过的chainz容器
     * @brief 筛选方法
     * ---- func返回true时当前节点被添加到筛选容器中
     * ---- 遍历本容器,通过func筛选出一个新的链表返回之 */
    chainz operator%(/**input*/ std::function<bool(const T& data)> func) const {
        chainz cnz = chainz();
        for (int i = 0; i < m_count; i++)
            if (func(*(m_pool[i]))) cnz << *(m_pool[i]);
        return std::move(cnz);
    }
    chainz map(/**input*/ std::function<bool(const T& data)> func) const { return operator%(func); }

    chainz operator+(const chainz& an) const {
        chainz res;
        int in = 0;
        res.m_count = an.m_count + m_count;
        if (res.m_count == 0) return std::move(res);
        res.m_pool = new R[res.m_count];
        for (const auto& i : *this) res.m_pool[in++] = new T(i);
        for (const auto& i : an) res.m_pool[in++] = new T(i);
        return std::move(res);
    }
    chainz operator+(chainz&& an) const {
        chainz res;
        int in = 0;
        res.m_count = an.m_count + m_count;
        if (res.m_count == 0) return std::move(res);
        res.m_pool = new R[res.m_count];
        for (const auto& i : *this) res.m_pool[in++] = new T(i);
        memcpy(res.m_pool + in, an.m_pool, an.m_count * sizeof(R));
        delete an.m_pool;
        an.m_pool = nullptr;
        an.m_count = 0;
        return std::move(res);
    }
    chainz& operator+=(const chainz& an) {
        int nc = m_count + an.m_count;
        int in = m_count;
        if (nc == 0) return *this;
        R* np = new R[nc];
        memcpy(np, m_pool, m_count * sizeof(R));
        for (const auto& i : an) np[in++] = new T(i);
        delete m_pool;
        m_pool = np;
        m_count = nc;
        return *this;
    }
    chainz& operator+=(chainz&& an) {
        int nc = m_count + an.m_count;
        if (nc == 0) return *this;
        R* np = new R[nc];
        memcpy(np, m_pool, m_count * sizeof(R));
        memcpy(np + m_count, an.m_pool, an.m_count * sizeof(R));
        delete an.m_pool;
        an.m_pool = nullptr;
        an.m_count = 0;
        delete m_pool;
        m_pool = np;
        m_count = nc;
        return *this;
    }
};
}  // namespace alioth
#endif