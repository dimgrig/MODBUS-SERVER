#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <iterator>
#include <vector>

#include "templates_cout.h"
#include "TestRunner.h"

void TemplatesTest();
void IteratorTest();
void SliceTest();
void ZipTest();
void TypeTraitsTest();


class TestValue {
public:
    TestValue() : value_(0) {};
    TestValue(int value) : value_(value) {};
    int get() const {return value_;};
private:
    int value_;
};

template <class T>
class Iterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = T*;
    using pointer           = T**;
    using reference         = T*&;

    explicit Iterator<T>(pointer ptr) : p_(ptr) {};
    ~Iterator() = default;

    reference operator*() const { return *p_; }
    pointer operator->() { return p_; }
    Iterator& operator++() { p_++; return *this; }
    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
    friend bool operator== (const Iterator& a, const Iterator& b) { return a.p_ == b.p_; };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.p_ != b.p_; };
private:
    pointer p_;
};

//template <class T>
//class Iterator_nonpointer
//{
//public:
//    using iterator_category = std::forward_iterator_tag;
//    using difference_type   = std::ptrdiff_t;
//    using value_type        = T;
//    using pointer           = T*;
//    using reference         = T&;

//    explicit Iterator_nonpointer<T>(pointer ptr) : p_(ptr) {};
//    ~Iterator_nonpointer<T>() = default;

//    reference operator*() const { return *p_; }
//    pointer operator->() { return p_; }
//    Iterator_nonpointer& operator++() { p_++; return *this; }
//    Iterator_nonpointer operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
//    friend bool operator== (const Iterator_nonpointer& a, const Iterator_nonpointer& b) { return a.p_ == b.p_; };
//    friend bool operator!= (const Iterator_nonpointer& a, const Iterator_nonpointer& b) { return a.p_ != b.p_; };
//private:
//    pointer p_;
//};

template <class C>
class IteratorImpl
{
public:
    IteratorImpl() {};
    ~IteratorImpl() {};
    //IteratorImpl(C & c) : c_(c) {}
    Iterator<C> begin() { return Iterator<C>(get_it_begin()); }
    Iterator<C> end()   { return Iterator<C>(get_it_end()); }

    virtual C** get_it_begin() = 0;
    virtual C** get_it_end() = 0;
//private:
    //C & c_;
};

template <class C>
class IntValueIterator : public IteratorImpl<C> {
public:
    IntValueIterator() {};
    explicit IntValueIterator(std::vector<C*> &v) : v_(v) {};
    ~IntValueIterator() {};
private:

    virtual C** get_it_begin() override {
        return &v_[0];
    };
    virtual C** get_it_end() override {
        return &v_[v_.size()];
    };

    std::vector<C*> v_;
};

template<typename T>
void slice(std::vector<T> const &v, std::pair<size_t, size_t> p, std::vector<T> &res) {
    res.clear();
    //if (p.first < 0) p.first = 0;
    if (p.first > v.size()) p.first = v.size();
    if (p.second > v.size()) p.second = v.size();

    if (p.first >= v.size()) {
        return;
    }
    if (p.first >= p.second) {
        return;
    }

    auto first = v.begin() + p.first;
    auto last = v.begin() + p.second;

    //res.insert(res.end(), first, last);
    res.assign(first, last);
}

//template <class C>
//class DebugDecorator {
//public:
//    DebugDecorator(const C &c, const std::string &s) : c_(c), s_(s) {};

//    template <typename  ... Args>
//    auto operator()(Args&& ... args) const {
//        std::cout << "Call " << s_ << std::endl;
//        auto res = c_(std::forward<Args>(args) ...);
//        std::cout << "Result: " << res << std::endl;
//        return res;
//    }

//private:
//    C &c_;
//    std::string s_;

//};

//#define DEBUG_CALL(x) decorated_debug(x, #x);

//template <class C>
//auto decorated_debug(const C &c, const std::string &s) {
//    return DebugDecorator(c, s);
//}


template <typename ...T>
std::vector<std::tuple<T...>> zip(std::vector<T> ... v)
{
    std::vector<std::tuple<T...>>  res;
    size_t size = std::min({v.size() ... });
    if (size != 0) {
        for (size_t i = 0; i < size; i++) {
            res.emplace_back(std::make_tuple(*(v.begin()+i) ... ));
        }
    }
    return res;
}

template <typename ...T>
QVector<std::tuple<T...>> qzip(QVector<T> ... v)
{
    QVector<std::tuple<T...>>  res;
    size_t size = std::min({v.size() ... });
    if (size != 0) {
        for (size_t i = 0; i < size; i++) {
            res.push_back(std::make_tuple(*(v.begin()+i) ... ));
        }
    }
    return res;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <typename ...T>
QList<std::tuple<T...>> qzip(QList<T> ... v)
{
    QList<std::tuple<T...>>  res;
    size_t size = std::min({v.size() ... });
    if (size != 0) {
        for (size_t i = 0; i < size; i++) {
            res.push_back(std::make_tuple(*(v.begin()+i) ... ));
        }
    }
    return res;
}
#endif

template <typename T>
QSet<T> QListToQSet(const QList<T>& qlist) {
    return QSet<T> (qlist.constBegin(), qlist.constEnd());
}

template <typename T>
QStringList QSetToQStringList(const QSet<T>& qset) {
    return QStringList(qset.constBegin(), qset.constEnd());
}

template <typename K, typename V>
static std::map<V, K> reverse_map(const std::map<K, V> &m) {
    std::map<V, K> r;
    for (const auto& kv : m) {
        r[kv.second] = kv.first;
    }
    return r;
}

template<typename T>
double get_average(std::vector<T> const& v)
{
    if (v.empty()) {
        return 0;
    }

    return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
}

template <typename T>
std::vector<std::vector<T>> vector_transpose(const std::vector<std::vector<T>> data) {
    std::vector<std::vector<T>> result(data[0].size(), std::vector<T>(data.size()));
    for (unsigned int i = 0; i < data[0].size(); ++i) {
        for (unsigned int j = 0; j < data.size(); ++j) {
            result[i][j] = data[j][i];
        }
    }
    return result;
}

template <typename To, typename From>
To container_cast(From && from) {
    return To(std::begin(from), std::end(from));
}

//type_traits
template <typename>
struct is_renderable : std::false_type {};

//must be before template!!!
class ClassRenderable {
public:
    bool render() {
        //std::cout << "Some render: ><((((`>";
        return true;
    }
};

class ClassNotRenderable {};

template <>
struct is_renderable<ClassRenderable> : std::true_type {};

template <typename T>
bool check_renderable(T obj) {
    if constexpr (is_renderable<T>::value) {
        return obj.render();
    } else {
        return false;
    }
}
#endif //TEMPLATES_H
